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

TPoint pE, pF, pG, pH, pJ, pK;

// two vanishing point perspective
TPoint transferPoint (TCoord xI, TCoord yI, TPoint *source, TPoint *destination)
{
    TCoord ADDING = 0.001; // to avoid dividing by zero

    TCoord xA = source[0].x;
    TCoord yA = source[0].y;

    TCoord xC = source[2].x;
    TCoord yC = source[2].y;

    TCoord xAu = destination[0].x;
    TCoord yAu = destination[0].y;

    TCoord xBu = destination[1].x;
    TCoord yBu = destination[1].y;

    TCoord xCu = destination[2].x;
    TCoord yCu = destination[2].y;

    TCoord xDu = destination[3].x;
    TCoord yDu = destination[3].y;

    // if points are the same, have to add a ADDING to avoid dividing by zero
    if (xBu==xCu) xCu+=ADDING;
    if (xAu==xDu) xDu+=ADDING;
    if (xAu==xBu) xBu+=ADDING;
    if (xDu==xCu) xCu+=ADDING;
    TCoord kBC = (yBu-yCu)/(xBu-xCu);
    TCoord kAD = (yAu-yDu)/(xAu-xDu);
    TCoord kAB = (yAu-yBu)/(xAu-xBu);
    TCoord kDC = (yDu-yCu)/(xDu-xCu);

    if (kBC==kAD) kAD+=ADDING;
    TCoord xE = (kBC*xBu - kAD*xAu + yAu - yBu) / (kBC-kAD);
    TCoord yE = kBC*(xE - xBu) + yBu;
pE.set(xE, yE);

    if (kAB==kDC) kDC+=ADDING;
    TCoord xF = (kAB*xBu - kDC*xCu + yCu - yBu) / (kAB-kDC);
    TCoord yF = kAB*(xF - xBu) + yBu;
pF.set(xF, yF);

    if (xE==xF) xF+=ADDING;
    TCoord kEF = (yE-yF) / (xE-xF);

    if (kEF==kAB) kAB+=ADDING;
    TCoord xG = (kEF*xDu - kAB*xAu + yAu - yDu) / (kEF-kAB);
    TCoord yG = kEF*(xG - xDu) + yDu;
pG.set(xG, yG);

    if (kEF==kBC) kBC+=ADDING;
    TCoord xH = (kEF*xDu - kBC*xBu + yBu - yDu) / (kEF-kBC);
    TCoord yH = kEF*(xH - xDu) + yDu;
pH.set(xH, yH);

    TCoord rG = (yC-yI)/(yC-yA);
    TCoord rH = (xI-xA)/(xC-xA);

    TCoord xJ = (xG-xDu)*rG + xDu;
    TCoord yJ = (yG-yDu)*rG + yDu;
pJ.set(xJ, yJ);

    TCoord xK = (xH-xDu)*rH + xDu;
    TCoord yK = (yH-yDu)*rH + yDu;
pK.set(xK, yK);


    if (xF==xJ) xJ+=ADDING;
    if (xE==xK) xK+=ADDING;
    TCoord kJF = (yF-yJ) / (xF-xJ); //23
    TCoord kKE = (yE-yK) / (xE-xK); //12

    TCoord xKE;
    if (kJF==kKE) kKE+=ADDING;
    TCoord xIu = (kJF*xF - kKE*xE + yE - yF) / (kJF-kKE);
    TCoord yIu = kJF * (xIu - xJ) + yJ;

    return TPoint(xIu, yIu); 
//    b.x=Math.round(b.x);
//    b.y=Math.round(b.y);
//    return b;
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
        if(p[i].x-2<=me.x && me.x <= p[i].x+2 &&
           p[i].y-2<=me.y && me.y <= p[i].y+2)
        {
          handle = i;
          break;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
      p[handle].set(me.x, me.y);
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
    TPoint pt = transferPoint(q[k].x, q[k].y, p0, p);
    r[k].x = pt.x;
    r[k].y = pt.y;
  }
  pen.setColor(1,0.5,0);
  pen.fillPolygon(r, 4);
  pen.setColor(0,1,0);
  pen.fillBezier(r, 4);

pen.setColor(0,0,0);
pen.drawString(pE.x, pE.y, "E"); // top/bottom
pen.drawString(pF.x, pF.y, "F"); // left/right
pen.drawString(pG.x, pG.y, "G");
pen.drawString(pH.x, pH.y, "H");
pen.drawString(pJ.x, pJ.y, "J");
pen.drawString(pK.x, pK.y, "K");

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
