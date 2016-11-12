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
#include <toad/font.hh>
#include <vector>
#include <iostream>

namespace toad {

using namespace std;

class TRegion;
class TBitmap;

class TPen:
  public TPenBase
{
  public:
    TWindow *window;
    CGContextRef ctx; // Quartz2D Graphics Context

    // Quartz 2D PDF
    CGContextRef pdfContext;
    CFDataRef pdfBoxData;
    CFMutableDictionaryRef pdfPageDictionary;
    
    NSMutableData *clipboardData;

    NSBezierPath *clipPath;
    
    TMatrix2D windowmatrix; // Cocoa's initial matrix for the window.
    typedef vector<TMatrix2D> mstack_t;
    mstack_t mstack;
    ELineStyle linestyle;   // FIXME: not in push/pop
    TCoord linewidth;       // FIXME: not in push/pop
    
  public:
    TPen();
    TPen(TWindow *window) { initWindow(window); }
    TPen(TBitmap *bitmap) { initBitmap(bitmap); }
    TPen(const string &filename) { initPDFFile(filename); }
    ~TPen();
    
    void init();
    void initWindow(TWindow*);
    void initBitmap(TBitmap*);
    void initPDFFile(const string &filename);
    void initClipboard(const TRectangle &r);

    void pagebreak();

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

    void setClipRect(const TRectangle&);
    void getClipBox(TRectangle*) const;

    void setMode(EMode);
    void setLineWidth(TCoord);
    void setLineStyle(ELineStyle);
    
    void move(TCoord x, TCoord y);
    void line(TCoord x, TCoord y);
    void move(const TPoint*);
    void line(const TPoint*);
    void curve(const TPoint*);
    void close();
    void stroke();
    void fill();

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

    void drawBezier(const TPoint *points, size_t n);
    inline void drawBezier(const vector<TPoint> v) { drawBezier(v.data(), v.size()); }
    void drawCurve(TCoord x0, TCoord y0, TCoord x1, TCoord y1, TCoord x2, TCoord y2, TCoord x3, TCoord y3);
    void drawBezier(const TPolygon &p);
    void fillBezier(const TPoint *points, size_t n);
    void fillBezier(const TPolygon &p);
};

}

#endif
