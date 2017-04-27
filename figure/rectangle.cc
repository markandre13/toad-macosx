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

#include <toad/vector.hh>
#include <toad/figure.hh>
#include <toad/figureeditor.hh>

using namespace toad;

TRectangle
TFRectangle::bounds() const
{
  return TRectangle(p1,p2);
}

void 
TFRectangle::paint(TPenBase &pen, EPaintType)
{
  pen.push();
  pen.setAlpha(alpha);
  if (filled) {
    pen.setFillColor(fill_color);
    pen.fillRectangle(p1,p2);
  }
  if (outline) {
    pen.setStrokeColor(line_color);
    pen.setLineStyle(line_style);
    pen.setLineWidth(line_width);
//cout << "TFRectangle: draw rectangle between " << p1 << " and " << p2 << endl;
    pen.drawRectangle(p1,p2);
  }
  pen.pop();
}

TCoord 
TFRectangle::distance(const TPoint &pos)
{
  if (filled && TRectangle(p1, p2).isInside(pos))
    return INSIDE;

  TCoord x1,y1,x2,y2;
  TCoord min = OUT_OF_RANGE, d;
  
  for(int i=0; i<4; i++) {
    switch(i) {
    case 0:
      x1=p1.x; y1=p1.y; x2=p2.x; y2=p1.y;
      break;
    case 1:
      x1=p2.x; y1=p2.y;
      break;
    case 2:
      x2=p1.x; y2=p2.y;
      break;
    case 3:
      x1=p1.x; y1=p1.y;
      break;
    }
    d = distance2Line(pos.x, pos.y, x1,y1, x2,y2);
    if (d<min)
      min = d;
  }
  return min;
}

void 
TFRectangle::translate(TCoord dx, TCoord dy)
{
  p1.x+=dx;
  p1.y+=dy;
  p2.x+=dx;
  p2.y+=dy;
}

bool 
TFRectangle::getHandle(unsigned handle, TPoint *p)
{
  switch(handle) {
    case 0:
      *p = p1;
      break;
    case 1:
      p->x = p2.x;
      p->y = p1.y;
      break;
    case 2:
      *p = p2;
      break;
    case 3:
      p->x = p1.x;
      p->y = p2.y;
      break;
  }
  if (handle<4)
    return true;
  return false;
}

void 
TFRectangle::translateHandle(unsigned handle, TCoord x, TCoord y, unsigned)
{
  switch(handle) {
    case 0:
      p1.x = x;
      p1.y = y;
      break;
    case 1:
      p2.x = x;
      p1.y = y;
      break;
    case 2:
      p2.x = x;
      p2.y = y;
      break;
    case 3:
      p1.x = x;
      p2.y = y;
      break;
  }
}

unsigned 
TFRectangle::mouseLDown(TFigureEditor *editor, TMouseEvent &m)
{
  switch(editor->state) {
    case TFigureEditor::STATE_START_CREATE:
      p1 = p2 = m.pos;
      editor->invalidateFigure(this);
      break;
    default:
      break;
  }
  return CONTINUE;
}

unsigned 
TFRectangle::mouseMove(TFigureEditor *editor, TMouseEvent &m)
{
  switch(editor->state) {
    case TFigureEditor::STATE_CREATE:
      editor->invalidateFigure(this);
      p2 = m.pos;
      editor->invalidateFigure(this);
      break;
    default:
      break;
  }
  return CONTINUE;
}

unsigned 
TFRectangle::mouseLUp(TFigureEditor *editor, TMouseEvent &m)
{
  switch(editor->state) {
    case TFigureEditor::STATE_CREATE:
      mouseMove(editor, m);
      if (p1.x==p2.x && p1.y==p2.y)
        return STOP|DELETE;
      return STOP;
    default:
      break;
  }
  return CONTINUE;
}

TVectorGraphic*
TFRectangle::getPath() const
{
  // auto vb = make_shared<sync_queue>(200);
  
  auto *path = new TVectorPath;
  path->move(p1.x, p1.y);
  path->line(p2.x, p1.y);
  path->line(p2.x, p2.y);
  path->line(p1.x, p2.y);
  path->close();
  
  auto *vg = new TVectorGraphic;
  vg->push_back(new TVectorPainter(this, path));
  return vg;
}

void
TFRectangle::store(TOutObjectStream &out) const
{
  super::store(out);
  TRectangle r(p1,p2);
  ::store(out, "x", r.origin.x);
  ::store(out, "y", r.origin.y);
  ::store(out, "w", r.size.width);
  ::store(out, "h", r.size.height);
}

bool
TFRectangle::restore(TInObjectStream &in)
{
  #warning "TFRectangle::restore: uses static variable"
  static TRectangle r;

  if (::finished(in)) {
    setShape(r);
    return true;
  }
  if (
    ::restore(in, "x", &r.origin.x) ||
    ::restore(in, "y", &r.origin.y) ||
    ::restore(in, "w", &r.size.width) ||
    ::restore(in, "h", &r.size.height) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}
