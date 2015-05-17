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

#include "booleanop.h"

using namespace toad;

TVectorPath
computeBoolean(const TVectorPath &path1, const TVectorPath &path2, int operation)
{
  return TVectorPath();
}

namespace {

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      setSize(320,300);
    }
    
    void paint() override;
};

static void
toad2cbop(const TVectorPath &in, cbop::Polygon *out)
{
  out->push_back(cbop::Contour());
  cbop::Contour &c(out->back());
  const TPoint *pt = in.points.data();
  for(auto p: in.type) {
    switch(p) {
      case TVectorPath::MOVE: c.add(cbop::Point_2(pt->x, pt->y)); ++pt; break;
      case TVectorPath::LINE: c.add(cbop::Point_2(pt->x, pt->y)); ++pt; break;
      case TVectorPath::CURVE: break;
      case TVectorPath::CLOSE: break;
    }
  }
}

static void
cbop2toad(const cbop::Polygon &in, TVectorPath *out)
{
  for(auto p: in) {
    cout << "    contour with " << p.nholes() << " holes" << endl;
    bool f=false;
    for(auto q: p) {
      cout << "      ("<< q.x() << ", " << q.y() << ")" << endl;
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

TVectorPath
boolean(const TVectorPath &a, const TVectorPath &b, cbop::BooleanOpType op)
{
  TVectorPath c;
  cbop::Polygon subj, clip, result;
  toad2cbop(a, &subj);
  toad2cbop(b, &clip);
  cbop::compute(subj, clip, result, op);
  result.computeHoles();
  cbop2toad(result, &c);
  return c;
}

void
TMyWindow::paint()
{
  TPen pen(this);

  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(150,10));
  p0.line(TPoint(80,80));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(10,90));
  p1.line(TPoint(150,90));
  p1.line(TPoint(80,20));
  p1.close();

  cout << "union --------------" << endl;
  TVectorPath poly = boolean(p0, p1, cbop::UNION);
  pen.setColor(1,0.5,0);
  poly.apply(pen);
  pen.fill();
  
  pen.setColor(1,0,0);
  p0.apply(pen);
  pen.stroke();

  pen.setColor(0,0,1);
  p1.apply(pen);
  pen.stroke();

  cout << "intersection -------" << endl;
  poly = boolean(p0, p1, cbop::INTERSECTION);
  pen.translate(160,0);
  poly.apply(pen);
  pen.fill();

  cout << "difference ---------" << endl;
  poly = boolean(p0, p1, cbop::DIFFERENCE);
  pen.translate(0,100);
  poly.apply(pen);
  pen.fill();

  cout << "xor ----------------" << endl;
  poly = boolean(p0, p1, cbop::XOR);
  pen.translate(-160,0);
  poly.apply(pen);
  pen.fill();
  
  cout << "xor with real hole -" << endl;
  TVectorPath p2;
  p2.move(TPoint(10,10));
  p2.line(TPoint(150,10));
  p2.line(TPoint(150,90));
  p2.line(TPoint(10,90));
  p2.close();

  TVectorPath p3;
  p3.move(TPoint(80,20));
  p3.line(TPoint(140,80));
  p3.line(TPoint(20,80));
  p3.close();
  
  poly = boolean(p2, p3, cbop::DIFFERENCE);
  pen.translate(0,100);
  poly.apply(pen);
  pen.fill();
}

} // unnamed namespace

void
test_path_bool()
{
  TMyWindow wnd(NULL, "test path bool");
  toad::mainLoop();
}