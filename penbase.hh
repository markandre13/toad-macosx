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

#ifndef __TOAD_PENBASE_HH
#define __TOAD_PENBASE_HH 1

#import <Cocoa/Cocoa.h>

#include <toad/color.hh>
#include <toad/font.hh>
#include <toad/matrix2d.hh>

namespace toad {

class TBitmap;
class TWindow;

/**
 * format is a wrapper for printf
 *
 * pen.drawString(0, 0, format("time %us", t));
 */
const char* format(const char *fmt, ...);

class TPenBase
{
  protected:
    TRGBA rgba_stroke, rgba_fill;
  public:
    virtual ~TPenBase();
    PFont font;

    enum EMode {
      NORMAL=3,
      XOR=6,
      INVERT=10
    };

    enum ELineStyle {
      SOLID=1,
      DASH,
      DOT,
      DASHDOT,
      DASHDOTDOT
    };

    virtual void setFont(const string&) = 0;
    
    virtual void move(TCoord x, TCoord y) = 0;
    virtual void line(TCoord x, TCoord y) = 0;
    virtual void move(const TPoint*) = 0;
    virtual void line(const TPoint*) = 0;
    virtual void curve(const TPoint*) = 0;
    virtual void close() = 0;
    virtual void stroke() = 0;
    virtual void fill() = 0;
    virtual void fillStroke() = 0;

    virtual void identity() = 0;
    virtual void translate(const TPoint &vector) = 0;
    inline void translate(TCoord x, TCoord y) {
      translate(TPoint(x, y));
    }
    virtual void scale(TCoord dx, TCoord dy) = 0;
    virtual void rotate(TCoord radiants) = 0;
    virtual void push() = 0;
    virtual void pop() = 0;
    virtual void multiply(const TMatrix2D*) = 0;
    void transform(const TMatrix2D &matrix) {
      multiply(&matrix);
    }
    virtual void setMatrix(TCoord a11, TCoord a21, TCoord a12, TCoord a22, TCoord tx, TCoord ty) = 0;
    virtual const TMatrix2D* getMatrix() const = 0;

    virtual void getClipBox(TRectangle*) const = 0;
    
    virtual void setMode(EMode) = 0;
    virtual void setLineWidth(TCoord) = 0;
    void setScreenLineWidth(TCoord);
    virtual void setLineStyle(ELineStyle) = 0;
    //virtual void setColorMode(TColor::EDitherMode) = 0;
    //virtual void setClipChildren(bool) = 0;

    virtual void vsetColor(TCoord r, TCoord g, TCoord b) = 0;
    virtual void vsetStrokeColor(TCoord r, TCoord g, TCoord b) = 0;
    virtual void vsetFillColor(TCoord r, TCoord g, TCoord b) = 0;
    
    virtual void vdrawBitmap(TCoord,TCoord,const TBitmap&) = 0;

