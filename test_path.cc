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

#include "GraphicsGems.h"

using namespace toad;

void
TVectorPath::apply(TPen &pen) const
{
//cout << "TVectorPath::apply" << endl;
  const TPoint *pt = points.data();
  for(auto p: type) {
//cout << "apply type " << p << ", left="<<(points.size()-(pt-points.data()))<<endl;
/*
switch(p) {
  case MOVE: cout << "  MOVE" << endl; break;
  case LINE: cout << "  LINE" << pt[0] << endl; break;
  case CURVE: cout << "  CURVE" << pt[0] << pt[1] << pt[2]<< endl; break;
  case CLOSE: cout << "  CLOSE" << endl;
}
*/
    switch(p) {
      case MOVE: pen.move(pt); ++pt; break;
      case LINE: pen.line(pt); ++pt; break;
      case CURVE: pen.curve(pt); pt+=3; break;
      case CLOSE: pen.close(); break;
    }
  }
}

TPen *gpen = 0;

static const TCoord epsilon = 1e-12;
static inline bool isZero(TCoord a) { return fabs(a) <= epsilon; }

void
TVectorPath::subdivideCutter(EType t, const TPoint *pt, const TRectangle &bounds, bool linesToBezier)
{
//cout << "TVectorPath::subdivideCutter --------------------------------" << endl;
  TIntersectionList ilist;
  
  TPoint bez[4];
  if (linesToBezier && t==LINE) {
    TPoint d=(pt[1]-pt[0])*0.25;
    bez[0]=pt[0];
    bez[1]=pt[0]+d;
    bez[2]=pt[0]-d;
    bez[3]=pt[1];
    t = CURVE;
    pt = bez;
  }

  for(TCoord x=0; x<320; x+=40) {
    TPoint line[] = {{x,0}, {x,200}};
    if (t==TVectorPath::LINE)
      intersectLineLine(ilist, pt, line);
    else
      intersectCurveLine(ilist, pt, line);
  }

  for(TCoord y=0; y<200; y+=40) {
    TPoint line[] = {{0,y}, {320,y}};
    if (t==TVectorPath::LINE)
      intersectLineLine(ilist, pt, line);
    else
      intersectCurveLine(ilist, pt, line);
  }

  sort(ilist.begin(), ilist.end(), [](const auto &p, const auto &q) {
    return (p.seg0.u < q.seg0.u);
  });

  TCoord s = 1.0/ilist.size();
  TCoord c = 0;
  for(auto p: ilist) {
    gpen->setColor(c,0,1-c); c+=s;
    gpen->drawRectangle(p.seg0.pt.x-1.5, p.seg0.pt.y-1.5, 4,4);
    gpen->drawRectangle(p.seg1.pt.x-0.5, p.seg1.pt.y-0.5, 2,2);
  }
  
  auto p = ilist.begin();
  if (p==ilist.end()) {
    type.push_back(t);
    points.push_back(pt[1]);
    if (t==CURVE) {
      points.push_back(pt[2]);
      points.push_back(pt[3]);
    }
    return;
  }
  
  if (t==LINE) {
    TCoord lastU=0.0;
    for(;p!=ilist.end();++p) {
      if (isZero(lastU-p->seg0.u))
        continue;
      type.push_back(LINE);
      points.push_back(p->seg0.pt);
      lastU = p->seg0.u;
    }
    if (isZero(1-lastU))
      return;
    type.push_back(LINE);
    points.push_back(pt[1]);
    return;
  }

  TPoint sub[4];
  TCoord lastU=0.0;
  for(;p!=ilist.end();++p) {
    if (isZero(lastU-p->seg0.u))
      continue;
    divideBezier(pt, sub, lastU, p->seg0.u);
    type.push_back(CURVE);
    points.push_back(sub[1]);
    points.push_back(sub[2]);
    points.push_back(sub[3]);
    lastU = p->seg0.u;
  }
  if (isZero(1-lastU))
    return;
  divideBezier(pt, sub, lastU, 1);
  type.push_back(CURVE);
  points.push_back(sub[1]);
  points.push_back(sub[2]);
  points.push_back(sub[3]);
}

void
TVectorPath::subdivide()
{
  vector<TPoint> oldpoints;
  vector<EType> oldtype;
  oldpoints.swap(points);
  oldtype.swap(type);

  TRectangle r; // = bounds();
  bool linesToBezier = true;

  const TPoint *start = 0;
  
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
          subdivideCutter(p, pt-1, r, linesToBezier);
        ++pt;
        break;
      case CURVE:
        if (pt>oldpoints.data())
          subdivideCutter(p, pt-1, r, linesToBezier);
        pt+=3;
        break;
      case CLOSE:
        if (start && pt>oldpoints.data()) {
          TPoint cl[2];
          cl[0]=*(pt-1);
          cl[1]=*start;
          subdivideCutter(TVectorPath::LINE, cl, r, linesToBezier);
        }
        break;
    }
  }
}

vector<TPoint> *pathOut;

void DrawBezierCurve(int n, BezierCurve curve)
{
  cout << "DrawBezierCurve" << endl;
  for(int i=0; i<n; ++i)
    pathOut->push_back(TPoint(curve[i].x, curve[i].y));
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
#if 0
  fitPath(p.points.data(), p.points.size(), 2.5, &out);
#else
  pathOut = &out;
  FitCurve(p.points.data(), p.points.size(), 2.5);
  out.push_back(p.points.back());
#endif
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
