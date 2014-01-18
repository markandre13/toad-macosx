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

#include <toad/matrix2d.hh>
#include <cmath>

using namespace toad;
using namespace std;

TMatrix2D::TMatrix2D()
{
  identity();
}

TMatrix2D::TMatrix2D(const TMatrix2D &m)
{
  *this = m;
}

TMatrix2D::~TMatrix2D()
{
}

TMatrix2D&
TMatrix2D::operator=(const TMatrix2D &in)
{
  *static_cast<CGAffineTransform*>(this) = in;
  return *this;
}

/**
 * Reset matrix to 'no transformation'.
 *
 * \pre
    / a11 a12  tx \      / 1.0 0.0 0.0 \
   |  a21 a22  ty  | := |  0.0 1.0 0.0  |
    \ 0.0 0.0 1.0 /      \ 0.0 0.0 1.0 /
   \endpre
 
 * Hmpf... Quartz says that their matrices are
 
   a  b  0
   c  d  0
   tx ty 1
   
   but then concat operates like this:
   
   new := a * old

 */
void
TMatrix2D::identity()
{
  *static_cast<CGAffineTransform*>(this) = CGAffineTransformIdentity;
}
 
/**
 * PostScript alike rotation of the coordinate system with the rotation
 * matrix multiplied after the current matrix.
 *
 * \pre
  M' := M * R
 
   / a11 a12  tx \       / a11 a12  tx \     / r11 r12 0.0 \
  |  a21 a22  ty  | :=  |  a21 a22  ty  | * |  r21 r22 0.0  |
   \ 0.0 0.0 1.0 /       \ 0.0 0.0 1.0 /     \ 0.0 0.0 1.0 / 
   \endpre
   
  Quartz says
  
    cos sin  0
    -sin cos 0
    0   0    1
   
 */
void
TMatrix2D::rotate(TCoord radiant)
{
  *static_cast<CGAffineTransform*>(this) = CGAffineTransformRotate(*this, radiant);
}

/**
 * PostScript alike tranlation of the coordinate system with the
 * translation matrix multiplied after the current matrix.
 *
 * \pre
  M' := M * T
 
   / a11 a12  tx \      / a11 a12 0 \     / 1 0 0 \
  |  a21 a22  ty  | := |  a21 a22 0  | * |  0 1 0  |
   \ 0.0 0.0 1.0 /      \  tx  ty 1 /     \ x y 1 / 
   \endpre
 */
void
TMatrix2D::translate(TCoord x, TCoord y)
{
  *static_cast<CGAffineTransform*>(this) = CGAffineTransformTranslate(*this, x, y);
}

/**
 * Rotate coordinate system around point (x, y).
 *
 * \pre
   M' := -T * R * T * M
   \endpre
 */
void
TMatrix2D::rotateAt(TCoord x, TCoord y, TCoord radiant)
{
#if 0
  TCoord r11, r12, r21, r22;
  r11 = r22 = cos(radiant);
  r21 = sin(radiant);
  r12 = -r21;  

  TCoord n11 = a11 * r11 + a21 * r12;
  TCoord n21 = a11 * r21 + a21 * r22;
  TCoord n12 = a12 * r11 + a22 * r12;
  TCoord n22 = a12 * r21 + a22 * r22;

  TCoord ntx = r11 * (tx-x) + r12 * (ty-y) + x;
  TCoord nty = r21 * (ty-y) + r22 * (ty-y) + x;
  
  a11 = n11;
  a21 = n21;
  a12 = n12;
  a22 = n22;
  tx = ntx; 
  ty = nty; 
  
  _identity = false;
#endif
}

/**
 *
 */
void
TMatrix2D::scale(TCoord xfactor, TCoord yfactor)
{
#if 0
  m.m11 *= xfactor;
  m.m12 *= xfactor;
  m.m21 *= yfactor;
  m.m22 *= yfactor;
  
  _identity = false;
#endif
}

void
TMatrix2D::shear(TCoord, TCoord)
{
}

/**
 *
 * \pre

  Cocoa/Quartz2D & Cairo style Matrix

   / a11 a21 0.0 \       / a11 a21 0.0 \     / b11 b21 0.0  \
  |  a12 a22 0.0  | :=  |  a12 a22 0.0  | * |  b12 b22 0.0  |
   \  tX  tY 1.0 /       \ atX atY 1.0 /     \ btX btY 1.0 / 

   \endpre
 */
void
TMatrix2D::multiply(const TMatrix2D *p)
{
  *static_cast<CGAffineTransform*>(this) = CGAffineTransformConcat(*this, *p);
}

TMatrix2D
TMatrix2D::operator*(const TMatrix2D &p) const
{
  TMatrix2D r(*this);
  r.multiply(&p);
  return r;
}
 
/**
 * \pre
   / outY \      / a11 a12  tx \     / inX \
  |  outX  | := |  a21 a22  ty  | * |  inY  |
   \ 1.0  /      \ 0.0 0.0 1.0 /     \ 1.0 / 
   \endpre
 */
void
TMatrix2D::map(TCoord inX, TCoord inY, short int *outX, short int *outY) const
{
/*
  TCoord x, y;
  x = inX; y=inY;
  *outX = static_cast<short int>(lround(m.m11 * x + m.m21 * y + m.tX));
  *outY = static_cast<short int>(lround(m.m12 * x + m.m22 * y + m.tY));
*/
}

void
TMatrix2D::map(TCoord inX, TCoord inY, int *outX, int *outY) const
{
/*
  TCoord x, y;
  x = inX; y=inY;
  *outX = static_cast<int>(lround(m.m11 * x + m.m21 * y + m.tX));
  *outY = static_cast<int>(lround(m.m12 * x + m.m22 * y + m.tY));
*/
}

void
TMatrix2D::map(TCoord inX, TCoord inY, long *outX, long *outY) const
{
/*
  TCoord x, y;
  x = inX; y=inY;
  *outX = lround(m.m11 * x + m.m12 * y + m.tX);
  *outY = lround(m.m21 * x + m.m22 * y + m.tY);
*/
}

void
TMatrix2D::map(TCoord inX, TCoord inY, float *outX, float *outY) const
{
/*
  TCoord x, y;
  x = inX; y=inY;
  *outX = m.m11 * x + m.m12 * y + m.tX;
  *outY = m.m21 * x + m.m22 * y + m.tY;
*/
}

void
TMatrix2D::map(TCoord inX, TCoord inY, double *outX, double *outY) const
{
/*
  TCoord x, y;
  x = inX; y=inY;
  *outX = m.m11 * x + m.m12 * y + m.tX;
  *outY = m.m21 * x + m.m22 * y + m.tY;
*/
}

/**
 * Invert the matrix.
 *
 * \pre
          -1
   A' := A 
   \endpre
 */
void
TMatrix2D::invert()
{
  *static_cast<CGAffineTransform*>(this) = CGAffineTransformInvert(*this);
}

void
TMatrix2D::store(TOutObjectStream &out) const
{
  ::store(out, a);
  ::store(out, b);
  ::store(out, c);
  ::store(out, d);
  ::store(out, tx);
  ::store(out, ty);
}

bool
TMatrix2D::restore(TInObjectStream &in)
{
  if (
    ::restore(in, 0, &a) ||
    ::restore(in, 1, &b) ||
    ::restore(in, 2, &c) ||
    ::restore(in, 3, &d) ||
    ::restore(in, 4, &tx) ||
    ::restore(in, 5, &ty) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in);
  return false;
}
