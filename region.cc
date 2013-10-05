/*
 * (C) 2004 - Geotechnical Software Services
 *            http://geosoft.no/
 *            Java Implementation based on X11 Region
 * (C) 2006 - Mark-André Hopf
 *            http://mark13.org/
 *            C++ Port
 * 
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, 
 * MA  02111-1307, USA.
 */

#include <toad/region.hh>

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <algorithm>

#include <iostream>
#include <toad/types.hh>

using namespace std;
using namespace toad;

/**
 * A <em>Region</em> is simply an area, as the name implies, and is
 * implemented as a so called "y-x-banded" array of rectangles; Each Region
 * is made up of a certain number of rectangles sorted by y coordinate first,
 * and then by x coordinate.
 * <p>
 * Furthermore, the rectangles are banded such that every rectangle with a
 * given upper-left y coordinate (y1) will have the same lower-right y
 * coordinate (y2) and vice versa. If a rectangle has scanlines in a band,
 * it will span the entire vertical distance of the band. This means that
 * some areas that could be merged into a taller rectangle will be represented
 * as several shorter rectangles to account for shorter rectangles to its
 * left or right but within its "vertical scope".
 * <p>
 * An added constraint on the rectangles is that they must cover as much
 * horizontal area as possible. E.g. no two rectangles in a band are allowed
 * to touch. Whenever possible, bands will be merged together to cover a
 * greater vertical distance (and thus reduce the number of rectangles).
 * Two bands can be merged only if the bottom of one touches the top of the
 * other and they have rectangles in the same places (of the same width, of
 * course). This maintains the y-x-banding.
 * <p>
 * Region operations includes add (union), subtract, intersect, and
 * exclusive-or.
 * <p>
 * This class corresponds to Region.c of the X11 distribution and the
 * implementation is based on it.
 * <p>
 * The <em>Region</em> is essentially equivalent to an AWT <em>Area</em>
 * but with different back-end implementation. Becnhmarking proves it more
 * than 100 times faster than AWT Area for binary CAG operations,
 * <p>
 * Thanks to:
 * <ul>
 * <li>Bryan Lin @ China Minmetals Corporation - for identifying
 *     synchronization errors when run on the MS WindowsXP platform.
 * <li>Maxim Butov @ Belhard - for identifying error in the
 *     isInside(Rect) method.
 * </ul>
 *
 * @author <a href="mailto:jacob.dreyer@geosoft.no">Jacob Dreyer</a>
 */   

/**
 * Create an empty region. Corresponds to XCreateRegion of X11.
 */
TRegion::TRegion()
{
  rectangles_  = new int[INITIAL_SIZE];
  length_      = INITIAL_SIZE;
  nRectangles_ = 0;
}
  
/**
 * Create a region constituting of a single rectangle as specified.
 * 
 * @param rectangle  Rectangle to create region from.
 */
TRegion::TRegion(const TRectangle &rectangle)
{
  rectangles_ = 0;
  set(rectangle);
}

/**
 * Create a region consisting of one rectangle as specified.
 * 
 * @param x       X position of upper left corner of rectangle.
 * @param y       Y position of upper left corner of rectangle.
 * @param width   Width of rectangle.
 * @param height  Height of rectangle.
 */
TRegion::TRegion(int x, int y, int width, int height)
{
  rectangles_ = 0;
  *this = TRectangle(x, y, width, height);
}

/**
 * Create a region consisting of one rectangle as specified.
 * 
 * @param box  Box specification of rectangle to create region from.
 */
TRegion::TRegion(const Box &box)
{
  rectangles_ = 0;
  *this = TRectangle(box);
}

/**
 * Create a region as a copy of the specified region.
 * 
 * @param region  Region to copy.
 */
TRegion::TRegion(const TRegion &region)
{
  rectangles_ = new int[region.nRectangles_ << 2];
  length_ = region.nRectangles_ << 2;
  *this = region;
}

TRegion::~TRegion() {
  delete[] rectangles_;
}


/**
 * Set the content of this region according to the specified
 * region.
 * 
 * @param region  Region to copy.
 */
