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

#include <toad/geometry.hh>
#include <cmath>
#include <algorithm>

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

namespace toad {

static const TCoord tolerance = 10e-6;
static const TCoord epsilon = 1e-12;
static const TCoord machine_epsilon = 1.12e-16;

// scalar product
static inline TCoord 
dot(TPoint const &a, TPoint const &b) { return a.x * b.x + a.y * b.y; }

static inline TCoord
cross(TPoint const &a, TPoint const &b) { return dot(a, TPoint(-b.y, b.x)); }

static inline bool isZero(TCoord a) { return fabs(a) <= epsilon; }

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
  sort(pt.begin(), pt.end(), [](const TPoint &p, const TPoint &q) {
    return (p.x < q.x) || (p.x == q.x && p.y < q.y);
  });
  size_t u = 2;
  for(auto i=2; i<pt.size(); ++i) {
    while(u>1 && !rightTurn(pt[u-2], pt[u-1], pt[i]))
      --u;
    swap(pt[u], pt[i]);
    ++u;
  }
  sort(pt.begin()+u, pt.end(), [](const TPoint &p, const TPoint &q) {
    return (p.x > q.x) || (p.x == q.x && p.y > q.y);
  });
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
  // FIXME: bx==0 || by==0
  TCoord lb = bx*bx+by*by;
#if 0
  TCoord t = (bx * ax + by * ay ) / lb;
  if (t<0.0 || t>1.0)
    return OUT_OF_RANGE;
#endif
  return (by * ax - bx * ay) / sqrt(lb);
}

static inline TCoord
min3(TCoord a, TCoord b, TCoord c)
{
  return (b<c) ?
    (a<b ? a : b):
    (a<c ? a : c);
}

static inline TCoord
max3(TCoord a, TCoord b, TCoord c)
{
  return (b>c) ?
    (a>b ? a : b):
    (a>c ? a : c);
}

void
intersectLineLine(TIntersectionList &ilist, const TPoint *lineA, TPoint *lineB)
{
  TCoord ax = lineA[1].x - lineA[0].x;
  TCoord ay = lineA[1].y - lineA[0].y;
  TCoord bx = lineB[1].x - lineB[0].x;
  TCoord by = lineB[1].y - lineB[0].y;

  TCoord cross = ax*by - ay*bx;
  if (isZero(cross))
    return;
    
  TCoord 
    dx = lineA[0].x - lineB[0].x,
    dy = lineA[0].y - lineB[0].y,
    a = (bx * dy - by * dx) / cross,
    b = (ax * dy - ay * dx) / cross;
  if (a<0.0 || a>1.0 || b<0.0 || b>1.0)
    return;
  TPoint p(lineA[0].x + a * ax, lineA[0].y + a * ay);
  ilist.add(TVectorPath::LINE, lineA, a, p,
            TVectorPath::LINE, lineB, b, p);
}

/*
// Fakultät
long fak(long n) {
  long r=1;
  for(long i=2; i<=n; ++i)
    r*=i;
  return r;
}

// Binomial Koeffizient
TCoord C(long n, long i) { return (TCoord)fak(n) / (TCoord)( fak(i) * fak(n-i) ); }

// Bernsteinpolynom
TCoord B(long i, long n, TCoord t) { return C(n, i) * pow(t, i) * pow(1-t, n-i);
}
*/

/**
 * \param[in] p four points of a cubic bézier curve
 * \param[in] t ∈[0, 1] position on the bézier curve
 * \return point on the curve at t
 */
TPoint bez2point(const TPoint *p, TCoord t)
{
  TCoord u=1-t;
  TCoord u2=u*u;
  TCoord t2=t*t;
  return p[0]*u2*u+p[1]*t*u2*3+p[2]*t2*u*3+p[3]*t2*t;
}

TCoord bez2x(const TPoint *p, TCoord t)
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
 * \param[in] p curve to be clipped
 * \param[in] q curve whose fat line will clip p
 * \param[out] min minimum of where to clip p
 * \param[out] max maximum of where to clip p
 */
static void
clipToFatLine(const TPoint *p, const TPoint *q, TCoord *min, TCoord *max)
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

  // bézier clipping
  vector<TPoint> d;
  for(int i=0; i<4; ++i) {
    d.push_back(TPoint(i/3.0, a*p[i].x + b*p[i].y + c));
  }

  convexHull(&d);

  bool below0, below1, above0, above1; // above or below fatline
  below0 = d[0].y < dmin;
  above0 = d[0].y > dmax;
  if (!(below0 || above0)) {
    *min=std::min(*min, d[0].x);
    *max=std::max(*max, d[0].x);
  }

  for(size_t i=0; i<d.size(); ++i) {
    const TPoint &p0 = d[i], &p1 = d[(i+1)%d.size()];
    below1 = p1.y < dmin;
    above1 = p1.y > dmax;
    if (!(below1 || above1)) {
      *min=std::min(*min, p1.x);
      *max=std::max(*max, p1.x);
    }
    if (below0 != below1) {
      TCoord x = (dmin - p0.y) * ((p1.x - p0.x)/(p1.y - p0.y)) + p0.x;
      *min=std::min(*min, x);
      *max=std::max(*max, x);
      below0 = below1;
    }
    if (above0 != above1) {
      TCoord x = (dmax - p0.y) * ((p1.x - p0.x)/(p1.y - p0.y)) + p0.x;
      *min=std::min(*min, x);
      *max=std::max(*max, x);
      above0 = above1;
    }
  }
  return;
}


