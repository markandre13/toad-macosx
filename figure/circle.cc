/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2005 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figure.hh>
#include <toad/figureeditor.hh>
#include <toad/vector.hh>

// avoid problems on AIX, IRIX, ...
#define exception c_exception
#include <cmath>
#undef exception

// missing in mingw
#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi */
#endif

using namespace toad;

// Michael Goldapp, "Approximation of circular arcs by cubic polynomials" Computer Aided Geometric Design (#8 1991 pp.227-238)
// Tor Dokken and Morten Daehlen, "Good Approximations of circles by curvature-continuous Bezier curves" Computer Aided Geometric Design (#7 1990 pp. 33-41).
// error is about 0.0273% of the circles radius
// n := 4 segments, f := (4/3)*tan(pi/(2n))
static const TCoord f = 0.552284749831;

void 
TFCircle::paint(TPenBase &pen, EPaintType)
{
  pen.setAlpha(alpha);
  pen.setLineColor(line_color);
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);
  if (!filled) {
#if 1
    pen.drawCircle(p1,p2);
#else
  TRectangle r = bounds();
  TCoord rx = 0.5*(r.w);
  TCoord ry = 0.5*(r.h);
  TCoord cx = (double)r.x+rx;
  TCoord cy = (double)r.y+ry;
  auto *path = new TVectorPath;
  path->move (cx         , cy-ry);
  path->curve(cx + rx * f, cy-ry,
              cx + rx    , cy-ry*f,
              cx + rx    , cy);
  path->curve(cx + rx    , cy+ry*f,
              cx + rx * f, cy+ry,
              cx         , cy+ry);
  path->curve(cx - rx * f, cy+ry,
              cx - rx    , cy+ry*f,
              cx - rx    , cy);
  path->curve(cx - rx    , cy-ry*f,
              cx - rx * f, cy-ry,
              cx         , cy-ry);
  path->close();
  path->apply(pen);
  delete path;
  pen.stroke();
#endif
  } else {
    pen.setFillColor(fill_color);
    pen.fillCircle(p1,p2);
    pen.drawCircle(p1,p2);
  }
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);
  pen.setAlpha(1);
}

TCoord
TFCircle::distance(const TPoint &pos)
{
  TRectangle r = bounds();
  TCoord rx = 0.5*(r.w);
  TCoord ry = 0.5*(r.h);
  TCoord cx = (double)r.x+rx;
  TCoord cy = (double)r.y+ry;
  TCoord dx = (double)pos.x - cx;
  TCoord dy = (double)pos.y - cy;
  
  TCoord phi = atan( (dy*rx) / (dx*ry) );
  if (dx<0.0)
    phi=phi+M_PI;
  TCoord ex = rx*cos(phi);
  TCoord ey = ry*sin(phi);
  if (filled) {
    TCoord d = sqrt(dx*dx+dy*dy)-sqrt(ex*ex+ey*ey);
    if (d<0.0)
      return INSIDE;
    return d;
  }
  dx -= ex;
  dy -= ey;
  return sqrt(dx*dx+dy*dy);
}

TVectorGraphic*
TFCircle::getPath() const
{
  TRectangle r = bounds();
  TCoord rx = 0.5*(r.w);
  TCoord ry = 0.5*(r.h);
  TCoord cx = (double)r.x+rx;
  TCoord cy = (double)r.y+ry;
  
  
  auto *path = new TVectorPath;
  path->move (cx         , cy-ry);
  path->curve(cx + rx * f, cy-ry,
              cx + rx    , cy-ry*f,
              cx + rx    , cy);
  path->curve(cx + rx    , cy+ry*f,
              cx + rx * f, cy+ry,
              cx         , cy+ry);
  path->curve(cx - rx * f, cy+ry,
              cx - rx    , cy+ry*f,
              cx - rx    , cy);
  path->curve(cx - rx    , cy-ry*f,
              cx - rx * f, cy-ry,
              cx         , cy-ry);
  path->close();
  
  auto *vg = new TVectorGraphic;
  vg->push_back(new TVectorPainter(
    new TVectorStrokeAndFillOp(line_color, fill_color),
    path
  ));
  return vg;
}

