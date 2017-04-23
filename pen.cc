/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2014 by Mark-André Hopf <mhopf@mark13.org>
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

//@import CoreFoundation;

// #import <AppKit/NSAttributedString.h>
// #include "CoreText/CoreText.h"
#include "CoreText/CTFontDescriptor.h"
#include <toad/core.hh>

using namespace toad;

TPen::TPen()
{
  font = 0;
  linestyle = SOLID;
  window = 0;
  ctx = 0;
  pdfContext = 0;
  clipboardData = 0;
}

void
TPen::init()
{
  font = new TFont;
  linestyle = SOLID;
  window = 0;
  ctx = 0;
  pdfContext = 0;
  clipboardData = 0;
}

void
TPen::initWindow(TWindow *w)
{
  init();
  window = w;

  ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
  if (!ctx) {
    cerr << "TPen(TWindow*) created outside TWindow::paint()" << endl;
  }
//  CGContextSetAllowsAntialiasing(ctx, FALSE);
  windowmatrix = CGContextGetCTM(ctx);
  setColor(0,0,0);
  setAlpha(1);
}

void
TPen::initPDFFile(const string &filename)
{
  init();
  CGRect pdfPageRect = CGRectMake(0,0,480*2, 640);

  CFStringRef path = CFStringCreateWithCString (NULL, filename.c_str(), kCFStringEncodingUTF8);

  CFURLRef url = CFURLCreateWithFileSystemPath (NULL, path, kCFURLPOSIXPathStyle, 0);
  CFRelease (path);

  CFMutableDictionaryRef myDictionary = CFDictionaryCreateMutable(NULL, 0,
                        &kCFTypeDictionaryKeyCallBacks,
                        &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(myDictionary, kCGPDFContextTitle, CFSTR("My PDF File"));
  CFDictionarySetValue(myDictionary, kCGPDFContextCreator, CFSTR("My Name"));

  pdfContext = CGPDFContextCreateWithURL(url, &pdfPageRect, myDictionary);
  if (!pdfContext) {
    cerr << "failed to open " << filename << endl;
    exit(EXIT_FAILURE);
  }

  CFRelease(myDictionary);
  CFRelease(url);

  pdfPageDictionary = CFDictionaryCreateMutable(NULL, 0,
                        &kCFTypeDictionaryKeyCallBacks,
                        &kCFTypeDictionaryValueCallBacks);
  pdfBoxData = CFDataCreate(NULL,(const UInt8 *)&pdfPageRect, sizeof (CGRect));
  CFDictionarySetValue(pdfPageDictionary, kCGPDFContextMediaBox, pdfBoxData);
  CGPDFContextBeginPage (pdfContext, pdfPageDictionary);

  CGAffineTransform m = { 1, 0, 0, -1, 0, pdfPageRect.size.height };
  CGContextConcatCTM(pdfContext, m);

  ctx = pdfContext;
  
  windowmatrix = CGContextGetCTM(ctx);
  setColor(0,0,0);
  setAlpha(1);
}

void
TPen::pagebreak()
{
  CGRect pdfPageRect = CGRectMake(0,0,480*2, 640);
  CGPDFContextEndPage(pdfContext);
  CGPDFContextBeginPage(pdfContext, pdfPageDictionary);
  CGAffineTransform m = { 1, 0, 0, -1, 0, pdfPageRect.size.height };
  CGContextConcatCTM(pdfContext, m);
}

@interface toadPDF2CB: NSObject <NSPasteboardWriting>
{
  NSData *data;
}
@end

@implementation toadPDF2CB

- (void)initWithData: (NSData*)inData;
{
  data = inData;
}

// types this object can provide to the pasteboard
- (NSArray*) writableTypesForPasteboard: (NSPasteboard*)pasteboard
{
  cerr << "writableTypesForPasteboard" << endl;
  return [NSArray arrayWithObjects:
    [NSString stringWithUTF8String: "com.adobe.pdf"],
    nil];
}

// we create the data 
- (NSPasteboardWritingOptions)writingOptionsForType: (NSString*)type
                                         pasteboard: (NSPasteboard*)pasteboard
{
  cerr << "writingOptionsForType" << endl;
  return NSPasteboardWritingPromised;
}

- (id)pasteboardPropertyListForType: (NSString *)type
{
  cerr << "pasteboardPropertyListForType " << [type UTF8String] << endl;
  return data;
}
@end

static size_t
putBytes(void *info, const void *bytes, size_t length)
{
  [(NSMutableData*)info appendBytes: bytes length: length];
  return length;
}

void
TPen::initClipboard(const TRectangle &clip)
{
cout << "initClipBoard( " << clip << " )" << endl;
  init();
  clipboardData = [[NSMutableData alloc] init];
  
  static CGDataConsumerCallbacks cb = { putBytes, 0 };
  CGDataConsumerRef dc = CGDataConsumerCreate(clipboardData, &cb);
  
  CFMutableDictionaryRef myDictionary = CFDictionaryCreateMutable(NULL, 0,
                        &kCFTypeDictionaryKeyCallBacks,
                        &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(myDictionary, kCGPDFContextTitle, CFSTR("My PDF File"));
  CFDictionarySetValue(myDictionary, kCGPDFContextCreator, CFSTR("My Name"));
  CGRect pdfPageRect = CGRectMake(clip.x, clip.y, clip.w, clip.h);
  pdfContext = CGPDFContextCreate(dc, &pdfPageRect, myDictionary);
  CFRelease(myDictionary);
  
  pdfPageDictionary = CFDictionaryCreateMutable(NULL, 0,
                        &kCFTypeDictionaryKeyCallBacks,
                        &kCFTypeDictionaryValueCallBacks);
  pdfBoxData = CFDataCreate(NULL,(const UInt8 *)&pdfPageRect, sizeof (CGRect));
  CFDictionarySetValue(pdfPageDictionary, kCGPDFContextMediaBox, pdfBoxData);
  CGPDFContextBeginPage(pdfContext, pdfPageDictionary);
cout << "h="<<pdfPageRect.size.height<<", y="<<pdfPageRect.origin.y<<endl;
  CGAffineTransform m = { 1, 0, 0, -1, 0, pdfPageRect.size.height+pdfPageRect.origin.y*2 };
  CGContextConcatCTM(pdfContext, m);

  ctx = pdfContext;
  windowmatrix = CGContextGetCTM(ctx);
  setColor(0,0,0);
  setAlpha(1);
}

void
TPen::initBitmap(TBitmap *)
{
  init();
}

TPen::~TPen()
{
  if (pdfContext) {
    CGPDFContextEndPage(pdfContext);
    CGContextRelease(pdfContext);
    CFRelease(pdfPageDictionary);
    CFRelease(pdfBoxData);
  }
  if (clipboardData) {
    NSPasteboard *pboard = [NSPasteboard generalPasteboard];
    [pboard clearContents];
    toadPDF2CB *f = [toadPDF2CB alloc];
    [f initWithData: clipboardData];
    [pboard writeObjects:
      [NSArray arrayWithObjects: f, nil]
    ];
  }
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
TPen::translate(const TPoint &vector)
{
  CGContextTranslateCTM(ctx, vector.x, vector.y);
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

// FIXME: could've returned NULL in the original TOAD... was that a good idea?
const TMatrix2D*
TPen::getMatrix() const
{
  CGAffineTransform m0 = CGAffineTransformInvert(windowmatrix);
  CGAffineTransform m1 = CGContextGetCTM(ctx);
  static TMatrix2D m;
  m = CGAffineTransformConcat(m1, m0);
#if 0
cout << "TPen::getMatrix m0" << endl
     << "  " << m0.a << ", " << m0.b << endl
     << "  " << m0.c << ", " << m0.d << endl
     << "  " << m0.tx << ", " << m0.ty << endl;

cout << "TPen::getMatrix m1" << endl
     << "  " << m1.a << ", " << m1.b << endl
     << "  " << m1.c << ", " << m1.d << endl
     << "  " << m1.tx << ", " << m1.ty << endl;


cout << "TPen::getMatrix =>" << endl
     << "  " << m.a << ", " << m.b << endl
     << "  " << m.c << ", " << m.d << endl
     << "  " << m.tx << ", " << m.ty << endl;
#endif
  return &m;
}

void
TPen::setClipRect(const TRectangle &r)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
//cout << "setClipRect(" << r.x << ", " << r.y << ", " << r.w << ", " << r.h << ")" << endl;
  if (window)
    CGContextClipToRect(ctx, CGRectMake(r.x, r.y, r.w, r.h));
/*
  NSBezierPath* clipPath = [NSBezierPath bezierPath];
  [clipPath appendBezierPathWithRect: NSMakeRect(r.x, r.y, r.w, r.h)];
  [clipPath addClip];
*/
}

void
TPen::getClipBox(TRectangle *r) const
{
#if 1
  if (window) {
    // FIXME: returning the rectangle from NSView drawRect: would be faster
    TRegion *region = window->getUpdateRegion();
    region->getBoundary(r);
  }
#else
  // FIXME: must be the rectangle given by NSView drawRect:
  if (window) {
    r->x = 0;
    r->y = 0;
    r->w = window->w;
    r->h = window->h;
  }
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
#endif
}

void
TPen::setFont(const string &fn)
{
  font->setFont(fn);
//  cerr << "TPen::setFont(\"" << fn << "\")" << endl;
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::vsetColor(TCoord r, TCoord g, TCoord b) {
  rgba_stroke.r = rgba_fill.r = r;
  rgba_stroke.g = rgba_fill.g = g;
  rgba_stroke.b = rgba_fill.b = b;
  if (!ctx)
    return;
  CGContextSetRGBStrokeColor(ctx, rgba_stroke.r, rgba_stroke.g, rgba_stroke.b, rgba_stroke.a);
  CGContextSetRGBFillColor(ctx, rgba_fill.r, rgba_fill.g, rgba_fill.b, rgba_fill.a);
}

void
TPen::vsetStrokeColor(TCoord r, TCoord g, TCoord b) {
  rgba_stroke.r = r;
  rgba_stroke.g = g;
  rgba_stroke.b = b;
  if (!ctx)
    return;
  CGContextSetRGBStrokeColor(ctx, rgba_stroke.r, rgba_stroke.g, rgba_stroke.b, rgba_stroke.a);
}

void
TPen::vsetFillColor(TCoord r, TCoord g, TCoord b) {
  rgba_fill.r = r;
  rgba_fill.g = g;
  rgba_fill.b = b;
  if (!ctx)
    return;
  CGContextSetRGBFillColor(ctx, rgba_fill.r, rgba_fill.g, rgba_fill.b, rgba_fill.a);
}

void
TPen::setAlpha(TCoord a) {
  rgba_stroke.a = rgba_fill.a = a;
  if (!ctx)
    return;
  CGContextSetRGBStrokeColor(ctx, rgba_stroke.r, rgba_stroke.g, rgba_stroke.b, rgba_stroke.a);
  CGContextSetRGBFillColor(ctx, rgba_fill.r, rgba_fill.g, rgba_fill.b, rgba_fill.a);
}

TCoord
TPen::getAlpha() const
{
  return rgba_stroke.a;
}

void
TPen::move(TCoord x, TCoord y)
{
  CGContextMoveToPoint(ctx, x, y);
}

void
TPen::line(TCoord x, TCoord y)
{
  CGContextAddLineToPoint(ctx, x, y);
}

void
TPen::move(const TPoint *pt)
{
  CGContextMoveToPoint(ctx, pt->x, pt->y);
}

void
TPen::line(const TPoint *pt)
{
  CGContextAddLineToPoint(ctx, pt->x, pt->y);
}

void
TPen::curve(const TPoint *pt)
{
  CGContextAddCurveToPoint(ctx, pt[0].x, pt[0].y, pt[1].x, pt[1].y, pt[2].x, pt[2].y);
}

void
TPen::close()
{
  CGContextClosePath(ctx);
}

void
TPen::stroke()
{
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::fill()
{
  CGContextDrawPath(ctx, kCGPathEOFill);
}

void
TPen::fillStroke()
{
  CGContextDrawPath(ctx, kCGPathFillStroke);
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
//  CGContextAddRect(ctx, CGRectMake(x, y, w, h));
//  CGContextDrawPath(ctx, kCGPathStroke);
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
//  CGContextAddEllipseInRect(ctx, CGRectMake(x, y, w, h));
//  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::vdrawArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2) {
cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TPen::vfillArc(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r1, TCoord r2) {
// FIXME  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
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
#if 0
  // create font descriptor
  CTFontRef font;
  CFStringRef fontname = CFSTR("Helvetica");
  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithNameAndSize(fontname, 1.2);
  font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);
  CFRelease(descriptor);

  // create attributes from the font
  CFStringRef keys[] = { kCTFontAttributeName };
  CFTypeRef values[] = { font };

  CFDictionaryRef attributes =
    CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
        (const void**)&values, 0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

  // create and attributed string from the attributes and the text
  CFStringRef text = CFStringCreateWithCString(NULL, aText, kCFStringEncodingUTF8);
  CFAttributedStringRef attrString = CFAttributedStringCreate(kCFAllocatorDefault, text, attributes);
  CFRelease(text);
  CFRelease(attributes);

  // create a line from the attributed string
  CTLineRef line = CTLineCreateWithAttributedString(attrString);

  // draw
  CGContextSetTextPosition(ctx, x, y);
  CTLineDraw(line, ctx);
  CFRelease(line);

  CFRelease(font);
#endif
#if 0
  // Cocoa
  NSDictionary *textAttributes =
    [NSDictionary dictionaryWithObjectsAndKeys:
      font->nsfont,
        NSFontAttributeName,
      [NSColor colorWithDeviceRed: stroke.r green: stroke.g blue: stroke.b alpha: 1.0],
        NSForegroundColorAttributeName,
      nil];
  NSString *nstr = [ [NSString alloc] 
                  initWithBytes: text
                         length: len
                       encoding: NSUTF8StringEncoding];

  if (!transparent) {
    NSSize size = [nstr sizeWithAttributes: textAttributes];
    TRGBA stroke2 = stroke, fill2 = fill;
    setColor(fill2.r, fill2.g, fill2.b);    
    fillRectanglePC(x,y,size.width,font->height);
    setStrokeColor(stroke2.r, stroke2.g, stroke2.b);
    setFillColor(fill2.r, fill2.g, fill2.b);
  }
  
  [nstr
    drawAtPoint: NSMakePoint(x, y)
    withAttributes: textAttributes];

  [nstr release];
#endif
#if 1

  CGColorRef cgcolor = CGColorCreateGenericRGB(rgba_stroke.r, rgba_stroke.g, rgba_stroke.b, rgba_stroke.a);

  // CoreText works better in flipped coordinate systems
  NSDictionary *textAttributes =
    [NSDictionary dictionaryWithObjectsAndKeys:
      font->nsfont,
        NSFontAttributeName,
#if 0
      [NSColor colorWithDeviceRed: rgba_stroke.r green: rgba_stroke.g blue: rgba_stroke.b alpha: rgba_stroke.a],
        NSForegroundColorAttributeName,
#else
      // use quartz color for pdf because there's no cocoa context
      cgcolor,
        NSForegroundColorAttributeName,
#endif
      nil];
  NSString *nstr = [ [NSString alloc] 
                  initWithBytes: text
                         length: len
                       encoding: NSUTF8StringEncoding];

  if (!transparent) {
    NSSize size = [nstr sizeWithAttributes: textAttributes];
    TRGBA rgba_stroke2 = rgba_stroke, rgba_fill2 = rgba_fill;
    setColor(rgba_fill2.r, rgba_fill2.g, rgba_fill2.b);
    setAlpha(1);
    fillRectanglePC(x,y,size.width,font->height);
    setStrokeColor(rgba_stroke2.r, rgba_stroke2.g, rgba_stroke2.b);
    setFillColor(rgba_fill2.r, rgba_fill2.g, rgba_fill2.b);
    setAlpha(rgba_stroke2.a);
  }

#if 1
  NSAttributedString *attrString = 
    [[NSAttributedString alloc]
      initWithString: nstr
      attributes:textAttributes];
#else
  CFAttributedStringRef attrString = CFAttributedStringCreate(
    NULL,
    nstr, // 
    textAttributes);
#endif
/*
  CFMutableAttributedStringRef attrString = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
  CFAttributedStringReplaceString(attrString, CFRangeMake(0, 0), (CFStringRef) nstr);
  CGContextSetTextMatrix(ctx, CGContextGetCTM(ctx));
*/
  CGAffineTransform m = { 1, 0, 0, -1, 0, 0 };
  CGContextSetTextMatrix(ctx, m);

  CTLineRef line = CTLineCreateWithAttributedString((CFAttributedStringRef)attrString);
  CGContextSetTextPosition(ctx, x, y + font->baseline);
  CTLineDraw(line, ctx);
  CFRelease(line);

  CGColorRelease(cgcolor);

  [nstr release];
  // [textAttributes release]; // managed via the autorelease pool
#endif
}

void
TPen::setLineStyle(ELineStyle style)
{
  this->linestyle = style;
  CGFloat f[6];
  TCoord w1 = linewidth;
  TCoord w3 = linewidth*3.0;
  switch(linestyle) {
    case SOLID:
      CGContextSetLineDash(ctx, 0, f, 0);
      break;     
    case DASH:   
      f[0] = w3; // painted
      f[1] = w1; // gap
      CGContextSetLineDash(ctx, 0, f, 2);
      break;     
    case DOT:  
      f[0] = w1; // painted
      f[1] = w1; // gap
      CGContextSetLineDash(ctx, 0, f, 2);
      break;  
    case DASHDOT:
      f[0] = w3; // painted
      f[1] = w1; // gap
      f[2] = w1; // painted
      f[3] = w1; // gap
      CGContextSetLineDash(ctx, 0, f, 4);
      break;
    case DASHDOTDOT:
      f[0] = w3; // painted
      f[1] = w1; // gap
      f[2] = w1; // painted
      f[3] = w1; // gap
      f[4] = w1; // painted
      f[5] = w1; // gap
      CGContextSetLineDash(ctx, 0, f, 6);
      break;
  }
}

void
TPen::setLineWidth(TCoord w)
{
  if (w<0)
    w = -w;
  if (w==0)
    w = 1;
  CGContextSetLineWidth(ctx, w);
  if (linewidth != w) {
    this->linewidth = w;
    setLineStyle(this->linestyle);
  }
}

void
TPen::setMode(EMode mode)
{
  switch(mode) {
    case NORMAL:
      CGContextSetBlendMode(ctx, kCGBlendModeNormal);
      break;
    case XOR:
      CGContextSetBlendMode(ctx, kCGBlendModeXOR);
      break;
    case INVERT:
      CGContextSetBlendMode(ctx, kCGBlendModeDifference);
      break;
  }
}

void
TPen::drawPoint(TCoord x, TCoord y)
{
  TCoord x0, y0, x1, y1, sx, sy;

  TMatrix2D m(CGAffineTransformInvert(CGContextGetCTM(ctx)));
  m.map(0, 0, &x0, &y0);
  m.map(1, 1, &x1, &y1);

  sx = x1-x0;
  sy = y0-y1;
  
//  x-=sx/2.0;
//  y-=sy/2.0;

  CGContextAddRect(ctx, CGRectMake(x, y, sx, sy));
  CGContextDrawPath(ctx, kCGPathFill);
}

void
TPen::drawLines(TPoint const *p, size_t n)
{
  CGContextAddLines(ctx, p, n);
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::drawLines(TPolygon const &p)
{
  CGContextAddLines(ctx, p.data(), p.size());
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::drawPolygon(const TPoint *p, size_t n)
{
  CGContextAddLines(ctx, p, n);
  CGContextClosePath(ctx);
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::fillPolygon(TPoint const *p, size_t n)
{
  CGContextAddLines(ctx, p, n);
  CGContextClosePath(ctx);
  CGContextDrawPath(ctx, kCGPathFill);
  CGContextAddLines(ctx, p, n);
  CGContextClosePath(ctx);
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::drawPolygon(const TPolygon &polygon)
{
  CGContextAddLines(ctx, polygon.data(), polygon.size());
  CGContextClosePath(ctx);
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::fillPolygon(const TPolygon &polygon)
{
  CGContextAddLines(ctx, polygon.data(), polygon.size());
  CGContextClosePath(ctx);
  CGContextDrawPath(ctx, kCGPathFill);
}

void
TPen::drawBezier(const TPolygon &polygon)
{
  drawBezier(polygon.data(), polygon.size());
}

void
TPen::fillBezier(const TPolygon &polygon)
{
  fillBezier(polygon.data(), polygon.size());
}

void
TPen::drawBezier(const TPoint *p, size_t n)
{
  if (n<4)
    return;
  
  CGContextMoveToPoint(ctx, p[0].x, p[0].y);
  ++p;
  --n;

  while(n>=3) {
    CGContextAddCurveToPoint(ctx, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
    p+=3;
    n-=3;
  }
  CGContextDrawPath(ctx, kCGPathStroke);
}

void
TPen::drawCurve(TCoord x0, TCoord y0, TCoord x1, TCoord y1, TCoord x2, TCoord y2, TCoord x3, TCoord y3)
{
  CGContextMoveToPoint(ctx, x0, y0);
  CGContextAddCurveToPoint(ctx, x1, y1, x2, y2, x3, y3);
  CGContextDrawPath(ctx, kCGPathStroke);
}


void
TPen::fillBezier(const TPoint *p, size_t n)
{
  if (n<4)
    return;
  
  CGContextMoveToPoint(ctx, p[0].x, p[0].y);
  ++p;
  --n;

  while(n>=3) {
    CGContextAddCurveToPoint(ctx, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
    p+=3;
    n-=3;
  }
  CGContextDrawPath(ctx, kCGPathFill);
}