TRegion&
TRegion::operator=(const TRegion &region)
{
  extent_ = region.extent_;

  checkMemory(this, region.nRectangles_);
  memcpy(rectangles_, region.rectangles_, region.nRectangles_*4*sizeof(int));
  nRectangles_ = region.nRectangles_;
  return *this;
}
  
/**
 * Set the content of this region according to the specified
 * rectangle.
 *
 * @param rectangle  Rectangle to set region according to.
 */
void
TRegion::set(const TRectangle &rectangle)
{
  if (rectangles_)
    delete[] rectangles_;
  rectangles_ = new int[INITIAL_SIZE];
  length_ = INITIAL_SIZE;

  if (rectangle.isEmpty()) {
    nRectangles_ = 0;
  } else {
    extent_.set(rectangle);
    rectangles_[0] = extent_.y1;
    rectangles_[1] = extent_.y2;    
    rectangles_[2] = extent_.x1;
    rectangles_[3] = extent_.x2;    
    nRectangles_ = 1;
  }
}

/**
 * Clear the region.
 */
void
TRegion::clear()
{
  nRectangles_ = 0;
  extent_.set(0, 0, 0, 0);
}

  /**
   * Return true if this region equals the specified object.
   * Corrensponds to XEqualRegion of X11.
   * 
   * @param object  Object to check against.
   * @return        True if the two regions equals, false otherwise.
   * @throws        ClassCastException if object is not of type Region.
   */
bool
TRegion::isEqual(const TRegion &region) const
{
  if      (nRectangles_ != region.nRectangles_) return false;
  else if (nRectangles_ == 0)                   return true;
  else if (extent_.x1 != region.extent_.x1)     return false;
  else if (extent_.x2 != region.extent_.x2)     return false;
  else if (extent_.y1 != region.extent_.y1)     return false;
  else if (extent_.y2 != region.extent_.y2)     return false;
  else {
    for (size_t i = 0; i < nRectangles_ << 2; i++)
      if (rectangles_[i] != region.rectangles_[i]) return false;
  }
    
  return true;
}

/**
 * Return true if the region is empty. Corresponds to XEmptyRegion in X11.
 * 
 * @return  True if the region is empty, false otherwise.
 */
bool
TRegion::isEmpty() const
{
  return nRectangles_ == 0;
}

/**
 * Offset the entire region a specified distance.
 * Corresponds to XOffsetRegion in X11.
 * 
 * @param dx  Offset in x direction.
 * @param dy  Offset in y direction.
 */
void
TRegion::translate(int dx, int dy)
{
  for (size_t i = 0; i < length_; i+=4) {
    rectangles_[i+0] += dy;
    rectangles_[i+1] += dy;      
    rectangles_[i+2] += dx;
    rectangles_[i+3] += dx;      
  }
  extent_.translate(dx, dy);
}

/**
 * Return true if the specified region intersect this region.
 * 
 * @param region  Region to check against.
 * @return        True if the region intersects this one, false otherwise.
 */
bool
TRegion::isIntersecting (const TRegion &region) const
{
  TRegion r(*this);
  r &= region;
  return r.isEmpty();
}

/**
 * Return true if the specified rectangle intersect this region.   
 * 
 * @param rectangle  Rectangle to check against.
 * @return           True if the rectangle intersects this, false otherwise.
 */
bool
TRegion::isIntersecting(const TRectangle &rectangle) const
{
  return isIntersecting(TRegion(rectangle));
}

#if 0
  /**
   * Return true if the specified point is inside this region.
   * This method corresponds to XPointInRegion in X11.
   * 
   * @param x  X part of point to check.
   * @param y  Y part of point to check.
   * @return   True if the point is inside the region, false otherwise.
   */
  bool isInside(int x, int y) const;

  /**
   * Return true if the specified rectangle is inside this region.
   * This method corresponds to XRectInRegion in X11.   
   * 
   * @param rectangle  Rectangle to check.
   * @return           True if the rectangle is inside this region,
   *                   false otherwise.
   */
  bool isInside (const Rect &rectangle) const;
  
  /**
   * Return true if this region is inside of the specified rectangle.
   * 
   * @param rectangle  Rectangle to check if this is inside of.
   * @return           True if this region is inside the specified rectangle,
   *                   false otherwise.
   */
  bool isInsideOf(const Rect &rectangle) const
  {
    TRegion r(*this);
    r -= rectangle;
    return r.isEmpty();
  }
