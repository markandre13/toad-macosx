/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.org>
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
  Example for TOAD first data interface implementation:
*/

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/bitmap.hh>
#include <toad/connect.hh>

#include <cstdarg>

using namespace toad;

namespace {

TBitmap bsrc;
TBitmap bdst;

TPoint src[4] = {
  { 0, 0 },
  { 174, 0 },
  { 174, 255 },
  { 0, 255 }
};

TPoint dst[5] = {
  { 360, 20 },
  { 360+174, 20 },
  { 360+174, 20+255 },
  { 360, 20+255 },
  { 360+174/2, 20+255/2 },
};

class TMyWindow:
  public TWindow
{
    int handle;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      setSize(800, 600);
      handle = -1;
    }
    
    void paint() override;
    void paintTransform(TPen &pen);
    void mouseEvent(const TMouseEvent&) override;
};

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<5; ++i) {
        if(dst[i].x-4<=me.pos.x && me.pos.x <= dst[i].x+4 &&
           dst[i].y-4<=me.pos.y && me.pos.y <= dst[i].y+4)
        {
          handle = i;
          break;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
      dst[handle] = me.pos;
      invalidateWindow();
      break;
    case TMouseEvent::LUP:
      handle = -1;
      break;
  }
}

/* http://www.mathopenref.com/coordpolygonarea.html
function polygonArea(X, Y, numPoints) 
{ 
  area = 0;         // Accumulates area in the loop
  j = numPoints-1;  // The last vertex is the 'previous' one to the first

  for (i=0; i<numPoints; i++)
    { area = area +  (X[j]+X[i]) * (Y[j]-Y[i]); 
      j = i;  //j is previous vertex to i
    }
  return area/2;
}
*/

// intersection of AB & CD
TPoint
intersection(const TPoint &a0,
             const TPoint &a1,
             const TPoint &b0,
             const TPoint &b1,
             TCoord *ra=NULL,
             TCoord *rb=NULL)
{
  TCoord ax = a1.x - a0.x;
  TCoord ay = a1.y - a0.y;
  TCoord bx = b1.x - b0.x;
  TCoord by = b1.y - b0.y;

  if (ax==0.0 && by==0.0) {
    return TPoint(a0.x, b0.y);
  }
  if (ay==0.0 && bx==0.0) {
    return TPoint(b0.x, a0.y);
  }

  if (ax!=0.0) {
    TCoord b = ((b0.x-a0.x)/ax*ay - b0.y + a0.y)/(by-bx/ax*ay);
    TCoord a = (b0.x - a0.x + b * bx) / ax;
    if (ra) *ra = a;
    if (rb) *rb = b;
    return TPoint(a0.x+a*ax, a0.y+a*ay);
  }
  TCoord a = ((a0.x-b0.x)/bx*by - a0.y + b0.y)/(ay-ax/bx*by);
  TCoord b = (a0.x - b0.x + a * ax) / bx;
  if (ra) *ra = a;
  if (rb) *rb = b;
  return TPoint(b0.x+b*bx, b0.y+b*by);
}

bool
intersection(const TPoint &a0,
             const TPoint &a1,
             const TPoint &b0,
             const TPoint &b1,
             TPoint *r,
             TCoord *ra=NULL,
             TCoord *rb=NULL)
{
  TCoord ax = a1.x - a0.x;
  TCoord ay = a1.y - a0.y;
  TCoord bx = b1.x - b0.x;
  TCoord by = b1.y - b0.y;

  if ((ax==0 && bx==0) ||
      (ay==0 && by==0) ||
      (ax !=0 && bx !=0 && ay/ax==by/bx) ||
      (ay !=0 && by !=0 && ax/ay==bx/by))
    return false;

  if (ax==0.0 && by==0.0) {
    r->set(a0.x, b0.y);
    if (ra) *ra = (b0.y-a0.y)/ay;
    return true;
  }
  if (ay==0.0 && bx==0.0) {
    r->set(b0.x, a0.y);
    if (ra) *ra = (b0.x-a0.x)/ax;
    return true;
  }

  if (ax!=0.0) {
    TCoord b = ((b0.x-a0.x)/ax*ay - b0.y + a0.y)/(by-bx/ax*ay);
    TCoord a = (b0.x - a0.x + b * bx) / ax;
    if (ra) *ra = a;
    if (rb) *rb = b;
    r->set(a0.x+a*ax, a0.y+a*ay);
    return true;
  }
  TCoord a = ((a0.x-b0.x)/bx*by - a0.y + b0.y)/(ay-ax/bx*by);
  TCoord b = (a0.x - b0.x + a * ax) / bx;
  if (ra) *ra = a;
  if (rb) *rb = b;
  r->set(b0.x+b*bx, b0.y+b*by);
  return true;
}

