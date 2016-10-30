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

using namespace toad;

namespace {

TPoint a[5] = {
/*
  { 10, 10 },
  { 10, 50 },
  { 50, 50 },
  { 60, 60 },
  { 70, 70 },
*/
  { 10, 10 },
  { 90, 30 },
  { 90, 50 },
  { 90, 70 },
  { 170, 90 },
};

class TMyWindow:
  public TWindow
{
  size_t handle;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      handle = -1;
    }
    
    void paint() override;
    void mouseEvent(const TMouseEvent&) override;
};

/*
stuff which doesn't work for simplify (0.1)
move (10,10)
line (14.418,44.4805)
line (27.0547,59.1875)
line (24.0273,75.1953)
line (22.9102,90.8008)
*/

void
TMyWindow::paint()
{
  TPen pen(this);
  pen.drawLines(a,5);
  pen.setColor(0,0.5,1);
  for(size_t i=0; i<5; ++i) {
    pen.drawRectangle(a[i].x-2.5, a[i].y-2.5,5,5);
  }
  
  TVectorPath path;
  path.move(a[0]);
  for(size_t i=1; i<5; ++i) {
    path.line(a[i]);
  }
  cout << "-----------------------------" << endl;
  path.simplify(0.001);

  for(auto p: path.points) {
    pen.drawCircle(p.x-2.5, p.y-2.5,5,5);
  }

  cout << path << endl;

  pen.setColor(1,0.5,0);
  path.apply(pen);
  pen.stroke();
  
}

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN: {
      for(size_t i=0; i<5; ++i) {
        if (a[i].x-2.5 <= me.pos.x && me.pos.x <= a[i].x+2.5 &&
            a[i].y-2.5 <= me.pos.y && me.pos.y <= a[i].y+2.5 )
        {
          handle = i;
        }
      }
    } break;
    case TMouseEvent::MOVE: {
//      double d = atan2(a[0].y-a[1].y, a[0].x-a[1].x) - atan2(a[1].y-a[2].y, a[1].x-a[2].x);
//      if (d<-M_PI)
//        d+=M_PI;
//      cout << d << endl;
      if (handle==-1)
        break;
      a[handle] = me.pos;
      invalidateWindow();
    } break;
    case TMouseEvent::LUP: {
      if (handle==-1)
        break;
      a[handle] = me.pos;
      invalidateWindow();
      handle=-1;
    } break;
  }
}


} // unnamed namespace

void
test_path()
{
  TMyWindow wnd(NULL, "test path");
  toad::mainLoop();
}
