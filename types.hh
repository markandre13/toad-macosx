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
#include <toad/io/serializable.hh>

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
  void translate(TCoord a, TCoord b) { x+=a; y+=b; }
  TPoint operator-() {
    return TPoint(-x, -y);
  }
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
  const TPoint& operator*=(TCoord f) {
    x *= f;
    y *= f;
    return *this;
  }
  const TPoint& operator/=(TCoord f) {
    x /= f;
    y /= f;
    return *this;
  }
};
struct TSize:
  public CGSize
{
  public:
    TSize() { width=height=0; }
    TSize(TCoord w, TCoord h) { width=w; height=h; }
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
inline TPoint operator/(const TPoint &a, TCoord b) {
  return TPoint(a.x/b, a.y/b);
}
inline ostream& operator<<(ostream &s, const TPoint& p) {
  return s<<'('<<p.x<<','<<p.y<<')';
}

void store(TOutObjectStream &out, const vector<TPoint> &p);
bool restore(TInObjectStream &in, vector<TPoint> *p);

inline TCoord squaredLength(const TPoint &a) {
  return a.x*a.x+a.y*a.y;
}

/** returns length of input vector */
inline TCoord length(const TPoint &a) {
  return sqrt(squaredLength(a));
}

/** return the distance between two points */
inline TCoord distance(const TPoint &a, const TPoint &b) {
  return length(a-b);
}

/** return the maximum distance between two points along either x- or y-axis */
inline TCoord maxDistanceAlongAxis(const TPoint &a, const TPoint &b) {
  return max(fabs(a.x-b.x), fabs(a.y-b.y));
}
	
/** normalize the input vector and returns it */
inline TPoint normalize(const TPoint &v) {
  TPoint result;
  TCoord len = length(v);
  if (len != 0.0) {
    result.x = v.x / len;
    result.y = v.y / len;
  } else {
    result.x = v.x;
    result.y = v.y;
  }
  return(result);
}

/** return the dot product of vectors a and b */
inline TCoord dot(const CGPoint &a, const CGPoint &b) {
  return a.x*b.x+a.y*b.y;
}

/** rotate the direction by 90 degrees */
inline TPoint rot90(const TPoint &in) {
  return TPoint(-in.y, in.x);
}

/** Signed area of the triangle (p0, p1, p2) */
inline TCoord signedArea(const TPoint& p0, const TPoint& p1, const TPoint& p2) { 
  return (p0.x- p2.x)*(p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
}

/** Signed area of the triangle ( (0,0), p1, p2) */
inline TCoord signedArea(const TPoint& p1, const TPoint& p2) { 
  return -p2.x*(p1.y - p2.y) - -p2.y*(p1.x - p2.x);
}

/** Sign of triangle (p1, p2, o) */
inline int sign(const TPoint& p1, const TPoint& p2, const TPoint& o) {
  TCoord det = (p1.x - o.x) * (p2.y - o.y) - (p2.x - o.x) * (p1.y - o.y);
  return (det < 0 ? -1 : (det > 0 ? +1 : 0));
}

#if 0
inline bool pointInTriangle (const Segment_2& s, const TPoint& o, const TPoint& p) {
	int x = sign (s.source (), s.target (), p);
	return ((x == sign (s.target (), o, p)) && (x == sign (o, s.source (), p)));
}
#endif



inline ostream& operator<<(ostream &s, const TSize& p) {
  return s<<'('<<p.width<<','<<p.height<<')';
}

struct TRectangle;

struct TBoundary
{
  bool empty;
  TPoint p0, p1;
  
  TBoundary() { empty=true; }
  TBoundary(const TBoundary &b) { empty=b.empty; p0=b.p0; p1=b.p1; }
  TBoundary(TCoord left, TCoord top, TCoord right, TCoord bottom) { set(left, top, right, bottom); }
  TBoundary(const TRectangle &rectangle);
  TCoord width() const { return p1.x - p0.x; }
  TCoord height() const { return p1.y - p0.y; }
  void clear() { empty = true; }
  void set(TCoord left, TCoord top, TCoord right, TCoord bottom) {
    empty = false;
    p0.x = left;
    p0.y = top;
    p1.x = right;
    p1.y = bottom;
  }
  
  void set(const TRectangle &rectangle);
  void expand(const TPoint &p) {
    if (empty) {
      p0 = p1 = p;
      empty=false;
    } else {
      if (p.x<p0.x)
        p0.x=p.x;
      else
      if (p.x>p1.x)
        p1.x=p.x;
      if (p.y<p0.y)
        p0.y=p.y;
      else
      if (p.y>p1.y)
        p1.y=p.y;
    }
  }
  void expand(const TBoundary &b) {
    if (b.empty)
      return;
    expand(b.p0);
    expand(b.p1);
  }

  bool isInside(TCoord x, TCoord y) const {
    return empty ? false : 
      p0.x <= x && x < p1.x &&
      p0.y <= y && y < p1.y;
  }
  bool isInsideOf(const TBoundary &b) const {
    return (empty || b.empty) ? false : 
      p0.x >= b.p0.x &&
      p0.y >= b.p0.y &&
      p1.x <= b.p1.x &&
      p1.y <= b.p1.y;
  }
  bool isOverlapping(const TBoundary &b) const {
    return (empty || b.empty) ? false :
      p1.x > b.p0.x &&
      p1.y > b.p0.y &&
      p0.x < b.p1.x &&
      p0.y < b.p1.y;
  }
  bool isOverlapping(const TRectangle &rectangle) const;
  bool intersects(const TBoundary &r) const;
  void translate(TCoord dx, TCoord dy) { translate(TPoint(dx, dy)); }
  void translate(const TPoint &p) {
    p0+=p;
    p1+=p;
  }
  TPoint center() const { return p0+((p1-p0)/2); }
};

struct TRectangle
{
  TCoord x, y, w, h;
//  TPoint origin;
//  TSize size;
  TRectangle() { set(0, 0, 0, 0); }
  TRectangle(TCoord x, TCoord y, TCoord w, TCoord h) { set(x, y, w, h); }
  TRectangle(const TPoint &p1, const TPoint &p2){ set(p1,p2); }
  TRectangle(const TBoundary &b) { set(b.p0.x, b.p0.y, b.p1.x - b.p0.x, b.p1.y - b.p0.y); }
  TPoint origin() const { return TPoint(x, y); }
  bool operator==(const TRectangle &r) const { return x==r.x && y==r.y && w==r.w && h==r.h; }
  bool isEmpty() const { return w<=0 || h<=0; }
  void set(TCoord x, TCoord y, TCoord w, TCoord h); // { this->x=x; this->y=y; this->w=w; this->h=h; }
  void set(const TPoint&, const TPoint&);
  bool isInside(TCoord px, TCoord py) const { return x<=px && px<=x+w && y<=py && py<=y+h; }
  bool isInside(const TPoint &p) const { return isInside(p.x, p.y); }
  bool isInside(const TRectangle &r) const { return r.isInside(TPoint(x, y)) && r.isInside(TPoint(x+w, y+h)); }
  bool intersects(const TRectangle &r) const;
  
  TRectangle& translate(const TPoint &point) {
    x += point.x;
    y += point.y;
    return *this;
  }
  TRectangle& expand(TCoord expansion) {
    x -= expansion;
    y -= expansion;
    expansion *= 2.0;
    w += expansion;
    h += expansion;
    return *this;
  }
};

inline TBoundary::TBoundary(const TRectangle &rectangle) { set(rectangle); }
inline void 
TBoundary::set(const TRectangle &r) {
  empty=false;
  p0.x=r.x;
  p0.y=r.y;
  p1.x=r.x + r.w;
  p1.y=r.y + r.h;
}
inline bool
TBoundary::isOverlapping(const TRectangle &r) const {
  return empty ? false :
    p1.x>r.x && p0.x<r.x+r.w && p1.y>r.y && p0.y<r.y+r.h;
}

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
