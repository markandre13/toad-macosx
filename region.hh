/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2003 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef __TOAD_REGION_HH
#define __TOAD_REGION_HH 1

#include <toad/types.hh>
#include <stdlib.h>

namespace toad {

#ifdef __WIN32__
#undef IN
#undef OUT
#undef PART
#endif

class TRegion
{
  public:
/*
    enum EInside {
      IN, OUT, PART
    };
*/
    TRegion();
    TRegion(const TRegion&);
    TRegion(int x, int y, int width, int height);
    TRegion(const TRectangle&);
    TRegion(const Box&);
    ~TRegion();

    bool isInside(const TRectangle &rectangle) const;
    void set(const TRectangle&);

    TRegion& operator=(const TRegion&);       // assign
    TRegion& operator&=(const TRegion&);      // union
    TRegion& operator|=(const TRegion&);      // intersect
    TRegion& operator-=(const TRegion&);      // subtract
    TRegion& operator^=(const TRegion&);      // xor
    TRegion& operator&=(const TRectangle&);   // union
    TRegion& operator|=(const TRectangle&);   // intersect
    TRegion& operator-=(const TRectangle&);   // subtract
    TRegion& operator^=(const TRectangle&);   // xor

    void translate(int dx,int dy);
    void clear();
    TRectangle getExtent() const;
    void getBoundary(TRectangle*) const;  // getExtents
    void getClipBox(TRectangle *r) const { getBoundary(r); }
    size_t getNumRects() const;
    bool getRect(size_t, TRectangle*) const;
    
    bool isEmpty() const;
    bool operator==(const TRegion &r) { return isEqual(r); }
    bool isEqual(const TRegion &r) const;
    bool isInside(int x, int y) const;
    // EInside isInside(const TRectangle &r) const;
    bool isIntersecting (const TRegion &region) const;
    bool isIntersecting(const TRectangle &rectangle) const;    

#ifdef __X11__
    _TOAD_REGION x11region;
#else
  private:
    enum {
      OPERATION_UNION,
      OPERATION_INTERSECTION,
      OPERATION_SUBTRACTION,
      OPERATION_XOR
    };

    static const int INITIAL_SIZE = 40; // 10 rectangles
  
    // temporary working area common for all regions for maximum performance
    static int    *gRectangles_;
    static size_t gLength_;
    static size_t gNRectangles_;
    static bool   isLocked_;

    Box           extent_;
    int           *rectangles_; // y0,y1,x0,x1,.....
    size_t        length_;
    size_t        nRectangles_;

    static void checkMemory(TRegion *region, size_t nRectangles);
    void collapse();

    /**
     * Perform a logical set operation between this and the specified
     * region. Corresponds to miRegionOp in Region.c of X11.
     * 
     * @param region         Region to combine with.
     * @param operationType  Combination operator.
     */
    void combine(const TRegion &region, int operationType);

    void nonOverlap1 (int rectangles[], int r, int rEnd,
                      int yTop, int yBottom, int operationType);
    void nonOverlap2 (int rectangles[], int r, int rEnd,
                      int yTop, int yBottom, int operationType);
    void overlap (int rectangles1[], int r1, int r1End,
                  int rectangles2[], int r2, int r2End,
                int yTop, int yBottom, int operationType);
    /**
     * Corresponds to miCoalesce in Region.c of X11.
     */
    int coalesceBands (int previousBand, int currentBand);

    /**
     * Update region extent based on rectangle values.
     */
    void updateExtent();
#endif
};

} // namespace toad

#endif
