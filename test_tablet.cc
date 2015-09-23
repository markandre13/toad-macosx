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
#include "booleanop.hh"
#include <fstream>

#include <toad/dialog.hh>
#include <toad/springlayout.hh>
#include <toad/textfield.hh>
#include <toad/scrollbar.hh>
#include <toad/fatradiobutton.hh>
#include <toad/scrollpane.hh>
#include <toad/floatmodel.hh>

#include "bop12/booleanop.h"


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

static void
toad2cbop(const TVectorPath &in, cbop::Polygon *out)
{
  // FIXME: this function should drop neighbouring equal points (degenarted case)
  out->push_back(cbop::Contour());
  cbop::Contour *c = &out->back();
  const TPoint *pt = in.points.data();
  for(auto p: in.type) {
    switch(p) {
      case TVectorPath::MOVE: c->add(cbop::Point_2(pt->x, pt->y)); ++pt; break;
      case TVectorPath::LINE: c->add(cbop::Point_2(pt->x, pt->y)); ++pt; break;
      case TVectorPath::CURVE: break;
      case TVectorPath::CLOSE:
        out->push_back(cbop::Contour());
        c = &out->back();
        break;
    }
  }
}

static void
cbop2toad(const cbop::Polygon &in, TVectorPath *out)
{
  for(auto p: in) {
    bool f=false;
    for(auto q: p) {
      if (f) {
        out->line(TPoint(q.x(), q.y()));
      } else {
        out->move(TPoint(q.x(), q.y()));
        f = true;
      }
    }
    out->close();
  }
}

void
oboolean(const TVectorPath &inSubj, const TVectorPath &inClip, TVectorPath *outResult, BooleanOpType op)
{
  cbop::Polygon subj, clip, result;
  toad2cbop(inSubj, &subj);
  toad2cbop(inClip, &clip);
  cbop::compute (subj, clip, result, (cbop::BooleanOpType)op);
  cbop2toad(result, outResult);
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
  pen.setColor(0.7,0.7,0.7);
  path.apply(pen);
  pen.fill();
  pen.setColor(0,0,0);
  path.apply(pen);
  pen.stroke();

#if 0
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
  if (me.type==TMouseEvent::LDOWN) {
cout << "down" << endl;
    backup.close();
    backup.open("backup.txt");
    handpath.clear();
    path.clear();
  } else
  if (me.type==TMouseEvent::LUP) {
cout << "up" << endl;
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
//cout << "added" << endl;
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

void replay(TEditModel *editmodel);

class TVisualization:
  public TScrollPane
{
    TEditModel *editmodel;
  public:
    TVisualization(TWindow *parent, const string &title, TEditModel *em):
      TScrollPane(parent, title), editmodel(em) {}
    void adjustPane() override;
    void paint() override;
    void mouseEvent(const TMouseEvent &) override;
};

void
TVisualization::adjustPane()
{
//  visible.set(0,0,getWidth(), getHeight());
  TBoundary b = editmodel->path.bounds();
  b.x2+=20; b.y2+=20;
  pane.set(0,0,b.x2*editmodel->zoom, b.y2*editmodel->zoom);
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
    case TEditModel::CUT:
      switch(me.type) {
        case TMouseEvent::LDOWN: {
          TPoint pt = me.pos * (1.0 / editmodel->zoom);
          cout << "find something near " << pt << endl;
          
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
          
        } break;
      }
      break;
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
#if 0
  for(auto p: editmodel->path.points) {
    pen.drawLine(p.x-2*s, p.y-2*s, p.x+2*s, p.y+2*s);
    pen.drawLine(p.x+2*s, p.y-2*s, p.x-2*s, p.y+2*s);
  }
#endif  
  pen.setColor(0,0,1);
  editmodel->nextstroke.apply(pen);
  pen.stroke();

#if 0
  for(auto p: editmodel->nextstroke.points) {
    pen.drawCircle(p.x-s*1.5, p.y-s*1.5, 3*s, 3*s);
  }
#endif
  pen.setColor(1,0,0);
  editmodel->nextpath.apply(pen);
  pen.stroke();
#if 0
  size_t i=0;
  for(auto p: editmodel->nextpath.points) {

    if (138 <= i && i <= 139) {
cout << "draw " << i << " " << p << endl;
      pen.setColor(0,0.7,0);
    } else
    if (i == 919) {
cout << "draw " << i << " " << p << endl;
      pen.setColor(1,0.5,0);
    } else {
      pen.setColor(1,0,0);
    }

    pen.drawCircle(p.x-2*s, p.y-2*s, 4*s, 4*s);
    ++i;
  }
#endif
}

extern bool global_debug;

// write TVectorPath in the format used by bop12
void
store(const TVectorPath &path, const char *fn)
{
  ofstream out(fn);
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

void
replay(TEditModel *editmodel)
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
/*      
      struct Compare {
        bool operator() (const TPoint& lhs, const TPoint& rhs) const {
          return (lhs.x==rhs.x) ? lhs.y==rhs.y : lhs.x==rhs.x;
        }
      };
      set<TPoint, Compare> hull2;
      hull2.insert(hull.begin(), hull.end());
      hull.clear();
      hull.insert(hull.begin(), hull2.begin(), hull2.end());
*/      
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
#if 0    
TVectorPath x;
x.move(TPoint(133.069,226.047));
x.line(TPoint(137,226.047));
x.line(TPoint(137,240));
x.line(TPoint(133.069,240));
x.close();
boolean(path, x, &path, INTERSECTION);
boolean(nextstroke, x, &nextstroke, INTERSECTION);
#endif    
//      cout << "----------------- FINAL UNION -------------------" << endl;
//global_debug = true;
      boolean(path, nextstroke, &nextpath, UNION);
//global_debug = false;
//      cout << "-------------------------------------------------" << endl;
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
#if 1
  TEditModel *editmodel = new TEditModel();
  editmodel->zoom = 1.0;

  TWindow *wnd = new TWindow(NULL, "Boolean Operations Debugger");
  TDialog *panel = new TDialog(wnd, "panel");
  TVisualization *pane = new TVisualization(wnd, "pane", editmodel);
  
  // load data
//  editmodel->handpath = loadHandpath("backup-hang004-glitch.txt"); // glitch at 547, fixed
//  editmodel->handpath = loadHandpath("backup-glitch006.txt"); // still an error (or a new one)
  editmodel->handpath = loadHandpath("backup-hang005.txt"); // an error at 544, not fixed
  editmodel->pos.setRangeProperties(0, 0, 0, editmodel->handpath.size()-1);
  connect(editmodel->pos.sigChanged, [pane, editmodel] {
    replay(editmodel);
    pane->invalidateWindow();
  });
  editmodel->pos = 0;
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
  toad::mainLoop();
#endif
}
