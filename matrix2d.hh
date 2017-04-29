/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 2014 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_MATRIX2D_HH
#define _TOAD_MATRIX2D_HH 1

#include "CoreGraphics/CGAffineTransform.h"

#include <toad/types.hh>
#include <toad/io/serializable.hh>

namespace toad {

class TMatrix2D:
  public CGAffineTransform, public TSerializable
{
    typedef TSerializable super;
  public:
    TMatrix2D();
    virtual ~TMatrix2D();
    TMatrix2D(const TMatrix2D&);
    TMatrix2D(const CGAffineTransform &m) {
      *static_cast<CGAffineTransform*>(this) = m;
    }
    TMatrix2D(TCoord a11, TCoord a21, TCoord a12, TCoord a22, TCoord tx, TCoord ty) {
      a = a11;
      b = a21;
      c = a12;
      d = a22;
      this->tx  = tx;
      this->ty  = ty;
    }
    TMatrix2D& operator=(const TMatrix2D &m) {
      *static_cast<CGAffineTransform*>(this) = m;
      return *this;
    }
    TMatrix2D& operator=(const CGAffineTransform &m) {
      *static_cast<CGAffineTransform*>(this) = m;
      return *this;
    }
    TMatrix2D& operator*=(const TMatrix2D &m) {
      multiply(&m);
      return *this;
    }
    TMatrix2D operator*(const TMatrix2D &m) const;
  
    void identity();
    bool isIdentity() const {
      return CGAffineTransformIsIdentity(*this);
    }
    
    // stretch, rotate, shear, 

    /**
     * returns true, when the matrix does neither rotate or shear
     */    
    bool isOnlyTranslate() const {
      return a == 1.0 && b == 0.0 && c == 0.0 && d == 1.0;
    }
    bool isOnlyTranslateAndScale() const {
      return b == 0.0 && c == 0.0;
    };
    
    
    void rotate(TCoord radiant);
    void rotateAt(TCoord x, TCoord y, TCoord radiant);
    void translate(TCoord dx, TCoord dy);
    void translate(const TPoint &vector) {
      translate(vector.x, vector.y);
    }

    void scale(TCoord, TCoord);
    void shear(TCoord, TCoord);
    
    void multiply(const TMatrix2D*);
    void invert();

    void set(TCoord a11, TCoord a21, TCoord a12, TCoord a22, TCoord tx, TCoord ty) {
      a = a11;
      b = a21;
      c = a12;
      d = a22;
      this->tx  = tx;
      this->ty  = ty;
    }
    
    TPoint map(const TPoint&) const;
    
    // obsolete
    void map(TCoord inX, TCoord inY, short int *outX, short int *outY) const;
    void map(TCoord inX, TCoord inY, int *outX, int *outY) const;
    void map(TCoord inX, TCoord inY, long *outX, long *outY) const;
    void map(TCoord inX, TCoord inY, float *outX, float *outY) const;
    void map(TCoord inX, TCoord inY, double *outX, double *outY) const;
    void map(const TPoint &in, TPoint *out) const { map(in.x, in.y, &out->x, &out->y); }
 
    SERIALIZABLE_INTERFACE(toad::, TMatrix2D);
};

inline std::ostream& operator<<(std::ostream &s, const TMatrix2D& m) {
  return s<<"( "<<m.a<<" "<<m.b<<endl
          <<"  "<<m.c<<" "<<m.d<<endl
          <<"  "<<m.tx<<" "<<m.ty<<" )";
}
  

} // namespace toad

#endif
