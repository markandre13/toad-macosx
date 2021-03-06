/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public 
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>
#include <toad/vector.hh>
#include <toad/geometry.hh>
#include <fstream>

#include <toad/dialog.hh>
#include <toad/springlayout.hh>
#include <toad/textfield.hh>
#include <toad/scrollbar.hh>
#include <toad/fatradiobutton.hh>
#include <toad/scrollpane.hh>
#include <toad/floatmodel.hh>

#include <toad/simpletimer.hh>

#include "booleanop.hh"


/*
  o Schneiers fitCurve algorithm
    o is not stable
    o with floating point samples from a tablet instead rounded screen
      coordinates from a mouse, it shows that jitter isn't smoothed
      -> algorithms to reduce points
        o see https://en.wikipedia.org/wiki/Smoothing
        o paper.js has a smoothing function which provides 4 different approaches
        o Line generalisation by repeated elimination of the smallest area
          Visvalingam, Maheswari; Whyatt, J. D. (James Duncan)
          https://hydra.hull.ac.uk/resources/hull:8338
    o other algorithms:
      o "Method for fitting a parametric representation to a set of objects"
        Sarah F. Frisken
        US 20090027396 A1
        (a patented algorithm? yo gotta be kiddin me!)
      o Efficient Curve Fitting
        Sarah F. Frisken
        Tufts University
        Journal of Graphics Tools, 13(2), pp. 37-37, 2008
      o Data fitting by G1 rational cubic Bézier curves using harmony search
        Najihah Mohamed a,*, Ahmad Abd Majid b, Abd Rahni Mt Piah
        Dec 2014
        Egyptian Informatics Journal
      o SketchIron: Workflow for Vector-based Sketching
        Timur Carpeev
        (this one gives a nice overview)
      o Optimal Spline Approximation via l0-Minimization
        Christopher Brandt, Hans-Peter Seidel, Klaus Hildebrandt
      
*/

// write test_tablet with a simulated g-pen which can rotate, and a nib which can split on pressure,
// and disappears when leaving the proximity of the tablet, etc.

// once done, experiment with fitCurve and pen shapes and width...
// nib   : pressure -> size
// pencil: pressure -> alpha
// (all of this should be configurable)
// when zooming into the image, the visual pen also becomes bigger so that
// one has a relation to the paper

// when the input device has not rotation but tilt, i could offer to
// calculate a rotation value from the tilt

using namespace toad;

namespace {

/**
 * out[i] := average of all points within radius epsilon of point in[i]
 */
void
movingAverage(const vector<TPoint> &in, double epsilon, vector<TPoint> &out)
{
  for(size_t i=0; i<in.size(); ++i) {

    // average
    TPoint p = in[i];
    for(size_t j=i-1; j>0; --j) {
      TCoord d = distance(in[i], in[j]);
      if (d>epsilon)
        break;
      p = 0.5 * (p + in[j]);
    }
    for(size_t j=i+1; j<in.size(); ++j) {
      TCoord d = distance(in[i], in[j]);
      if (d>epsilon)
        break;
      p = 0.5 * (p + in[j]);
    }

    // reduce
    if (!out.empty()) {
      TPoint d = out.back() - p;
      if (isZero(d.x) && isZero(d.y))
        continue;
    }
    
    out.push_back(p);
  }
}

struct TFreehandPoint:
  public TPoint
{
  TFreehandPoint(TCoord x, TCoord y, TCoord r, TCoord p):TPoint(x,y), rotation(r), pressure(p){}
  TCoord rotation, pressure;
};

class TMyWindow:
  public TWindow
{
public:
    // information from the tablet
    vector<TFreehandPoint> handpath;
    
    // cummulated path (only line segments)
    TVectorPath path;
    
    ofstream backup;

    // additional values from the last mouse event
    bool proximity;
    float pressure;
    float rotation;
    TPoint tilt;
    TPoint pos;
    int pointingDeviceType=0;
    unsigned long long uniqueID=0;
    unsigned capability;

  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title), proximity(false)
    {
      setSize(640, 480);
      setAllMouseMoveEvents(true);
    }
    
