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
#include <toad/figureeditor.hh>
#include <toad/vector.hh>
#include <toad/geometry.hh>

using namespace toad;

bool
TFConnection::editEvent(TFigureEditEvent &editEvent)
{
  // FIXME: add helper functions for RELATION_REMOVED and REMOVED
  switch(editEvent.type) {
    case TFigureEditEvent::RELATION_MODIFIED:
      updatePoints();
      break;
    case TFigureEditEvent::RELATION_REMOVED:
      if (editEvent.model->figures.find(start) != editEvent.model->figures.end()) {
        start = nullptr;
      }
      if (editEvent.model->figures.find(end) != editEvent.model->figures.end()) {
        end = nullptr;
      }
      break;
    case TFigureEditEvent::REMOVED: {
      if (start) {
        auto p = TFigureEditor::relatedTo.find(start);
        if (p!=TFigureEditor::relatedTo.end()) {
          p->second.erase(this);
          if (p->second.empty())
            TFigureEditor::relatedTo.erase(p);
        }
      }
      if (end) {
        auto p = TFigureEditor::relatedTo.find(end);
        if (p!=TFigureEditor::relatedTo.end()) {
          p->second.erase(this);
          if (p->second.empty())
            TFigureEditor::relatedTo.erase(p);
        }
      }
    } break;
    default:
      ;
  }
}

TRectangle
TFConnection::bounds() const
{
  TRectangle r0, r1;
  if (start) {
    r0 = start->bounds();
  } else {
    r0.set(p[0], p[0]);
  }
  if (end) {
    r1 = end->bounds();
  } else {
    r1.set(p[1], p[1]);
  }
  
  TPoint p0(min(r0.x, r1.x), min(r0.y, r1.y));
  TPoint p1(max(r0.x+r0.w, r1.x+r1.w), max(r0.y+r0.h, r1.y+r1.h));
  
  return TRectangle(p0,p1);
}

TCoord
TFConnection::distance(const TPoint &pos)
{
  return TFigure::OUT_OF_RANGE;
}

void
TFConnection::paint(TPenBase &pen, EPaintType type)
{
  pen.setLineColor(line_color);	// FIXME: move to TColoredFigure and update all other figures
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);
  pen.setAlpha(alpha);
          
  pen.drawLine(p[0], p[1]);

  if (arrowmode == NONE) {
    pen.setAlpha(1);
    return;
  }
  pen.setLineStyle(TPenBase::SOLID);

  TCoord aw = arrowwidth * line_width;
  TCoord ah = arrowheight * line_width;

  if (arrowmode == HEAD || arrowmode == BOTH)
    drawArrow(pen, p[1], p[0], line_color, fill_color, aw, ah, arrowtype);
  if (arrowmode == TAIL || arrowmode == BOTH)
    drawArrow(pen, p[0], p[1], line_color, fill_color, aw, ah, arrowtype);
  pen.setAlpha(1);

  
}

void 
TFConnection::updatePoints()
{
  TVectorGraphic *f0 = start->getPath();
  TVectorGraphic *f1 = end->getPath();
  
  TBoundary b0, b1;
  for(auto &p: *f0)
    b0.expand(p->path->bounds());
  for(auto &p: *f1)
    b1.expand(p->path->bounds());
  
  TPoint p0 = b0.center();
  TPoint p1 = b1.center();

  TVectorPath line;
  line.move(p0);
  line.line(p1);
  
  TIntersectionList il;
  TCoord d;

  for(auto &p: *f0)
    p->path->intersect(il, line);
//  cout << "found " << il.size() << " intersections" << endl;
  
  d=0.0;
  for(auto &p: il) {
    if (p.seg1.u>d) {
      d = p.seg1.u;
      p0 = p.seg1.pt;
    }
//    cout << "  " << p.seg1.u << " -> " << p.seg1.pt << endl;
  }
  
  il.clear();

  for(auto &p: *f1)
    p->path->intersect(il, line);
  d=1.0;
  for(auto &p: il) {
    if (p.seg1.u<d) {
      d = p.seg1.u;
      p1 = p.seg1.pt;
    }
//    cout << "  " << p.seg1.u << " -> " << p.seg1.pt << endl;
  }

  p[0] = p0;
  p[1] = p1;

cout << "updatePoints " << p0 << " " << p1 << endl;

  delete f0;
  delete f1;
}

void
TFConnection::translate(TCoord dx, TCoord dy) {
}

bool
TFConnection::getHandle(unsigned n, TPoint *p) {
  return false;
}

void
TFConnection::translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier)
{
}

void
TFConnection::setAttributes(const TFigureAttributes *attributes)
{
  TFigureArrow::setAttributes(attributes);
}

void
TFConnection::getAttributes(TFigureAttributes *attributes) const
{
  TFigureArrow::getAttributes(attributes);
}

void
TFConnection::store(TOutObjectStream &out) const
{
  super::store(out);
  TFigureArrow::store(out);
  ::storePointer(out, "start", start);
  ::storePointer(out, "end", end);
}

bool
TFConnection::restore(TInObjectStream &in)
{
  if (::finished(in)) {
    TFigureEditor::restoreRelation(const_cast<const TFigure**>(&start), this);
    TFigureEditor::restoreRelation(const_cast<const TFigure**>(&end  ), this);
  }
  if (
    TFigureArrow::restore(in) ||
    ::restorePointer(in, "start", &start) ||
    ::restorePointer(in, "end",   &end) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}