#endif

/**
 * Return the extent of the region.
 * Correspond to XClipBox in X11.
 * 
 * @return  The extent of this region.
 */
#if 0
TRectangle
TRegion::getExtent() const
{
  return TRect(extent_);
}
#endif

void
TRegion::getBoundary(TRectangle *r) const
{
  cerr << __FILE__ << ":" << __LINE__ << " not implemented\n";
  exit(1);
}

/**
 * Return the number of rectangles in the region. In case the number
 * is getting very high, the application might choose to call collapse().
 *
 * @return  Number of rectangles this region consists of.
 */
size_t
TRegion::getNumRects() const
{
  return nRectangles_;
}

bool
TRegion::getRect(size_t i, TRectangle *r) const
{
  if (i>=nRectangles_)
    return false;
  i<<=2;
  r->x = rectangles_[i];
  r->y = rectangles_[i+2];
  r->w = rectangles_[i+3] - rectangles_[i+2];
  r->h = rectangles_[i+1] - rectangles_[i];
  return true;
}

/**
 * Collapse the region into its extent box. Useful if the region becomes
 * very complex (number of rectangles is getting high) and the client
 * accepts the (in general) coarser result region.
 */
void
TRegion::collapse()
{
  rectangles_[0] = extent_.y1;
  rectangles_[1] = extent_.y2;    
  rectangles_[2] = extent_.x1;
  rectangles_[3] = extent_.x2;
  nRectangles_ = 1;
}


TRegion&
TRegion::operator|=(const TRectangle &rectangle)
{
  if (rectangle.isEmpty())
    return *this;
  return *this |= TRegion(rectangle);
}

TRegion&
TRegion::operator&=(const TRegion &region)
{
  // Trivial case which results in an empty region
  if (isEmpty() || region.isEmpty() ||
      !extent_.isOverlapping (region.extent_)) {
    clear();
    return *this;
  }

  // General case
  combine(region, OPERATION_INTERSECTION);

  // Update extent
  updateExtent();
  return *this;
}

/**
 * Leave this region as the intersection between this region and the
 * specified rectangle.
 * 
 * @param rectangle  Rectangle to intersect this with.
 */
TRegion&
TRegion::operator&=(const TRectangle &rectangle)
{
  if (rectangle.isEmpty()) {
    clear();
  } else {
    *this &= TRegion(rectangle);
  }
  return *this;
}

/**
 * Subtract the specified region from this region.
 * Corresponds to XSubtractRegion in X11.
 * 
 * @param region  Region to subtract from this region.
 */
TRegion&
TRegion::operator-=(const TRegion &region)
{
  // Trivial check for non-op
  if (isEmpty() || region.isEmpty() || !extent_.isOverlapping (region.extent_))
    return *this;

  // General case
  combine(region, OPERATION_SUBTRACTION);

  // Update extent
  updateExtent();
  return *this;
}

/**
 * Subtract the specified rectangle from this region.
 * 
 * @param rectangle  Rectangle to subtract from this region.
 */
TRegion&
TRegion::operator-=(const TRectangle &rectangle)
{
  if (rectangle.isEmpty())
    return *this;
  *this -= TRegion(rectangle);
  return *this;
}

/**
 * Leave the exclusive-or between this and the specified region in
 * this region. Corresponds to the XXorRegion in X11.
 * 
 * @param region  Region to xor this region with.
 */
TRegion&
TRegion::operator^=(const TRegion &region)
{
  TRegion r(region);
  r -= *this;
  *this -= region;
  *this |= r;
  return *this;
}

/**
 * Leave the exclusive-or between this and the specified rectangle in
 * this region.
 * 
 * @param rectangle  Rectangle to xor this region with.
 */
TRegion&
TRegion::operator^=(const TRectangle &rectangle)
{
  if (rectangle.isEmpty()) {
    clear();
  } else {
    *this ^= TRegion(rectangle);
  }
  return *this;
}

int*   TRegion::gRectangles_   = new int[INITIAL_SIZE];
size_t TRegion::gLength_       = TRegion::INITIAL_SIZE;
size_t TRegion::gNRectangles_  = 0;
bool   TRegion::isLocked_      = false;

