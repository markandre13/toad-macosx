/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2003 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307,  USA
 */

#include <toad/types.hh>
#include <toad/matrix2d.hh>

#include <limits>

using namespace toad;
using namespace std;

TPolygon::TPolygon(const TRectangle &r)
{
  addPoint(r.origin);
  addPoint(r.origin.x+r.size.width, r.origin.y);
  addPoint(r.origin+r.size);
  addPoint(r.origin.x, r.origin.y+r.size.height);
}

void
TPolygon::transform(const TMatrix2D &matrix)
{
  for(auto &&point: *this)
    matrix.map(point, &point);
}

TCoord
TPolygon::distance(const TPoint &point) {
  TPoint p1, p2;
  TCoord x1,y1,x2,y2;
  TCoord min = numeric_limits<TCoord>::infinity();
  const_iterator p(begin());
  p2 = *p;
  ++p;
  while(p!=end()) {
    p1 = p2;
    p2 = *p;
    TCoord d = toad::distance(point, p1, p2);
    if (d<min)
      min = d;
    ++p;
  }
  return min;
}

/**
 * Returns 'true' when the point (x, y) is within the polygon.
 */
bool 
TPolygon::isInside(TCoord x, TCoord y) const
{
  int hits = 0;
  int ySave = 0;
  size_t i = 0;
  size_t npoints = size();
  while (i < npoints && (*this)[i].y == y) {
    i++;
  }
      
  for (size_t n = 0; n < npoints; n++) {
    size_t j = (i + 1) % npoints;
    TCoord x1 = (*this)[i].x, y1 = (*this)[i].y;
    TCoord x2 = (*this)[j].x, y2 = (*this)[j].y;
        
    TCoord dx = x2 - x1;
    TCoord dy = y2 - y1;
        
    if (dy != 0) {
      TCoord rx = x - x1;
      TCoord ry = y - y1;
      
      if (y2 == y && x2 >= x) {
        ySave = y1;
      }
      if (y1 == y && x1 >= x) {
        if ((ySave > y) != (y2 > y)) {
          hits--;
        }
      }
      
      TCoord s = ry / dy;
      if (s >= 0.0 && s <= 1.0 && (s * dx) >= rx) {
        hits++;
      }
    }
    i=j;
  }
  return (hits % 2) != 0;
}

bool
TPolygon::getShape(TRectangle *r) const
{
  TPoint p1, p2;

  const_iterator p(begin()), e(end());
  
  if (p==e) {
    r->set(0,0,0,0);
    return false;
  }

  p1.x = p2.x = p->x;
  p1.y = p2.y = p->y;

  ++p;
  while(p!=e) {
    if (p->x < p1.x)
      p1.x = p->x;
    if (p->x > p2.x)
      p2.x = p->x;
    if (p->y < p1.y)
      p1.y = p->y;
    if (p->y > p2.y)
      p2.y = p->y;
    ++p;
  }

  r->set(p1,p2);
  return true;
}
