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

#include <cmath>
#include <toad/figure.hh>
#include <toad/figureeditor.hh>
#include <toad/penbase.hh>

using namespace toad;

#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi */
#endif

void
TFigureArrow::setAttributes(const TFigureAttributeModel *preferences)
{
  if (preferences->reason.arrowmode)
    arrowmode = preferences->arrowmode;
  if (preferences->reason.arrowstyle) {
    arrowtype = preferences->arrowtype;
  }
}

TFigureArrow::TFigureArrow()
{
  arrowmode = NONE;
  arrowtype = SIMPLE;
  arrowheight = 8;
  arrowwidth = 4;
}

void
TFigureArrow::getAttributes(TFigureAttributeModel *preferences) const
{
  preferences->arrowmode = arrowmode;
  preferences->arrowtype = arrowtype;
}

namespace {
  const char* arrowmodename[] = {
    "none",
    "head",
    "tail",
    "both"
  };
  
  const char* arrowtypename[] = {
    "simple",
    "empty",
    "filled",
    "empty-concave",
    "filled-concave",
    "empty-convex",
    "filled-convex"
  };
} // namespace

void
TFigureArrow::store(TOutObjectStream &out) const
{
  if (arrowmode!=NONE) {
   out.indent();
   out << "arrowmode = " << arrowmodename[arrowmode];
   out.indent();
   out << "arrowtype = " << arrowtypename[arrowtype];
   out.indent();
   out << "arrowheight = " << arrowheight;
   out.indent();
   out << "arrowwidth = " << arrowwidth;
  }
}

bool
TFigureArrow::restore(TInObjectStream &in)
{
  static bool flag;
  static TCoord x;
  TCoord y;

  if (in.what == ATV_START)
    flag = false;

  string s;

  if (::restore(in, "arrowmode", &s)) {
    for(unsigned i=0; i<sizeof(arrowmodename)/sizeof(char*); ++i) {
      if (s == arrowmodename[i]) {
        arrowmode = static_cast<EArrowMode>(i);
        return true;
      }
    }
  } else
  if (::restore(in, "arrowtype", &s)) {
    for(unsigned i=0; i<sizeof(arrowtypename)/sizeof(char*); ++i) {
      if (s == arrowtypename[i]) {
        arrowtype = static_cast<EArrowType>(i);
        return true;
      }
    }
  } else
  if (::restore(in, "arrowheight", &arrowheight) ||
      ::restore(in, "arrowwidth", &arrowwidth))
  {
    return true;
  }

  return false;
}


void
TFigureArrow::drawArrow(TPenBase &pen, 
                  const TPoint &p1, const TPoint &p2,
                  const TRGB &line, const TRGB &fill,
                  TCoord w, TCoord h,
                  EArrowType type)
{
  TCoord d = atan2(p2.y - p1.y, 
                   p2.x - p1.x);
  
  TCoord height = h;
  TCoord width  = 0.5 * w;
  
  TPoint p0;
  
  TPoint p[4];
  
  p[0] = p1;
  
  p0.x = cos(d) * height + p1.x;
  p0.y = sin(d) * height + p1.y;

  TCoord r = 90.0 / 360.0 * (2.0 * M_PI);
  p[1].x = p0.x + cos(d-r) * width;
  p[1].y = p0.y + sin(d-r) * width;
  p[3].x = p0.x + cos(d+r) * width;
  p[3].y = p0.y + sin(d+r) * width;
  
  switch(type) {
    case SIMPLE:
      pen.drawLine(p[0], p[1]);
      pen.drawLine(p[0], p[3]);
      break;
    case EMPTY:
      p[2] = p0;
      pen.fillPolygon(p, 4);
      break;
    case FILLED:
      p[2] = p0;
      pen.fillPolygon(p, 4);
      break;
    case EMPTY_CONCAVE:
      height -= height / 4;
      p[2].x = cos(d) * height + p1.x;
      p[2].y = sin(d) * height + p1.y;
      pen.fillPolygon(p, 4);
      break;
    case FILLED_CONCAVE:
      height -= height / 4;
      p[2].x = cos(d) * height + p1.x;
      p[2].y = sin(d) * height + p1.y;
      pen.fillPolygon(p, 4);
      break;
    case EMPTY_CONVEX:
      height += height / 4;
      p[2].x = cos(d) * height + p1.x;
      p[2].y = sin(d) * height + p1.y;
      pen.fillPolygon(p, 4);
      break;
    case FILLED_CONVEX:
      height += height / 4;
      p[2].x = cos(d) * height + p1.x;
      p[2].y = sin(d) * height + p1.y;
      pen.fillPolygon(p, 4);
      break;
  }
}