void
TRegion::checkMemory(TRegion *region, size_t nRectangles)
{
  size_t nEntries = nRectangles << 2;

  if (region == 0) {
    if (gLength_ < nEntries) {
      size_t newSize = nEntries * 2;
      int *newArray = new int[newSize];
      memcpy(newArray, gRectangles_, gLength_*sizeof(int));
      delete[] gRectangles_;
      gRectangles_ = newArray;
      gLength_ = newSize;
    }
  } else {
    if (region->length_ < nEntries) {
      size_t newSize = nEntries * 2;
      int *newArray = new int[newSize];
      memcpy(newArray, region->rectangles_, region->length_*sizeof(int));
      delete[] region->rectangles_;
      region->rectangles_ = newArray;
      region->length_ = newSize;
    }
  }
}

bool
TRegion::isInside(int x, int y) const
{
  if (isEmpty())
    return false;
  
  if (!extent_.isInside (x, y))
    return false;

  size_t rEnd = nRectangles_ << 2;
  
  // Find correct band
  size_t i = 0;
//printf("find correct band for %i, %i\n", x, y);
//printf("  rectangles_[i+1]=%d\n", rectangles_[i+1]);
  while (i < rEnd && rectangles_[i+1] <= y) {
//printf("  rectangles_[i]=%d\n", rectangles_[i]);
    if (rectangles_[i] > y)
      return false; // Passed the band
    i += 4;
  }

  // Check each rectangle in the band
  while (i < rEnd && rectangles_[i] <= y) {
    if (x >= rectangles_[i+2] && x < rectangles_[i+3]) {
//      printf("rectangles_[i]=rectangles_[i]\n", rectangles_[i]);
//      printf("%d, %d is inside\n", x, y);
      return true;
    }
    i += 4;
  }

  return false;
}

bool
TRegion::isInside(const TRectangle &rectangle) const
{
  // Trivial reject case 1 
  if (isEmpty() || rectangle.isEmpty())
    return false;

  // Trivial reject case 2
  if (!extent_.isOverlapping (rectangle))
    return false;

  int x1 = rectangle.x;
  int x2 = rectangle.x + rectangle.w;
  int y1 = rectangle.y;
  int y2 = rectangle.y + rectangle.h;

  int rEnd = nRectangles_ << 2;

  // Trivial reject case 3
  if (rectangles_[0] > y1) return false;
  
  // Loop to start band
  int i = 0;
  while (i < rEnd && rectangles_[i+1] <= y1) {
    i += 4;
    if (rectangles_[i] > y1) return false;
  }
  
  while (i < rEnd) {
    int yTop    = rectangles_[i];
    int yBottom = rectangles_[i+1];      
    
    // Find start rectangle within band
    while (i < rEnd && rectangles_[i+3] <= x1) {
      i += 4;
      if (rectangles_[i] > yTop) return false; // Passed the band
    }
    
    if (i == rEnd) return false;
    
    // This rectangle must cover the entire rectangle horizontally
    if (x1 < rectangles_[i+2] || x2 > rectangles_[i+3]) return false;
    
    // See if we are done
    if (rectangles_[i+1] >= y2) return true;
    
    // Move to next band
    i += 4;
    while (i < rEnd && rectangles_[i] == yTop)
      i += 4;

    if (i == rEnd) return false;

    if (rectangles_[i] > yBottom) return false;
  }

  return false;
}

