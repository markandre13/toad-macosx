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

#include <toad/bitmap.hh>

#import <Cocoa/Cocoa.h>

using namespace toad;

TBitmap::~TBitmap()
{
  cerr << "release bitmap=" << this << ", image=" << img << endl;
  if (img)
    [img release];
}

bool
TBitmap::load(const string &fn)
{
//  cerr << "load bitmap file " << fn << ", bitmap=" << this << endl;
  NSError *err = nil;
  NSData *data = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String: fn.c_str()]
                         options: 0
                         error: &err];
  if (data==nil) {
    cerr << "TBitmap::load: failed to load file "
         << [[err localizedDescription] UTF8String] << endl;
    return false;
  }
  img = [NSBitmapImageRep imageRepWithData: data];
  if (img == nil) {
    cerr << "TBitmap::load: failed to decode image data" << endl;
    return false;
  }
  width = [img pixelsWide];
  height = [img pixelsHigh];

//  cerr << "  image=" << img << endl;
  
  [img retain];
  
  // don't release data, NSBitmapImageRep will parse it later
  // FIXME: find out if we have to release data
//  [data release];
  
  return true;
}

bool
TBitmap::load(istream&)
{
  return false;
}

void
TBitmap::setPixel(TCoord x, TCoord y, TCoord r, TCoord g, TCoord b)
{
  if (!img) {
    NSRect offscreenRect = NSMakeRect(0.0, 0.0, width, height);
    NSBitmapImageRep* offscreenRep = nil;
    img = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes: nil
            pixelsWide: offscreenRect.size.width
            pixelsHigh: offscreenRect.size.height
            bitsPerSample:8
            samplesPerPixel:4
            hasAlpha:YES
            isPlanar:NO
            colorSpaceName:NSCalibratedRGBColorSpace
            bitmapFormat:0
            bytesPerRow:(4 * offscreenRect.size.width)
            bitsPerPixel:32];
    CGContextRef ctx = (CGContextRef)[[NSGraphicsContext graphicsContextWithBitmapImageRep:img] graphicsPort];
    CGContextSetRGBFillColor(ctx, 0, 0, 0, 0);
    CGContextAddRect(ctx, CGRectMake(0, 0, width, height));
    CGContextDrawPath(ctx, kCGPathFill);
  }

  CGContextRef ctx = (CGContextRef)[[NSGraphicsContext graphicsContextWithBitmapImageRep:img] graphicsPort];
  CGContextSetRGBFillColor(ctx, r, g, b, 1.0);
  CGContextAddRect(ctx, CGRectMake(x, height - y - 1, 1, 1));
  CGContextDrawPath(ctx, kCGPathFill);
}
