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

#ifndef _TOAD_FIGURE_HH
#define _TOAD_FIGURE_HH 1

#include <math.h>
#include <toad/penbase.hh>
#include <toad/window.hh>
#include <toad/bitmap.hh>
#include <toad/figuremodel.hh>
#include <toad/io/serializable.hh>

namespace toad {

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
    enum EType {
      MOVE, LINE, CURVE, CLOSE
    };
    vector<TPoint> points;
    vector<EType> type;
    
    void move(const TPoint &p) { type.push_back(MOVE); points.push_back(p); }
    void line(const TPoint &p) { type.push_back(LINE); points.push_back(p); }
    void curve(const TPoint &p0, const TPoint &p1, const TPoint &p2) {
      type.push_back(CURVE); points.push_back(p0);  points.push_back(p1);  points.push_back(p2);
    }
    void close() {  type.push_back(CLOSE); }
    TRectangle bounds() const;
    TRectangle editBounds() const;
};

class TVectorOperation
{
  TVectorOperation *next;
  virtual void paint(TPen &pen, const TVectorPath*) = 0;
};

class TVectorStrokeOp:
  public TVectorOperation
{
};

class TVectorFillOp
  public TVectorOperation
{
};

class TVectorPainter
{
  TVectorPath *path;
  TVectorOperation *operation;
};

class TVectorBuffer:
  public vector<TVectorPainter*>
{
} // namespace
