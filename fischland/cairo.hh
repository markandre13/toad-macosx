/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2006 by Mark-André Hopf <mhopf@mark13.org>
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

#include "config.h"

#ifdef HAVE_LIBCAIRO

#ifndef _TOAD_CAIRO_HH
#define _TOAD_CAIRO_HH

extern "C" {
#include <cairo-xlib-xrender.h>
#include <cairo-pdf.h>
}

#include <stack>
#include <toad/pen.hh>

namespace toad {

class TCairo:
  public TPenBase
{
public:
    typedef TPenBase super;
    TRectangle clipbox;
    
    stack<cairo_matrix_t> mstack;

    cairo_surface_t *cs;
    cairo_t *cr;

    TRGB foreground, background;
    byte alpha;
    int width;

  public:
    TCairo(TWindow*);
    TCairo(TBitmap*);
    TCairo(cairo_surface_t *);
    TCairo(const string &pdffilename);
    ~TCairo();
    
    void draw(double x, double y, cairo_surface_t *s);
    
    void showPage();
    void identity();
    void translate(double dx, double dy);
    void rotate(double);
    void scale(double, double);
    // void shear(double, double);
    void multiply(const TMatrix2D*);
    void setMatrix(double a11, double a12, double a21, double a22, double tx, double ty);
    TMatrix2D *getMatrix() const;
    void push();
    void pop();
    void popAll();
    
    void _updateCairoMatrix();

    void setColor(const TColor &c) { setColor(c.r, c.g, c.b); }
    void setColor(const TRGB &c) { setColor(c.r, c.g, c.b); }
    void setColor(byte r,byte g,byte b);
    
    void setFillColor(const TColor&);
    void setLineColor(const TColor&);
    void _fill();
    
    void setAlpha(byte);
    byte getAlpha() const;
    
    void setLineWidth(int w);
    void setLineStyle(ELineStyle n);
    
    void vdrawLine(int x1, int y1, int x2, int y2);
    void drawLines(const TPoint *points, int n);
    void drawLines(const TPolygon&);
    
    void vdrawRectangle(int x,int y,int w,int h);
    void vfillRectangle(int x,int y,int w,int h);

    void vdrawCircle(int x,int y,int w,int h);
    void vfillCircle(int x,int y,int w,int h);

    void fillPolygon(const TPoint *points, int n);
    void drawPolygon(const TPoint *points, int n);
    void fillPolygon(const TPolygon &polygon);
    void drawPolygon(const TPolygon &polygon);
    
    void drawPolyBezier(const TPoint *points, int n);
    void drawPolyBezier(const TPolygon &p);
    void fillPolyBezier(const TPolygon &p);

    void shear(double x, double y);
    void setBitmap(TBitmap *b);
    void setFont(const string &fn);
    void setMode(EMode m);
    void setColorMode(TColor::EDitherMode m);
    void setClipChildren(bool b);
    void setClipRegion(TRegion *r);
    void setClipRect(const TRectangle &r);
    void clrClipBox();
    void getClipBox(TRectangle *r) const;
    void operator&=(const TRectangle&);
    void operator|=(const TRectangle&);
    void operator&=(const TRegion&);
    void operator|=(const TRegion&);
    void drawPoint(int, int);
    void vdrawArc(int, int, int, int, double, double);
    void vfillArc(int, int, int, int, double, double);
    void drawBezier(int, int, int, int, int, int, int, int);
    void drawBezier(double, double, double, double, double, double, double, double);
    void drawPolyBezier(const TDPoint*, int);
    void drawPolyBezier(const TDPolygon&);
    void fillPolyBezier(const TPoint*, int);
    void fillPolyBezier(const TDPoint*, int);
    void fillPolyBezier(const TDPolygon&);
    void vdraw3DRectangle(int, int, int, int, bool);
    int vgetTextWidth(const char *t, size_t l) const;
    int getAscent() const;
    int getDescent() const;
    int getHeight() const;
    void vdrawString(int, int, const char*, int, bool);
    int drawTextWidth(int x, int y, const string &t, unsigned int l);
    void drawBitmap(int, int, const TBitmap*);
    void drawBitmap(int, int, const TBitmap&);
    void drawBitmap(int, int, const TBitmap*, int, int, int, int);
    void drawBitmap(int, int, const TBitmap&, int, int, int, int);
    void moveTo(int, int);
    void moveTo(const TPoint&);
    void lineTo(int, int);
    void lineTo(const TPoint&);
    void curveTo(int, int, int, int, int, int);
    void curveTo(const TPoint&, const TPoint&, const TPoint&);
    void curveTo(double, double, double, double, double, double);
};

} // namespace toad

#endif

#endif
