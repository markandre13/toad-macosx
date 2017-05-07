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

#include <toad/vector.hh>
#include <toad/figure.hh>
#include <toad/figureeditor.hh>

using namespace toad;

void 
TFTransform::paint(TPenBase &pen, EPaintType type)
{
/*
  if (type==EDIT || type==SELECT) {
    pen.setStrokeColor(TColor::FIGURE_SELECTION);
    pen.setFillColor(TColor::WHITE);
  }
*/
  TVectorGraphic *vg = getPath();
  if (!vg)
    return;
  vg->paint(pen);
  delete vg;
}

TVectorGraphic*
TFTransform::getPath() const
{
  TVectorGraphic *vg = figure->getPath();
  if (vg)
    vg->transform(matrix);
  return vg;
}

TRectangle
TFTransform::bounds() const
{
  TVectorPath path;
  path.addRect(figure->bounds());
  path.transform(matrix);
  return path.bounds();
}

TCoord
TFTransform::distance(const TPoint &pos)
{
  TMatrix2D m(matrix);
  m.invert();
  TPoint p;
  m.map(pos, &p);
  return figure->distance(p);
}

bool
TFTransform::getHandle(unsigned n, TPoint *p)
{
  bool result = figure->getHandle(n, p);
  if (result) {
    matrix.map(*p, p);
  }
  return result;
}

bool
TFTransform::startTranslateHandle()
{
  figure->startTranslateHandle();
}

void
TFTransform::translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier)
{
  TMatrix2D m(matrix);
  m.invert();
  m.map(x, y, &x, &y);
  figure->translateHandle(handle, x, y, modifier);
}

void
TFTransform::endTranslateHandle()
{
  figure->endTranslateHandle();
}

void
TFTransform::store(TOutObjectStream &out) const
{
//  figures.store(out);
}

bool
TFTransform::restore(TInObjectStream &in)
{
/*
  if (in.what == ATV_FINISHED) {
    return true;
  }
  if (figures.restore(in))
    return true;
  // FIXME: store handles
  ATV_FAILED(in);
*/
  return false;
}
