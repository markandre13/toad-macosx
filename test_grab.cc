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

#include <toad/connect.hh>

using namespace toad;

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
    void mouseEvent(const TMouseEvent&) override;
};

// FakultÃ¤t
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

TPoint E, F, G, H, J, K;

// two vanishing point perspective
static TPoint
transferPoint (const TPoint &I, TPoint *source, TPoint *destination)
{
    TCoord ADDING = 0.001; // to avoid dividing by zero

    TPoint A = source[0];
    TPoint C = source[2];

    TPoint Au = destination[0];
    TPoint Bu = destination[1];
    TPoint Cu = destination[2];
    TPoint Du = destination[3];

    // if points are the same, have to add a ADDING to avoid dividing by zero
    if (Bu.x==Cu.x) Cu.x+=ADDING;
    if (Au.x==Du.x) Du.x+=ADDING;
    if (Au.x==Bu.x) Bu.x+=ADDING;
    if (Du.x==Cu.x) Cu.x+=ADDING;
    TCoord kBC = (Bu.y-Cu.y)/(Bu.x-Cu.x);
    TCoord kAD = (Au.y-Du.y)/(Au.x-Du.x);
    TCoord kAB = (Au.y-Bu.y)/(Au.x-Bu.x);
    TCoord kDC = (Du.y-Cu.y)/(Du.x-Cu.x);

    if (kBC==kAD) kAD+=ADDING;
    E.x = (kBC*Bu.x - kAD*Au.x + Au.y - Bu.y) / (kBC-kAD);
    E.y = kBC*(E.x - Bu.x) + Bu.y;

    if (kAB==kDC) kDC+=ADDING;
    F.x = (kAB*Bu.x - kDC*Cu.x + Cu.y - Bu.y) / (kAB-kDC);
    F.y = kAB*(F.x - Bu.x) + Bu.y;

    if (E.x==F.x) F.x+=ADDING;
    TCoord kEF = (E.y-F.y) / (E.x-F.x);

    if (kEF==kAB) kAB+=ADDING;
    G.x = (kEF*Du.x - kAB*Au.x + Au.y - Du.y) / (kEF-kAB);
    G.y = kEF*(G.x - Du.x) + Du.y;

    if (kEF==kBC) kBC+=ADDING;
    H.x = (kEF*Du.x - kBC*Bu.x + Bu.y - Du.y) / (kEF-kBC);
    H.y = kEF*(H.x - Du.x) + Du.y;

// ---

    TCoord rG = (C.y-I.y)/(C.y-A.y);
    TCoord rH = (I.x-A.x)/(C.x-A.x);

    J.x = (G.x-Du.x)*rG + Du.x;
    J.y = (G.y-Du.y)*rG + Du.y;

    K.x = (H.x-Du.x)*rH + Du.x;
    K.y = (H.y-Du.y)*rH + Du.y;

    if (F.x==J.x) J.x+=ADDING;
    if (E.x==K.x) K.x+=ADDING;
    TCoord kJF = (F.y-J.y) / (F.x-J.x); //23
    TCoord kKE = (E.y-K.y) / (E.x-K.x); //12

    TCoord xKE;
    if (kJF==kKE) kKE+=ADDING;

    TPoint Iu;
    Iu.x = (kJF*F.x - kKE*E.x + E.y - F.y) / (kJF-kKE);
    Iu.y = kJF * (Iu.x - J.x) + J.y;

    return Iu;
}


