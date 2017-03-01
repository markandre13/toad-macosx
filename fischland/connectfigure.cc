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

#include "connectfigure.hh"
#include <toad/vector.hh>

using namespace toad;

/*
  a 1st idea on how to handle figure id's for figure references:
  
  o we do not store the id's within TFigure
  o before saving, TFigure::prepareIDs(TFigureIDs &ids) is invoked for all figures
  o after loading, TFigure::resolveIDs(TFigureIDs &ids) is invoked for all figures
*/

TRectangle
TFConnection::bounds() const
{
  return TRectangle(0,0,320,200);
  // return TRectangle(p1,p2);
}

void 
TFConnection::paint(TPenBase &pen, EPaintType)
{
  pen.push();
  pen.setLineColor(line_color);
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);
  
  TVectorGraphic *f0 = start->getPath();
  TVectorGraphic *f1 = end->getPath();
  
  TBoundary b0, b1;
  for(auto &p: *f0)
    b0.expand(p->path->bounds());
  for(auto &p: *f1)
    b1.expand(p->path->bounds());
  
  TPoint p0 = b0.center();
  TPoint p1 = b1.center();
  
  pen.drawLine(p0, p1);
  
  delete f0;
  delete f1;
  
  pen.pop();
/*
  pen.push();
  pen.setAlpha(alpha);
  if (filled) {
    pen.setFillColor(fill_color);
    pen.fillRectangle(p1,p2);
  }
  if (outline) {
    pen.setLineColor(line_color);
    pen.setLineStyle(line_style);
    pen.setLineWidth(line_width);
//cout << "TFRectangle: draw rectangle between " << p1 << " and " << p2 << endl;
    pen.drawRectangle(p1,p2);
  }
  pen.pop();
*/
}

void
TFConnection::store(TOutObjectStream &out) const
{
  super::store(out);
  ::storePointer(out, "start", start);
  ::storePointer(out, "end", end);
}

bool
TFConnection::restore(TInObjectStream &in)
{
  if (
    ::restorePointer(in, "start", &start) ||
    ::restorePointer(in, "end",   &end) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}
