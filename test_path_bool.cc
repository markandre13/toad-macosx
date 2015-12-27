/* 
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>
#include <toad/vector.hh>
#include <toad/geometry.hh>

#include <toad/booleanop.hh>

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

#if 0

  pen.scale(4,4);
  pen.translate(-137, -49);

  TPoint p[8] = {
    { 138.87270391526317,74.116354064879516 },
    { 137.56655333463428,80.32056932286693 },
    { 144.10999471605044,84.207155298278522 },
    { 172.54618572748129,82.546185727481287 },

    { 138.87270391526317,74.11635406487953 },
    { 140.31180066532067,67.28064450210627 },
    { 151.27974918196568,57.631496229352749 },
    { 152.99397991594583,49.488900242947025 },
  };

  pen.setColor(1,0.5,0.5);
  pen.drawBezier(p, 4);
  pen.setColor(0.5,0.5,1);
  pen.drawBezier(p+4, 4);
  pen.setColor(1,0,0);
  pen.drawLines(p, 4);
  pen.setColor(0,0,1);
  pen.drawLines(p+4, 4);

  return;
  
#endif

#if 0
  TPoint p[4] = {
/*
    { 10, 40 },
    { 300, 10 },
    { 30, 100 },
*/
    { 200, 80 },
    { 300, 80 },
    { 30, 150 },
    { 10, 50 }
  };

  pen.drawLines(p, 4);
  pen.drawBezier(p, 4);
  
  // find zero in the 1st derivative
  TCoord a = 3 * (p[1].y - p[2].y) - p[0].y + p[3].y,
         b = 2 * (p[0].y + p[2].y) - 4 * p[1].y,
         c = p[1].y - p[0].y;
  TCoord root[2];
  int count = solveQuadratic(a, b, c, root);
  if (count == 2) {
    if (root[0]<0.0 || root[0]>1.0) {
      root[0] == root[1];
      --count;
    } else
    if (root[1]<0.0 || root[1]>1.0) {
      --count;
    }
  }
  if (count == 1) {
    if (root[0]<0.0 || root[0]>1.0) {
      --count;
    }
  }
  
  
  pen.setColor(0,0,1);
  for(int i=0; i<count; ++i) {
cout << "    " << root[i] << endl;
    TPoint pt = bez2point(p, root[i]);
    pen.drawCircle(pt.x-2,pt.y-2,4,4);
  }

  return;
  
#endif
  
#if 1
  TVectorPath p0;
  p0.move(TPoint(10,40));
  p0.curve(TPoint(300,10),
           TPoint(30,100),
           TPoint(200,80));
  p0.curve(TPoint(300,80),
           TPoint(30,150),
           TPoint(10,50));
  p0.close();

  p0.apply(pen);
  pen.stroke();

  TVectorPath p1;
  p1.move(TPoint(30,90));
  p1.line(TPoint(180,90));
  p1.line(TPoint(110,20));
  p1.close();

  pen.setColor(0,1,0);
  p0.apply(pen);
  pen.stroke();

  pen.setColor(0,0,1);
  p1.apply(pen);
  pen.stroke();

  pen.setColor(1,0,0);
  TVectorPath poly;
#endif  
#if 0
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
#endif  
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
#if 1
  cout << "intersection -------" << endl; // HERE
booleanop_debug=true;
  boolean(p0, p1, &poly, UNION);
  pen.translate(0,150);
  poly.apply(pen);
  pen.stroke();
#endif
#if 0
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