    void mouseEvent(const TMouseEvent&) override;
    void paint() override;
};

#define		kTransducerDeviceIdBitMask					0x0001
#define		kTransducerAbsXBitMask 						0x0002
#define		kTransducerAbsYBitMask 						0x0004
#define		kTransducerVendor1BitMask					0x0008
#define		kTransducerVendor2BitMask 					0x0010
#define		kTransducerVendor3BitMask 					0x0020
#define		kTransducerButtonsBitMask					0x0040
#define		kTransducerTiltXBitMask 					0x0080
#define		kTransducerTiltYBitMask 					0x0100
#define		kTransducerAbsZBitMask 						0x0200
#define		kTransducerPressureBitMask		 			0x0400
#define		kTransducerTangentialPressureBitMask		                0x0800
// deprecated, don't use
#define		kTransducerOrientInfoBitMask 				        0x1000
#define		kTransducerRotationBitMask	 				0x2000

TPoint operator*(const TMatrix2D &m, const TPoint &p) {
  TPoint r;
  m.map(p.x, p.y, &r.x, &r.y);
  return r;
}

void
TMyWindow::paint()
{
  TPen pen(this);

  // draw union in gray
  pen.setStrokeColor(0,0,0);
  pen.setFillColor(0.7,0.7,0.7);
  path.apply(pen);
  pen.fill();
  path.apply(pen);
  pen.stroke();
//  pen.fillStroke();
#if 0  
  // plot raw data in orange
  pen.setColor(1,0.5,0);
  for(auto p: handpath) {
    pen.drawLine(p.x-0.5,p.y-0.5,p.x+0.5,p.y+0.5);
    pen.drawLine(p.x-0.5,p.y+0.5,p.x+0.5,p.y-0.5);
  }
  
  vector<TPoint> a, b, c;
  for(auto p: handpath)
    a.push_back(p);

  if (!a.empty()) {

    // reduce noise of tablet input
//    movingAverage(a, 1.5, b);
    
#if 0
    // plot denoised data in red
    pen.setColor(0.8,0,0);
    for(auto p: b) {
      pen.drawLine(p.x-0.5,p.y-0.5,p.x+0.5,p.y+0.5);
      pen.drawLine(p.x-0.5,p.y+0.5,p.x+0.5,p.y-0.5);
    }

    // draw fitted raw data in light blue
    fitPath(a, 4.0, &c);
    pen.setColor(0,0.5,1);
    pen.drawBezier(c);
#endif

    // draw fitted denoised data in regular blue
    c.clear();
    fitPath(a, 4.0, &c);
    pen.setColor(0,0,1);
    pen.drawBezier(c);
  }
  
//  fitPath(const TPoint *inPoints, size_t size, TCoord tolerance, vector<TPoint> *out);
#endif    
  
#if 1
  // draw caret
  if (proximity) {
    TCoord r = 50.0*pressure+4.0;
    pen.drawCircle(pos.x-r, pos.y-r, 2*r, 2*r);
    
    TPoint a[3] = {
      { pos.x+sin((rotation+170)/360.0 * 2.0 * M_PI)*r,
        pos.y+cos((rotation+170)/360.0 * 2.0 * M_PI)*r },
      { pos.x+sin((rotation+180)/360.0 * 2.0 * M_PI)*(r*1.2),
        pos.y+cos((rotation+180)/360.0 * 2.0 * M_PI)*(r*1.2) },
      { pos.x+sin((rotation+190)/360.0 * 2.0 * M_PI)*r,
        pos.y+cos((rotation+190)/360.0 * 2.0 * M_PI)*r },
    };
    pen.drawLines(a, 3);
    
    pen.drawCircle(pos.x+tilt.x*r, pos.y-tilt.y*r, 4, 4);
    
    const char *pdt="";
    switch(pointingDeviceType) {
      case NX_TABLET_POINTER_UNKNOWN: pdt="unknown"; break;
      case NX_TABLET_POINTER_PEN:     pdt="pen"; break;
      case NX_TABLET_POINTER_CURSOR:  pdt="cursor"; break;
      case NX_TABLET_POINTER_ERASER:  pdt="eraser"; break;
    }
    pen.drawString(pos.x+sin((180-135)/360.0 * 2.0 * M_PI)*r,
                   pos.y+cos((180-135)/360.0 * 2.0 * M_PI)*r,
                   format("type=%s, uniqueID=%llu", pdt, uniqueID));
  }
#endif
}

