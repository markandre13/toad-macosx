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

#include <toad/figure.hh>
#include <toad/figureeditor.hh>

using namespace toad;
TFText::TFText() {
  relation = nullptr;
  fontname = "arial,helvetica,sans-serif:size=12";
  wp.init(text);
  calcSize();
}

TFText::TFText(const TFText &t) {
  relation = nullptr;
  matrix = t.matrix;
  fontname = t.fontname;
  wp.init(text);
  calcSize();
}

TFText::TFText(TCoord x,TCoord y, const string &aText, TFigure *aRelation) {
  matrix.translate(x, y);
  fontname = "arial,helvetica,sans-serif:size=12";
  text = aText;
  relation = aRelation;
  if (relation) {
    TRectangle bounds = relation->bounds();
    matrix.identity();
    matrix.translate(bounds.origin);
    matrix.translate(3,3);
  }
  wp.init(text);
  calcSize();
}

bool
TFText::editEvent(TFigureEditEvent &editEvent)
{
  // FIXME: add helper functions for RELATION_REMOVED and REMOVED
  switch(editEvent.type) {
    case TFigureEditEvent::RELATION_MODIFIED: {
      if (!relation) {
        cout << "TFText::editEvent: event for relation modifed but pointer is null" << endl;
        return false;
      }
      TRectangle bounds = relation->bounds();
      matrix.translate(-bounds.origin);
      matrix.translate(-3, -3);
      // in theory, an TFText with a relation is wrapped inside the relation and
      // we shouldn't need to invalidate
      } break;
    case TFigureEditEvent::RELATION_REMOVED:
      if (editEvent.model->figures.find(relation) != editEvent.model->figures.end()) {
        relation = nullptr;
      }
      break;
    case TFigureEditEvent::REMOVED: {
      if (relation) {
        auto p = TFigureEditor::relatedTo.find(relation);
        if (p!=TFigureEditor::relatedTo.end()) {
          p->second.erase(this);
          if (p->second.empty())
            TFigureEditor::relatedTo.erase(p);
        }
      }
    } break;
    case TFigureEditEvent::TRANSLATE:
      if (relation)
        break;
    default:
      return super::editEvent(editEvent);
      ;
  }
  return false;
}

void 
TFText::calcSize()
{
//cout << "TFText::calcSize()" << endl;
  size.width = size.height = 0;
  for(auto &&line: wp.document.lines) {
//cout << "  line " << line->size.width << ", " << line->size.height << endl;
    size.width  = max(size.width, line->size.width);
    size.height += line->size.height;
  }
//cout << "  new area: " << p1 << ", " << p2 << endl;
}

void 
TFText::paint(TPenBase &pen, EPaintType type)
{
  if (type==TFigure::EDIT)
    return;
  pen.push();
  pen.setFont(fontname);
  pen.setColor(line_color);
  pen.transform(matrix);
  wp.renderPrepared(pen);
  pen.pop();
}

bool
TFText::transform(const TMatrix2D &matrix)
{
  if (relation) {
    relation->transform(matrix);
    TRectangle r = relation->bounds();
    this->matrix.tx = r.origin.x+3;
    this->matrix.ty = r.origin.y+3;
    calcSize();
    return true;
  }
  this->matrix = matrix * this->matrix;
  calcSize();
  return true;
}

TCoord
TFText::distance(const TPoint &pos)
{
/*
  if (relation) {
    return relation->distance(pos);
  }
*/
  TRectangle r;
  r.size = size;
  TPolygon p(r);
  p.transform(matrix);
  if (p.isInside(pos))
    return INSIDE;
  return p.distance(pos);
}

TRectangle
TFText::bounds() const
{
  if (relation) {
    return relation->bounds();
  }
  TRectangle r;
  r.size = size;
  TPolygon p(r);
  p.transform(matrix);
  p.getShape(&r);
  return r;
}

bool
TFText::getHandle(unsigned, TPoint*)
{
  return false;
}

bool 
TFText::startInPlace()
{
  return true;
}

void 
TFText::startCreate(const TPoint&)
{
}

unsigned 
TFText::stop(TFigureEditor*)
{
/*
  if (text.empty())
    return STOP|DELETE;
  return STOP;
*/
  return 0;
}

unsigned 
TFText::keyDown(TFigureEditor *editor, TKey key, char *str, unsigned modifier)
{
  TKeyEvent ke(TKeyEvent::DOWN, key, str, modifier);
  if (wp.keyDown(ke)) {
    editor->invalidateFigure(this);
    calcSize();
    editor->invalidateFigure(this);
  }
  return 0;
}

unsigned 
TFText::mouseLDown(TFigureEditor *editor, TMouseEvent &m)
{
  return 0;
}

unsigned
TFText::mouseMove(TFigureEditor*, TMouseEvent &)
{
  return 0;
}

unsigned 
TFText::mouseLUp(TFigureEditor*, TMouseEvent &)
{
  return 0;
}

void
TFText::setAttributes(const TFigureAttributeModel *preferences)
{
  super::setAttributes(preferences);
  if (preferences->reason.fontname) {
    fontname = preferences->fontname;
    calcSize();
  }
  
/*
  switch(preferences->reason) {
    case TFigureAttributeModel::ALLCHANGED:
    case TFigureAttributeModel::FONTNAME:
      fontname = preferences->fontname;
      calcSize();
      break;
  }
*/
}

void
TFText::getAttributes(TFigureAttributeModel *preferences) const
{
  super::getAttributes(preferences);
  preferences->fontname = fontname;
}

void 
TFText::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "fontname", fontname);
  ::store(out, "text", text);
  ::storePointer(out, "relation", relation);
}

bool
TFText::restore(TInObjectStream &in)
{
  if (::finished(in)) {
    TFigureEditor::restoreRelation(const_cast<const TFigure**>(&relation), this);
    wp.init(text);
    calcSize();
  }
  if (
    ::restore(in, "text", &text) ||
    ::restore(in, "fontname", &fontname) ||
    ::restorePointer(in, "relation", &relation) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}
