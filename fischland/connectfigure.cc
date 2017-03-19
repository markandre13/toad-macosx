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
    r0.set(p.front(), p.front());
  }
  if (end) {
    r1 = end->bounds();
  } else {
    r1.set(p.back(), p.back());
  }
  
  TPoint p0(min(r0.x, r1.x), min(r0.y, r1.y));
  TPoint p1(max(r0.x+r0.w, r1.x+r1.w), max(r0.y+r0.h, r1.y+r1.h));
  
  for(size_t i=1; i<p.size()-1; ++i) {
    auto &&pt = p[i];
    if (pt.x < p0.x)
      p0.x = pt.x;
    if (pt.x > p1.x)
      p1.x = pt.x;
    if (pt.y < p0.y)
      p0.y = pt.y;
    if (pt.y > p1.y)
      p1.y = pt.y;
  }
  
  return TRectangle(p0,p1);
}

TCoord
TFConnection::distance(const TPoint &pos)
{
  auto p(this->p.begin()), e(this->p.end());
  TCoord x1,y1,x2,y2;
  TCoord min = OUT_OF_RANGE, d;

  assert(p!=e);
  --e;
  assert(p!=e);
  x2=e->x;
  y2=e->y;
  ++e;
  while(p!=e) {
    x1=x2;
    y1=y2;
    x2=p->x;
    y2=p->y;
    d = distance2Line(pos.x, pos.y, x1,y1, x2,y2);
    if (d<min)
      min = d;
    ++p;
  }
  return min;
//  return distance2Line(pos.x, pos.y, p[0].x,p[0].y, p[1].x,p[1].y);
}

void
TFConnection::paint(TPenBase &pen, EPaintType type)
{
  if (type!=EDIT) {
    pen.setColor(line_color);	// FIXME: move to TColoredFigure and update all other figures
  }
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);
  pen.setAlpha(alpha);
//cout << "TFConnection::paint()" << endl;          
  // pen.drawLine(p[0], p[1]);
  //pen.drawPolygon(p);
//cout << "  move " << p[0] << endl;
  pen.move(&p[0]);
  for(size_t i=1; i<p.size(); ++i) {
//    cout << "  line " << p[i] << endl;
    pen.line(&p[i]);
  }
  pen.stroke();

  if (arrowmode == NONE) {
    pen.setAlpha(1);
    return;
  }
  pen.setLineStyle(TPenBase::SOLID);

  TCoord aw = arrowwidth * line_width;
  TCoord ah = arrowheight * line_width;

  if (arrowmode == HEAD || arrowmode == BOTH)
    drawArrow(pen, p.back(), *(p.end()-2), line_color, fill_color, aw, ah, arrowtype);
  if (arrowmode == TAIL || arrowmode == BOTH)
    drawArrow(pen, p.front(), *(p.begin()+1), line_color, fill_color, aw, ah, arrowtype);
  pen.setAlpha(1);
}

TVectorGraphic*
TFConnection::getPath() const
{
  auto *path = new TVectorPath;
  path->move(p[0]);
  for(size_t i=1; i<p.size(); ++i)
    path->line(p[i]);
  
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

  if (start) {
    f0 = start->getPath();
    if (!f0) {
      cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ": figure " << start->getClassName() << " returned no path" << endl;
      return;
    }
    TBoundary b;
    for(auto &p: *f0)
      b.expand(p->path->bounds());
    p.front() = b.center();
  }

  if (end) {
    f1 = end->getPath();
    if (!f1) {
      cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ": figure " << end->getClassName() << " returned no path" << endl;
      return;
    }
    TBoundary b;
    for(auto &p: *f1)
      b.expand(p->path->bounds());
    p.back() = b.center();
  }

  // FIXME: we now need two different lines (the head and tail segments)
  
  if (start) {
    TVectorPath line;
    line.move(p.front());
    line.line(*(this->p.begin()+1));
    TIntersectionList il;
    for(auto &p: *f0)
      p->path->intersect(il, line);
    TCoord d=0.0;
    TPoint pt(p.front());
    for(auto &p: il) {
      if (p.seg1.u>d) {
        d = p.seg1.u;
        pt = p.seg1.pt;
      }
    }
    p.front() = pt;
    delete f0;
  }
  
  if (end) {
    TVectorPath line;
    line.move(*(this->p.end()-2));
    line.line(p.back());
    TIntersectionList il;
    for(auto &p: *f1)
      p->path->intersect(il, line);
    TCoord d=1.0;
    TPoint pt(p.back());
    for(auto &p: il) {
      if (p.seg1.u<d) {
        d = p.seg1.u;
        pt = p.seg1.pt;
      }
    }
    p.back() = pt;
    delete f1;
  }
}

void
TFConnection::translate(TCoord dx, TCoord dy) {
  // we are bound to other figures (for now), so we can not translate
}

bool
TFConnection::getHandle(unsigned handle, TPoint *p)
{
//cout << "TFConnection::getHandle("<<handle<<", "<<*p<<")" << endl;
  if (handle > this->p.size())
    return false;
  *p = this->p[handle];
  return true;
}


void
TFConnection::translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier)
{
  if (handle==0 || handle>=p.size()-1)
    return;
  p[handle] = TPoint(x, y);
}

static vector<TPoint>::iterator newpoint;

unsigned
TFConnection::mouseLDown(TFigureEditor *fe, TMouseEvent &me)
{
//  cout << "TFConnection::mouseLDown" << endl;

  auto p0(this->p.begin()),
       p1(this->p.begin()+1),
       e(this->p.end()),
       m(this->p.end());
  TCoord min = OUT_OF_RANGE, d;
  
  while(p1!=e) {
    d = distance2Line(me.pos.x, me.pos.y, p0->x, p0->y, p1->x, p1->y);
    if (d<min) {
      min = d;
      m = p0;
    }
    p0=p1;
    ++p1;
  }
  if (min > TFigure::RANGE)
    return TFigure::STOP;

  newpoint = this->p.insert(m+1, me.pos);
  return TFigure::CONTINUE;
}

unsigned
TFConnection::mouseMove(TFigureEditor *fe, TMouseEvent &me)
{
//  cout << "TFConnection::mouseMove" << endl;
  *newpoint = me.pos;
  updatePoints();
  return TFigure::CONTINUE;
}

unsigned
TFConnection::mouseLUp(TFigureEditor *fe, TMouseEvent &me)
{
//  cout << "TFConnection::mouseLUp" << endl;
  *newpoint = me.pos;
  updatePoints();
  return TFigure::STOP;
}

void
TFConnection::setAttributes(const TFigureAttributes *attributes)
{
  super::setAttributes(attributes);
  TFigureArrow::setAttributes(attributes);
}

void
TFConnection::getAttributes(TFigureAttributes *attributes) const
{
  super::getAttributes(attributes);
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
