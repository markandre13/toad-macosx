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

#include "directselectiontool.hh"

using namespace fischland;

TDirectSelectionTool*
TDirectSelectionTool::getTool()
{
  static TDirectSelectionTool* tool = 0;
  if (!tool)
    tool = new TDirectSelectionTool();
  return tool;
}

void
TDirectSelectionTool::stop(TFigureEditor *fe)
{
  fe->getWindow()->setAllMouseMoveEvents(false);
  fe->getWindow()->setCursor(0);
  fe->state = TFigureEditor::STATE_NONE;
  fe->quick = false;
  if (figure) {
    fe->invalidateFigure(figure);
    figure = 0;
  }
}

void
TDirectSelectionTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  // figure was deleted but we were not informed...
  if (figure && fe->selection.find(figure)==fe->selection.end()) {
    stop(fe);
    return;
  }
  
  switch(me.type) {
    case TMouseEvent::RDOWN: {
      TPoint pos;
      fe->mouse2sheet(me.pos, &pos);
      TFigure *f =  fe->findFigureAt(pos);
      if (f && f==figure) {
        TMouseEvent me2(me, pos);
        f->mouseRDown(fe, me2);
      }
    } break;
    case TMouseEvent::LDOWN: {
      fe->getWindow()->setAllMouseMoveEvents(true);
      fe->quick = true;
//cout << "mouse down" << endl;
      if (figure && !(me.dblClick) ) {
//cout << "  selection is not empty" << endl;
        TPoint pos;
        fe->mouse2sheet(me.pos, &pos);
        // map desktop (mx,my) to figure (x,y) (copied from findFigureAt)
        TCoord x, y;
        if (figure->mat || figure->cmat) {
          TMatrix2D m;
          if (figure->mat)
            m = *figure->mat;
          if (figure->cmat)
            m.multiply(figure->cmat);
          m.invert();
          m.map(pos, &pos);
        }

        // loop over all handles
        unsigned h = 0;
        while(true) {
          if (!figure->getHandle(h,&memo_pt))
            break;
          if (memo_pt.x-fe->fuzziness<=pos.x && pos.x<=memo_pt.x+fe->fuzziness &&
              memo_pt.y-fe->fuzziness<=pos.y && pos.y<=memo_pt.y+fe->fuzziness)
          {
//cout << "got handle " << h << endl;
            fe->getFigureShape(figure, &oldshape, fe->getMatrix());
            oldshape.x+=fe->getVisible().x;
            oldshape.y+=fe->getVisible().y;
            handle = h;
            hndl = true;
            tht = figure->startTranslateHandle();
            TUndoManager::beginUndoGrouping(fe->getModel());
            return;
          }
          ++h;
        }
      }

      TPoint pos;
      fe->mouse2sheet(me.pos, &pos);
//cout << "mouse:" <<me.x<<","<<me.y<<", sheet:"<<x<<","<<y<<endl;
      TFigure *f = fe->findFigureAt(pos);
      if (f!=figure) {
        if (figure)
          fe->invalidateFigure(figure);
        figure = f;
        if (figure)
          fe->invalidateFigure(figure);
        fe->getWindow()->invalidateWindow();
        fe->quickready = false;
        fe->clearSelection();
        if (figure) {
cout << "selected figure " << figure << endl;
          fe->selection.insert(figure);
        }
      } else {
//          cout << "start drag" << endl;
#if 0
          fe->sheet2grid(x, y, &last_x, &last_y);
          last_sx = me.x;
          last_sy = me.y;
          grab = true;
          cout << "start move" << endl;
#endif
      }
    } break;
    case TMouseEvent::MOVE:
      if (hndl) {
        TPoint pos;
        fe->mouse2sheet(me.pos, &pos);
        fe->sheet2grid(pos, &pos);
        if (tht && (figure->mat || figure->cmat)) {
          TMatrix2D m;
          if (figure->mat)
            m = *figure->mat;
          if (figure->cmat)
            m.multiply(figure->cmat);
          m.invert();
          m.map(pos, &pos);
        }
        fe->getModel()->translateHandle(figure, handle, pos.x, pos.y, me.modifier());
      } else
      if (figure) {

        TPoint pos;
        fe->mouse2sheet(me.pos, &pos);
        // map desktop (mx,my) to figure (x,y) (copied from findFigureAt)
        if (figure->mat || figure->cmat) {
          TMatrix2D m;
          if (figure->mat)
            m = *figure->mat;
          if (figure->cmat)
            m.multiply(figure->cmat);
          m.invert();
          m.map(pos, &pos);
        }

        // loop over all handles
        unsigned h = 0;
        while(true) {
          if (!figure->getHandle(h,&memo_pt))
            break;
          if (memo_pt.x-fe->fuzziness<=pos.x && pos.x<=memo_pt.x+fe->fuzziness &&
              memo_pt.y-fe->fuzziness<=pos.y && pos.y<=memo_pt.y+fe->fuzziness)
          {
//cout << "got handle " << h << endl;
            fe->getWindow()->setCursor(TCursor::RESIZE);
            return;
          }
          ++h;
        }
        fe->getWindow()->setCursor(TCursor::DEFAULT);
      }
      break;
    case TMouseEvent::LUP:
      if (hndl) {
        TUndoManager::endUndoGrouping(fe->getModel());
        hndl = false;
        fe->quickready = false;
        fe->getWindow()->invalidateWindow(oldshape);
        fe->invalidateFigure(figure);
      }
      break;
  }
}

bool
TDirectSelectionTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (!figure)
    return false;

  // figure was deleted but we were not informed...
  if (fe->selection.find(figure)==fe->selection.end()) {
    stop(fe);
    return false;
  }

  pen.push();
  pen.setColor(TColor::FIGURE_SELECTION);
  if (figure->mat)
    pen.multiply(figure->mat);
  pen.setLineWidth(1);
  figure->paint(pen);
  pen.setLineWidth(1);
  figure->paintSelection(pen, -1);
  pen.pop();
  return true;
}
