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

#import <AppKit/NSAttributedString.h>
#include <toad/core.hh>

using namespace toad;

TPen::TPen(TWindow *w)
{
  font = new TFont;
  linestyle = SOLID;
  window = w;
  ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
  windowmatrix = CGContextGetCTM(ctx);
  setColor(0,0,0);
  setAlpha(1);
}

TPen::TPen(TBitmap *)
{
}

TPen::~TPen()
{
}

void
TPen::identity()
{
  NSAffineTransform* xform = [NSAffineTransform transform];
  NSAffineTransformStruct m;
  m.m11 = windowmatrix.a;
  m.m12 = windowmatrix.b;
  m.m21 = windowmatrix.c;
  m.m22 = windowmatrix.d;
  m.tX  = windowmatrix.tx;
  m.tY  = windowmatrix.ty;
  [xform setTransformStruct: m];
  [xform set];
}

void
TPen::translate(TCoord dx, TCoord dy)
{
  CGContextTranslateCTM(ctx, dx, dy);
}

void
TPen::scale(TCoord sx, TCoord sy)
{
  CGContextScaleCTM(ctx, sx, sy);
}

void
TPen::rotate(TCoord radians)
{
  CGContextRotateCTM(ctx, radians);
}

void
TPen::multiply(const TMatrix2D *m)
{
  CGContextConcatCTM(ctx, *m);
}

void
TPen::push()
{
#if 1
  CGContextSaveGState(ctx);
#else
  mstack.push_back(CGContextGetCTM(ctx));
#endif
}

void
TPen::pop()
{
#if 1
  CGContextRestoreGState(ctx);
#else
  if (mstack.empty())
    return;
  NSAffineTransform* xform = [NSAffineTransform transform];
  const TMatrix2D &a = mstack.back();
  NSAffineTransformStruct m;
  m.m11 = a.a;
  m.m12 = a.b;
  m.m21 = a.c;
  m.m22 = a.d;
  m.tX  = a.tx;
  m.tY  = a.ty;
  [xform setTransformStruct: m];
  [xform set];
  mstack.pop_back();
#endif
}

void
TPen::setMatrix(const TMatrix2D &m)
{
  setMatrix(m.a, m.b, m.c, m.d, m.tx, m.ty);
}

void
TPen::setMatrix(TCoord a11, TCoord a12, TCoord a21, TCoord a22, TCoord tx, TCoord ty)
{
  identity();
  NSAffineTransform* xform = [NSAffineTransform transform];
  NSAffineTransformStruct m0;
  m0.m11 = a11;
  m0.m12 = a12;
  m0.m21 = a21;
  m0.m22 = a22;
  m0.tX  = tx;
  m0.tY  = ty;
  [xform setTransformStruct: m0];
  [xform concat];
}

/* could've returned NULL in the original TOAD... was that a good idea? */
const TMatrix2D*
TPen::getMatrix() const
{
  CGAffineTransform m0 = CGAffineTransformInvert(windowmatrix);
  CGAffineTransform m1 = CGContextGetCTM(ctx);
  static TMatrix2D m = CGAffineTransformConcat(m1, m0);
  return &m;
}

void
TPen::getClipBox(TRectangle *r) const
{
  if (window) {
    r->x = 0;
    r->y = 0;
    r->w = window->w;
    r->h = window->h;
  }
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::operator&=(const TRectangle &r)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
/*
  NSBezierPath* clipPath = [NSBezierPath bezierPath];
  [clipPath appendBezierPathWithRect: NSMakeRect(r.x, r.y, r.w, r.h)];
  [clipPath addClip];
*/
}

void
TPen::operator|=(const TRectangle &r)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
/*
  NSBezierPath* clipPath = [NSBezierPath bezierPath];
  [clipPath appendBezierPathWithRect: NSMakeRect(r.x, r.y, r.w, r.h)];
  [clipPath addClip];
*/
}

void
TPen::operator&=(const TRegion &r)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
/*
  NSBezierPath* clipPath = [NSBezierPath bezierPath];
  [clipPath appendBezierPathWithRect: NSMakeRect(r.x, r.y, r.w, r.h)];
  [clipPath addClip];
*/
}

void
TPen::operator|=(const TRegion &r)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
/*
  NSBezierPath* clipPath = [NSBezierPath bezierPath];
  [clipPath appendBezierPathWithRect: NSMakeRect(r.x, r.y, r.w, r.h)];
  [clipPath addClip];
*/
}

void
TPen::setFont(const string &)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::vsetColor(TCoord r, TCoord g, TCoord b) {
  stroke.r = fill.r = r;
  stroke.g = fill.g = g;
  stroke.b = fill.b = b;
  if (!ctx)
    return;
  CGContextSetRGBStrokeColor(ctx, stroke.r, stroke.g, stroke.b, stroke.a);
  CGContextSetRGBFillColor(ctx, fill.r, fill.g, fill.b, fill.a);
}

void
TPen::vsetLineColor(TCoord r, TCoord g, TCoord b) {
  stroke.r = r;
  stroke.g = g;
  stroke.b = b;
  CGContextSetRGBStrokeColor(ctx, stroke.r, stroke.g, stroke.b, stroke.a);
}

