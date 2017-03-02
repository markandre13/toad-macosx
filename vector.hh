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

#ifndef _TOAD_VECTOR_HH
#define _TOAD_VECTOR_HH 1

#include <math.h>
#include <toad/penbase.hh>
#include <toad/window.hh>
#include <toad/bitmap.hh>
#include <toad/figuremodel.hh>
#include <toad/io/serializable.hh>

namespace toad {

class TIntersectionList;

/**
 * \class TVectorPath
 * a variant of NSBezierPath or cairo_path_t
 *
 * For vector operations like fill, deform, union, etc. to work on TFigures,
 * TVectorPath and the other classes defined here, provide a common storage
 * type, independent from how the figure is represented in TFigure.
 *
 * TFigure objects provides functions to return a TVectorBuffer.
 */
class TVectorPath
{
  public:
    TVectorPath() {}
    TVectorPath(const TVectorPath &v): points(v.points), type(v.type) {}
    TVectorPath& operator=(const TVectorPath &v) { points=v.points; type=v.type; return *this; }
    TVectorPath(const TVectorPath &&v): points(std::move(v.points)), type(std::move(v.type)) {}
    TVectorPath& operator=(const TVectorPath &&v) { points=std::move(v.points); type=std::move(v.type); return *this; }
  
    enum EType {
      MOVE, LINE, CURVE, CLOSE
    };
    vector<TPoint> points;
    vector<EType> type;

    bool empty() const { return type.empty(); }
    void join(const TVectorPath &); // append
    void clear();
    void move(const TPoint &p) { type.push_back(MOVE); points.push_back(p); }
    void move(TCoord x, TCoord y) { move(TPoint(x, y)); }
    void line(const TPoint &p) { type.push_back(LINE); points.push_back(p); }
    void line(TCoord x, TCoord y) { line(TPoint(x, y)); }
    void curve(const TPoint &p0, const TPoint &p1, const TPoint &p2) {
      type.push_back(CURVE); points.push_back(p0);  points.push_back(p1);  points.push_back(p2);
    }
    void close(){  type.push_back(CLOSE); }
    TBoundary bounds() const;
    TBoundary editBounds() const;
    
    void apply(TPenBase &pen) const;
    
    void intersectHelper(TIntersectionList &ilist, TVectorPath::EType type0, const TPoint *pt0, const TVectorPath &vp) const;
    void intersect(TIntersectionList &ilist, const TVectorPath &vp) const;
    void subdivide();
    
    // reduce()
    void simplify(TCoord tolerance=2.5, TCoord radians=2.0*M_PI/360.0*15.0);
    // tangent
    // normal
    // curvature
    
    
  private:
    void subdivideCutter(EType type, const TPoint *pt, const TRectangle &bounds, bool linesToBezier);
};

bool operator==(const TVectorPath &p0, const TVectorPath &p1);
ostream& operator<<(ostream &s, const TVectorPath& p);

class TVectorOperation
{
  public:
    TVectorOperation *next;
    virtual void paint(TPenBase &pen, const TVectorPath*) = 0;
};

class TVectorStrokeOp:
  public TVectorOperation
{
};

class TVectorFillOp:
  public TVectorOperation
{
};

// what about TFigureAttributes?
class TVectorStrokeAndFillOp:
  public TVectorOperation
{
    TRGB stroke, fill;
  public:
    TVectorStrokeAndFillOp(const TRGB &stoke, const TRGB &fill);
    void paint(TPenBase &pen, const TVectorPath*);
};

/**
 * Combines a TVectorPath with a TVectorOperation which paints the path
 */
class TVectorPainter
{
  public:
    TVectorPainter(): operation(0), path(0) {}
    TVectorPainter(TVectorOperation *o, TVectorPath *p): operation(o), path(p) {}
    TVectorOperation *operation;
    TVectorPath *path;
    void paint(TPenBase &pen);
};

/**
 * A collection of TVectorPainter
 */
class TVectorGraphic:
  public vector<TVectorPainter*>
{
  public:
    void paint(TPenBase &pen);
};

} // namespace

#endif