/**
 * \param[in] p P from [pMin, pMax]
 * \param[in] q Q from [qMin, qMax]
 * \param[in] P the complete curve P
 * \param[in] Q the complete curve Q
 * \param[in] pMin see p
 * \param[in] pMax see p
 * \param[in] qMin see q
 * \param[in] qMax see q
 * \param[in] oldPDiff the width of the previous fatline
 * \param[in] depth number of recursions
 * \param[out] ilist intersections found
 */
void
bezierClipping(const TPoint *p,
               const TPoint *q,
               const TPoint *P,
               const TPoint *Q,
               TCoord pMin, TCoord pMax,
               TCoord qMin, TCoord qMax,
               TCoord oldPDiff,
               bool reverse,
               unsigned depth,
               TIntersectionList &ilist)
{
  if (depth>=32) {
    cerr << "WARNING: bezierClipping went too deep" << endl;
    return;
  }

  TPoint pClipped[4];
  
  TCoord pMinNew, pMaxNew, pDiff;
  if (q[0].x == q[3].x && qMax - qMin <= tolerance && depth > 3) {
    pMaxNew = pMinNew = (pMax + pMin) / 2.0;
    pDiff = 0.0;
  } else {
    TCoord pMinClip, pMaxClip;
    clipToFatLine(p, q, &pMinClip, &pMaxClip);
    if (pMinClip > pMaxClip) {
      return;
    }

    divideBezier(p, pClipped, pMinClip, pMaxClip);
    p = pClipped;

    pDiff = pMaxClip - pMinClip;

    pMinNew = pMax * pMinClip + pMin * (1 - pMinClip);
    pMaxNew = pMax * pMaxClip + pMin * (1 - pMaxClip);
  }

  if (oldPDiff > 0.5 && pDiff > 0.5) {
    TPoint o[7];
    if (pMaxNew - pMinNew > qMax - qMin) {
      divideBezier(p, o, 0.5);
      TCoord pMiddle =  pMinNew + (pMaxNew - pMinNew) / 2;
      bezierClipping(q, o  , Q, P, qMin, qMax, pMinNew, pMiddle, pDiff, !reverse, depth+1, ilist);
      bezierClipping(q, o+3, Q, P, qMin, qMax, pMiddle, pMaxNew, pDiff, !reverse, depth+1, ilist);
    } else {
      divideBezier(q, o, 0.5);
      TCoord qMiddle =  qMin + (qMax - qMin) / 2;
      bezierClipping(o  , p, Q, P, qMin, qMiddle, pMinNew, pMaxNew, pDiff, !reverse, depth+1, ilist);
      bezierClipping(o+3, p, Q, P, qMiddle, qMax, pMinNew, pMaxNew, pDiff, !reverse, depth+1, ilist);
    }
    return;
  }
  
  if (max(qMax-qMin, pMaxNew-pMaxNew) < tolerance) {
    TCoord t1 = pMinNew + (pMaxNew - pMinNew) / 2;
    TCoord t2 = qMin + (qMax - qMin) / 2;
    if (reverse)    
      ilist.add(TVectorPath::CURVE, Q, t2, bez2point(Q, t2),
                TVectorPath::CURVE, P, t1, bez2point(P, t1));
    else
      ilist.add(TVectorPath::CURVE, P, t1, bez2point(P, t1),
                TVectorPath::CURVE, Q, t2, bez2point(Q, t2));
    return;
  }
  if (pDiff > 0)
    bezierClipping(q, p, Q, P, qMin, qMax, pMinNew, pMaxNew, pDiff, !reverse, depth+1, ilist);
}

void
intersectCurveCurve(TIntersectionList &ilist, TPoint *curve0, TPoint *curve1)
{
  bezierClipping(curve0, curve0, curve1, curve1, 0.0, 1.0, 0.0, 1.0, 0.0, false, 0, ilist);
}