void
TPen::vsetFillColor(TCoord r, TCoord g, TCoord b) {
  fill.r = r;
  fill.g = g;
  fill.b = b;
  CGContextSetRGBFillColor(ctx, fill.r, fill.g, fill.b, fill.a);
}

void
TPen::setAlpha(TCoord a) {
  stroke.a = fill.a = a;
  CGContextSetRGBStrokeColor(ctx, stroke.r, stroke.g, stroke.b, stroke.a);
  CGContextSetRGBFillColor(ctx, fill.r, fill.g, fill.b, fill.a);
}

TCoord
TPen::getAlpha() const
{
  return stroke.a;
}


void
TPen::vdrawRectangle(TCoord x, TCoord y, TCoord w, TCoord h) {
  CGContextAddRect(ctx, CGRectMake(x, y, w, h));
  CGContextDrawPath(ctx, kCGPathStroke);
//  CGContextStrokePath(ctx);
}

void
TPen::vfillRectangle(TCoord x, TCoord y, TCoord w, TCoord h) {
  if (w<0) {
    x += w;
    w = -w;
  }
  if (h<0) {
    y += h;
    h = -h;
  }
  CGContextAddRect(ctx, CGRectMake(x, y, w, h));
  CGContextDrawPath(ctx, kCGPathFill);
}

