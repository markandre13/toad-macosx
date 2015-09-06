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
    path = boolean(path, np, cbop::UNION);
//cout << "added" << endl;
    invalidateWindow();
  }
  
//  cout << "mouseEvent " << me.name() << ", " << me.pressure() << ", " << me.tangentialPressure() << ", " << me.rotation() << ", " << me.tilt() << endl;
}

} // unnamed namespace

void
replay(const char *filename)
{
  ifstream in;
  in.open(filename);
  
  TVectorPath path;
  vector<TFreehandPoint> handpath;
  
  while(true) {
    TCoord x, y, r, p;
    in>>x;
    in>>y;
    in>>r;
    in>>p;
    if (!in)
      break;
    cout << x << " " << y << " " << r << " " << p << endl;

    vector<TPoint> hull;
    if (handpath.empty()) {
      handpath.push_back(TFreehandPoint(x, y, r, p));
      addHull(&hull, handpath, 0);
    } else {
      handpath.push_back(TFreehandPoint(x, y, r, p));
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

cout << "==========================================================================" << endl;
cout << "boolean: to " << path.points.size() << " add " << np.points.size() << endl;
    path = boolean(path, np, cbop::UNION);
cout << "added" << endl;
  }
}

void
test_tablet()
{
#if 0
//  replay("test.txt");
  replay("backup-hang000.txt");
#else
  TMyWindow wnd(NULL, "test tablet");
  toad::mainLoop();
#endif
}
