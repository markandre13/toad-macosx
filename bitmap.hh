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

#ifndef __TOAD_BITMAP_HH
#define __TOAD_BITMAP_HH 1

#include <toad/penbase.hh>
#include <toad/pointer.hh>
#include <string>

@class NSBitmapImageRep;

namespace toad {

using namespace std;

class TPen;

class TBitmap:
  public TSmartObject
{
    friend class TPen;
  public:
    NSBitmapImageRep *img;
    TBitmap() {
      img = nil;
      width = height = 0;
    }
    TBitmap(unsigned width, unsigned height) {
      img = nil;
      this->width = width;
      this->height = height;
    }
    ~TBitmap();
    bool load(const string &filename);
    bool load(istream&);
    unsigned getWidth() const { return width; }
    unsigned getHeight() const { return height; }
    void setPixel(TCoord x, TCoord y, TCoord r, TCoord g, TCoord b);
    void getPixel(TCoord x, TCoord y, TCoord *r, TCoord *g, TCoord *b);
    unsigned width, height;
  private:
    TBitmap(const TBitmap&) {}
};
  
typedef GSmartPointer<TBitmap> PBitmap;

} // namespace toad

#endif