void
TRegion::combine(const TRegion &region, int operationType)
{
//printf("combine\n");
  // This is the only method (with sub methods) that utilize the
  // common working area gRectangles_. The lock ensures that only
  // one thread access this variable at any time.
  while (isLocked_); // OUCH!!!
  isLocked_ = true;

  size_t r1 = 0;
  size_t r2 = 0;
  size_t r1End = nRectangles_        << 2;
  size_t r2End = region.nRectangles_ << 2;

  // Initialize the working region
  gNRectangles_ = 0;

  int yTop    = 0;
  int yBottom = extent_.y1 < region.extent_.y1 ?
                extent_.y1 : region.extent_.y1;

  size_t previousBand = 0;
  size_t currentBand;

  size_t r1BandEnd, r2BandEnd;
  int top, bottom;
  
  // Main loop
  do {
    currentBand = gNRectangles_;

    // Find end of the current r1 band
    r1BandEnd = r1 + 4;
    while (r1BandEnd != r1End &&
           rectangles_[r1BandEnd] == rectangles_[r1])
      r1BandEnd += 4;

    // Find end of the current r2 band
    r2BandEnd = r2 + 4;
    while (r2BandEnd != r2End &&
           region.rectangles_[r2BandEnd] == region.rectangles_[r2])
      r2BandEnd += 4;

    // First handle non-intersection band if any
    if (rectangles_[r1] < region.rectangles_[r2]) {
      top    = max (rectangles_[r1],    yBottom);
      bottom = min (rectangles_[r1+1],  region.rectangles_[r2]);

      if (top != bottom)
        nonOverlap1 (rectangles_, r1, r1BandEnd, top, bottom, operationType);

      yTop = region.rectangles_[r2];
    }
    else if (region.rectangles_[r2] < rectangles_[r1]) {
      top    = max (region.rectangles_[r2],   yBottom);
      bottom = min (region.rectangles_[r2+1], rectangles_[r1]);

      if (top != bottom)
        nonOverlap2 (region.rectangles_,
                     r2, r2BandEnd, top, bottom, operationType);

      yTop = rectangles_[r1];
    }
    else
      yTop = rectangles_[r1];
    
    // Then coalesce if possible
    if (gNRectangles_ != currentBand)
      previousBand = coalesceBands (previousBand, currentBand);
    currentBand = gNRectangles_;

    // Check if this is an intersecting band
    yBottom = min (rectangles_[r1+1], region.rectangles_[r2+1]);
    if (yBottom > yTop)
      overlap (rectangles_,        r1, r1BandEnd,
               region.rectangles_, r2, r2BandEnd,
               yTop, yBottom, operationType);

    // Coalesce again
    if (gNRectangles_ != currentBand)
      previousBand = coalesceBands (previousBand, currentBand);

    // If we're done with a band, skip forward in the region to the next band
    if (rectangles_[r1+1]        == yBottom) r1 = r1BandEnd;
    if (region.rectangles_[r2+1] == yBottom) r2 = r2BandEnd;

  } while (r1 != r1End && r2 != r2End);

  currentBand = gNRectangles_;
  
  //
  // Deal with whichever region still has rectangles left
  //
  if (r1 != r1End) {
    do {

      r1BandEnd = r1;
      while (r1BandEnd < r1End &&
             rectangles_[r1BandEnd] == rectangles_[r1])
        r1BandEnd += 4;

      top    = max (rectangles_[r1], yBottom);
      bottom = rectangles_[r1+1];
      
      nonOverlap1 (rectangles_, r1, r1BandEnd, top, bottom, operationType);
      r1 = r1BandEnd;
      
    } while (r1 != r1End);
  }
  else if (r2 != r2End) {
    do {

      r2BandEnd = r2;
      while (r2BandEnd < r2End &&
             region.rectangles_[r2BandEnd] == region.rectangles_[r2])
        r2BandEnd += 4;

      top    = max (region.rectangles_[r2], yBottom);
      bottom = region.rectangles_[r2+1];
      
      nonOverlap2 (region.rectangles_, r2, r2BandEnd, top, bottom,
                   operationType);
      r2 = r2BandEnd;
      
    } while (r2 != r2End);
  }

  // Coalesce again
  if (currentBand != gNRectangles_)
    coalesceBands(previousBand, currentBand);

  // Copy the work region into this
  checkMemory (this, gNRectangles_);
  memcpy(rectangles_, gRectangles_, gNRectangles_*4*sizeof(int));
  nRectangles_ = gNRectangles_;
#if 0
printf("number of rectangles: %u\n", nRectangles_);
for(size_t i=0; i<nRectangles_*4; i+=4) {
  printf( "%i %i %i %i\n", rectangles_[i],
                           rectangles_[i+1],
                           rectangles_[i+2],
                           rectangles_[i+3]);
}
#endif
  isLocked_ = false;
}

