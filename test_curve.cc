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
 * Paper.js - The Swiss Army Knife of Vector Graphics Scripting.
 * http://paperjs.org/
 *
 * Copyright (c) 2011 - 2014, Juerg Lehni & Jonathan Puckey
 * http://scratchdisk.com/ & http://jonathanpuckey.com/
 *
 * All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */   


/*
  playing around with paths
  o cairo/cocoa compatible path representation
  o bézier curve intersection
  o subdivide
  o visible bounding box
  o ...
  o (convex hull)
*/

#include <vector>

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/bitmap.hh>
#include <toad/connect.hh>

#include <cstdarg>
#include <algorithm>

using namespace toad;

#define MODERN 1

namespace {

class TMyWindow:
  public TWindow
{
    vector<TPoint> pt;
    vector<TPoint> curve;
    int handle;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      handle=-1;
      setSize(800, 600);
#ifdef MODERN
      for(int i=0; i<20; ++i)
        pt.push_back(TPoint(arc4random()%700+50, arc4random()%500+50));
#else
      for(int i=0; i<20; ++i)
        pt.push_back(TPoint(rand()%700+50, rand()%500+50));
#endif
/*
      for(int i=0; i<4; ++i)
        curve.push_back(TPoint(arc4random()%700+50, arc4random()%500+50));
*/
#if 0
      curve.push_back(TPoint(200,400));
      curve.push_back(TPoint(250,200));
      curve.push_back(TPoint(500,300));
      curve.push_back(TPoint(600,400));

      curve.push_back(TPoint(400,100));
      curve.push_back(TPoint(350,200));
      curve.push_back(TPoint(450,350));
      curve.push_back(TPoint(300,500));
#endif
#if 1
      // too deep
//      curve = {{200,400},{250,200},{500,300},{600,400},{274,284},{342,43},{450,350},{300,500}};
      // 9 intersections
      curve = {{116,478},{398,52},{121,565},{296,266},{332,424},{5,167},{483,512},{113,207}};
#endif
    }
    
    void paint() override;
    void mouseEvent(const TMouseEvent&) override;
};

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<8; ++i) {
#ifdef MODERN
        if(curve[i].x-4<=me.pos.x && me.pos.x <= curve[i].x+4 &&
           curve[i].y-4<=me.pos.y && me.pos.y <= curve[i].y+4)
        {
          handle = i;
          break;
        }
      }
#else
        if(curve[i].x-4<=me.x && me.x <= curve[i].x+4 &&
           curve[i].y-4<=me.y && me.y <= curve[i].y+4)
        {
          handle = i;
          break;
        }
      }
#endif
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
#ifdef MODERN
      curve[handle] = me.pos;
#else
      curve[handle].x = me.x;
      curve[handle].y = me.y;
#endif
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

const char*
format(const char *fmt, ...)
{
  static char buffer[8192];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);
  return buffer;
}

// scalar product
static inline TCoord 
dot(TPoint const &a, TPoint const &b) { return a.x * b.x + a.y * b.y; }

static inline TCoord
cross(TPoint const &a, TPoint const &b) { return dot(a, TPoint(-b.y, b.x)); }

static inline bool
rightTurn(TPoint const &p0, TPoint const &p1, TPoint const &p2)
{
  if (p1==p2)
    return false;
  TPoint q1 = p1 - p0;
  TPoint q2 = p2 - p0;
  if (q1==-q2)
    return false;
  return (cross(q1, q2) < 0);
} 

