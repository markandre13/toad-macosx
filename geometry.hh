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

#ifndef _TOAD_GEOMETRY_HH
#define _TOAD_GEOMETRY_HH 1

#include <toad/types.hh>
#include <toad/vector.hh>

namespace toad {

static const TCoord tolerance = 10e-6;
static const TCoord epsilon = 1e-12;
static const TCoord machine_epsilon = 1.12e-16;

static inline bool isZero(TCoord a) { return fabs(a) <= epsilon; }

struct TEnd {
  TEnd(TVectorPath::EType t, const TPoint *s, TCoord U, TPoint p):
    type(t), src(s), u(U), pt(p) {}
  TVectorPath::EType type;
  const TPoint *src;
  TCoord u;
  TPoint pt;
};

struct TIntersection {
  TIntersection(TVectorPath::EType t0, const TPoint *s0, TCoord u0, TPoint p0,
                TVectorPath::EType t1, const TPoint *s1, TCoord u1, TPoint p1):
    seg0(t0, s0, u0, p0), seg1(t1, s1, u1, p1) {}
  TEnd seg0, seg1; // segment 0 & 1
};

class TIntersectionList:
  public vector<TIntersection>
{
  public:
    void add(TVectorPath::EType t0, const TPoint *s0, TCoord u0, TPoint p0,
             TVectorPath::EType t1, const TPoint *s1, TCoord u1, TPoint p1)
    {
      push_back(TIntersection(t0, s0, u0, p0, t1, s1, u1, p1));
    }
};

void intersectCurveCurve(TIntersectionList &ilist, const TPoint *curve0, const TPoint *curve1);
void intersectCurveLine(TIntersectionList &ilist, const TPoint *curve, const TPoint *line);
void intersectLineCurve(TIntersectionList &ilist, const TPoint *line, const TPoint *curve);
void intersectLineLine(TIntersectionList &ilist, const TPoint *line0, const TPoint *line1);

TRectangle curveBounds(const TPoint *curve);

void divideBezier(const TPoint *a, TPoint *p, TCoord min, TCoord max);
void divideBezier(const TPoint *a, TPoint *p, TCoord u);
TPoint bez2point(const TPoint *p, TCoord t);
TPoint bez2direction(const TPoint *p, TCoord u);
void fitPath(const TPoint *inPoints, size_t size, TCoord tolerance, vector<TPoint> *out);
inline void fitPath(const vector<TPoint> &in, TCoord tolerance, vector<TPoint> *out) {
  fitPath(in.data(), in.size(), tolerance, out);
}
void convexHull(vector<TPoint> *points);

/*
A Bézier curve does not have self-intersections if the sum of all absolute
values of the outer rotation angles of the Bézier polygon is smaller than or
equal to π.
bool isSelfIntersecting(...)
*/

int solveQuadratic(TCoord a, TCoord b, TCoord c, TCoord *roots);

enum BooleanOpType { INTERSECTION, UNION, DIFFERENCE, XOR };
void boolean(const TVectorPath &subj, const TVectorPath &clip, TVectorPath *out, BooleanOpType op);

} // namespace

#endif