void
TRegion::updateExtent()
{
  if (nRectangles_ == 0)
    extent_.set (0, 0, 0, 0);
  else {
    // Y values
    extent_.y1 = rectangles_[0];
    extent_.y2 = rectangles_[(nRectangles_ << 2) - 3];

    // X values initialize
    extent_.x1 = rectangles_[2];
    extent_.x2 = rectangles_[3];
    
    // Scan all rectangles for extreme X values
    for (size_t i = 4; i < nRectangles_ << 2; i += 4) {
      if (rectangles_[i+2] < extent_.x1) extent_.x1 = rectangles_[i+2];
      if (rectangles_[i+3] > extent_.x2) extent_.x2 = rectangles_[i+3];
    }
  }
}

void 
TRegion::nonOverlap1(int rectangles[], int r, int rEnd,
                    int yTop, int yBottom, int operationType)
{
  int i = gNRectangles_ << 2;
  
  if (operationType == OPERATION_UNION ||
      operationType == OPERATION_SUBTRACTION) {
    while (r != rEnd) {
      checkMemory (NULL, gNRectangles_ + 1);

      gRectangles_[i] = yTop;            i++;
      gRectangles_[i] = yBottom;         i++;
      gRectangles_[i] = rectangles[r+2]; i++;
      gRectangles_[i] = rectangles[r+3]; i++;
      gNRectangles_++;
      r += 4;
    }
  }
}

void
TRegion::nonOverlap2(int rectangles[], int r, int rEnd,
                     int yTop, int yBottom, int operationType)
{
  int i = gNRectangles_ << 2;
  
  if (operationType == OPERATION_UNION) {
    while (r != rEnd) {
      checkMemory (NULL, gNRectangles_ + 1);
      gRectangles_[i] = yTop;            i++;
      gRectangles_[i] = yBottom;         i++;
      gRectangles_[i] = rectangles[r+2]; i++;
      gRectangles_[i] = rectangles[r+3]; i++;

      gNRectangles_++;
      r += 4;
    }
  }
}