static void
addHull(vector<TPoint> *hull, vector<TFreehandPoint> handpath, size_t idx)
{
  for(TCoord d=0.0; d<2*M_PI; d+=2*M_PI/40) {
    TPoint p(sin(d)*2.0*handpath[idx].pressure, cos(d)*10.0*handpath[idx].pressure);
    TMatrix2D m;
    m.rotate(handpath[idx].rotation);
    p = m*p+handpath[idx];
    hull->push_back(p);
  }
}

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
static double lasttime;
static TPoint lastpos;
static const TCoord fidility = 4.0; // smallest fidility setting in Illustrator [0.5, 20], default 4
  if (me.type==TMouseEvent::LDOWN) {
//cout << "down" << endl;
    [NSEvent setMouseCoalescingEnabled: FALSE];
    lasttime = [me.nsevent timestamp];
    lastpos = me.pos;
    backup.open("backup.txt", ofstream::out | ofstream::trunc);
    handpath.clear();
    path.clear();
  } else
  if (me.type==TMouseEvent::LUP) {
//cout << "up" << endl;
    [NSEvent setMouseCoalescingEnabled: TRUE];
    backup.close();
    path.simplify(4.0, 2.0*M_PI/360.0*30.0);
cout << "path is now simplified" << endl;
    // quantify raw data
    // fit curve raw data
    // fit curve outline
    invalidateWindow();
  } else
  if (me.type==TMouseEvent::TABLET_POINT) {
//cout << "point " << me.pos << endl;
  } else
  if (me.type==TMouseEvent::TABLET_PROXIMITY) { // can we translate these into enter & leave events?
    if ([me.nsevent isEnteringProximity]) {
      pointingDeviceType = [me.nsevent pointingDeviceType];
      uniqueID = [me.nsevent uniqueID];
      proximity = true;
cout << uniqueID << " entered proximity" << endl;
    } else {
cout << uniqueID << " left proximity" << endl;
      pointingDeviceType = 0;
      uniqueID = 0;
      proximity = false;
    }
    capability = [me.nsevent capabilityMask];
#if 0
    if (capability & kTransducerDeviceIdBitMask)
      cout << "  kTransducerDeviceIdBitMask" << endl;
    if (capability & kTransducerAbsXBitMask)
      cout << "  kTransducerAbsXBitMask" << endl;
    if (capability & kTransducerAbsYBitMask)
      cout << "  kTransducerAbsYBitMask" << endl;
    if (capability & kTransducerVendor1BitMask)
      cout << "  kTransducerVendor1BitMask" << endl;
    if (capability & kTransducerVendor2BitMask)
      cout << "  kTransducerVendor2BitMask" << endl;
    if (capability & kTransducerVendor3BitMask)
      cout << "  kTransducerVendor3BitMask" << endl;
    if (capability & kTransducerButtonsBitMask)
      cout << "  kTransducerButtonsBitMask" << endl;
    if (capability & kTransducerTiltXBitMask)
      cout << "  kTransducerTiltXBitMask" << endl;
    if (capability & kTransducerTiltYBitMask)
      cout << "  kTransducerTiltYBitMask" << endl;
    if (capability & kTransducerAbsZBitMask)
      cout << "  kTransducerAbsZBitMask" << endl;
    if (capability & kTransducerPressureBitMask)
      cout << "  kTransducerPressureBitMask" << endl;
    if (capability & kTransducerTangentialPressureBitMask)
      cout << "  kTransducerTangentialPressureBitMask" << endl;
    if (capability & kTransducerOrientInfoBitMask)
      cout << "  kTransducerOrientInfoBitMask" << endl;
    if (capability & kTransducerRotationBitMask)
      cout << "  kTransducerRotationBitMask" << endl;
#endif
    invalidateWindow();
    return;
  }

  if(distance(me.pos, lastpos)<fidility)
    return;
  lastpos = me.pos;

  pos = me.pos;
  pressure = me.pressure();
  rotation = me.rotation();
  tilt = me.tilt();

  // use wacom specific capabilities to see if we can fake the rotation from
  // the tilt
  if (!(capability & kTransducerRotationBitMask) &&
       (capability & kTransducerTiltXBitMask) &&
       (capability & kTransducerTiltYBitMask) )
  {
    rotation = atan2(tilt.y, tilt.x) * 360.0 / (2.0*M_PI) + 90;
  }

  if (me.modifier() & MK_LBUTTON) {

    vector<TPoint> hull;
    if (handpath.empty()) {
      backup << me.pos.x << " " << me.pos.y << " " << rotation/360.0 * 2.0 * M_PI << " " << pressure << endl;
      handpath.push_back(TFreehandPoint(me.pos.x, me.pos.y, rotation/360.0 * 2.0 * M_PI, pressure));
      addHull(&hull, handpath, 0);
    } else {
      backup << me.pos.x << " " << me.pos.y << " " << rotation/360.0 * 2.0 * M_PI << " " << pressure << endl;
      handpath.push_back(TFreehandPoint(me.pos.x, me.pos.y, rotation/360.0 * 2.0 * M_PI, pressure));
      addHull(&hull, handpath, handpath.size()-2);
      addHull(&hull, handpath, handpath.size()-1);
      convexHull(&hull);
    }
    
    // this could be more efficient
    TVectorPath np;
    for(auto p: hull) {
      if (np.empty())
        np.move(p);
      else
        np.line(p);
    }
    np.close();
//cout << "boolean: to " << path.points.size() << " add " << np.points.size() << endl;
    boolean(path, np, &path, UNION);
    if (booleanop_gap_error) {
      backup.close();
      cerr << "found gap error" << endl;
      exit(EXIT_FAILURE);
    }
//cout << "added" << endl;

    // reduce window updates to 60fps, otherwise the program slows down too
    // much well: cocoa is already coalescing screen updates and even syncs
    // them with the screen refresh rate but some reason then the screen
    // updates are lagging
    double thistime = [me.nsevent timestamp];
    if (thistime - lasttime < 1.0/60)
      return;
    lasttime = thistime;

    invalidateWindow();
  }
  