    virtual void drawPoint(TCoord x, TCoord y) = 0;
    virtual void vdrawRectangle(TCoord x,TCoord y,TCoord w,TCoord h) = 0;
    virtual void vfillRectangle(TCoord x,TCoord y,TCoord w,TCoord h) = 0;
    virtual void vdrawCircle(TCoord x,TCoord y,TCoord w,TCoord h) = 0;
    virtual void vfillCircle(TCoord x,TCoord y,TCoord w,TCoord h) = 0;
    virtual void vdrawArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2) = 0;
    virtual void vfillArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2) = 0;
    virtual void vdrawString(TCoord x, TCoord y, const char *str, int len, bool transparent) = 0;

    void drawLine(TCoord x0, TCoord y0, TCoord x1, TCoord y1) {
      TPoint p[2];
      p[0].set(x0, y0);
      p[1].set(x1, y1);
      drawLines(p, 2);
    }
    void drawLine(const TPoint &a, const TPoint &b) {
      drawLine(a.x, a.y, b.x, b.y);
    }
    virtual void drawLines(const TPoint *points, size_t n) = 0; 
    virtual void drawLines(const TPolygon&) = 0;

    TCoord getTextWidth(const char* text, size_t len) const { return font->getTextWidth(text, len); }
    TCoord getTextWidth(const char* text) { return font->getTextWidth(text, strlen(text)); }
    TCoord getTextWidth(const string &text) const { return font->getTextWidth(text.c_str(), text.size()); }
    TCoord getAscent() const { return font->getAscent(); }
    TCoord getDescent() const { return font->getDescent(); }
    TCoord getHeight() const { return font->getHeight(); }
    TCoord underlinePosition() const { return font->underlinePosition(); }
    TCoord underlineThickness() const { return font->underlineThickness(); }

    void setColor(TCoord r, TCoord g, TCoord b) { vsetColor(r, g, b); }
    void setStrokeColor(TCoord r, TCoord g, TCoord b) { vsetStrokeColor(r, g, b); }
    void setFillColor(TCoord r, TCoord g, TCoord b) { vsetFillColor(r, g, b); }
    void setColor(const TRGB &rgb) { vsetColor(rgb.r, rgb.g, rgb.b); }
    void setStrokeColor(const TRGB &rgb) { vsetStrokeColor(rgb.r, rgb.g, rgb.b); }
    void setFillColor(const TRGB &rgb) { vsetFillColor(rgb.r, rgb.g, rgb.b); }
    void setColor(TColor::EColor e);
    void setStrokeColor(TColor::EColor e);
    void setFillColor(TColor::EColor e);
    virtual void setAlpha(TCoord a) = 0;
    virtual TCoord getAlpha() const = 0;

    void drawRectanglePC(TCoord x,TCoord y,TCoord w,TCoord h);
    void drawRectanglePC(const TRectangle &r) { drawRectanglePC(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void drawRectanglePC(const TRectangle *r) { drawRectanglePC(*r); }
    void drawRectanglePC(const TPoint &a, const TPoint &b) { vdrawRectangle(a.x+0.5, a.y+0.5, b.x-a.x+1, b.y-a.y+1); }

    void drawRectangle(TCoord x,TCoord y,TCoord w,TCoord h) { vdrawRectangle(x, y, w, h); } 
    void drawRectangle(const TRectangle &r) { vdrawRectangle(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void drawRectangle(const TRectangle *r) { drawRectangle(*r); }
    void drawRectangle(const TPoint &a, const TPoint &b) { vdrawRectangle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void fillRectangle(TCoord x,TCoord y,TCoord w,TCoord h) { vfillRectangle(x, y, w, h); }
    void fillRectangle(const TRectangle &r) { vfillRectangle(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void fillRectangle(const TRectangle *r) { fillRectangle(*r); }
    void fillRectangle(const TPoint &a, const TPoint &b) { vfillRectangle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void fillRectanglePC(TCoord x,TCoord y,TCoord w,TCoord h);
    void fillRectanglePC(const TRectangle &r) { fillRectanglePC(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void fillRectanglePC(const TRectangle *r) { fillRectanglePC(*r); }
    void fillRectanglePC(const TPoint &a, const TPoint &b) { vfillRectangle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void drawCirclePC(TCoord x,TCoord y,TCoord w,TCoord h);
    void drawCirclePC(const TRectangle &r) { drawCirclePC(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void drawCirclePC(const TRectangle *r) { drawCirclePC(*r); }
    void drawCirclePC(const TPoint &a, const TPoint &b) { vdrawCircle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void drawCircle(TCoord x,TCoord y,TCoord w,TCoord h) { vdrawCircle(x, y, w, h); } 
    void drawCircle(const TRectangle &r) { vdrawCircle(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void drawCircle(const TRectangle *r) { drawCircle(*r); }
    void drawCircle(const TPoint &a, const TPoint &b) { vdrawCircle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void fillCircle(TCoord x,TCoord y,TCoord w,TCoord h) { vfillCircle(x, y, w, h); }
    void fillCircle(const TRectangle &r) { vfillCircle(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void fillCircle(const TRectangle *r) { fillCircle(*r); }
    void fillCircle(const TPoint &a, const TPoint &b) { vfillCircle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void fillCirclePC(TCoord x,TCoord y,TCoord w,TCoord h);
    void fillCirclePC(const TRectangle &r) { fillCirclePC(r.origin.x,r.origin.y,r.size.width,r.size.height); }
    void fillCirclePC(const TRectangle *r) { fillCirclePC(*r); }
    void fillCirclePC(const TPoint &a, const TPoint &b) { vfillCircle(a.x, a.y, b.x-a.x, b.y-a.y); }

    void drawArc(TCoord x,TCoord y,TCoord w,TCoord h, TCoord r1, TCoord r2) { vdrawArc(x, y, w, h, r1, r2); }
    void drawArc(const TRectangle &r, TCoord r1, TCoord r2) { vdrawArc(r.origin.x,r.origin.y,r.size.width,r.size.height, r1, r2); }
    void drawArc(const TRectangle *r, TCoord r1, TCoord r2) { drawArc(*r, r1, r2); }
    void drawArc(const TPoint &a, const TPoint &b, TCoord r1, TCoord r2) { vdrawArc(a.x, a.y, b.x-a.x, b.y-a.y, r1, r2); }

    void drawArcPC(TCoord x,TCoord y,TCoord w,TCoord h, TCoord r1, TCoord r2);
    void drawArcPC(const TRectangle &r, TCoord r1, TCoord r2) { drawArcPC(r.origin.x,r.origin.y,r.size.width,r.size.height, r1, r2); }
    void drawArcPC(const TRectangle *r, TCoord r1, TCoord r2) { drawArcPC(*r, r1, r2); }
    void drawArcPC(const TPoint &a, const TPoint &b, TCoord r1, TCoord r2) { vdrawArc(a.x, a.y, b.x-a.x, b.y-a.y, r1, r2); }

    void fillArc(TCoord x,TCoord y,TCoord w,TCoord h, TCoord r1, TCoord r2) { vfillArc(x, y, w, h, r1, r2); }
    void fillArc(const TRectangle &r, TCoord r1, TCoord r2) { vfillArc(r.origin.x,r.origin.y,r.size.width,r.size.height, r1, r2); }
    void fillArc(const TRectangle *r, TCoord r1, TCoord r2) { fillArc(*r, r1, r2); }
    void fillArc(const TPoint &a, const TPoint &b, TCoord r1, TCoord r2) { vfillArc(a.x, a.y, b.x-a.x, b.y-a.y, r1, r2); }

    void fillArcPC(TCoord x,TCoord y,TCoord w,TCoord h, TCoord r1, TCoord r2);
    void fillArcPC(const TRectangle &r, TCoord r1, TCoord r2) { fillArcPC(r.origin.x,r.origin.y,r.size.width,r.size.height, r1, r2); }
    void fillArcPC(const TRectangle *r, TCoord r1, TCoord r2) { fillArcPC(*r, r1, r2); }
    void fillArcPC(const TPoint &a, const TPoint &b, TCoord r1, TCoord r2) { vfillArc(a.x, a.y, b.x-a.x, b.y-a.y, r1, r2); }

    virtual void drawPolygon(const TPoint *points, size_t n) = 0;
    virtual void drawPolygon(const TPolygon &p) = 0;
    virtual void fillPolygon(const TPoint *points, size_t n) = 0;
    virtual void fillPolygon(const TPolygon &p) = 0;
    
    virtual void drawBezier(const TPoint *points, size_t n) = 0;
    virtual void drawBezier(const TPolygon &p) = 0;
    virtual void fillBezier(const TPoint *points, size_t n) = 0;
    virtual void fillBezier(const TPolygon &p) = 0;

    // 3D rectangle
    //-----------------------
    void vdraw3DRectangle(TCoord x, TCoord y, TCoord w, TCoord h, bool inset=true);

    void draw3DRectangle(TCoord x, TCoord y, TCoord w, TCoord h, bool inset=true) { vdraw3DRectangle(x,y,w,h,inset); }
    void draw3DRectangle(const TRectangle &r, bool inset=true) { vdraw3DRectangle(r.origin.x,r.origin.y,r.size.width,r.size.height,inset); }
    void draw3DRectangle(const TRectangle *r, bool inset=true) { draw3DRectangle(*r,inset); }
    void draw3DRectangle(const TPoint &a, const TPoint &b, double r1, double r2, bool inset=true) { vdraw3DRectangle(a.x, a.y, b.x-a.x, b.y-a.y, inset); }

    void draw3DRectanglePC(TCoord x, TCoord y, TCoord w, TCoord h, bool inset=true);
    void draw3DRectanglePC(const TRectangle &r, bool inset=true) { draw3DRectanglePC(r.origin.x,r.origin.y,r.size.width,r.size.height,inset); }
    void draw3DRectanglePC(const TRectangle *r, bool inset=true) { draw3DRectanglePC(*r,inset); }
    void draw3DRectanglePC(const TPoint &a, const TPoint &b, double r1, double r2, bool inset=true) { vdraw3DRectangle(a.x, a.y, b.x-a.x, b.y-a.y, inset); }

    void drawString(TCoord x, TCoord y, const char *str, size_t len) { vdrawString(x, y, str, len, true); }
    void drawString(TCoord x, TCoord y, const char *str) { vdrawString(x, y, str, strlen(str), true); }
    void drawString(TCoord x, TCoord y, const string &s) { vdrawString(x, y, s.c_str(), s.size(), true); }
    void fillString(TCoord x, TCoord y, const char *str, size_t len) { vdrawString(x, y, str, len, false); }
    void fillString(TCoord x, TCoord y, const char *str) { vdrawString(x, y, str, strlen(str), false); }
    void fillString(TCoord x, TCoord y, const string &s) { vdrawString(x, y, s.c_str(), s.size(), false); }

    int drawTextWidth(TCoord x, TCoord y, const string &text, unsigned width);
    static int getHeightOfTextFromWidth(TFont *font, const string &text, int width);
    static int textRatio(TCoord x,TCoord y,const string &str, unsigned width, TFont *font, TPenBase *pen);
    void drawBitmap(TCoord x, TCoord y, const TBitmap &bmp) { vdrawBitmap(x,y,bmp); }
    void drawBitmap(TCoord x, TCoord y, const TBitmap *bmp) { vdrawBitmap(x,y,*bmp); }

    static void poly2Bezier(const TPoint *src, size_t n, TPolygon &dst);
    static void poly2Bezier(const TPolygon &p, TPolygon &d);

    virtual void showPage();
};

}

#endif
