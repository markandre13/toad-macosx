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
  { 375, 0 },
  { 375, 297 },
  { 0, 297 }
};

TPoint dst[16] = {
  { 300, 100 },
  { 400, 100 },
  { 500, 100 },
  { 600, 100 },

  { 300, 200 },
  { 400, 200 },
  { 500, 200 },
  { 600, 200 },

  { 300, 300 },
  { 400, 300 },
  { 500, 300 },
  { 600, 300 },

  { 300, 400 },
  { 400, 400 },
  { 500, 400 },
  { 600, 400 }
};

class TMyWindow:
  public TWindow
{
    int handle;
    bool bitmapDirty;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      setSize(800, 600);
      handle = -1;
      bitmapDirty = true;
    }
    
    void paint() override;
    void clearBitmap();
    void transformBitmap(const TPoint *dst, TCoord x=0.0, TCoord y=0.0, TCoord w=1.0, TCoord h=1.0);
    void mouseEvent(const TMouseEvent&) override;
};

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<16; ++i) {
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
      bitmapDirty = true;
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
static const TCoord epsilon = 1e-12;
static inline bool isZero(TCoord a) { return fabs(a) <= epsilon; }

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

  TCoord cross = ax*by - ay*bx;
  if (isZero(cross)) {
    return false;
  }
    
  TCoord 
    dx = a0.x - b0.x,
    dy = a0.y - b0.y,
    a = (bx * dy - by * dx) / cross,
    b = (ax * dy - ay * dx) / cross;
  if (ra) *ra = a;
  if (rb) *rb = b;
  if (r) r->set(a0.x + a * ax, a0.y + a * ay);
  return true;
}

void
TMyWindow::clearBitmap()
{
  if (bdst.img) {
    unsigned char *data = [bdst.img bitmapData];
    memset(data, 0, 4* bdst.width* bdst.height);
  }
}

void
TMyWindow::transformBitmap(const TPoint *dst, TCoord sx, TCoord sy, TCoord sw, TCoord sh)
{
  // find min and max of dst
  TPoint A(dst[0]);
  TPoint B(dst[1]);
  TPoint C(dst[2]);
  TPoint D(dst[3]);

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
      bsrc.getPixel((sx+x*sw)*bsrc.width, (sy+y*sh)*bsrc.height, &r, &g ,&b);
//cout << a*174 << ", " << b*255 << endl;
//      pen.setColor(r,g,b);
//      pen.fillRectangle(Q.x, Q.y, 1,1);
      bdst.setPixel(Q.x, Q.y, r, g, b);
      bdst.setPixel(Q.x+1, Q.y, r, g, b);
      bdst.setPixel(Q.x+1, Q.y+1, r, g, b);
      bdst.setPixel(Q.x, Q.y+1, r, g, b);
    }
  }
}

// Fakultät
long fak(long n) {
  long r=1;
  for(long i=2; i<=n; ++i)
    r*=i;
  return r;
}

// Binomial Koeffizient
TCoord C(long n, long i)
{
  return (TCoord)fak(n) / (TCoord)( fak(i) * fak(n-i) );
}

// Bernsteinpolynom
TCoord B(long i, long n, TCoord t)
{
  return C(n, i) * pow(t, i) * pow(1-t, n-i);
}


void
TMyWindow::paint()
{
  if (bitmapDirty) {
    clearBitmap();
//    transformBitmap();
    bitmapDirty = false;
  }

  TPen pen(this);
  pen.translate(0.5, 0.5);

  TCoord s = 0.05;
  for(TCoord u=0.0; u<1.0; u+=s) { // x
    for(TCoord v=0.0; v<1.0; v+=s) { // y
//cout << "uv="<<u<<","<<v<<endl;
      TPoint pt[4];
      for(int i=0; i<4; ++i) {
        for(int j=0; j<4; ++j) {
          pt[0] += B(i, 3, u) * B(j, 3, v) * dst[i+j*4];
          pt[1] += B(i, 3, u+s) * B(j, 3, v) * dst[i+j*4];
          pt[2] += B(i, 3, u+s) * B(j, 3, v+s) * dst[i+j*4];
          pt[3] += B(i, 3, u) * B(j, 3, v+s) * dst[i+j*4];
        }
      }
      transformBitmap(pt, u, v, s, s);
      // pen.fillRectangle(pt.x, pt.y,1,1);
    }
  }
  pen.drawBitmap(0,0,bdst);
  pen.drawBitmap(0,0,bsrc);
  
  pen.setColor(1,1,1);
  for(int i=0; i<16; ++i) {
    pen.drawRectangle(dst[i].x-2, dst[i].y-2, 5, 5);
  }
  
  pen.drawBezier(dst, 4);
  pen.drawBezier(dst+12, 4);
  pen.drawBezier(dst+12, 4);
  pen.drawBezier(dst+12, 4);
  TPoint foo[4];
  foo[0]=dst[3];
  foo[1]=dst[7];
  foo[2]=dst[11];
  foo[3]=dst[15];
  pen.drawBezier(foo, 4);
  foo[0]=dst[12];
  foo[1]=dst[8];
  foo[2]=dst[4];
  foo[3]=dst[0];
  pen.drawBezier(foo, 4);
  
  pen.setLineStyle(TPen::DOT);
  pen.drawLine(dst[0], dst[1]);
  pen.drawLine(dst[2], dst[3]);
  
  pen.drawLine(dst[3], dst[7]);
  pen.drawLine(dst[11], dst[15]);
  
  pen.drawLine(dst[15], dst[14]);
  pen.drawLine(dst[13], dst[12]);
  
  pen.drawLine(dst[12], dst[8]);
  pen.drawLine(dst[4], dst[0]);

  pen.drawBezier(dst+4, 4);
  pen.drawBezier(dst+8, 4);

/*
  pen.setColor(1,0,0);
  for(int x=300; x<=450; x+=10) {
    for(int y=100; y<=250; y+=10) {
      pen.fillRectangle(x,y,1,1);
    }
  }
*/
//cout<<"----------"<<endl;


#if 0  
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

//  pen.setColor(1,0,0);
//  pen.drawPolygon(src, 4);
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
#endif
}

} // unnamed namespace

void
test_image()
{
  TMyWindow wnd(NULL, "test image");
  bsrc.load("test.jpg"); // 375x297
  bdst.width = 800;
  bdst.height = 600;
  
  // find bounds of dst
  // scanline over bounds
  // map position to src
  
  
  toad::mainLoop();
}