//  cout << "mouseEvent " << me.name() << ", " << me.pressure() << ", " << me.tangentialPressure() << ", " << me.rotation() << ", " << me.tilt() << endl;
}

} // unnamed namespace

vector<TFreehandPoint>
loadHandpath(const char *filename)
{
  ifstream in;
  in.open(filename);
  
  vector<TFreehandPoint> handpath;
  
  while(true) {
    TCoord x, y, r, p;
    in>>x;
    in>>y;
    in>>r;
    in>>p;
    if (!in)
      break;
    handpath.push_back(TFreehandPoint(x, y, r, p));
  }
  
  return handpath;
}

struct TEditModel
{
  TEditModel() { cout << "new TEditModel " << this << endl; }
  ~TEditModel() { cout << "old ~TEditModel " << this << endl; }
  enum ETool {
    ZOOM,
    CUT
  };
  GRadioStateModel<ETool> tool;

  vector<TFreehandPoint> handpath;
  TVectorPath path;
  TVectorPath nextstroke;
  TVectorPath nextpath;

  TIntegerModel pos;
  TFloatModel zoom;
};

void replay(TEditModel *editmodel, TBoundary *clip=nullptr);

class TVisualization:
  public TScrollPane
{
    TEditModel *editmodel;
    bool haveClip:1;
    bool editClip:1;
    TBoundary clip;
  public:
    TVisualization(TWindow *parent, const string &title, TEditModel *em):
      TScrollPane(parent, title), editmodel(em), haveClip(false), editClip(false) {}
    void adjustPane() override;
    void paint() override;
    void mouseEvent(const TMouseEvent &) override;
};

