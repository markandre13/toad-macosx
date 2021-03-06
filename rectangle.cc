/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/types.hh>

using namespace toad;

void
TRectangle::set(TCoord x, TCoord y, TCoord w, TCoord h)
{
  if (w<0) {
    w=-w;
    x-=w;
  }
  if (h<0) {
    h=-h;
    y-=h;
  }
  origin.x = x;
  origin.y = y;
  size.width = w;
  size.height = h;
}

void
TRectangle::set(const TPoint &p1, const TPoint &p2)
{
  origin = p1;
  size = p2-p1;
  if (size.width<0) {
    size.width=-size.width;
    origin.x-=size.width;
  }
  if (size.height<0) {
    size.height=-size.height;
    origin.y-=size.height;
  }
}

// based on Dan Cohen and Ivan Sutherlands clipping algorithm
bool
TRectangle::intersects(const TRectangle &r) const
{
  unsigned f0, f1;
  TCoord a;
  TCoord x00, x01, x10, x11;
  TCoord y00, y01, y10, y11;
  
  x00=origin.x;
  x01=origin.x+size.width;
  if (x00>x01) {
    a=x00; x00=x01; x01=a;
  }

  x10=r.origin.x;
  x11=r.origin.x+r.size.width;
  if (x10>x11) {
    a=x10; x10=x11; x11=a;
  }

  y00=origin.y;
  y01=origin.y+size.height;
  if (y00>y01) {
    a=y00; y00=y01; y01=a;
  }

  y10=r.origin.y;
  y11=r.origin.y+r.size.height;
  if (y10>y11) {
    a=y10; y10=y11; y11=a;
  }
//cout << "check x:"<<x00<<","<<x01<<" and "<<x10<<","<<x11<<endl;
//cout << "check y:"<<y00<<","<<y01<<" and "<<y10<<","<<y11<<endl;
  f0 = 0;
  if (x00 < x10)
    f0 |= 1;
  if (x00 > x11)
    f0 |= 2;
  if (y00 < y10)
    f0 |= 4;
  if (y00 > y11)
    f0 |= 8;

  f1 = 0;
  if (x01 < x10)
    f1 |= 1;
  if (x01 > x11)
    f1 |= 2;
  if (y01 < y10)
    f1 |= 4;
  if (y01 > y11)
    f1 |= 8;
//cout << "f0="<<f0<<", f1="<<f1<<", f0&f1="<<(f0&f1)<<endl;    
  return (f0 & f1)==0;
}

// based on Dan Cohen and Ivan Sutherlands clipping algorithm
bool
TBoundary::intersects(const TBoundary &r) const
{
  if (empty || r.empty)
    return false;
    
  unsigned f0, f1;
  TCoord a;
  TCoord x00, x01, x10, x11;
  TCoord y00, y01, y10, y11;
  
  x00=p0.x;
  x01=p1.x;
  if (x00>x01) {
    a=x00; x00=x01; x01=a;
  }

  x10=r.p0.x;
  x11=r.p1.x;
  if (x10>x11) {
    a=x10; x10=x11; x11=a;
  }

  y00=p0.y;
  y01=p1.y;
  if (y00>y01) {
    a=y00; y00=y01; y01=a;
  }

  y10=r.p0.y;
  y11=r.p1.y;
  if (y10>y11) {
    a=y10; y10=y11; y11=a;
  }
//cout << "check x:"<<x00<<","<<x01<<" and "<<x10<<","<<x11<<endl;
//cout << "check y:"<<y00<<","<<y01<<" and "<<y10<<","<<y11<<endl;
  f0 = 0;
  if (x00 < x10)
    f0 |= 1;
  if (x00 > x11)
    f0 |= 2;
  if (y00 < y10)
    f0 |= 4;
  if (y00 > y11)
    f0 |= 8;

  f1 = 0;
  if (x01 < x10)
    f1 |= 1;
  if (x01 > x11)
    f1 |= 2;
  if (y01 < y10)
    f1 |= 4;
  if (y01 > y11)
    f1 |= 8;
//cout << "f0="<<f0<<", f1="<<f1<<", f0&f1="<<(f0&f1)<<endl;    
  return (f0 & f1)==0;
}