/****************************************************************************
 *                                                                          *
 *                          CURVE-LINE-INTERSECTION                         *
 *                                                                          *
 ****************************************************************************/
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
  for(i=0, j=0; i<n; ++i) {
    if (j!=i)
      roots[j] = roots[i];
    if (roots[i]>=min && roots[i]<=max)
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
    r = t > 0 ? 1.3247179572 * std::max(r, sqrt(t)) : r;
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
    roots[count++] = x;
  }
  return count;
}


// Converts from the point coordinates (p1, c1, c2, p2) for one axis to
// the polynomial coefficients and solves the polynomial for val
static int
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

/**
 * \param vc
 *   four points of a bézier curve
 * \param vl
 *   two points of a line
 */
void
intersectCurveLine(TIntersectionList &ilist, const TPoint *vc, const TPoint *vl)
{
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

  TCoord roots[4];
  int count = solveCubic(rvc, 1, 0, roots, 0, 1);

  // NOTE: count could be -1 for infinite solutions, but that should only
  // happen with lines, in which case we should not be here.
  for (int i = 0; i < count; ++i) {
    TCoord tc = roots[i];
    TCoord x = bez2x(rvc, tc);
    if (x>=0 && x<= rlx2) {
      TCoord tl = x/rlx2;
      ilist.add(TVectorPath::CURVE, vc, tc, bez2point(vc, tc),
                TVectorPath::LINE,  vl, tl, TPoint(vl[0].x + tl * ldx, vl[0].y + tl * ldy)); // FIXME
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

TRectangle
bounds(const TPoint *v)
{
  TPoint min = v[0];
  TPoint max = v[0];
  _addBounds(v[0].x, v[1].x, v[2].x, v[3].x, 0, 0, min, max);
  _addBounds(v[0].y, v[1].y, v[2].y, v[3].y, 1, 0, min, max);
  return TRectangle(min.x, min.y, max.x - min.x, max.y - min.y);
}

// PathFitter from paper.js

/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 * This code is in the public domain.
 */

static inline
TPoint normalize(const TPoint &p, TCoord d)
{
  return TPoint(p.x/d, p.y/d); // FIXME: I guessed this
}

static inline
TPoint normalize(const TPoint &p)
{
  return normalize(p, sqrt(p.x*p.x+p.y*p.y)); // FIXME: I guessed this
}

static inline
TCoord distance(const TPoint &a, const TPoint &b)
{
  TPoint p = b-a;
  return sqrt(p.x*p.x+p.y*p.y);
}

static inline TPoint
evaluate(size_t degree, const TPoint *curve, TCoord t)
{
  assert(degree==3);
  return bez2point(curve, t);
}

static void
addCurve(const TPoint *curve)
{
}

static void
generateBezier(const TPoint *points, size_t first, size_t last, const TCoord *uPrime, const TPoint &tan1, const TPoint &tan2, TPoint *curve)
{
  TCoord epsilon = 1e-12;
  TPoint pt1 = points[first];
  TPoint pt2 = points[last];
  TCoord C[2][2] = {{0,0},{0,0}};
  TCoord X[2] = {0,0};
  
  for(size_t i = 0, l = last - first + 1; i < l; i++) {
    TCoord u = uPrime[i],
           t = 1 - u,
           b = 3 * u * t,
           b0 = t * t * t,
           b1 = b * t,
           b2 = b * u,
           b3 = u * u * u;
    TPoint a1 = normalize(tan1, b1),
           a2 = normalize(tan2, b2),
           tmp = points[first + i] - pt1*(b0+b1) - pt2*(b2 + b3);
    C[0][0] += dot(a1, a1);
    C[0][1] += dot(a1, a2);
    C[1][0] = C[0][1];
    C[1][1] += dot(a2, a2);
    X[0] += dot(a1, tmp);
    X[1] += dot(a2, tmp);
  }
  
  TCoord detC0C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1],
         alpha1, alpha2;
  if (fabs(detC0C1) > epsilon) {
    TCoord detC0X = C[0][0] * X[1] - C[1][0] * X[0],
           detXC1 = X[0] * C[1][1] - X[1] * C[0][1];
    alpha1 = detXC1 / detC0C1;
    alpha2 = detC0X / detC0C1;
  } else {
    TCoord c0 = C[0][0] + C[0][1],
           c1 = C[1][0] + C[1][1];
    if (fabs(c0) > epsilon) {
      alpha1 = alpha2 = X[0] / c0;
    } else
    if (fabs(c1) > epsilon) {
      alpha1 = alpha2 = X[1] / c1;
    } else {
      alpha1 = alpha2 = 0;
    }
    
    TCoord segLength = distance(pt2, pt1);
    epsilon *= segLength;
    if (alpha1 < epsilon || alpha2 < epsilon) {
      alpha1 = alpha2 = segLength / 3;
    }
    
    curve[0] = pt1;
    curve[1] = pt1 + normalize(tan1, alpha1);
    curve[2] = pt2 + normalize(tan2, alpha2);
    curve[3] = pt2;
  }
}

static TCoord
findRoot(const TPoint *points, const TPoint *curve, const TPoint &point, TCoord u)
{
  TPoint curve1[4];
  TPoint curve2[4];
  for (size_t i = 0; i <= 2; i++) {
    curve1[i] = (curve[i + 1] - curve[i])*3.0;
  }
  for (size_t i = 0; i <= 1; i++) {
    curve2[i] = (curve1[i + 1] - curve1[i])*2.0;
  }
  TPoint pt = evaluate(3, curve, u),
         pt1 = evaluate(2, curve1, u),
         pt2 = evaluate(1, curve2, u),
         diff = pt - point;
  TCoord df = dot(pt1, pt1) + dot(diff, pt2);
  if (fabs(df) < 0.000001)
    return u;
  return u - dot(diff, pt1)/df;
}

struct TMaxError
{
  size_t index;
  TCoord error;
};

static TMaxError
findMaxError(const TPoint *points, size_t first, size_t last, const TPoint *curve, const TCoord *u)
{
  TMaxError result;
  result.index = floor((last - first + 1) / 2);
  result.error = 0.0;
  for(size_t i = first + 1; i < last; ++i) {
    TPoint P = evaluate(3, curve, u[i - first]);
    TPoint v = P - points[i];
    TCoord dist = v.x * v.x + v.y * v.y;
    if (dist >= result.error) {
      result.error = dist;
      result.index = i;
    }
  }
  return result;
}

static void
reparameterize(const TPoint *points, size_t first, size_t last, TCoord *u, const TPoint *curve)
{
  for(size_t i=first; i<=last; ++i)
    u[i-first] = findRoot(points, curve, points[i], u[i-first]);
}

/**
 * assign parameter values in [0,1] to the points
 * u[0]=0.0 1st point
 * u[x]=0.5 point in the middle of the path (when available)
 * u[n]=1.0 last point
 */
static void
chordLenghtParametrize(const TPoint *points, size_t first, size_t last, TCoord *u)
{
  // u[i] := distance from first to first+i
  u[0] = 0;
  for(size_t i= first + 1; i <= last; ++i) {
    u[i-first] = u[i-first-1] + distance(points[i-1], points[i]);
  }
  // normalize
  size_t m = last-first;
  for(size_t i = 1; i <= m; i++) {
    u[i] /= u[m];
  }
}

/**
 * \param first first point's index
 * \param last  last point's index
 * \param tan1  normalized tangent vector at first point
 * \param tan2  normalized tangent vector at last point
 */
static void
fitCubic(const TPoint *points, TCoord error, size_t first, size_t last, const TPoint &tan1, const TPoint &tan2)
{
  // not a curve, just two neigbouring points
  if (last-first == 1) {
    TCoord dist = distance(points[first], points[last]) / 3;
    TPoint curve[4] = {
      points[first],
      points[first]+normalize(tan1, dist),
      points[last]+normalize(tan2, dist),
      points[last]
    };
    addCurve(curve);
    return;
  }
  
  TCoord uPrime[last-first+1];
  chordLenghtParametrize(points, first, last, uPrime);
  TCoord maxError = std::max(error, error*error);
  size_t split = 0;
  
  // 5 iterations
  for(int i=0; i<=4; ++i) {
    TPoint curve[4];
    generateBezier(points, first, last, uPrime, tan1, tan2, curve);
    TMaxError max = findMaxError(points, first, last, curve, uPrime);
    if (max.error < error) {
      addCurve(curve);
      return;
    }
    split = max.index;
    if (max.error >= maxError)
      break;
    reparameterize(points, first, last, uPrime, curve);
    maxError = max.error;
  }
  // we didn't find a good enough curve within five iteration
  // split and try again on both sides
  TPoint V1 = points[split - 1] - points[split],
         V2 = points[split] - points[split + 1],
         tanCenter = normalize((V1+V2)*0.5);
  fitCubic(points, error, first, split, tan1, tanCenter);
  fitCubic(points, error, split, last, -tanCenter, tan2);
}

void
fitPath(const TPoint *inPoints, size_t size, TCoord tolerance)
{
  vector<TPoint> points;

  // initialize
  const TPoint *prev = nullptr;
  for(size_t i=0; i<size; ++i) {
    if (!prev || *prev!=inPoints[i]) {
      points.push_back(inPoints[i]);
      prev = inPoints+i;
    }
  }
  
  // FIXME: handle closed path
  
  size_t n = points.size()-1; // n := index of last point in points
  if (n>1) {
    fitCubic(points.data(), tolerance, 0,n, normalize(points[1]-points[0]), normalize(points[n-1] - points[n]));
  }
  
  // FIXME: handle closed path
  
  // FIXME: return result
}

} // namespace