void
TVisualization::adjustPane()
{
//  visible.set(0,0,getWidth(), getHeight());
  TBoundary b = editmodel->path.bounds();
  b.p1.x+=20; b.p1.y+=20;
  pane.set(0,0,b.p1.x*editmodel->zoom, b.p1.y*editmodel->zoom);
//  pane.set(0,0,800*editmodel->zoom, 600*editmodel->zoom);
}

void
TVisualization::mouseEvent(const TMouseEvent &me)
{
  switch(editmodel->tool.getValue()) {
    case TEditModel::ZOOM:
      switch(me.type) {
        case TMouseEvent::LDOWN: {
          TCoord oldzoom = editmodel->zoom;
          if (me.modifier() & MK_SHIFT) {
            editmodel->zoom = editmodel->zoom * 0.5;
          } else {
            editmodel->zoom = editmodel->zoom * 2;
          }
          doLayout();

          TPoint M = me.pos*(1.0/oldzoom);
          TPoint O = M * editmodel->zoom - M;

          setPanePos(O.x, O.y);
          invalidateWindow();
        } break;
      }
      break;
    case TEditModel::CUT: {
      TPoint pt = me.pos * (1.0 / editmodel->zoom);
      switch(me.type) {
        case TMouseEvent::LDOWN: {
          cout << "find something near " << pt << endl;
          if (haveClip) {
            cout << "have clip" << endl;
            haveClip=false;
            invalidateWindow();
            break;
          }
          cout << "clip" << endl;
          haveClip=false;
          editClip=true;
          clip.p0 = clip.p1 = pt;
          break;
        case TMouseEvent::MOVE:
          if (!editClip)
            break;
          clip.p1=pt;
          invalidateWindow();
          break;
        case TMouseEvent::LUP:
          if (!editClip)
            break;
          clip.p1=pt;
          haveClip=true;
          editClip=false;
          replay(editmodel, &clip);
          invalidateWindow();
          break;
/*          
          TCoord dm;
          size_t i, f;
          
          dm = 1.0/0.0;
          i=0; f=-1;
          for(auto p: editmodel->path.points) {
            TCoord d = distance(pt, p);
            if (d<dm) { f = i; dm = d; }
            ++i;
          }
          cout << "found black point " << f << ", " << editmodel->path.points[f] << endl;

          dm = 1.0/0.0;
          i=0; f=-1;
          for(auto p: editmodel->nextstroke.points) {
            TCoord d = distance(pt, p);
            if (d<dm) { f = i; dm = d; }
            ++i;
          }
          cout << "found blue point " << f << ", " << editmodel->nextstroke.points[f] << endl;


          dm = 1.0/0.0;
          i=0; f=-1;
          for(auto p: editmodel->nextpath.points) {
            TCoord d = distance(pt, p);
            if (d<dm) { f = i; dm = d; }
            ++i;
          }
          cout << "found red point " << f << ", " << editmodel->nextpath.points[f] << endl;
*/          
        } break;
      }
    } break;
  }
}

void
TVisualization::paint()
{
  TPen pen(this);
  TCoord x, y;
  getPanePos(&x, &y);
  pen.translate(-x, -y);
  
  pen.setLineWidth(1.0/editmodel->zoom);
  pen.scale(editmodel->zoom, editmodel->zoom);
  TCoord s = 2/editmodel->zoom;
  
  pen.setColor(0.7,0.7,0.7);
  editmodel->path.apply(pen);
  pen.fill();
  pen.setColor(0,0,0);
  editmodel->path.apply(pen);
  pen.stroke();

  pen.setColor(0,0,1);
  editmodel->nextstroke.apply(pen);
  pen.stroke();

  pen.setColor(1,0,0);
  editmodel->nextpath.apply(pen);
  pen.stroke();
  
  if (editClip) {
    pen.setColor(0,0,0);
    pen.drawRectangle(clip);
  }
}

