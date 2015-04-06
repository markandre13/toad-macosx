/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public 
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
  Example for TOAD first data interface implementation:
*/

//#import <AppKit/NSAttributedString.h>
//#import <AppKit/NSPasteboard.h>
//@import AppKit;

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>

using namespace toad;

namespace {

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
};

// pattern & gradients experiment
// goal 1: manga patterns as in manga studio

// tone pattern:
//   lines  : free or 27.5, 30, 32.5, 42.5, 50, 55, 60, 65, 70, 85
//   gray   : 0-100%
//   degree :
//   shape  : circle, rectangle, line, cross, star, ...
// gradiant as pattern?
// whole layer to pattern? (let the printer do that, to gray is enough)
// patterns are just used for effects

// also try colored patterns

#define PSIZE 16    // size of the pattern cell
 
static void drawStar(void *info, CGContextRef ctx)
{
#if 0
    int k;
    double r, theta;
 
    CGContextSetRGBFillColor (myContext, 0, 0, 1, 1);

    r = 0.8 * PSIZE / 2;
    theta = 2 * M_PI * (2.0 / 5.0); // 144 degrees
 
    CGContextTranslateCTM (myContext, PSIZE/2, PSIZE/2);
 
    CGContextMoveToPoint(myContext, 0, r);
    for (k = 1; k < 5; k++) {
        CGContextAddLineToPoint (myContext,
                    r * sin(k * theta),
                    r * cos(k * theta));
    }
    CGContextClosePath(myContext);
    CGContextFillPath(myContext);
#else
  CGContextAddEllipseInRect(ctx, CGRectMake(4, 4, 8, 8));
  CGContextDrawPath(ctx, kCGPathFill);
  CGContextAddEllipseInRect(ctx, CGRectMake(-4, -4, 8, 8));
  CGContextDrawPath(ctx, kCGPathFill);
  CGContextAddEllipseInRect(ctx, CGRectMake(-4, 12, 8, 8));
  CGContextDrawPath(ctx, kCGPathFill);
  CGContextAddEllipseInRect(ctx, CGRectMake(12, 12, 8, 8));
  CGContextDrawPath(ctx, kCGPathFill);
  CGContextAddEllipseInRect(ctx, CGRectMake(12, -4, 8, 8));
  CGContextDrawPath(ctx, kCGPathFill);
#endif
}

void
TMyWindow::paint()
{
  TPen pen(this);

#if 0
  CGColorSpaceRef baseSpace = CGColorSpaceCreateDeviceRGB();
  CGColorSpaceRef patternSpace = CGColorSpaceCreatePattern(baseSpace);
  CGContextSetFillColorSpace(pen.ctx, patternSpace);
  CGColorSpaceRelease(patternSpace);
  CGColorSpaceRelease(baseSpace);
#else
  CGColorSpaceRef patternSpace = CGColorSpaceCreatePattern(NULL);
  CGContextSetFillColorSpace(pen.ctx, patternSpace);
  CGColorSpaceRelease(patternSpace);
#endif
  static const CGPatternCallbacks callbacks = {
    0,         // version
    &drawStar, // 
    NULL       // release info callback
  };

  CGPatternRef pattern = CGPatternCreate(
    NULL, 			     // void *info
    CGRectMake(0, 0, PSIZE, PSIZE),  // bounds
    CGAffineTransformIdentity,
    PSIZE, PSIZE,                    // xstep, ystep
    kCGPatternTilingConstantSpacing,
    true,                            // isColored
    &callbacks);

  CGFloat alpha = 1;
  CGContextSetFillPattern(pen.ctx, pattern, &alpha);
//  static const CGFloat color[4] = { 0, 0, 1, 1 };
//  CGContextSetFillPattern(pen.ctx, pattern, color);
  CGPatternRelease (pattern);
  
  CGContextAddRect(pen.ctx, CGRectMake(10, 10, 300, 180));
  CGContextDrawPath(pen.ctx, kCGPathFill);

  

#if 0
  TPen pen(this);
  TPath p;
  
  cout << "elements: " << p.size() << endl;
  
  p.move(TPoint(120, 20));
  cout << "elements: " << p.size() << endl;
  p.line(TPoint(300, 50));
  cout << "elements: " << p.size() << endl;
  p.curve(TPoint(310,190), TPoint(10,190), TPoint(50, 30));
  cout << "elements: " << p.size() << endl;
  p.close();
  cout << "elements: " << p.size() << endl;
  
  p.move(TPoint(1,2));
  
  for(int i=0; i<p.size(); ++i) {
    TPoint pts[3];
    TPath::EType type = p.point(i, pts);
    cout << i << ": " << type << " ";
    switch(p.point(i, pts)) {
      case TPath::CURVE:
        cout << pts[0] << ", " << pts[1] << ", " << pts[2] << endl;
        break;
      default:
        cout << pts[0] << endl;
        break;
    }
  }
  
  p.stroke();
  
  pen.setColor(1,0,0);
  pen.drawRectangle(p.bounds());
  pen.setColor(0,0,1);
  pen.drawRectangle(p.editBounds());
#endif
}

} // unnamed namespace

@interface foo: NSObject <NSPasteboardWriting>
{
}
@end

@implementation foo

// types this object can provide to the pasteboard
- (NSArray*) writableTypesForPasteboard: (NSPasteboard*)pasteboard
{
  cerr << "writableTypesForPasteboard" << endl;
  return [NSArray arrayWithObjects:
    [NSString stringWithUTF8String: "public.utf8-plain-text"],
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
  return [NSString stringWithUTF8String: "How cool is that?"];
}
@end

void
test_path()
{
  TMyWindow wnd(NULL, "test path");

  // com.adobe.pdf
  // public.tiff
  // public.png
  // public.utf8-plain-text
  
  NSPasteboard *pboard = [NSPasteboard generalPasteboard];
#if 0
  NSString *string = [NSString stringWithUTF8String: "Vielleicht"];
  [pboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType, nil] owner:nil];
  [pboard setString:string forType:NSStringPboardType];
#else
  [pboard clearContents];
  [pboard writeObjects:
    [NSArray arrayWithObjects: [[foo alloc] init], nil]
  ];
#endif
  toad::mainLoop();
}
