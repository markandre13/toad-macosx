/*
 * poly/solve_cubic.c
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007, 2009 Brian Gough
 *
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
void
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
intersectCurveCurve(TIntersectionList &ilist, const TPoint *curve0, const TPoint *curve1)
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
int
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

// The function gsl_poly_solve_cubic is copied from the GNU Scientific Library.
// It's modified to return only unique solutions.
// For the math and history, which dates back to Scipione del Ferro (1465-1526)
// see:
//   https://en.wikipedia.org/wiki/Cubic_function
//   http://mathworld.wolfram.com/CubicFormula.html
static int 
gsl_poly_solve_cubic(TCoord a, TCoord b, TCoord c, TCoord *roots)
{
  TCoord q = (a * a - 3 * b);
  TCoord r = (2 * a * a * a - 9 * a * b + 27 * c);

  TCoord Q = q / 9;
  TCoord R = r / 54;

  TCoord Q3 = Q * Q * Q;
  TCoord R2 = R * R;

  TCoord CR2 = 729 * r * r;
  TCoord CQ3 = 2916 * q * q * q;

  if (R == 0 && Q == 0) {
    roots[0] = - a / 3 ;
    return 1; // the original gsl function return 3 values here
  } else
  if (CR2 == CQ3) {
    /* this test is actually R2 == Q3, written in a form suitable
       for exact computation with integers */

    /* Due to finite precision some double roots may be missed, and
       considered to be a pair of complex roots z = x +/- epsilon i
       close to the real axis. */

    TCoord sqrtQ = sqrt(Q);

    if (R > 0) {
      roots[0] = -2 * sqrtQ  - a / 3;
      roots[1] = sqrtQ - a / 3;
    } else {
      roots[0] = - sqrtQ  - a / 3;
      roots[1] = 2 * sqrtQ - a / 3;
    }
    return 2; // the original gsl function returns 3 value here
  } else
  if (R2 < Q3) {
    double sgnR = (R >= 0 ? 1 : -1);
    double ratio = sgnR * sqrt (R2 / Q3);
    double theta = acos (ratio);
    double norm = -2 * sqrt (Q);
    roots[0] = norm * cos (theta / 3) - a / 3;
    roots[1] = norm * cos ((theta + 2.0 * M_PI) / 3) - a / 3;
    roots[2] = norm * cos ((theta - 2.0 * M_PI) / 3) - a / 3;
      
    /* Sort *x0, *x1, *x2 into increasing order */

    if (roots[0] > roots[1])
      swap(roots[0], roots[1]);
      
    if (roots[1] > roots[2]) {
      swap(roots[1], roots[2]);
      if (roots[0] > roots[1])
        swap(roots[0], roots[1]);
    }
    return 3;
  } else {
    double sgnR = (R >= 0 ? 1 : -1);
    double A = -sgnR * pow (fabs (R) + sqrt (R2 - Q3), 1.0/3.0);
    double B = Q / A ;
    roots[0] = A + B - a / 3;
    return 1;
  }
}

