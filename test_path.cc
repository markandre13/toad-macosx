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

void
TVectorPath::apply(TPen &pen) const
{
  const TPoint *pt = points.data();
  for(auto p: type) {
    switch(p) {
      case MOVE: pen.move(pt); ++pt; break;
      case LINE: pen.line(pt); ++pt; break;
      case CURVE: pen.curve(pt); pt+=3; break;
      case CLOSE: pen.close(); break;
    }
  }
}

TPen *gpen = 0;

void
TVectorPath::subdivide()
{
  vector<TPoint> oldpoints;
  vector<EType> oldtype;
  oldpoints.swap(points);
  oldtype.swap(type);

  const TPoint *start = 0;
  
  TIntersectionList ilist;

for(TCoord x=0; x<320; x+=10) {
  TPoint line[] = {{x,0}, {x,200}};
  
  const TPoint *pt = oldpoints.data();
  for(auto p: oldtype) {
    switch(p) {
      case MOVE:
        type.push_back(MOVE);
        points.push_back(*pt);
        start=pt;
        ++pt;
        break;
      case LINE:
        if (pt>oldpoints.data())
          intersectLineLine(ilist, pt-1, line);
        ++pt;
        break;
      case CURVE:
        if (pt>oldpoints.data())
          intersectCurveLine(ilist, pt-1, line);
        pt+=3;
        break;
      case CLOSE:
        if (start && pt>oldpoints.data()) {
          TPoint cl[2];
          cl[0]=*start;
          cl[1]=*(pt-1);
          intersectLineLine(ilist, cl, line);
        }
        break;
    }
  }
}
  for(auto p: ilist) {
    gpen->drawRectangle(p.seg0.pt.x-1.5, p.seg0.pt.y-1.5, 4,4);
    gpen->drawRectangle(p.seg1.pt.x-0.5, p.seg1.pt.y-0.5, 2,2);
  }
}

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
  
  p.move(TPoint(120, 20));
  p.line(TPoint(300, 50));
  p.curve(TPoint(310,190), TPoint(10,190), TPoint(50, 30));
  p.close();

  p.move(TPoint(120, 40));
  p.curve(TPoint(180,100), TPoint(180,190), TPoint(50, 190));
  p.curve(TPoint(40,150), TPoint(40,100), TPoint(120, 40));

  p.apply(pen);
  pen.stroke();
  
  pen.setColor(0,0,1);
  gpen = &pen;
cout << "call subdivide" << endl;
  p.subdivide();  

  pen.setColor(1,0.5,0);
  p.apply(pen);
  pen.stroke();

  pen.setColor(TColor::FIGURE_SELECTION);
  for(auto a: p.points) {
    pen.drawRectangle(a.x-1.5, a.y-1.5,4,4);
  }
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
