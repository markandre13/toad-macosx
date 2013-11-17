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
//  [[NSGraphicsContext currentContext] saveGraphicsState];
//  [[NSGraphicsContext currentContext] setShouldAntialias: NO];

  font = new TFont;
  setColor(0,0,0);
  linestyle = SOLID;
  window = w;

/*
  According to the manual, Cocoa already sets up a clip path during [NSView
  drawRect: <rect>], hence there shouldn't be a need to do this.
  
  clipPath = [NSBezierPath bezierPath];
  [clipPath appendBezierPathWithRect: NSMakeRect(0, 0, w->w, w->h)];
  [clipPath setClip];
 */ 
  mstack.push_back([NSAffineTransform transform]);

  NSAffineTransform* xform = [NSAffineTransform transform];
  int x, y;
  w->getOrigin(&x, &y);
  [xform translateXBy: x yBy: y];
  [xform concat];
}

TPen::TPen(TBitmap *)
{
}

TPen::~TPen()
{
  [NSBezierPath setDefaultLineWidth: 1];
//  [[NSGraphicsContext currentContext] restoreGraphicsState];

  // [clipPath release];
}

void
TPen::identity()
{
  NSAffineTransform* xform = [NSAffineTransform transform];
  for(mstack_t::const_iterator p = mstack.begin();
      p != mstack.end();
      ++p)
  {
    [xform appendTransform: *p];
  }
  [xform invert];
  [xform concat];
  [mstack.back() appendTransform: xform];
}

void
TPen::translate(TCoord dx, TCoord dy)
{
  NSAffineTransform* xform = [NSAffineTransform transform];
  [xform translateXBy: dx yBy: dy];
  [xform concat];
  [mstack.back() appendTransform: xform];
}

void
TPen::scale(TCoord sx, TCoord sy)
{
  NSAffineTransform* xform = [NSAffineTransform transform];
  [xform scaleXBy: sx yBy: sy];
  [xform concat];
  [mstack.back() appendTransform: xform];
}

void
TPen::rotate(TCoord radians)
{
  NSAffineTransform* xform = [NSAffineTransform transform];
  [xform rotateByRadians: radians];
  [xform concat];
  [mstack.back() appendTransform: xform];
}

void
TPen::multiply(const TMatrix2D *m)
{
  NSAffineTransform* xform = [NSAffineTransform transform];
  NSAffineTransformStruct m0;
  m0.m11 = m->a11;
  m0.m12 = m->a12;
  m0.m21 = m->a21;
  m0.m22 = m->a22;
  m0.tX  = m->tx;
  m0.tY  = m->ty;
  [xform setTransformStruct: m0];
  [xform concat];
  [mstack.back() appendTransform: xform];
}

void
TPen::push()
{
  mstack.push_back([NSAffineTransform transform]);
}

void
TPen::pop()
{
  [mstack.back() invert];
  [mstack.back() concat];
  mstack.pop_back();
}

void
TPen::setMatrix(TCoord a11, TCoord a21, TCoord a12, TCoord a22, TCoord tx, TCoord ty)
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
  [mstack.back() appendTransform: xform];
}

const TMatrix2D*
TPen::getMatrix() const
{
  static TMatrix2D m0;
  m0.identity();
  for(mstack_t::const_iterator p = mstack.begin();
      p != mstack.end();
      ++p)
  {
    TMatrix2D m1;
    NSAffineTransformStruct m2 = [*p transformStruct];
    m1.set(m2.m11, m2.m12, m2.m21, m2.m22, m2.tX, m2.tY);
    m0*=m1;
  }
  return &m0;
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
  [[NSColor colorWithDeviceRed: r green: g blue: b alpha: stroke.a] set];
}

void
TPen::vsetLineColor(TCoord r, TCoord g, TCoord b) {
  stroke.r = r;
  stroke.g = g;
  stroke.b = b;
  [[NSColor colorWithDeviceRed: r green: g blue: b alpha: stroke.a] setStroke];
}

void
TPen::vsetFillColor(TCoord r, TCoord g, TCoord b) {
  fill.r = r;
  fill.g = g;
  fill.b = b;
  [[NSColor colorWithDeviceRed: r green: g blue: b alpha: fill.a] setFill];
}

void
TPen::setAlpha(TCoord a) {
  stroke.a = fill.a = a;
  [[NSColor colorWithDeviceRed: stroke.r green: stroke.g blue: stroke.b alpha: stroke.a] setStroke];
  [[NSColor colorWithDeviceRed: fill.r green: fill.g blue: fill.b alpha: fill.a] setFill];
}

TCoord
TPen::getAlpha() const
{
  return stroke.a;
}


void
TPen::vdrawRectangle(TCoord x, TCoord y, TCoord w, TCoord h) {
  if (w<0) {
    x += w;
    w = -w;
  }
  if (h<0) {
    y += h;
    h = -h;
  }

  NSRect r = NSMakeRect(x,y,w,h);
  if (linestyle==SOLID) {
    [NSBezierPath strokeRect: r];
    return;
  }

  NSBezierPath *path = [NSBezierPath bezierPathWithRect: r];
  CGFloat f[6];
  switch(linestyle) {
    case SOLID:  
      break;     
    case DASH:   
      f[0] = 3.0; // painted
      f[1] = 1.0; // gap
      [path setLineDash: f count: 2 phase: 0.0];
      break;     
    case DOT:  
      f[0] = 1.0; // painted
      f[1] = 1.0; // gap
      [path setLineDash: f count: 2 phase: 0.0];
      break;  
    case DASHDOT:
      f[0] = 3.0; // painted
      f[1] = 1.0; // gap
      f[2] = 1.0; // painted
      f[3] = 1.0; // gap
      [path setLineDash: f count: 4 phase: 0.0];
      break;
    case DASHDOTDOT:
      f[0] = 3.0; // painted
      f[1] = 1.0; // gap
      f[2] = 1.0; // painted
      f[3] = 1.0; // gap
      f[4] = 1.0; // painted
      f[5] = 1.0; // gap
      [path setLineDash: f count: 6 phase: 0.0];
      break;
  }
  [path stroke];
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
  NSRect r = NSMakeRect(x,y,w,h);
  [NSBezierPath fillRect: r];
}

void
TPen::vdrawCircle(TCoord x,TCoord y,TCoord w,TCoord h) {
  NSRect r = NSMakeRect(x,y,w,h);
  NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect: r];
  [path stroke];
}

void
TPen::vfillCircle(TCoord x,TCoord y,TCoord w,TCoord h) {
  NSRect r = NSMakeRect(x,y,w,h);
  NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect: r];
  [path fill];
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
  switch(style) {
    case SOLID:
      break;
    case DASH:
      break;
    case DOT:
      break;
    case DASHDOT:
      break;
    case DASHDOTDOT:
      break;
  }
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
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