static int
solveCubic(TCoord a, TCoord b, TCoord c, TCoord d, TCoord *roots, TCoord min, TCoord max)
{
  int i, j, n = gsl_poly_solve_cubic(b/a, c/a, d/a, roots);
  for(i=0, j=0; i<n; ++i) {
    if (j!=i)
      roots[j] = roots[i];
    if (roots[i]>=min && roots[i]<=max)
      ++j;
  }
  return j;
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
  TCoord d = p1-val;
  
//  a=2;b=-4;c=-22;d=24; // this returns 1
  
  return solveCubic(a, b, c, d, roots, min, max);
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
 *                               CURVE BOUNDS                               *
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
  int count = solveQuadratic(a, b, c, roots, 0, 1);
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
curveBounds(const TPoint *v)
{
  TPoint min = v[0];
  TPoint max = v[0];
  _addBounds(v[0].x, v[1].x, v[2].x, v[3].x, 0, 0, min, max);
  _addBounds(v[0].y, v[1].y, v[2].y, v[3].y, 1, 0, min, max);
  return TRectangle(min.x, min.y, max.x - min.x, max.y - min.y);
}

/****************************************************************************
 *                                                                          *
 *                             POINTS TO BEZIER                             *
 *                                                                          *
 ****************************************************************************/

/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 * This code is in the public domain.
 */

// NOTE: should look for other algorithms too
//       ie. "least-squares, approximations to bézier curves and surfaces"
//       from Graphic Gems II, p.406

/* returns squared length of input vector */	
/*
 *  B0, B1, B2, B3 :
 *	Bezier multipliers
 */
static inline double
B0(double u)
{
  double tmp = 1.0 - u;
  return (tmp * tmp * tmp);
}

static inline double
B1(double u)
{
  double tmp = 1.0 - u;
  return (3 * u * (tmp * tmp));
}

static inline double
B2(double u)
{
  double tmp = 1.0 - u;
  return (3 * u * u * tmp);
}

static inline double
B3(double u)
{
  return (u * u * u);
}

/*
 *  GenerateBezier :
 *  Use least-squares method to find Bezier control points for region.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 *  \param uPrime Parameter values for region
 *  \param tHat1, tHat2 Unit tangents at endpoints
 *
 */
static void
generateBezier(const TPoint *d, int first, int last, const double *uPrime, TPoint tHat1, TPoint tHat2, TPoint *bezCurve)
{
  int 	i;
  int 	nPts;			/* Number of pts in sub-curve 	*/
  double 	C[2][2];		/* Matrix C			*/
  double 	X[2];			/* Matrix X			*/
  double 	det_C0_C1,		/* Determinants of matrices	*/
    	   	det_C0_X,
	   	det_X_C1;
  double 	alpha_l,		/* Alpha values, left and right	*/
    	   	alpha_r;
  TPoint 	tmp;			/* Utility variable		*/
  nPts = last - first + 1;
  CGPoint      A[nPts][2];		/* Precomputed rhs for eqn      */
 
  /* Compute the A's	*/
  for (i = 0; i < nPts; i++) {
    A[i][0] = tHat1 * B1(uPrime[i]);
    A[i][1] = tHat2 * B2(uPrime[i]);
  }

  /* Create the C and X matrices */
  C[0][0] = 0.0;
  C[0][1] = 0.0;
  C[1][0] = 0.0;
  C[1][1] = 0.0;
  X[0]    = 0.0;
  X[1]    = 0.0;

  for (i = 0; i < nPts; i++) {
    C[0][0] += dot(A[i][0], A[i][0]);
    C[0][1] += dot(A[i][0], A[i][1]);
    // C[1][0] += dot(A[i][0], A[i][1]);
    C[1][0] = C[0][1];
    C[1][1] += dot(A[i][1], A[i][1]);
    tmp = d[first + i]
          - d[first]* B0(uPrime[i])
          - d[first]* B1(uPrime[i])
          - d[last] * B2(uPrime[i])
          - d[last] * B3(uPrime[i]);
    X[0] += dot(A[i][0], tmp);
    X[1] += dot(A[i][1], tmp);
  }

  /* Compute the determinants of C and X	*/
  det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  det_C0_X  = C[0][0] * X[1]    - C[1][0] * X[0];
  det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

  /* Finally, derive alpha values	*/
  alpha_l = (det_C0_C1 == 0) ? 0.0 : det_X_C1 / det_C0_C1;
  alpha_r = (det_C0_C1 == 0) ? 0.0 : det_C0_X / det_C0_C1;

  /* If alpha negative, use the Wu/Barsky heuristic (see text) */
  /* (if alpha is 0, you get coincident control points that lead to
   * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
  double segLength = distance(d[last], d[first]);
  double epsilon = 1.0e-6 * segLength;
  if (alpha_l < epsilon || alpha_r < epsilon) {
    /* fall back on standard (probably inaccurate) formula, and subdivide further if needed. */
    double dist = segLength / 3.0;
    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    bezCurve[1] = bezCurve[0] + tHat1 * dist;
    bezCurve[2] = bezCurve[3] + tHat2 * dist;
    return;
  }

  /*  First and last control points of the Bezier curve are */
  /*  positioned exactly at the first and last data points */
  /*  Control points 1 and 2 are positioned an alpha distance out */
  /*  on the tangent vectors, left and right, respectively */
  bezCurve[0] = d[first];
  bezCurve[3] = d[last];
  bezCurve[1] = bezCurve[0] + tHat1 * alpha_l;
  bezCurve[2] = bezCurve[3] + tHat2 * alpha_r;
}

