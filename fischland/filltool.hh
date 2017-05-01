/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2007 by Mark-André Hopf <mhopf@mark13.org>
 * Visit http://www.mark13.org/fischland/.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _FISCHLAND_FILLTOOL_HH
#define _FISCHLAND_FILLTOOL_HH 1

#include <toad/figuretool.hh>

#include "fpath.hh"

namespace fischland {

using namespace std;
using namespace toad;

class TFillTool:
  public TFigureTool
{
  public:
    static TFillTool* getTool();
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
};

double angleAtPoint(const TFPath *a, TCoord u);
void divideBezier(const TPoint *a, TPoint *p, TCoord u=0.5);
void divideBezier(const TFPath *a, TPoint *p, TCoord u=0.5);
bool lineIntersection(const TPoint &a0, const TPoint &a1,
                      const TPoint &b0, const TPoint &b1);
bool pointsOverlap(const TPoint *p, TCoord u, const TPoint *q, TCoord v, TCoord range=1.0);
bool pointsOverlap(const TFPath *p0, TCoord u0, const TFPath *p1, TCoord u1, TCoord range=1.0);

struct IntersectionPoint {
  IntersectionPoint(const TPoint &p, TCoord u, TCoord v, const TFPath *pu, const TFPath *pv):
    coord(p), a(u), b(v), ap(pu), bp(pv) {}
  TPoint coord;
  TCoord a, b;
  const TFPath *ap, *bp;
};
typedef vector<IntersectionPoint> IntersectionPointList;

void
bezierIntersection(IntersectionPointList &found,
                   const TPoint *a,
                   const TPoint *b,
                   const TFPath *ap,
                   const TFPath *bp,
                   TCoord as=0.0, TCoord bs=0.0,
                   TCoord a0=0.0, TCoord a1=1.0,
                   TCoord b0=0.0, TCoord b1=1.0,
                   unsigned d=0);

void
bezierIntersection(IntersectionPointList &found,
                   const TFPath *ap,
                   const TFPath *bp);

struct direction_t {
  direction_t(TCoord aDegree, const TFPath *aPath, TCoord aPosition, bool aForward)
    :degree(aDegree), path(aPath), position(aPosition), forward(aForward)
  {}
  TCoord degree;
  const TFPath *path;
  TCoord position;
  bool forward;
};
typedef vector<direction_t> directionstack_t;

struct segment_t {
  segment_t() {
    path = 0;
    u0 = u1 = 0.0;
    mina = 0.0;
    neara = 0.0;
    nearpath = 0;
  }
  // found path segment
  const TFPath *path;
  TCoord u0, u1;

  // mina we found during the last time we've searched 'list'
  TCoord mina;

  // values for reset
  TCoord neara;
  const TFPath *nearpath;
};
typedef vector<segment_t> segmentstack_t;

bool
insidepath(TCoord x,
           const IntersectionPointList &linelist,
           const segmentstack_t &stack);

TFPath* segmentstack2path(const segmentstack_t &stack);
TFPath* floodfill(TFigureModel &model, TCoord x, TCoord y);

static inline TCoord
fac(unsigned n)
{
  TCoord u = 1;
  for(unsigned i=2; i<=n; ++i)
    u *= i;
  return u;
}

static inline TCoord
C(unsigned n, unsigned k)
{
  return fac(n) / (fac(k) * fac(n-k));
}

static inline TCoord
B(unsigned i, TCoord u)
{
  return C(3, i) * powl(1.0-u, 3-i) * powl(u, i);
}

} // namespace fischland

#endif
