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

/*
  playing around with paths
  o cairo/cocoa compatible path representation
  o bézier curve intersection
  o subdivide
  o visible bounding box
  o ...
  o (convex hull)
*/

#include <vector>

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/bitmap.hh>
#include <toad/connect.hh>

#include <cstdarg>
#include <algorithm>

using namespace toad;

#define MODERN 1

namespace {

class TMyWindow:
  public TWindow
{
    vector<TPoint> pt;
    vector<TPoint> curve;
    int handle;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      handle=-1;
      setSize(800, 600);
#ifdef MODERN
      for(int i=0; i<20; ++i)
        pt.push_back(TPoint(arc4random()%700+50, arc4random()%500+50));
#else
      for(int i=0; i<20; ++i)
        pt.push_back(TPoint(rand()%700+50, rand()%500+50));
#endif
/*
      for(int i=0; i<4; ++i)
        curve.push_back(TPoint(arc4random()%700+50, arc4random()%500+50));
*/
#if 0
      curve.push_back(TPoint(200,400));
      curve.push_back(TPoint(250,200));
      curve.push_back(TPoint(500,300));
      curve.push_back(TPoint(600,400));

      curve.push_back(TPoint(400,100));
      curve.push_back(TPoint(350,200));
      curve.push_back(TPoint(450,350));
      curve.push_back(TPoint(300,500));
#endif
#if 1
      // too deep
//      curve = {{200,400},{250,200},{500,300},{600,400},{274,284},{342,43},{450,350},{300,500}};
      // 9 intersections
      curve = {{116,478},{398,52},{121,565},{296,266},{332,424},{5,167},{483,512},{113,207}};
#endif
    }
    
    void paint() override;
    void mouseEvent(const TMouseEvent&) override;
};

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<8; ++i) {
#ifdef MODERN
        if(curve[i].x-4<=me.pos.x && me.pos.x <= curve[i].x+4 &&
           curve[i].y-4<=me.pos.y && me.pos.y <= curve[i].y+4)
        {
          handle = i;
          break;
        }
      }
#else
        if(curve[i].x-4<=me.x && me.x <= curve[i].x+4 &&
           curve[i].y-4<=me.y && me.y <= curve[i].y+4)
        {
          handle = i;
          break;
        }
      }
#endif
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
#ifdef MODERN
      curve[handle] = me.pos;
#else
      curve[handle].x = me.x;
      curve[handle].y = me.y;
#endif
      invalidateWindow();
      break;
    case TMouseEvent::LUP:
      handle = -1;
      break;
  }
}


void
TMyWindow::paint()
{
cout << "paint --------------------- " << endl;
  TPen pen(this);
  pen.translate(0.5, 0.5);

#if 0
  vector<TPoint> pt(this->pt);
  convexHull(&pt);

  // paint
  int i=1;
  for(auto p: this->pt) {
    pen.drawCircle(p.x-3, p.y-3, 7, 7);
    pen.drawString(p.x+3, p.y+4, format("%i", i));
    ++i;
  }
  pen.setColor(1,0,0);
  for(auto p: pt) {
    pen.drawCircle(p.x-2, p.y-2, 5, 5);
    ++i;
  }
#endif

  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(curve.data(), 4);
//  pen.drawLines(curve.data()+4, 4);
  for(auto p: curve)
    pen.drawRectangle(p.x-2, p.y-2, 5, 5);

  pen.setColor(0,0,0);
  pen.drawBezier(curve.data(), 4);
//  pen.drawBezier(curve.data()+4, 4);
  
  pen.setColor(0,1,0);
  pen.drawLines(curve.data()+4, 2);
  
  for(auto p: curve)
    cout << "{"<<p.x<<","<<p.y<<"},";
  cout<<endl;
/*
  deeptimer=0;
  bezierClipping(pen, curve.data(), curve.data()+4, curve.data(), curve.data()+4);
  if (deeptimer)
    cout << deeptimer << " times too deep" << endl;
*/

#if 0
  intersectCurveLine(pen, curve.data(), curve.data()+4);

  pen.setColor(1,0.5,0);
  TRectangle r = bounds(curve.data());
cout << "bounds: " << r << endl;

//r.w=100;
//r.h=100;
  pen.drawRectangle(r);
//  pen.drawRectangle(bounds(curve.data()+4));
#endif
}

} // unnamed namespace

void
test_curve()
{
  TMyWindow wnd(NULL, "test curve");
  toad::mainLoop();
}