/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 *    \param degree The degree of the bezier curve
 *    \param V      Array of control points
 *    \param t      Parametric value to find point for
 */
static TPoint
evaluateBezier(int degree, const TPoint *V, double t)
{
  int i, j;		

  /* Copy array	*/
  CGPoint Vtemp[degree+1];     	/* Local copy of control points         */
  for (i = 0; i <= degree; i++) {
    Vtemp[i] = V[i];
  }

  /* Triangle computation	*/
  for (i = 1; i <= degree; i++) {	
    for (j = 0; j <= degree-i; j++) {
      Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j+1].x;
      Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j+1].y;
    }
  }

  return TPoint(Vtemp->x, Vtemp->y); // Point on curve at parameter t
}

/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 *  \param Q Current fitted curve
 *  \param P Digitized point
 *  \param u Parameter value for "P"
 */
static double
newtonRaphsonRootFind(const TPoint* Q, TPoint P, double u)
{
  double numerator, denominator;
  TPoint Q1[3], Q2[2];    // Q' and Q''
  TPoint Q_u, Q1_u, Q2_u; // u evaluated at Q, Q', & Q''
  int i;
    
  /* Compute Q(u)	*/
  Q_u = evaluateBezier(3, Q, u);
    
  /* Generate control vertices for Q'	*/
  for (i = 0; i <= 2; i++) {
    Q1[i].x = (Q[i+1].x - Q[i].x) * 3.0;
    Q1[i].y = (Q[i+1].y - Q[i].y) * 3.0;
  }
    
  /* Generate control vertices for Q'' */
  for (i = 0; i <= 1; i++) {
    Q2[i].x = (Q1[i+1].x - Q1[i].x) * 2.0;
    Q2[i].y = (Q1[i+1].y - Q1[i].y) * 2.0;
  }
    
  /* Compute Q'(u) and Q''(u)	*/
  Q1_u = evaluateBezier(2, Q1, u);
  Q2_u = evaluateBezier(1, Q2, u);
    
  /* Compute f(u)/f'(u) */
  numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
  denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		      	  (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
  if (denominator == 0.0f) return u;

  /* u = u - f(u)/f'(u) */
  return u - (numerator/denominator);
}

/*
 *  Reparameterize:
 *   Given set of points and their parameterization, try to find
 *   a better parameterization.
 *   \param d Array of digitized points
 *   \param first, last Indices defining region
 *   \param[inout] u Current parameter values
 *   \param bezCurve Current fitted curve
 */
static void
reparameterize(const TPoint *d, int first, int last, double *u, const TPoint* bezCurve)
{
  for (int i = first; i <= last; i++) {
    u[i-first] = newtonRaphsonRootFind(bezCurve, d[i], u[i-first]);
  }
}

/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 * Approximate unit tangents at endpoints and "center" of digitized curve
 * \param d Digitized points
 * \param end Index to "left" end of region
 */
static inline TPoint
computeLeftTangent(const TPoint *d, int end)
{
  return normalize(d[end+1] - d[end]);
}

/**
 * \param d Digitized points
 * \param end Index to "right" end of region
 */
static inline TPoint
computeRightTangent(const TPoint *d, int end)
{
  return normalize(d[end-1] - d[end]);
}

/**
 * \param d Digitized points
 * \param center Index to point inside region
 */
static TPoint
computeCenterTangent(const TPoint *d, int center)
{
  TPoint V1, V2, tHatCenter;

  V1 = d[center-1] - d[center];
  V2 = d[center] - d[center+1];
  tHatCenter.x = (V1.x + V2.x)/2.0;
  tHatCenter.y = (V1.y + V2.y)/2.0;
  return normalize(tHatCenter);
}


/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points 
 *	using relative distances between points.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 */
static void
chordLengthParameterize(const TPoint *d, int first, int last, double *u)
{
  int i;	

  u[0] = 0.0;
  for (i = first+1; i <= last; i++) {
    u[i-first] = u[i-first-1] +	distance(d[i], d[i-1]);
  }

  for (i = first + 1; i <= last; i++) {
    u[i-first] = u[i-first] / u[last-first];
  }
}

/*
 *  ComputeMaxError :
 *	Find the maximum squared distance of digitized points
 *	to fitted curve.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 *  \param bezCurve Fitted Bezier curve
 *  \param u Parameterization of points
 *  \param splitPoint Point of maximum error
 */
static double
computeMaxError(const TPoint *d, int first, int last, const TPoint* bezCurve, const double *u, int *splitPoint)
{
  int		i;
  double	maxDist;	/*  Maximum error		*/
  double	dist;		/*  Current error		*/
  TPoint	P;		/*  Point on curve		*/
  TPoint	v;		/*  Vector from point to curve	*/

  *splitPoint = (last - first + 1)/2;
  maxDist = 0.0;
  for (i = first + 1; i < last; i++) {
    P = evaluateBezier(3, bezCurve, u[i-first]);
    v = P - d[i];
    dist = squaredLength(v);
    if (dist >= maxDist) {
      maxDist = dist;
      *splitPoint = i;
    }
  }
  return maxDist;
}

static inline void
drawBezierCurve(int n, const TPoint *curve, vector<TPoint> *pathOut)
{
  for(int i=0; i<n; ++i)
    pathOut->push_back(TPoint(curve[i].x, curve[i].y));
}

/*
 *  FitCubic :
 *  	Fit a Bezier curve to a (sub)set of digitized points
 *  \param d Array of digitized points
 *  \param first, last Indices of first and last pts in region
 *  \param tHat1, tHat2 Unit tangent vectors at endpoints
 *  \param error User-defined error squared
 */
static void
fitCubic(const TPoint *d, int first, int last, TPoint tHat1, TPoint tHat2, double error, vector<TPoint> *out)
{
  TPoint        curve[4];	/*Control points of fitted Bezier curve*/
  double	maxError;	/*  Maximum fitting error	 */
  int		splitPoint;	/*  Point to split point set at	 */
  int		nPts;		/*  Number of points in subset  */
  double	iterationError; /*Error below which you try iterating  */
  int		maxIterations = 4; /*  Max times to try iterating  */
  TPoint	tHatCenter;   	/* Unit tangent vector at splitPoint */
  int		i;		

  iterationError = error * error;
  nPts = last - first + 1;

  /*  Use heuristic if region only has two points in it */
  if (nPts == 2) {
    double dist = distance(d[last], d[first]) / 3.0;
    curve[0] = d[first];
    curve[3] = d[last];
    curve[1] = curve[0] + tHat1 * dist;
    curve[2] = curve[3] + tHat2 * dist;
    drawBezierCurve(3, curve, out);
    return;
  }

  /*  Parameterize points, and attempt to fit curve */
  double u[last-first+1]; // parameter values for point
  chordLengthParameterize(d, first, last, u);
  generateBezier(d, first, last, u, tHat1, tHat2, curve);

  /*  Find max deviation of points to fitted curve */
  maxError = computeMaxError(d, first, last, curve, u, &splitPoint);
  if (maxError < error) {
    drawBezierCurve(3, curve, out);
    return;
  }

  /*  If error not too large, try some reparameterization  */
  /*  and iteration */
  if (maxError < iterationError) {
    for (i = 0; i < maxIterations; i++) {
      reparameterize(d, first, last, u, curve);
      generateBezier(d, first, last, u, tHat1, tHat2, curve);
      maxError = computeMaxError(d, first, last, curve, u, &splitPoint);
      if (maxError < error) {
        drawBezierCurve(3, curve, out);
        return;
      }
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  tHatCenter = computeCenterTangent(d, splitPoint);
  fitCubic(d, first, splitPoint, tHat1, tHatCenter, error, out);
  fitCubic(d, splitPoint, last, -tHatCenter, tHat2, error, out);
}

/*
 *  fitPath:
 *  Fit a Bezier curve to a set of digitized points 
 *  \param d Array of digitized points
 *  \param nPts Number of digitized points
 *  \param error User-defined error squared
 */
void
fitPath(const TPoint *d, size_t nPts, TCoord error, vector<TPoint> *out)
{
  TPoint	tHat1, tHat2;	/*  Unit tangent vectors at endpoints */

  tHat1 = computeLeftTangent(d, 0);
  tHat2 = computeRightTangent(d, nPts - 1);
  fitCubic(d, 0, nPts - 1, tHat1, tHat2, error, out);
  out->push_back(d[nPts-1]);
}

} // namespace