void 
TRegion::overlap(int rectangles1[], int r1, int r1End,
                 int rectangles2[], int r2, int r2End,
                 int yTop, int yBottom, int operationType)
{
  int i = gNRectangles_ << 2;

  //
  // UNION
  //
  if (operationType == OPERATION_UNION) {
    while (r1 != r1End && r2 != r2End) {
      if (rectangles1[r1+2] < rectangles2[r2+2]) {
        if (gNRectangles_ > 0            &&
            gRectangles_[i-4] == yTop    &&
            gRectangles_[i-3] == yBottom &&
            gRectangles_[i-1] >= rectangles1[r1+2]) {
          if (gRectangles_[i-1] < rectangles1[r1+3])
            gRectangles_[i-1] = rectangles1[r1+3];
        }
        else {
          checkMemory (NULL, gNRectangles_ + 1);
          
          gRectangles_[i]   = yTop;
          gRectangles_[i+1] = yBottom;
          gRectangles_[i+2] = rectangles1[r1+2];
          gRectangles_[i+3] = rectangles1[r1+3];
          
          i += 4;
          gNRectangles_++;
        }

        r1 += 4;
      }
      else {
        if (gNRectangles_ > 0            &&
            gRectangles_[i-4] == yTop    &&
            gRectangles_[i-3] == yBottom &&
            gRectangles_[i-1] >= rectangles2[r2+2]) {
          if (gRectangles_[i-1] < rectangles2[r2+3])
            gRectangles_[i-1] = rectangles2[r2+3];
        }
        else {
          checkMemory (NULL, gNRectangles_ + 1);

          gRectangles_[i]   = yTop;
          gRectangles_[i+1] = yBottom;
          gRectangles_[i+2] = rectangles2[r2+2];
          gRectangles_[i+3] = rectangles2[r2+3];

          i += 4;
          gNRectangles_++;
        }

        r2 += 4;
      }
    }

    if (r1 != r1End) {
      do {
        if (gNRectangles_ > 0            &&
            gRectangles_[i-4] == yTop    &&
            gRectangles_[i-3] == yBottom &&
            gRectangles_[i-1] >= rectangles1[r1+2]) {
          if (gRectangles_[i-1] < rectangles1[r1+3])
            gRectangles_[i-1] = rectangles1[r1+3];
        }
        else {
          checkMemory (NULL, gNRectangles_ + 1);

          gRectangles_[i]   = yTop;
          gRectangles_[i+1] = yBottom;
          gRectangles_[i+2] = rectangles1[r1+2];
          gRectangles_[i+3] = rectangles1[r1+3];

          i += 4;
          gNRectangles_++;
        }

        r1 += 4;
        
      } while (r1 != r1End);
    }
    else {
      while (r2 != r2End) {
        if (gNRectangles_ > 0            &&
            gRectangles_[i-4] == yTop    &&
            gRectangles_[i-3] == yBottom &&
            gRectangles_[i-1] >= rectangles2[r2+2]) {
          if (gRectangles_[i-1] < rectangles2[r2+3])
            gRectangles_[i-1] = rectangles2[r2+3];
        }
        else {
          checkMemory (NULL, gNRectangles_ + 1);

          gRectangles_[i]   = yTop;
          gRectangles_[i+1] = yBottom;
          gRectangles_[i+2] = rectangles2[r2+2];
          gRectangles_[i+3] = rectangles2[r2+3];

          i += 4;
          gNRectangles_++;
        }

        r2 += 4;
      }
    }
  }

  //
  // SUBTRACT
  //
  else if (operationType == OPERATION_SUBTRACTION) {
    int x1 = rectangles1[r1+2];
    
    while (r1 != r1End && r2 != r2End) {
      if (rectangles2[r2+3] <= x1)
        r2 += 4;
      else if (rectangles2[r2+2] <= x1) {
        x1 = rectangles2[r2+3];
        if (x1 >= rectangles1[r1+3]) {
          r1 += 4;
          if (r1 != r1End) x1 = rectangles1[r1+2];
        }
        else
          r2 += 4;
      }
      else if (rectangles2[r2+2] < rectangles1[r1+3]) {
        checkMemory (NULL, gNRectangles_ + 1);
        
        gRectangles_[i+0] = yTop;
        gRectangles_[i+1] = yBottom;
        gRectangles_[i+2] = x1;
        gRectangles_[i+3] = rectangles2[r2+2];

        i += 4;
        gNRectangles_++;

        x1 = rectangles2[r2+3];
        if (x1 >= rectangles1[r1+3]) {
          r1 += 4;
          if (r1 != r1End) x1 = rectangles1[r1+2];
          else             r2 += 4;
        }
      }
      else {
        if (rectangles1[r1+3] > x1) {
          checkMemory (NULL, gNRectangles_ + 1);
          
          gRectangles_[i+0] = yTop;
          gRectangles_[i+1] = yBottom;
          gRectangles_[i+2] = x1;
          gRectangles_[i+3] = rectangles1[r1+3];

          i += 4;
          gNRectangles_++;
        }
        
        r1 += 4;
        if (r1 != r1End) x1 = rectangles1[r1+2];
      }
    }
    while (r1 != r1End) {
      checkMemory (NULL, gNRectangles_ + 1);
        
      gRectangles_[i+0] = yTop;
      gRectangles_[i+1] = yBottom;
      gRectangles_[i+2] = x1;
      gRectangles_[i+3] = rectangles1[r1+3];

      i += 4;
      gNRectangles_++;

      r1 += 4;
      if (r1 != r1End) x1 = rectangles1[r1+2];
    }
  }

  //
  // INTERSECT
  //
  else if (operationType == OPERATION_INTERSECTION) {
    while (r1 != r1End && r2 != r2End) {
      int x1 = max (rectangles1[r1+2], rectangles2[r2+2]);
      int x2 = min (rectangles1[r1+3], rectangles2[r2+3]);

      if (x1 < x2) {
        checkMemory (NULL, gNRectangles_ + 1);

        gRectangles_[i]   = yTop;
        gRectangles_[i+1] = yBottom;
        gRectangles_[i+2] = x1;
        gRectangles_[i+3] = x2;
        
        i += 4;
        gNRectangles_++;
      }

      if      (rectangles1[r1+3] < rectangles2[r2+3]) r1 += 4;
      else if (rectangles2[r2+3] < rectangles1[r1+3]) r2 += 4;
      else {
        r1 += 4;
        r2 += 4;
      }
    }
  }
}

/**
 * Corresponds to miCoalesce in Region.c of X11.
 */
