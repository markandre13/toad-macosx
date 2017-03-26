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

size_t TFText::cx = 0;

void 
TFText::calcSize()
{
  TCoord w=0, h=0;
  cout << "TFText::calcSize()" << endl;
  for(auto &&line: wp.document.lines) {
    cout << "  line " << line->size.width << ", " << line->size.height << endl;
    w = max(w, line->size.width);
    h += line->size.height;
  }
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
  cx = 0;
  return true;
}

void 
TFText::startCreate()
{
  cx = 0;
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
  switch(editor->state) {
    case TFigureEditor::STATE_START_CREATE:
      cx = 0;
      p1 = m.pos;
      calcSize();
      editor->invalidateFigure(this);
      startInPlace();
      return CONTINUE|NOGRAB;
      
    case TFigureEditor::STATE_CREATE:
    case TFigureEditor::STATE_EDIT:
      if (distance(m.pos)>RANGE) {
        editor->invalidateFigure(this);
        if (text.empty())
          return STOP|DELETE|REPEAT;
        return STOP|REPEAT;
      }
      break;
      
    default:
      break;
  }
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
}

bool
TFText::restore(TInObjectStream &in)
{
  if (
    ::restore(in, "text", &text) ||
    ::restore(in, "fontname", &fontname) ||
    super::restore(in)
  ) {
    if (in.what == ATV_FINISHED) {
      wp.init(text);
      calcSize();
    }
    return true;
  }
  ATV_FAILED(in)
  return false;
}