extern bool global_debug;

// write TVectorPath in the format used by bop12
void
store(const TVectorPath &path, const char *fn)
{
  ofstream out(fn);
  out.precision(numeric_limits<double>::max_digits10);
  size_t n;
  n = 0;
  for(auto x: path.type)
    if (x==TVectorPath::CLOSE)
      ++n;
  out << n << endl;
  
  const TPoint *pt = path.points.data();
  for(auto p0 = path.type.begin(); p0 != path.type.end(); ++p0) {
    switch(*p0) {
      case TVectorPath::MOVE:
        n = 1;
        for(auto p1=p0+1; p1!=path.type.end() && *p1!=TVectorPath::CLOSE; ++p1)
          ++n;
        out << n << endl;
      case TVectorPath::LINE:
        out << "\t" << pt->x << " " << pt->y << endl;
        ++pt;
        break;
      case TVectorPath::CURVE:
        cout << "  curve not supported" << endl;
        exit(EXIT_FAILURE);
        break;
      case TVectorPath::CLOSE:
//        cout << "  close" << endl;
        break;
    }
  }
}

// read TVectorPath in the format used by bop12
void
restore(TVectorPath *path, const char *fn)
{
  ifstream in(fn);
  size_t n, o;
  in>>n;
  for(auto i=0; i<n; ++i) {
    in>>o;
    for(auto j=0; j<o; ++j) {
      TCoord x,y;
      in>>x;
      in>>y;
      if (j==0)
        path->move(TPoint(x,y));
      else
        path->line(TPoint(x,y));
    }
    path->close();
  }
}

void
replay(TEditModel *editmodel, TBoundary *clip)
{
  auto &pos(editmodel->pos);
  auto &path(editmodel->path);
  auto &handpath(editmodel->handpath);
  auto &nextstroke(editmodel->nextstroke);
  auto &nextpath(editmodel->nextpath);

  assert(pos<handpath.size());

  path.clear();
  nextpath.clear();
  for(size_t i=0; i<pos; ++i) {
    vector<TPoint> hull;

    if (i==0) {
      addHull(&hull, handpath, 0);
    } else {
      addHull(&hull, handpath, i-1);
      addHull(&hull, handpath, i);
      convexHull(&hull);
    }
    
    // this could be more efficient
    nextstroke.clear();
    for(auto p: hull) {
      if (nextstroke.empty())
        nextstroke.move(p);
      else
        nextstroke.line(p);
    }
    nextstroke.close();
    if (i+1<pos) {
      boolean(path, nextstroke, &path, UNION);
    } else {
      if (clip) {
        TVectorPath x;
        x.move(clip->p0);
        x.line(TPoint(clip->p1.x, clip->p0.y));
        x.line(clip->p1);
        x.line(TPoint(clip->p0.x, clip->p1.y));
        x.close();
        boolean(path, x, &path, INTERSECTION);
        boolean(nextstroke, x, &nextstroke, INTERSECTION);
      }
//booleanop_debug = true;
      boolean(path, nextstroke, &nextpath, UNION);
booleanop_debug = false;
    }
  }
  
  store(path, "subj");
  store(nextstroke, "clip");
}

class TMyPainter:
  public TWindow
{
  public:
    TVectorPath subj, clip, result;
  
    TMyPainter(TWindow *parent, const string &title): TWindow(parent, title) {}
//    using TMyPainter::TMyPainter;
    void paint() override;
};

void TMyPainter::paint()
{
  TPen pen(this);
  
  pen.setColor(0,0,0);
  subj.apply(pen);
  pen.stroke();
  
  pen.setColor(0,0,1);
  clip.apply(pen);
  pen.stroke();
  
  pen.setColor(1,0,0);
  result.apply(pen);
  pen.stroke();
}