int
TRegion::coalesceBands (int previousBand, int currentBand)
{
  int r1   = previousBand  << 2;
  int r2   = currentBand   << 2;
  int rEnd = gNRectangles_ << 2;

  // Number of rectangles in prevoius band
  int nRectanglesInPreviousBand = currentBand - previousBand;

  // Number of rectangles in current band
  int nRectanglesInCurrentBand  = 0;
  int r = r2;
  int y = gRectangles_[r2];
  while (r != rEnd && gRectangles_[r] == y) {
    nRectanglesInCurrentBand++;
    r += 4;
  }

  // If more than one band was added, we have to find the start
  // of the last band added so the next coalescing job can start
  // at the right place.
  if (r != rEnd) {
    rEnd -= 4;
    while (gRectangles_[rEnd-4] == gRectangles_[rEnd])
      rEnd -= 4;

    currentBand = rEnd >> 2 - gNRectangles_;
    rEnd = gNRectangles_ << 2;
  }

  if (nRectanglesInCurrentBand == nRectanglesInPreviousBand &&
      nRectanglesInCurrentBand != 0) {
    
    // The bands may only be coalesced if the bottom of the previous
    // band matches the top of the current.
    if (gRectangles_[r1+1] == gRectangles_[r2]) {
      
      // Chek that the bands have boxes in the same places
      do {
        if ((gRectangles_[r1+2] != gRectangles_[r2+2]) ||
            (gRectangles_[r1+3] != gRectangles_[r2+3]))
          return currentBand; // No coalescing
        
        r1 += 4;
        r2 += 4;
        
        nRectanglesInPreviousBand--;
      } while (nRectanglesInPreviousBand != 0);

      //
      // OK, the band can be coalesced
      //
      
      // Adjust number of rectangles and set pointers back to start
      gNRectangles_ -= nRectanglesInCurrentBand;
      r1 -= nRectanglesInCurrentBand << 2;
      r2 -= nRectanglesInCurrentBand << 2;        

      // Do the merge
      do {
        gRectangles_[r1+1] = gRectangles_[r2+1];
        r1 += 4;
        r2 += 4;
        nRectanglesInCurrentBand--;
      } while (nRectanglesInCurrentBand != 0);

      // If only one band was added we back up the current pointer
      if (r2 == rEnd)
        currentBand = previousBand;
      else {
        do {
          gRectangles_[r1] = gRectangles_[r2];
          r1++;
          r2++;
        } while (r2 != rEnd);
      }
    }
  }

  return currentBand;
}

TRegion&
TRegion::operator|=(const TRegion &region)
{
  // Trivial case #1. Region is this or empty
  if (this == &region || region.isEmpty())
    return *this;

  // Trivial case #2. This is empty
  if (isEmpty()) {
    return *this = region;
  }
  
  // Trivial case #3. This region covers the specified one
  if (length_ == 1 && region.extent_.isInsideOf (extent_))
    return *this;

  // Trivial case #4. The specified region covers this one
  if (length_ == 1 &&
      extent_.isInsideOf (region.extent_)) {
    return *this = region;
  }

  // Ceneral case
  combine(region, OPERATION_UNION);

  // Update extent
  extent_.x1 = min(extent_.x1, region.extent_.x1);
  extent_.y1 = min(extent_.y1, region.extent_.y1);
  extent_.x2 = max(extent_.x2, region.extent_.x2);
  extent_.y2 = max(extent_.y2, region.extent_.y2);
  
  return *this;
}
#if 0
int
main()
{
  TRegion rgn, rgn0;
  rgn|=TRectangle(1,1,4,4);
  rgn|=TRectangle(5,5,2,2);
  rgn|=TRectangle(10,10,10,10);
  rgn&=TRectangle(3,3,10,10);
  rgn|=TRectangle(15,15,20,5);
  rgn-=TRectangle(18,18,20,5);
#if 0
  printf("%d\n", rgn.isInside(1,3));
#else
  for(int y=0; y<24; ++y) {
    for(int x=0; x<78; ++x) {
      if (rgn.isInside(x,y))
        printf("X");
      else
        printf(".");
    }
    printf("\n");
  }
#endif
  return 0;
}
#endif
