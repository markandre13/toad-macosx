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
  p1.x = p1.y = 0;
  fontname = "arial,helvetica,sans-serif:size=12";
  wp.init(text);
  calcSize();
}

TFText::TFText(const TFText &t) {
  relation = nullptr;
  p1 = t.p1;
  p2 = t.p2;
  fontname = t.fontname;
  wp.init(text);
  calcSize();
}

TFText::TFText(TCoord x,TCoord y, const string &aText, TFigure *aRelation) {
  p1.x = x;
  p1.y = y;
  fontname = "arial,helvetica,sans-serif:size=12";
  text = aText;
  relation = aRelation;
  if (relation) {
    TRectangle bounds = relation->bounds();
    p1.set(bounds.x+3, bounds.y+3);
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
      TPoint difference = TPoint(p1.x-bounds.x-3, p1.y-bounds.y-3);
      p1 -= difference;
      p2 -= difference;
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
  TCoord w=0, h=0;
//cout << "TFText::calcSize()" << endl;
  for(auto &&line: wp.document.lines) {
//cout << "  line " << line->size.width << ", " << line->size.height << endl;
    w = max(w, line->size.width);
    h += line->size.height;
  }
//cout << "  new area: " << p1 << ", " << p2 << endl;
  p2.x=p1.x+w;
  p2.y=p1.y+h;
}

void 
TFText::paint(TPenBase &pen, EPaintType type)
{
  if (type==TFigure::EDIT)
    return;
  pen.push();
  pen.setFont(fontname);
  pen.setColor(line_color);
  pen.translate(p1.x, p1.y);
  wp.renderPrepared(pen);
  pen.pop();
}

TCoord
TFText::distance(const TPoint &pos)
{
  TRectangle r(p1, p2);
// cerr << "mouse at (" << mx << ", " << my << "), text " << r << endl;

  if (TRectangle(p1, p2).isInside(pos))
    return INSIDE;
  return super::distance(pos);
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
TFText::startCreate()
{
}

unsigned 
TFText::stop(TFigureEditor*)
{
  if (text.empty())
    return STOP|DELETE;
  return STOP;
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
  return CONTINUE;
}

unsigned 
TFText::mouseLDown(TFigureEditor *editor, TMouseEvent &m)
{
  return CONTINUE;
}

unsigned
TFText::mouseMove(TFigureEditor*, TMouseEvent &)
{
  return CONTINUE;
}

unsigned 
TFText::mouseLUp(TFigureEditor*, TMouseEvent &)
{
  return CONTINUE;
}

void
TFText::setAttributes(const TFigureAttributes *preferences)
{
  super::setAttributes(preferences);
  if (preferences->reason.fontname) {
    fontname = preferences->fontname;
    calcSize();
  }
  
/*
  switch(preferences->reason) {
    case TFigureAttributes::ALLCHANGED:
    case TFigureAttributes::FONTNAME:
      fontname = preferences->fontname;
      calcSize();
      break;
  }
*/
}

void
TFText::getAttributes(TFigureAttributes *preferences) const
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