void
TPen::vdrawCircle(TCoord x,TCoord y,TCoord w,TCoord h) {
  CGContextAddEllipseInRect(ctx, CGRectMake(x, y, w, h));
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::vfillCircle(TCoord x,TCoord y,TCoord w,TCoord h) {
  CGContextAddEllipseInRect(ctx, CGRectMake(x, y, w, h));
  CGContextDrawPath(ctx, kCGPathFill);
}

void
TPen::vdrawArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2) {
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::vfillArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2) {
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::vdrawBitmap(TCoord x, TCoord y, const TBitmap &b)
{
  if (b.img==nil)
    return;
  NSAffineTransform* xform = [NSAffineTransform transform];
  [xform translateXBy: x yBy: y+b.height];
  [xform scaleXBy: 1.0 yBy: -1.0];
  [xform concat];
//cerr << "TPen::vdrawBitmap: bitmap=" << &b << ", image=" << b.img << endl;
  [b.img drawAtPoint: NSMakePoint(0, 0)];
  [xform invert];
  [xform concat];
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
/*
NSImage* myImage = [NSImage imageNamed: @"/Pictures/image.jpg"];

NSImage* newImage = [[NSImage alloc] initWithSize: [myImage size]];
[newImage lockFocus];

[myImage compositeToPoint: NSMakePoint(0, 0) operation: NSCompositeCopy];
*/
}

void
TPen::vdrawString(TCoord x, TCoord y, char const *text, int len, bool transparent)
{
  char *t = 0;
  if (strlen(text)!=len) {
    t = strdup(text);
    t[len] = 0;
  }
//cerr<<"vdrawString("<<x<<","<<y<<",\""<<text<<"\","<<len<<","<<transparent<<")\n";
  if (!transparent) {
//cerr << "  not transparent" << endl;
    TRGBA stroke2 = stroke, fill2 = fill;
    setColor(fill2.r, fill2.g, fill2.b);    
    fillRectanglePC(x,y,getTextWidth(t?t:text),getHeight());
    setLineColor(stroke2.r, stroke2.g, stroke2.b);
    setFillColor(fill2.r, fill2.g, fill2.b);
  }
  NSDictionary *textAttributes =
    [NSDictionary
      dictionaryWithObject: 
        [NSColor colorWithDeviceRed: stroke.r 
                              green: stroke.g 
                              blue:  stroke.b
                              alpha: 1.0]
      forKey: NSForegroundColorAttributeName];
  [[NSString stringWithUTF8String: t?t:text]
    drawAtPoint: NSMakePoint(x, y-getDescent())
    withAttributes: textAttributes];
/*
  [[NSString stringWithUTF8String: text]
    drawAtPoint: NSMakePoint(x, y)
    withAttributes: [[NSGraphicsContext currentContext] attributes]];
*/
  if (t)
    free(t);
}

void
TPen::setLineStyle(ELineStyle style)
{
  this->linestyle = style;
  CGFloat f[6];
  switch(linestyle) {
    case SOLID:
      CGContextSetLineDash(ctx, 0, f, 0);
      break;     
    case DASH:   
      f[0] = 3.0; // painted
      f[1] = 1.0; // gap
      CGContextSetLineDash(ctx, 0, f, 2);
      break;     
    case DOT:  
      f[0] = 1.0; // painted
      f[1] = 1.0; // gap
      CGContextSetLineDash(ctx, 0, f, 2);
      break;  
    case DASHDOT:
      f[0] = 3.0; // painted
      f[1] = 1.0; // gap
      f[2] = 1.0; // painted
      f[3] = 1.0; // gap
      CGContextSetLineDash(ctx, 0, f, 4);
      break;
    case DASHDOTDOT:
      f[0] = 3.0; // painted
      f[1] = 1.0; // gap
      f[2] = 1.0; // painted
      f[3] = 1.0; // gap
      f[4] = 1.0; // painted
      f[5] = 1.0; // gap
      CGContextSetLineDash(ctx, 0, f, 6);
      break;
  }
}

void
TPen::setLineWidth(int w)
{
  if (w<0)
    w = -w;
  [NSBezierPath setDefaultLineWidth: w];
}

void
TPen::setMode(EMode)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::drawPoint(TCoord x, TCoord y)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::drawLines(TPoint const *p, size_t n)
{
  if (n<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  [path moveToPoint: NSMakePoint(p[0].x, p[0].y)];
  for(size_t i=1; i<n; ++i)
    [path lineToPoint: NSMakePoint(p[i].x, p[i].y)];
  [path stroke];
}

void
TPen::drawLines(TPolygon const &p)
{
  if (p.size()<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  [path moveToPoint: NSMakePoint(p[0].x, p[0].y)];
  for(size_t i=1; i<p.size(); ++i)
    [path lineToPoint: NSMakePoint(p[i].x, p[i].y)];
  [path stroke];
}

void
TPen::drawPolygon(const TPoint *p, size_t n)
{
  if (n<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  [path moveToPoint: NSMakePoint(p[0].x, p[0].y)];
  for(size_t i=1; i<n; ++i)
    [path lineToPoint: NSMakePoint(p[i].x, p[i].y)];
  [path closePath];
  [path stroke];
}

void
TPen::fillPolygon(TPoint const *p, size_t n)
{
  if (n<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  [path moveToPoint: NSMakePoint(p[0].x, p[0].y)];
  for(size_t i=1; i<n; ++i)
    [path lineToPoint: NSMakePoint(p[i].x, p[i].y)];
  [path closePath];
  [path fill];
}

void
TPen::drawPolygon(const TPolygon &polygon)
{
  if (polygon.size()<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  NSPoint np[polygon.size()];
  NSPoint *q = np;
  for(TPolygon::const_iterator p = polygon.begin();
      p != polygon.end();
      ++p, ++q)
  {
    q->x = p->x;
    q->y = p->y;
  }
  [path appendBezierPathWithPoints: np count:polygon.size()];
  [path closePath];
  [path stroke];
}

void
TPen::fillPolygon(const TPolygon &polygon)
{
  if (polygon.size()<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  NSPoint np[polygon.size()];
  NSPoint *q = np;
  for(TPolygon::const_iterator p = polygon.begin();
      p != polygon.end();
      ++p, ++q)
  {
    q->x = p->x;
    q->y = p->y;
  }
  [path appendBezierPathWithPoints: np count:polygon.size()];
  [path closePath];
  [path fill];
}

void
TPen::drawPolyBezier(const TPolygon &polygon)
{
  if (polygon.size()<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  NSPoint np[3];
  
  np[0].x = polygon[0].x;
  np[0].y = polygon[0].y;
  [path moveToPoint: np[0]];
  
  TPolygon::const_iterator p = polygon.begin();
  while(true) {
    ++p;
    if (p==polygon.end())
      break;
    np[1].x = p->x;
    np[1].y = p->y;
    ++p;
    if (p==polygon.end())
      break;
    np[2].x = p->x;
    np[2].y = p->y;
    ++p;
    if (p==polygon.end())
      break;
    np[0].x = p->x;
    np[0].y = p->y;
    [path curveToPoint: np[0] controlPoint1: np[1] controlPoint2: np[2]];
  }
#if 0
  [path appendBezierPathWithPoints: np count:polygon.size()];
#endif
  // [path closePath];
  [path stroke];
}

void
TPen::fillPolyBezier(const TPolygon &polygon)
{
  if (polygon.size()<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  NSPoint np[polygon.size()];
  NSPoint *q = np;
  for(TPolygon::const_iterator p = polygon.begin();
      p != polygon.end();
      ++p, ++q)
  {
    q->x = p->x;
    q->y = p->y;
  }
  [path appendBezierPathWithPoints: np count:polygon.size()];
  [path closePath];
  [path fill];
}

void
TPen::drawPolyBezier(const TPoint *p, size_t n)
{
  if (n<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  NSPoint np[n];
  NSPoint *q = np;
  const TPoint *e = p+n;
  for(; p!=e; ++p, ++q)
  {
    q->x = p->x;
    q->y = p->y;
  }
  [path appendBezierPathWithPoints: np count:n];
  [path closePath];
  [path stroke];
}

void
TPen::fillPolyBezier(const TPoint *p, size_t n)
{
  if (n<2)
    return;
  NSBezierPath *path = [NSBezierPath bezierPath];
  NSPoint np[n];
  NSPoint *q = np;
  const TPoint *e = p+n;
  for(; p!=e; ++p, ++q)
  {
    q->x = p->x;
    q->y = p->y;
  }
  [path appendBezierPathWithPoints: np count:n];
  [path closePath];
  [path fill];
}