// calculate the convex hull with Andrew's algorithm
// http://en.wikipedia.org/wiki/Convex_hull_algorithms
static void
convexHull(vector<TPoint> *pp)
{
  vector<TPoint> &pt = *pp;
#ifdef MODERN
  sort(pt.begin(), pt.end(), [](const TPoint &p, const TPoint &q) {
    return (p.x < q.x) || (p.x == q.x && p.y < q.y);
  });
#else
  struct { bool operator() (const TPoint &p, const TPoint &q) {
    return (p.x < q.x) || (p.x == q.x && p.y < q.y);
  } } cmp1;
  sort(pt.begin(), pt.end(), cmp1);
#endif
  size_t u = 2;
  for(auto i=2; i<pt.size(); ++i) {
    while(u>1 && !rightTurn(pt[u-2], pt[u-1], pt[i]))
      --u;
    swap(pt[u], pt[i]);
    ++u;
  }
#ifdef MODERN
  sort(pt.begin()+u, pt.end(), [](const TPoint &p, const TPoint &q) {
    return (p.x > q.x) || (p.x == q.x && p.y > q.y);
  });
#else
  struct { bool operator() (const TPoint &p, const TPoint &q) {
    return (p.x > q.x) || (p.x == q.x && p.y > q.y);
  } } cmp2;
  sort(pt.begin()+u, pt.end(), cmp2);
#endif
  rotate(pt.begin(), pt.begin()+1, pt.end());
  size_t l = u;
  size_t k = u - 1;
  for (size_t i = l; i < pt.size(); ++i) {
    while (l > k && !rightTurn(pt[l-2], pt[l-1], pt[i]))
      --l;
    swap(pt[l], pt[i]);
    ++l;
  }
  pt.resize(l);
}

// distance of p to the line through p0 & p1
TCoord 
distance2Line(const TPoint &p, const TPoint &p0, const TPoint &p1)
{
  TCoord bx = p1.x - p0.x;
  TCoord by = p1.y - p0.y;
  TCoord ax = p.x - p0.x;
  TCoord ay = p.y - p0.y;
  if (bx==0.0 && by==0.0) {
    return sqrt(ax*ax+ay*ay);
  }
  TCoord lb = bx*bx+by*by;
#if 0
  TCoord t = (bx * ax + by * ay ) / lb;
  if (t<0.0 || t>1.0)
    return OUT_OF_RANGE;
#endif
  return (by * ax - bx * ay) / sqrt(lb);
}

inline TCoord
min3(TCoord a, TCoord b, TCoord c)
{
  return (b<c) ?
    (a<b ? a : b):
    (a<c ? a : c);
}

inline TCoord
max3(TCoord a, TCoord b, TCoord c)
{
  return (b>c) ?
    (a>b ? a : b):
    (a>c ? a : c);
}

