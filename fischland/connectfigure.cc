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
  return distance2Line(pos.x, pos.y, p[0].x,p[0].y, p[1].x,p[1].y);
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

TVectorGraphic*
TFConnection::getPath() const
{
  auto *path = new TVectorPath;
  path->move(p[0]);
  path->line(p[1]);
  
  auto *vg = new TVectorGraphic;
  vg->push_back(new TVectorPainter(
    new TVectorStrokeAndFillOp(line_color, fill_color),
    path
  ));
  return vg;
}

void 
TFConnection::updatePoints()
{
  TVectorGraphic *f0, *f1;
  TPoint p0, p1;

  if (start) {
    f0 = start->getPath();
    TBoundary b;
    for(auto &p: *f0)
      b.expand(p->path->bounds());
    p0 = b.center();
  } else {
    p0 = p[0];
  }

  if (end) {
    f1 = end->getPath();
    TBoundary b;
    for(auto &p: *f1)
      b.expand(p->path->bounds());
    p1 = b.center();
  } else {
    p1 = p[1];
  }

  TVectorPath line;
  line.move(p0);
  line.line(p1);
  
  if (start) {
    TIntersectionList il;
    for(auto &p: *f0)
      p->path->intersect(il, line);
    TCoord d=0.0;
    for(auto &p: il) {
      if (p.seg1.u>d) {
        d = p.seg1.u;
        p0 = p.seg1.pt;
      }
    }
  }
  
  if (end) {
    TIntersectionList il;
    for(auto &p: *f1)
      p->path->intersect(il, line);
    TCoord d=1.0;
    for(auto &p: il) {
      if (p.seg1.u<d) {
        d = p.seg1.u;
        p1 = p.seg1.pt;
      }
    }
  }

  p[0] = p0;
  p[1] = p1;

  if (start)
    delete f0;
  if (end)
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
