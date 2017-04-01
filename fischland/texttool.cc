/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2007 by Mark-André Hopf <mhopf@mark13.org>
 * Visit http://www.mark13.org/fischland/.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "texttool.hh"
#include "fischland.hh"

#include <toad/boolmodel.hh>
#include <toad/integermodel.hh>

using namespace fischland;

TTextTool*
TTextTool::getTool()
{
  static TTextTool* tool = 0;
  if (!tool)
    tool = new TTextTool();
  return tool;
}

void
TTextTool::cursor(TFigureEditor *fe, int x, int y)
{
  fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT]);
#if 0
  if (!path) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN]);
    return;
  }
  if (down) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN_DRAG]);
    return;
  }
  if (!path->polygon.empty() &&
       path->polygon.front().x-fe->fuzziness<=x && x<=path->polygon.front().x+fe->fuzziness &&
       path->polygon.front().y-fe->fuzziness<=y && y<=path->polygon.front().y+fe->fuzziness)
  {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN_CLOSE]);
    return;
  }
  if (!path->polygon.empty() &&
       path->polygon.back().x-fe->fuzziness<=x && x<=path->polygon.back().x+fe->fuzziness &&
       path->polygon.back().y-fe->fuzziness<=y && y<=path->polygon.back().y+fe->fuzziness)
  {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN_EDGE]);
    return;
  }
  fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN]);
#endif
}

void
TTextTool::stop(TFigureEditor *fe)
{
  text = nullptr;
  TWindow::ungrabMouse();
  fe->getWindow()->setCursor(nullptr);
  fe->state = TFigureEditor::STATE_NONE;
  fe->invalidateWindow();
}

void
TTextTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
//  cout << "TTextTool::mouseEvent " << me.pos << " " << fe->getWindow() << " '" << fe->getWindow()->getTitle() << "' " << me.name() << endl;
  TPoint pos;
  fe->mouse2sheet(me.pos, &pos);

  TFText *textUnderMouse = nullptr;
  TFigure *figureUnderMouse = fe->findFigureAt(pos);
  if (!figureUnderMouse) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT_AREA]);
  } else {
    textUnderMouse = dynamic_cast<TFText*>(figureUnderMouse);
    if (textUnderMouse) {
      fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT]);
    } else {
      fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT_SHAPE]);
    }
  }
  
  switch(me.type) {
    case TMouseEvent::ENTER:
      fe->getWindow()->grabMouse();
      break;
    case TMouseEvent::LEAVE:
      TWindow::ungrabMouse();
      return;
    case TMouseEvent::LDOWN:
      if (textUnderMouse && textUnderMouse != text) {
        textUnderMouse->startInPlace();
        fe->state = TFigureEditor::STATE_EDIT;
        fe->clearSelection();
      }
      if (!textUnderMouse && text)
        fe->invalidateFigure(text);
      text = textUnderMouse;
      if (text) {
        TMouseEvent me2(me, pos);
        text->mouseEvent(me2);
        fe->invalidateFigure(text);
      } else {
        TPoint posInGrid;
        fe->sheet2grid(pos, &posInGrid);
        text = new TFText(posInGrid.x, posInGrid.y, "", figureUnderMouse);
        if (figureUnderMouse) {
          TFigureEditor::relatedTo[figureUnderMouse].insert(text);
        }
        text->removeable = true;
        fe->getAttributes()->setAllReasons();
        text->setAttributes(fe->getAttributes());
        fe->addFigure(text);
        text->startInPlace();
        fe->state = TFigureEditor::STATE_EDIT;
        fe->clearSelection();
        fe->invalidateFigure(text);
      }
      break;
  }
}

void
TTextTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
  if (!text)
    return;

  switch(ke.type) {
    case TKeyEvent::DOWN:
      unsigned r = text->keyDown(fe, ke.key, const_cast<char*>(ke.string.c_str()), ke.modifier);
      // FIXME: delete figure if required
      if (r & TFigure::STOP) {
        stop(fe);
      }
      break;
  }
}

bool
TTextTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
#if 0
  if (!path)
    return false;
  path->paint(pen, TFigure::EDIT);

  const TMatrix2D *m0 = pen.getMatrix();
  if (m0) {
    pen.push();
    pen.identity();
  }
  TPolygon &polygon = path->polygon;
  int i = polygon.size();

  if (i<4 || (i%3)!=1)  
    return true;
  --i;
  int x0 = polygon[i].x;
  int y0 = polygon[i].y;
  int x1 = polygon[i].x - (polygon[i-1].x - polygon[i].x);
  int y1 = polygon[i].y - (polygon[i-1].y - polygon[i].y);
  if (m0) {
    m0->map(x0, y0, &x0, &y0);
    m0->map(x1, y1, &x1, &y1);
  }
  pen.drawLine(x0, y0, x1, y1);
  pen.fillCirclePC(x1-2,y1-2,6,6);

  if (m0) {
    pen.pop();
  }
  return true;
#endif
  return false;
}