TCoord
lineIntersection(const TPoint &a0,
                 const TPoint &a1,
                 const TPoint &b0,
                 const TPoint &b1)
{
  TCoord ax = a1.x - a0.x;
  TCoord ay = a1.y - a0.y;
  TCoord bx = b1.x - b0.x;
  TCoord by = b1.y - b0.y;

  if (bx>ax) {
    TCoord a = ((a0.x-b0.x)/bx*by - a0.y + b0.y)/(ay-ax/bx*by);
    TCoord b = (a0.x - b0.x + a * ax) / bx;
    return b;
  }
  TCoord b = ((b0.x-a0.x)/ax*ay - b0.y + a0.y)/(by-bx/ax*ay);
  return b;
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

TPoint bez2point(TPoint *p, TCoord t)
{
  TCoord u=1-t;
  TCoord u2=u*u;
  TCoord t2=t*t;
  return p[0]*u2*u+p[1]*t*u2*3+p[2]*t2*u*3+p[3]*t2*t;
}

TCoord bez2x(TPoint *p, TCoord t)
{
  TCoord u=1-t;
  TCoord u2=u*u;
  TCoord t2=t*t;
  return p[0].x*u2*u+p[1].x*t*u2*3+p[2].x*t2*u*3+p[3].x*t2*t;
}

/**
 * Split a Bézier Curve at u.
 * \param[in]  a 4 points of a cubic bézier curve
 * \param[out] p 7 points of two cubic bézier curves
 * \param[in]  u a value between 0 and 1
 */
void
divideBezier(const TPoint *a, TPoint *p, TCoord u)
{
  if (u==0.5) {
    p[0] = a[0];
    p[1].x = (a[0].x+a[1].x)*0.5;
    p[1].y = (a[0].y+a[1].y)*0.5;
    p[5].x = (a[2].x+a[3].x)*0.5;
    p[5].y = (a[2].y+a[3].y)*0.5;
    p[6] = a[3];
  
    TCoord cx = (a[1].x+a[2].x)*0.5;
    TCoord cy = (a[1].y+a[2].y)*0.5;
    p[2].x = (p[1].x+cx)*0.5;
    p[2].y = (p[1].y+cy)*0.5;
    p[4].x = (cx+p[5].x)*0.5;
    p[4].y = (cy+p[5].y)*0.5;

    p[3].x = (p[2].x+p[4].x)*0.5;
    p[3].y = (p[2].y+p[4].y)*0.5;
    return;
  }
  p[0] = a[0];
  p[1].x = a[0].x + (a[1].x-a[0].x)*u;
  p[1].y = a[0].y + (a[1].y-a[0].y)*u;
  p[5].x = a[2].x + (a[3].x-a[2].x)*u;
  p[5].y = a[2].y + (a[3].y-a[2].y)*u;
  p[6] = a[3];
  
  TCoord cx = a[1].x + (a[2].x-a[1].x)*u;
  TCoord cy = a[1].y + (a[2].y-a[1].y)*u;
  p[2].x = p[1].x + (cx-p[1].x)*u;
  p[2].y = p[1].y + (cy-p[1].y)*u;
  p[4].x = cx + (p[5].x-cx)*u;
  p[4].y = cy + (p[5].y-cy)*u;

  p[3].x = p[2].x+(p[4].x-p[2].x)*u;
  p[3].y = p[2].y+(p[4].y-p[2].y)*u;
}

void
divideBezier(const TPoint *a, TPoint *p, TCoord min, TCoord max)
{
  TPoint o0[7], o1[7];
  divideBezier(a, o0, min);
  divideBezier(o0+3, o1, (max-min)/(1-min));
  for(int i=0; i<4; ++i)
    p[i] = o1[i];
}

/****************************************************************************
 *                                                                          *
 *                         CURVE-CURVE-INTERSECTION                         *
 *                                                                          *
 ****************************************************************************/

// Curve intersection using Bézier clipping by T.W.Sederberg and T.Nishita
// written with looking at the code of paperjs and inkscape

/**
 * \param p line to be clipped
 * \param q to be used as fat line
 */
static void
clipToFatLine(const TPoint *p, const TPoint *q, TPen &pen, unsigned depth, TCoord *min, TCoord *max)
{
  *min=1.0;
  *max=0.0;

  // define dmin & dmax for the fat line
  TCoord dmin, dmax;

  TCoord a, b, c;
  TCoord dx = q[3].x - q[0].x;
  TCoord dy = q[3].y - q[0].y;
  
  TCoord n = sqrt(dx*dx+dy*dy);
  dx /= n;
  dy /= n;
  a = dy;
  b = -dx;
  c = q[0].y*dx - q[0].x*dy;

  if (q[1]==q[2]) {
    // quadratic case
    TCoord d =  a*q[1].x + b*q[1].y + c;
    if (d<0.0) {
      dmin = d;
      dmax = 0.0;
    } else {
      dmin = 0.0;
      dmax = d;
    }
  } else {
    // cubic case
    TCoord d1 = a*q[1].x + b*q[1].y + c;
    TCoord d2 = a*q[2].x + b*q[2].y + c;

    if (d1<0.0 && d2>0.0) {
      dmin = d1 * 4.0 / 9.0;
      dmax = d2 * 4.0 / 9.0;
    } else
    if (d1>0.0 && d2<0.0) {
      dmin = d2 * 4.0 / 9.0;
      dmax = d1 * 4.0 / 9.0;
    } else {
      dmin = min3(0.0, d1, d2) * 3.0 / 4.0;
      dmax = max3(0.0, d1, d2) * 3.0 / 4.0;
    }
  }

#if 0
  pen.setColor(1,0,0);
  pen.drawLine(q[0], q[3]);
  
  TPoint r0(q[0].x+dy*dmin, q[0].y-dx*dmin);
  TPoint r1(q[3].x+dy*dmin, q[3].y-dx*dmin);
  pen.drawLine(r0, r1);
  TPoint r2(q[0].x+dy*dmax, q[0].y-dx*dmax);
  TPoint r3(q[3].x+dy*dmax, q[3].y-dx*dmax);
  pen.drawLine(r2, r3);
  
  pen.drawLine(r0, r2);
  pen.drawLine(r1, r3);
#endif
#if 0  
  pen.setColor(0,1,0);
  pen.drawLine(0,300-dmin,120,300-dmin);
  pen.drawLine(0,300,120,300);
  pen.drawLine(0,300-dmax,120,300-dmax);
#endif  
  // bézier clipping
  vector<TPoint> d;
  for(int i=0; i<4; ++i) {
    d.push_back(TPoint(i/3.0, a*p[i].x + b*p[i].y + c));
  }

#if 0  
  vector<TPoint> D;
  for(int i=0; i<4; ++i)
    D.push_back(TPoint(d[i].x*3*40, 300-d[i].y));
  pen.drawLines(D.data(), D.size());
  pen.drawBezier(D.data(), D.size());
#endif
  
  convexHull(&d);

//cout << "--------------" << endl;
//cout << "dmax=" << dmax << ", dmin=" << dmin << endl;
//for(size_t i=0; i<4; ++i)
//  cout << i << ": " << d[i].y << endl;

  bool below0, below1, above0, above1; // above or below fatline
  below0 = d[0].y < dmin;
  above0 = d[0].y > dmax;
//cout << "0 is "; if (below0) cout << "below "; if (above0) cout << "above"; cout << endl;
  if (!(below0 || above0)) {
    *min=std::min(*min, d[0].x);
    *max=std::max(*max, d[0].x);
//cout << "0 is inside" << endl;
  }

  for(size_t i=0; i<d.size(); ++i) {
    const TPoint &p0 = d[i], &p1 = d[(i+1)%d.size()];
    below1 = p1.y < dmin;
    above1 = p1.y > dmax;
//cout << (i+1) << " is "; if (below1) cout << "below "; if (above1) cout << "above"; cout << endl;
    if (!(below1 || above1)) {
      *min=std::min(*min, p1.x);
      *max=std::max(*max, p1.x);
//cout << (i+1) << " is inside" << endl;
    }
    if (below0 != below1) {
//cout << (i+1) << " below switch" << endl;
      TCoord x = (dmin - p0.y) * ((p1.x - p0.x)/(p1.y - p0.y)) + p0.x;
      *min=std::min(*min, x);
      *max=std::max(*max, x);
      below0 = below1;
    }
    if (above0 != above1) {
//cout << (i+1) << " above switch" << endl;
      TCoord x = (dmax - p0.y) * ((p1.x - p0.x)/(p1.y - p0.y)) + p0.x;
      *min=std::min(*min, x);
      *max=std::max(*max, x);
      above0 = above1;
    }
  }
  return;
}

unsigned deeptimer;

static const TCoord tolerance = 10e-6;
static const TCoord epsilon = 1e-12;
static const TCoord machine_epsilon = 1.12e-16;

void
bezierClipping(TPen &pen,
               const TPoint *p,
               const TPoint *q,
               const TPoint *P,
               const TPoint *Q,
               TCoord pMin=0.0, TCoord pMax=1.0,
               TCoord qMin=0.0, TCoord qMax=1.0,
               TCoord oldPDiff=0.0,
               unsigned depth=0)
{
  if (depth>=32) {
    ++deeptimer;
    return;
  }

  TPoint pClipped[4];
  
  TCoord pMinNew, pMaxNew, pDiff;
  if (q[0].x == q[3].x && qMax - qMin <= tolerance && depth > 3) {
    pMaxNew = pMinNew = (pMax + pMin) / 2.0;
    pDiff = 0.0;
//cout << "depth " << depth << ": nothing to clip" << endl;
  } else {
    TCoord pMinClip, pMaxClip;
    clipToFatLine(p, q, pen, depth, &pMinClip, &pMaxClip);
    if (pMinClip > pMaxClip) {
//      cout << "no fat line overlap" << endl;
      return;
    }

    divideBezier(p, pClipped, pMinClip, pMaxClip);
    p = pClipped;

    pDiff = pMaxClip - pMinClip;

    pMinNew = pMax * pMinClip + pMin * (1 - pMinClip);
    pMaxNew = pMax * pMaxClip + pMin * (1 - pMaxClip);
//cout << "depth " << depth << ": clipped to " << pMinNew << "-" << pMaxNew << " (diff=" << pDiff << ")" << endl;
  }

  if (oldPDiff > 0.5 && pDiff > 0.5) {
    TPoint o[7];
    if (pMaxNew - pMinNew > qMax - qMin) {
//cout << "  subdivide p" << endl;
      divideBezier(p, o, 0.5);
      TCoord pMiddle =  pMinNew + (pMaxNew - pMinNew) / 2;
      bezierClipping(pen, q, o  , Q, P, qMin, qMax, pMinNew, pMiddle, pDiff, depth+1);
      bezierClipping(pen, q, o+3, Q, P, qMin, qMax, pMiddle, pMaxNew, pDiff, depth+1);
    } else {
//cout << "  subdivide q" << endl;
      divideBezier(q, o, 0.5);
      TCoord qMiddle =  qMin + (qMax - qMin) / 2;
      bezierClipping(pen, o  , p, Q, P, qMin, qMiddle, pMinNew, pMaxNew, pDiff, depth+1);
      bezierClipping(pen, o+3, p, Q, P, qMiddle, qMax, pMinNew, pMaxNew, pDiff, depth+1);
    }
    return;
  }
  
  if (max(qMax-qMin, pMaxNew-pMaxNew) < tolerance) {
    TCoord t1 = pMinNew + (pMaxNew - pMinNew) / 2;
    TCoord t2 = qMin + (qMax - qMin) / 2;

    TPoint pt = P[0] * B(0, 3, t1);
    for(int j=1; j<4; ++j)
      pt += P[j] * B(j, 3, t1);
    pen.setColor(1,0,0);
    pen.drawCircle(pt.x-2, pt.y-2, 3,3);
//    cout << "got one at " << t1 << endl;
    return;
  }
  if (pDiff > 0)
    bezierClipping(pen, q, p, Q, P, qMin, qMax, pMinNew, pMaxNew, pDiff, depth+1);
}

/****************************************************************************
 *                                                                          *
 *                          CURVE-LINE-INTERSECTION                         *
 *                                                                          *
 ****************************************************************************/
static inline bool
isZero(TCoord a) {
  return fabs(a) <= epsilon;
}

/**
 * Solves the quadratic polynomial with coefficients a, b, c for roots
 * (zero crossings) and and returns the solutions in an array.
 *
 * a*x^2 + b*x + c = 0
 */
static int
solveQuadratic(TCoord a, TCoord b, TCoord c, TCoord *roots) {
  // After Numerical Recipes in C, 2nd edition, Press et al.,
  // 5.6, Quadratic and Cubic Equations
  // If problem is actually linear, return 0 or 1 easy roots
  if (fabs(a) < tolerance) {
    if (fabs(b) >= tolerance) {
      roots[0] = -c / b;
      return 1;
    }
    // If all the coefficients are 0, infinite values are possible!
    if (fabs(c) < tolerance)
      return -1; // Infinite solutions
    return 0; // 0 solutions
  }
  TCoord q = b * b - 4 * a * c;
  if (q < 0)
    return 0; // 0 solutions
  q = sqrt(q);
  if (b < 0)
    q = -q;
  q = (b + q) * -0.5;
  int n = 0;
  if (fabs(q) >= tolerance)
    roots[n++] = c / q;
  if (fabs(a) >= tolerance)
    roots[n++] = q / a;
  return n; // 0, 1 or 2 solutions
}

static int 
solveQuadratic(TCoord a, TCoord b, TCoord c, TCoord *roots, TCoord min, TCoord max)
{
  int i, j, n = solveQuadratic(a, b, c, roots);
cout << "n=" << n << endl;
  for(i=0, j=0; i<n; ++i) {
    cout << i << ":" << roots[i] << endl;
    if (j!=i)
      roots[j] = roots[i];
    if (roots[i]>=min && roots[i]<=max)
      ++j;
  }
  return j;
}

static int
solveCubic(TCoord a, TCoord b, TCoord c, TCoord d, TCoord *roots, TCoord min, TCoord max)
{
  TCoord x, b1, c2;
  int count = 0;
  if (a == 0) {
    a = b;
    b1 = c;
    c2 = d;
    x = INFINITY;
  } else if (d == 0) {
    b1 = b;
    c2 = c;
    x = 0;
  } else {
    TCoord ec = 1 + machine_epsilon,
            x0, q, qd, t, r, s, tmp;
    x = -(b / a) / 3;
    tmp = a * x,
    b1 = tmp + b,
    c2 = b1 * x + c,
    qd = (tmp + b1) * x + c2,
    q = c2 * x + d;
    t = q /a;
    r = pow(fabs(t), 1/3);
    s = t < 0 ? -1 : 1;
    t = -qd / a;
    r = t > 0 ? 1.3247179572 * ::max(r, sqrt(t)) : r;
    x0 = x - s * r;
    if (x0 != x) {
      do {
        x = x0;
        tmp = a * x,
        b1 = tmp + b,
        c2 = b1 * x + c,
        qd = (tmp + b1) * x + c2,
        q = c2 * x + d;
        x0 = qd == 0 ? x : x - q / qd / ec;
        if (x0 == x) {
          x = x0;
          break;
        }
      } while (s * x0 > s * x);
      if (fabs(a) * x * x > fabs(d / x)) {
        c2 = -d / x;
        b1 = (c2 - c) / x;
      }
    }
  }
  count = solveQuadratic(a, b1, c2, roots, min, max);
  if (x!=INFINITY && (count == 0 || x != roots[count - 1])
                  && (x >= min && x <= max))
  {
      cout << "x:" << x << endl;
      roots[count++] = x;
  }
  return count;
}


// Converts from the point coordinates (p1, c1, c2, p2) for one axis to
// the polynomial coefficients and solves the polynomial for val
int
solveCubic(TPoint *v, int coord, TCoord val, TCoord *roots, TCoord min, TCoord max) {
  TCoord p1, c1, c2, p2, a, b, c;
  if (!coord) {
    p1 = v[0].x;
    c1 = v[1].x;
    c2 = v[2].x;
    p2 = v[3].x;
  } else {
    p1 = v[0].y;
    c1 = v[1].y;
    c2 = v[2].y;
    p2 = v[3].y;
  }
  c = 3 * (c1 - p1),
  b = 3 * (c2 - c1) - c,
  a = p2 - p1 - c - b;
  // If both a and b are near zero, we should treat the curve as a line in
  // order to find the right solutions in some edge-cases in
  // Curve.getParameterOf()
  if (isZero(a) && isZero(b))
    a = b = 0;
  return solveCubic(a, b, c, p1 - val, roots, min, max);
}

void // curve, line
intersectCurveLine(TPen &pen, TPoint *vc, TPoint *vl) {
  TCoord lx1 = vl[0].x, ly1 = vl[0].y,
         lx2 = vl[1].x, ly2 = vl[1].y,
         // Rotate both curve and line around l1 so that line is on x axis.
         ldx = lx2 - lx1,
         ldy = ly2 - ly1,
         // Calculate angle to the x-axis (1, 0).
         angle = atan2(-ldy, ldx),
         sin = ::sin(angle),
         cos = ::cos(angle),
         // (rlx1, rly1) = (0, 0)
         rlx2 = ldx * cos - ldy * sin;
  // rotated line: The curve values for the rotated line.
  TPoint rvl[2] = {{0, 0}, {rlx2, 0}};
  // rotated curve: Calculate the curve values of the rotated curve.
  TPoint rvc[4];
  for(int i = 0; i < 4; ++i) {
      TCoord x = vc[i].x - lx1,
             y = vc[i].y - ly1;
      rvc[i].x = x * cos - y * sin;
      rvc[i].y = y * cos + x * sin;
  }
  
  pen.push();
  pen.translate(0,300);
  pen.setColor(1,0,0);
  pen.drawBezier(rvc, 4);
  pen.drawLines(rvl, 2);
  pen.pop();
  
  TCoord roots[4];
  int count = solveCubic(rvc, 1, 0, roots, 0, 1);

cout << "curve & line have " << count << " intersections" << endl;

  // NOTE: count could be -1 for infinite solutions, but that should only
  // happen with lines, in which case we should not be here.
  for (int i = 0; i < count; ++i) {
    TCoord tc = roots[i];
    TCoord x = bez2x(rvc, tc);
    if (x>=0 && x<= rlx2) {
      TPoint pt = bez2point(vc, tc);
      pen.setColor(1,0,0);
      pen.drawCircle(pt.x-2, pt.y-2, 3,3);
    }
  }
}

/****************************************************************************
 *                                                                          *
 *                                  BOUNDS                                  *
 *                                                                          *
 ****************************************************************************/



// Code ported and further optimised from:
// http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
static void
add(TCoord value, TCoord padding, int coord, TPoint &min, TPoint &max) {
  TCoord left = value - padding, 
         right = value + padding;
  if (coord==0) {
    if (left < min.x) 
      min.x = left; 
    if (right > max.x)
      max.x = right; 
  } else {
    if (left < min.y) 
      min.y = left; 
    if (right > max.y)
      max.y = right; 
  }
}

static void
_addBounds(TCoord v0, TCoord v1, TCoord v2, TCoord v3, int coord, TCoord padding, TPoint &min, TPoint &max)
{
  // Calculate derivative of our bezier polynomial, divided by 3.
  // Doing so allows for simpler calculations of a, b, c and leads to the
  // same quadratic roots.
  TCoord a = 3 * (v1 - v2) - v0 + v3,
         b = 2 * (v0 + v2) - 4 * v1,
         c = v1 - v0;
  TCoord roots[2];
  int count = solveQuadratic(a, b, c, roots);
  // Add some tolerance for good roots, as t = 0, 1 are added
  // separately anyhow, and we don't want joins to be added with radii
  // in getStrokeBounds()
  TCoord tMin = tolerance,
         tMax = 1 - tMin;
  // Only add strokeWidth to bounds for points which lie  within 0 < t < 1
  // The corner cases for cap and join are handled in getStrokeBounds()
  add(v3, 0, coord, min, max);
  for (int i = 0; i < count; i++) {
     TCoord t = roots[i];
     // Test for good roots and only add to bounds if good.
     if (tMin < t && t < tMax) {
       // Calculate bezier polynomial at t.
       TCoord t2 = t*t,
              u  = 1-t,
              u2 = u*u;
       add(u2 * u * v0
           + 3 * u2 * t * v1
           + 3 * u * t2 * v2
           + t2 * t * v3,
           padding, coord, min, max);
     }
  }
}

TRectangle bounds(const TPoint *v)
{
  TPoint min = v[0];
  TPoint max = v[0];
  _addBounds(v[0].x, v[1].x, v[2].x, v[3].x, 0, 0, min, max);
  _addBounds(v[0].y, v[1].y, v[2].y, v[3].y, 1, 0, min, max);
  return TRectangle(min.x, min.y, max.x - min.x, max.y - min.y);
}

void
TMyWindow::paint()
{
cout << "paint --------------------- " << endl;
  TPen pen(this);
  pen.translate(0.5, 0.5);

#if 0
  vector<TPoint> pt(this->pt);
  convexHull(&pt);

  // paint
  int i=1;
  for(auto p: this->pt) {
    pen.drawCircle(p.x-3, p.y-3, 7, 7);
    pen.drawString(p.x+3, p.y+4, format("%i", i));
    ++i;
  }
  pen.setColor(1,0,0);
  for(auto p: pt) {
    pen.drawCircle(p.x-2, p.y-2, 5, 5);
    ++i;
  }
#endif

  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(curve.data(), 4);
//  pen.drawLines(curve.data()+4, 4);
  for(auto p: curve)
    pen.drawRectangle(p.x-2, p.y-2, 5, 5);

  pen.setColor(0,0,0);
  pen.drawBezier(curve.data(), 4);
//  pen.drawBezier(curve.data()+4, 4);
  
  pen.setColor(0,1,0);
  pen.drawLines(curve.data()+4, 2);
  
  for(auto p: curve)
    cout << "{"<<p.x<<","<<p.y<<"},";
  cout<<endl;
/*
  deeptimer=0;
  bezierClipping(pen, curve.data(), curve.data()+4, curve.data(), curve.data()+4);
  if (deeptimer)
    cout << deeptimer << " times too deep" << endl;
*/

  intersectCurveLine(pen, curve.data(), curve.data()+4);

  pen.setColor(1,0.5,0);
  TRectangle r = bounds(curve.data());
cout << "bounds: " << r << endl;

//r.w=100;
//r.h=100;
  pen.drawRectangle(r);
//  pen.drawRectangle(bounds(curve.data()+4));
}

} // unnamed namespace

void
test_curve()
{
  TMyWindow wnd(NULL, "test curve");
  toad::mainLoop();
}
