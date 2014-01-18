/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2006 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef __TOAD_PEN_HH
#define __TOAD_PEN_HH 1

#include <toad/penbase.hh>
#include <vector>
#include <iostream>

namespace toad {

using namespace std;

class TRegion;
class TBitmap;

class TPen:
  public TPenBase
{
    TWindow *window;
    CGContextRef ctx; // Quartz2D Graphics Context

    NSBezierPath *clipPath;
    
    TMatrix2D windowmatrix; // Cocoa's initial matrix for the window.
    typedef vector<TMatrix2D> mstack_t;
    mstack_t mstack;
    ELineStyle linestyle;
    
  public:
    TPen(TWindow *);
    TPen(TBitmap *);
    ~TPen();

    void setFont(const string&);

    void identity();
    void translate(TCoord dx, TCoord dy);
    void scale(TCoord dx, TCoord dy);
    void rotate(TCoord radiants);
    void push();
    void pop();
    void multiply(const TMatrix2D*);
    void setMatrix(TCoord a11, TCoord a21, TCoord a12, TCoord a22, TCoord tx, TCoord ty);
    void setMatrix(const TMatrix2D&);
    const TMatrix2D* getMatrix() const;

    void getClipBox(TRectangle*) const;
    void operator&=(const TRectangle&);
    void operator|=(const TRectangle&);
    void operator&=(const TRegion&);
    void operator|=(const TRegion&);

    void setMode(EMode);
    void setLineWidth(int);
    void setLineStyle(ELineStyle);

    void vsetColor(TCoord r, TCoord g, TCoord b);
    void vsetLineColor(TCoord r, TCoord g, TCoord b);
    void vsetFillColor(TCoord r, TCoord g, TCoord b);
    void setAlpha(TCoord a);
    TCoord getAlpha() const;
    void vdrawRectangle(TCoord x, TCoord y, TCoord w, TCoord h);
    void vfillRectangle(TCoord x, TCoord y, TCoord w, TCoord h);
    void vdrawCircle(TCoord x,TCoord y,TCoord w,TCoord h);
    void vfillCircle(TCoord x,TCoord y,TCoord w,TCoord h);
    void vdrawArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2);
    void vfillArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2);
    void vdrawBitmap(TCoord x, TCoord y, const TBitmap&);
    void vdrawString(TCoord x, TCoord y, const char *str, int len, bool transparent);

    void drawPoint(TCoord x, TCoord y);
    void drawLines(const TPoint *points, size_t n); 
    void drawLines(const TPolygon&);

    void drawPolygon(const TPoint *points, size_t n);
    void drawPolygon(const TPolygon &p);
    void fillPolygon(const TPoint *points, size_t n); 
    void fillPolygon(const TPolygon &p);

    void drawPolyBezier(const TPoint *points, size_t n);
    void drawPolyBezier(const TPolygon &p);
    void fillPolyBezier(const TPoint *points, size_t n);
    void fillPolyBezier(const TPolygon &p);
};

}

#endif