void
test_tablet()
{
#if 0
  TMyPainter p(NULL, "TMyPainter");
  
  restore(&p.subj, "subj");
  restore(&p.clip, "clip");
  
  booleanop_debug = true;
  boolean(p.subj, p.clip, &p.result, UNION);
  
  toad::mainLoop();
  return;
#endif

#if 0
  TMyPainter p(NULL, "TMyPainter");
  
  p.clip.move(TPoint(10,10));
  p.clip.line(TPoint(30,10));
  p.clip.line(TPoint(35,30));
  p.clip.line(TPoint(15,30));
  p.clip.close();
  
  p.subj.move(TPoint(20,20));
  p.subj.line(TPoint(40,20));
  p.subj.line(TPoint(45,40));
  p.subj.line(TPoint(25,40));
  p.subj.close();
global_debug = true;
  boolean(p.subj, p.clip, &p.result, UNION);
  
  toad::mainLoop();
  return;
#endif

#if 0
  TEditModel *editmodel = new TEditModel();
  editmodel->zoom = 1.0;

  TWindow *wnd = new TWindow(NULL, "Boolean Operations Debugger");
  TDialog *panel = new TDialog(wnd, "panel");
  TVisualization *pane = new TVisualization(wnd, "pane", editmodel);
  
  // load data
//  editmodel->handpath = loadHandpath("backup-hang004-glitch.txt"); // glitch at 547, fixed with new findIntersection
//  editmodel->handpath = loadHandpath("backup-hang005.txt"); // an error at 544, fixed
//  editmodel->handpath = loadHandpath("backup-glitch009.txt"); // an error at 283, caused by above's fix
//  editmodel->handpath = loadHandpath("backup-glitch-newrecompute001.txt");
//  editmodel->handpath = loadHandpath("backup-glitch010.txt"); // one at 1976, multiple others, all fixed
//  editmodel->handpath = loadHandpath("backup-glitch011.txt"); // okay
//  editmodel->handpath = loadHandpath("backup.txt");
  editmodel->handpath = loadHandpath("fitcurve-segfault.txt");
  editmodel->pos.setRangeProperties(0, 0, 0, editmodel->handpath.size()-1);
  connect(editmodel->pos.sigChanged, [pane, editmodel] {
    replay(editmodel);
    pane->invalidateWindow();
  });
  editmodel->pos = editmodel->handpath.size()-1;
  wnd->w = 640;
  wnd->h = 480;

  // panel
  panel->flagShell = false;
  panel->bDrawFocus = true;

  editmodel->tool.add(new TFatRadioButton(panel, "zoom"), TEditModel::ZOOM);
  editmodel->tool.add(new TFatRadioButton(panel, "cut"), TEditModel::CUT);

  new TTextArea(panel, "replay-pos-text", &editmodel->pos);
  new TScrollBar(panel, "replay-pos-slider", &editmodel->pos);
  
  panel->loadLayout("boolean-op-debug-panel-atv");

  // layout

  TSpringLayout *layout = new TSpringLayout;
  layout->attach("panel", TSpringLayout::TOP | TSpringLayout::LEFT | TSpringLayout::BOTTOM);
  layout->attach("pane", TSpringLayout::TOP | TSpringLayout::RIGHT | TSpringLayout::BOTTOM);
  layout->attach("pane", TSpringLayout::LEFT, "panel");
  wnd->setLayout(layout);

  toad::mainLoop();
#else
  TMyWindow wnd(NULL, "test tablet");

#if 0  
  TEditModel m;
  m.handpath = loadHandpath("fitcurve-segfault.txt");
  m.pos = m.handpath.size()-1;
  replay(&m);
  wnd.path = m.path;
  wnd.path.simplify(4.0, 2.0*M_PI/360.0*30.0);
#endif  
  
  toad::mainLoop();
#endif
}
