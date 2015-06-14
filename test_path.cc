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

TPen *gpen = 0;

namespace {

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
};

void
TMyWindow::paint()
{
  TPen pen(this);
  TVectorPath p;
  gpen = &pen;

#if 0
  p.move(TPoint(120, 20));
  p.line(TPoint(300, 50));
  p.curve(TPoint(310,190), TPoint(10,190), TPoint(50, 30));
//  p.line(TPoint(210, 180));
  p.close();

  p.move(TPoint(120, 40));
  p.curve(TPoint(180,100), TPoint(180,190), TPoint(50, 190));
  p.curve(TPoint(40,150), TPoint(40,100), TPoint(120, 40));

  p.apply(pen);
  pen.stroke();
  
  pen.setColor(0,0,1);
  gpen = &pen;
  p.subdivide();  

#endif
  p.move(TPoint(10, 10));
  p.curve(TPoint(310,10), TPoint(100,190), TPoint(10, 190));
/*
  pen.setColor(1,0.5,0);
  p.apply(pen);
  pen.stroke();
*/
  p.subdivide();
/*
  pen.setColor(TColor::FIGURE_SELECTION);
  for(auto a: p.points) {
    pen.drawRectangle(a.x-1.5, a.y-1.5,4,4);
  }
*/  
  vector<TPoint> out;
  fitPath(p.points.data(), p.points.size(), 2.5, &out);

  pen.setColor(0,1,0);
  for(auto a: out) {
    pen.drawRectangle(a.x-2.5, a.y-2.5,6,6);
  }
  
  cout << "reduced " << p.points.size() << " to " << out.size() << endl;

  pen.drawBezier(out.data(), out.size());

/*  
  pen.setColor(1,0,0);
  pen.drawRectangle(p.bounds());
  pen.setColor(0,0,1);
  pen.drawRectangle(p.editBounds());
*/
}

} // unnamed namespace

void
test_path()
{
  TMyWindow wnd(NULL, "test path");
  toad::mainLoop();
}
