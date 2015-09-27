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

void
TMyWindow::paint()
{
  TPen pen(this);

  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.curve(TPoint(55,1),
           TPoint(100,2),
           TPoint(150,10));
//  p0.line(TPoint(150,10));
  p0.line(TPoint(80,80));
  p0.close();

  p0.apply(pen);
  pen.stroke();

  TVectorPath p1;
  p1.move(TPoint(10,90));
  p1.line(TPoint(150,90));
  p1.line(TPoint(80,20));
  p1.close();
  
  TVectorPath poly;
  
#if 0
  cout << "union --------------" << endl;
  poly = boolean(p0, p1, UNION);
  pen.setColor(1,0.5,0);
  poly.apply(pen);
  pen.fill();
  
  pen.setColor(1,0,0);
  p0.apply(pen);
  pen.stroke();

  pen.setColor(0,0,1);
  p1.apply(pen);
  pen.stroke();
#endif
#if 0
  cout << "intersection -------" << endl;
  poly = boolean(p0, p1, INTERSECTION);
  pen.translate(160,0);
  poly.apply(pen);
  pen.fill();
#endif
#if 1
  cout << "difference ---------" << endl;
  boolean(p0, p1, &poly, DIFFERENCE);
  pen.translate(0,100);
  poly.apply(pen);
  pen.fill();
#endif
#if 0
  cout << "xor ----------------" << endl;
  poly = boolean(p0, p1, XOR);
  pen.translate(-160,0);
  poly.apply(pen);
  pen.fill();
#endif
#if 0  
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

  poly = boolean(p2, p3, DIFFERENCE);
  pen.translate(0,100);
  poly.apply(pen);
  pen.fill();
#endif
#if 0
cout << "---------------------------------------------------" << endl;
  TVectorPath p4;
  p4.move(TPoint(90, 20));
  p4.line(TPoint(140, 20));
  p4.line(TPoint(140, 70));
  p4.close();
  poly = boolean(poly, p4, DIFFERENCE);

  pen.translate(160, 0);
  poly.apply(pen);
  pen.fill();
#endif
}

} // unnamed namespace

void
test_path_bool()
{
  TMyWindow wnd(NULL, "test path bool");
  toad::mainLoop();
}