string
format(const char *fmt, ...)
{
  char buffer[8192];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);
  return buffer;
}

void
TMyWindow::paintTransform(TPen &pen)
{
  TPoint A(dst[0]);
  TPoint B(dst[1]);
  TPoint C(dst[2]);
  TPoint D(dst[3]);
  
pen.setColor(0.5, 0.5, 0.5);

  TPoint p0=dst[0], p1=dst[0];
  for(int i=1; i<4; ++i) {
    p0.x = min(p0.x, dst[i].x);
    p0.y = min(p0.y, dst[i].y);
    p1.x = max(p1.x, dst[i].x);
    p1.y = max(p1.y, dst[i].y);
  }
  TPoint Q;
  for(Q.y=p0.y; Q.y<=p1.y; ++Q.y) {
    for(Q.x=p0.x; Q.x<=p1.x; ++Q.x) {
      TCoord x, y;
      
      TPoint E, G;
      if (!intersection(A,D, B,C, &E))
        E = Q + (A-D);
      intersection(A,B, E,Q, &G, &x);

      TPoint F, I;
      if (!intersection(A,B, C,D, &F))
        F = Q + (D-C);
      intersection(A,D, F,Q, &I, &y);
      if (x<0 || x>1 || y<0 || y>1)
        continue;
      
      TCoord r,g,b;
      bsrc.getPixel(x*bsrc.width, y*bsrc.height, &r, &g ,&b);
//cout << a*174 << ", " << b*255 << endl;
      pen.setColor(r,g,b);
      pen.fillRectangle(Q.x, Q.y, 1,1);
    }
  }
}

void
TMyWindow::paint()
{
  TPen pen(this);
  pen.translate(0.5, 0.5);

#if 0
  for(TCoord x=0; x<174; ++x) {
    for(int y=0; y<255; ++y) {
      TCoord r,g,b;
      bsrc.getPixel(x,y,&r,&g,&b);
      bdst.setPixel(x+200, y+20, r,g,b);
    }
  }
  pen.drawBitmap(0,0,bdst);
#endif
  pen.drawBitmap(0,0,bsrc);
  paintTransform(pen);
  
  TPoint A(dst[0]);
  TPoint B(dst[1]);
  TPoint C(dst[2]);
  TPoint D(dst[3]);
  TPoint Q(dst[4]);
  
  TCoord a, b;

  TPoint E, G;
  if (!intersection(A,D, B,C, &E))
    E = Q + (A-D);
  intersection(A,B, E,Q, &G, &a);

  TPoint F, I;
  if (!intersection(A,B, C,D, &F))
    F = Q + (D-C);
  intersection(A,D, F,Q, &I, &b);

  pen.setColor(1,0,0);
  pen.drawPolygon(src, 4);
  pen.setColor(0,0.5,1);
  pen.drawPolygon(dst, 4);
  pen.drawCircle(dst[4].x-3, dst[4].y-3, 7, 7);
  
  pen.setColor(0,1,0);

  pen.drawCircle(E.x-3, E.y-3,7,7);
  pen.drawCircle(F.x-3, F.y-3,7,7);

  TPoint H(intersection(C,D, E,Q));
  TPoint J(intersection(B,C, F,Q));

  pen.setColor(1,1,1);
  pen.drawCircle(G.x-3, G.y-3,7,7);
  pen.drawCircle(H.x-3, H.y-3,7,7);
  pen.drawCircle(I.x-3, I.y-3,7,7);
  pen.drawCircle(J.x-3, J.y-3,7,7);
  
  pen.setColor(1,0.5,0);
  pen.drawLine(G,H);
  pen.drawLine(I,J);

  pen.drawString(10,200, format("a=%f, b=%f", a, b));
}

} // unnamed namespace

void
test_image()
{
  TMyWindow wnd(NULL, "test image");
  bsrc.load("test.jpg"); // 174x255
  bdst.width = 800;
  bdst.height = 600;
  
  // find bounds of dst
  // scanline over bounds
  // map position to src
  
  
  toad::mainLoop();
}