int handle=-1;
TPoint p0[4] = { { 10, 10 }, { 110, 10 }, { 110, 110}, { 10, 110 } };
TPoint p[4] = { { 10, 10 }, { 110, 10 }, { 110, 110}, { 10, 110 } };

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<4; ++i) {
        if(p[i].x-2<=me.pos.x && me.pos.x <= p[i].x+2 &&
           p[i].y-2<=me.pos.y && me.pos.y <= p[i].y+2)
        {
          handle = i;
          break;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
      p[handle] = me.pos;
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
  TPen pen(this);

#if 0  
for(int i=0; i<=3; ++i) {
  for(TCoord x=0; x<320; x+=5) {
    pen.drawRectangle(x, 200-200*B(i, 3, x/320.0), 1, 1);
  }
}
#endif

#if 1
  TPoint q[4] = { { 20, 20 }, { 100, 20 }, { 100, 100}, { 20, 100 } };
  pen.setColor(1,0.5,0);
  pen.fillPolygon(q, 4);
  pen.setColor(0,1,0);
  pen.fillBezier(q, 4);

  pen.setColor(0,0,1);
  pen.drawPolygon(p, 4);
  
  TPoint r[4];
  for(int k=0; k<4; ++k) {
    TPoint pt = transferPoint(q[k], p0, p);
    r[k].x = pt.x;
    r[k].y = pt.y;
  }
  pen.setColor(1,0.5,0);
  pen.fillPolygon(r, 4);
  pen.setColor(0,1,0);
  pen.fillBezier(r, 4);

pen.setColor(0,0,0);
pen.drawString(E.x, E.y, "E"); // top/bottom
pen.drawString(F.x, F.y, "F"); // left/right
pen.drawString(G.x, G.y, "G");
pen.drawString(H.x, H.y, "H");
pen.drawString(J.x, J.y, "J");
pen.drawString(K.x, K.y, "K");

#endif
  
#if 0
  TPoint q[4] = { { 20, 20 }, { 100, 20 }, { 100, 100}, { 20, 100 } };
  pen.setColor(1,0.5,0);
  pen.fillPolygon(q, 4);
  pen.setColor(0,1,0);
  pen.fillBezier(q, 4);

  pen.setColor(0,0,1);
  pen.drawPolygon(p, 4);
  
  TPoint r[4];
  for(int k=0; k<4; ++k) {
    TCoord s = (q[k].x - 10) / 100;
    TCoord t = (q[k].y - 10) / 100;
    
    int n = 1;
    int m = 1;
    r[k].x = r[k].y = 0;
    int l=0;
    for(int j=0; j<=n; ++j) {
      for(int i=0; i<=m; ++i) {
        TCoord b = B(i, m, s) * B(j, n, t);
        int l0[4] = { 0, 1, 3, 2 };
        r[k].x += b * p[l0[l]].x;
        r[k].y += b * p[l0[l]].y;
        ++l;
      }
    }
  }
  pen.setColor(1,0.5,0);
  pen.fillPolygon(r, 4);
  pen.setColor(0,1,0);
  pen.fillBezier(r, 4);
#endif
#if 0
  pen.drawRectangle(50,50,100,50);
  
  pen.translate(100,75);
  pen.rotate(2.0 * M_PI / 4);
  pen.translate(-100,-75);
  
  pen.drawRectangle(50,50,100,50);
  
  // via pen
  pen.setColor(1,0,0);
  pen.identity();
  pen.drawRectangle(50,50,100,50);

  pen.translate(50, 50);
pen.drawRectangle(-2,-2,5,5);
  pen.rotate(2.0 * M_PI / 16);
  pen.translate(-50, -50);

  pen.translate(100,75);
  pen.rotate(2.0 * M_PI / 4);
  pen.translate(-100,-75);

  pen.setColor(0,0,1);
  pen.drawRectangle(50,50,100,50);

  // via matrix
  TMatrix2D m;
  pen.identity();
  pen.translate(50, 50);
  pen.rotate(2.0 * M_PI / 16);
  pen.translate(-50, -50);

  TMatrix2D m1;
  m1.translate(100,75);
  m1.rotate(2.0 * M_PI / 4);
  m1.translate(-100,-75);

  TMatrix2D m2;
  m2 = m1;
  
  pen.multiply(&m2);

  pen.setColor(0,1,0);
  pen.drawRectangle(50,50,100,50);
#endif
}

} // unnamed namespace

void
test_grab()
{
  TMyWindow wnd(NULL, "my test");
  toad::mainLoop();
}
