/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2015 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_TYPES_HH
#define _TOAD_TYPES_HH

#include <iostream>
#include <vector>
#include <cmath>

#import <CoreGraphics/CGBase.h>
#import <CoreGraphics/CGGeometry.h>

typedef unsigned long ulong;
typedef unsigned char ubyte;
typedef unsigned char byte;

namespace toad {

typedef CGFloat TCoord; // cocoa
// typedef double TCoord; // cairo

using namespace std;

struct TPoint:
  public CGPoint
{
  TPoint() { x = y = 0; }
  TPoint(const TPoint &p) {x=p.x; y=p.y;}
  TPoint(TCoord inX, TCoord inY) { x=inX; y=inY; }
  void set(TCoord a, TCoord b) { x=a;y=b; }
  const TPoint& operator+=(const TPoint &a) {
    x += a.x;
    y += a.y;
    return *this;
  }
  const TPoint& operator-=(const TPoint &a) {
    x -= a.x;
    y -= a.y;
    return *this;
  }
  TPoint operator-() {
    return TPoint(-x, -y);
  }
};

inline bool operator==(const TPoint &a, const TPoint &b) {
  return a.x==b.x && a.y==b.y;
}
inline bool operator!=(const TPoint &a, const TPoint &b) {
  return a.x!=b.x || a.y!=b.y;
}
inline TPoint operator-(const TPoint &a, const TPoint &b) {
  return TPoint(a.x-b.x, a.y-b.y);
}
inline TPoint operator+(const TPoint &a, const TPoint &b) {
  return TPoint(a.x+b.x, a.y+b.y);
}
inline TPoint operator*(TCoord a, const TPoint &b) {
  return TPoint(a*b.x, a*b.y);
}
inline TPoint operator*(const TPoint &b, TCoord a) {
  return TPoint(a*b.x, a*b.y);
}
inline ostream& operator<<(ostream &s, const TPoint& p) {
  return s<<'('<<p.x<<','<<p.y<<')';
}
inline double squaredLength(const TPoint &a) {
  return a.x*a.x+a.y*a.y;
}

/* returns length of input vector */
inline double length(const TPoint &a) {
  return sqrt(squaredLength(a));
}

/* return the distance between two points */
inline double distance(const TPoint &a, const TPoint &b) {
  return length(a-b);
}
	
/* normalizes the input vector and returns it */
inline TPoint normalize(const TPoint &v) {
  TPoint result;
  double len = length(v);
  if (len != 0.0) {
    result.x = v.x / len;
    result.y = v.y / len;
  } else {
    result.x = v.x;
    result.y = v.y;
  }
  return(result);
}

/* return the dot product of vectors a and b */
inline double dot(const CGPoint &a, const CGPoint &b) {
  return a.x*b.x+a.y*b.y;
}

struct TSize:
  public CGSize
{
};

struct TRectangle;

struct Box 
{
  TCoord  x1, x2, y1, y2;
  
  Box() { set(0, 0, 0, 0); }
  Box(const Box &box) { set(box.x1, box.y1, box.x2, box.y2); }
  Box(TCoord left, TCoord top, TCoord right, TCoord bottom) { set(left, top, right, bottom); }
  Box(const TRectangle &rectangle);
  void set(TCoord left, TCoord top, TCoord right, TCoord bottom) {
    this->x1 = left;
    this->y1 = top;
    this->x2 = right;
    this->y2 = bottom;
  }
  
  void set(const TRectangle &rectangle);

  bool isInside(TCoord x, TCoord y) const { return x >= x1 && x < x2 && y >= y1 && y < y2; }
  bool isInsideOf(const Box &box) const { return x1 >= box.x1 && y1 >= box.y1 && x2 <= box.x2 && y2 <= box.y2; }
  bool isOverlapping(const Box &box) const { return x2 > box.x1 && y2 > box.y1 && x1 < box.x2 && y1 < box.y2; }
  bool isOverlapping(const TRectangle &rectangle) const;
  bool intersects(const Box &r) const;
  void translate(TCoord dx, TCoord dy) { x1 += dx; y1 += dy; x2 += dx; y2 += dy; }
};

struct TRectangle
{
  TCoord x, y, h, w;
  TRectangle() { set(0, 0, 0, 0); }
  TRectangle(TCoord x, TCoord y, TCoord w, TCoord h) { set(x, y, w, h); }
  TRectangle(const TPoint &p1, const TPoint &p2){ set(p1,p2); }
  TRectangle(const Box &b) { set(b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1); }
  TPoint origin() const { return TPoint(x, y); }
  bool operator==(const TRectangle &r) const { return x==r.x && y==r.y && w==r.w && h==r.h; }
  bool isEmpty() const { return w<=0 || h<=0; }
  void set(TCoord x, TCoord y, TCoord w, TCoord h); // { this->x=x; this->y=y; this->w=w; this->h=h; }
  void set(const TPoint&, const TPoint&);
  bool isInside(TCoord px, TCoord py) const { return x<=px && px<=x+w && y<=py && py<=y+h; }
  bool isInside(TPoint p) const { return isInside(p.x, p.y); }
  bool intersects(const TRectangle &r) const;
};

inline Box::Box(const TRectangle &rectangle) { set(rectangle); }
inline void Box::set(const TRectangle &r) { x1=r.x; y1=r.y; x2=r.x + r.w; y2=r.y + r.h; }
inline bool Box::isOverlapping(const TRectangle &r) const { return x2>r.x && x1<r.x+r.w && y2>r.y && y1<r.y+r.h; }

inline ostream& operator<<(ostream &s, const TRectangle& r) {
  return s<<'('<<r.x<<','<<r.y<<','<<r.w<<','<<r.h<<')';
}

class TPolygon: 
  public std::vector<TPoint>
{
  public:
    void addPoint(const TPoint &p) { push_back(p); }
    void addPoint(TCoord x, TCoord y) { push_back(TPoint(x,y)); }
    bool isInside(TCoord x, TCoord y) const;
    bool isInside(const TPoint &p) const { return isInside(p.x, p.y); }
    bool getShape(TRectangle *r) const;
};

} // namespace toad

#endif
