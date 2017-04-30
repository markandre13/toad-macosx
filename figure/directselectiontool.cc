/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2017 by Mark-André Hopf <mhopf@mark13.org>
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

// FIXME: the directselection tool in Illustrator CS2 is not why I thought it was
//        there the pencil tool is accompanied by three tools to
//        o add points
//        o remove points
//        o move points
//        instead I am going to make this more like the knot tool in Affinity Designer
//        o click on the outline to add a point
//        o click on a point to select it, del key can then delete it
//        o click'n drag point to move it
//        o click'n drag outline to adjust control points so that outline follows mouse

#include <toad/figure/directselectiontool.hh>

using namespace toad;

TDirectSelectionTool*
TDirectSelectionTool::getTool()
{
  static TDirectSelectionTool* tool = nullptr;
  if (!tool)
    tool = new TDirectSelectionTool();
  return tool;
}

void
TDirectSelectionTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  if (state == STATE_FIGURE_HANDLES_MOUSE) {
    TPoint pos;
    fe->mouse2sheet(me.pos, &pos);
    TMouseEvent me2(me, pos);
    switch(me.type) {
      case TMouseEvent::MOVE:
        figure->mouseMove(fe, me2);
        break;
      case TMouseEvent::LUP:
        figure->mouseLUp(fe, me2);
      default:
        state = STATE_NONE;
    }
    fe->invalidateFigure(figure);
    return;
  }

#if 0
  // figure was deleted but we were not informed...
  if (figure && fe->selection.find(figure)==fe->selection.end()) {
    stop(fe);
    return;
  }
#endif
  
  switch(me.type) {
#if 0
    case TMouseEvent::RDOWN: {
      TPoint pos;
      fe->mouse2sheet(me.pos, &pos);
      TFigure *f =  fe->findFigureAt(pos);
      if (f && f==figure) {
        TMouseEvent me2(me, pos);
        f->mouseRDown(fe, me2);
      }
    } break;
#endif

    case TMouseEvent::LDOWN: {
      fe->start();
      fe->getWindow()->setAllMouseMoveEvents(true);
      fe->quick = true;
      
      TPoint pos;
      fe->mouse2sheet(me.pos, &pos);

      // when a figure has been selected, handle the handles
      if (figure && !me.dblClick ) {

        if (handleLDown(fe, pos))
          return;
/*
        // this is something i added for TFConnection to add new points in
        // edit mode, but it breaks switching to another figure
        TMouseEvent me2(me, pos);
        if (figure->mouseLDown(fe, me2)!=TFigure::STOP) {
          cout << "TDirectSelectionTool: figure handles mouse" << endl;
          state = STATE_FIGURE_HANDLES_MOUSE;
          return;
        }
*/
      }

      TFigure *f = fe->findFigureAt(pos);
      if (f!=figure) {
        if (figure)
          fe->invalidateFigure(figure);
        figure = f;
        if (figure)
          fe->invalidateFigure(figure);

        // fe->getWindow()->invalidateWindow();
        fe->quickready = false;
        
        fe->clearSelection();
        if (figure) {
cout << "TDirectSelectionTool::selected figure " << figure << " " << figure->getClassName() << endl;
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
        
        TCoord d, v;
        
        v = pos.x;
        d = numeric_limits<TCoord>::max();
        if (xaxis)
          fe->invalidateWindow(); // FIXME: invalidates too much
        xaxis = false;
        for(TCoord x: xalign) {
          TCoord d0 = fabs(pos.x - x);
          if (d0 < TFigure::RANGE && d0 < d) {
            v = x;
            d = d0;
            xaxis = true;
            xaxisv = v;
          }
        }
        if (xaxis)
          fe->invalidateWindow(); // FIXME: invalidates too much
        // FIXME: shouldn't invalidate xaxis when it hasn't changed
        pos.x = v;

        if (yaxis)
          fe->invalidateWindow(); // FIXME: invalidates too much
        yaxis = false;
        v = pos.y;
        d = numeric_limits<TCoord>::max();
        for(TCoord y: yalign) {
          TCoord d0 = fabs(pos.y - y);
          if (d0 < TFigure::RANGE && d0 < d) {
            v = y;
            d = d0;
            yaxis = true;
            yaxisv = v;
          }
        }
        pos.y = v;
        if (yaxis)
          fe->invalidateWindow(); // FIXME: invalidates too much
        
        fe->getModel()->translateHandle(figure, handle, pos.x, pos.y, me.modifier());
      } else
      if (figure) {

        TPoint pos;
        fe->mouse2sheet(me.pos, &pos);

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

/**
 *
 * @return 'true' when a handle was found 
 */
bool
TDirectSelectionTool::handleLDown(TFigureEditor *fe, const TPoint &pos)
{        
  unsigned h = 0;
  while(true) {
    if (!figure->getHandle(h, &memo_pt))
      break;
    if (memo_pt.x-fe->fuzziness<=pos.x && pos.x<=memo_pt.x+fe->fuzziness &&
        memo_pt.y-fe->fuzziness<=pos.y && pos.y<=memo_pt.y+fe->fuzziness)
    {
      fe->getFigureShape(figure, &oldshape, fe->getMatrix());
      oldshape.origin += fe->getVisible().origin;
      handle = h;
      hndl = true;
      tht = figure->startTranslateHandle();
      TUndoManager::beginUndoGrouping(fe->getModel());
      
      TPoint pt;
      xalign.clear();
      yalign.clear();
      for(h=0; figure->getHandle(h,&pt); ++h) {
        if (hndl && handle==h)
          continue;
        xalign.insert(pt.x);
        yalign.insert(pt.y);
      }
      
      return true;
    }
    ++h;
  }
  return false;
}

void
TDirectSelectionTool::stop(TFigureEditor *fe)
{
  fe->clearSelection();
  fe->getWindow()->setAllMouseMoveEvents(false);
  fe->getWindow()->setCursor(0);
//  fe->state = TFigureEditor::STATE_NONE;
  fe->quick = false;
  if (figure) {
    fe->invalidateFigure(figure);
    figure = 0;
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

//cout << "TDirectSelectionTool::paintSelection()" << endl;

  pen.push();
  pen.setColor(TColor::FIGURE_SELECTION);
  pen.setLineWidth(1);
/*
  figure->paint(pen, TFigure::EDIT);
  pen.setLineWidth(1);
*/  
  if (xaxis || yaxis) {
    pen.setLineStyle(TPen::DOT);
    if (xaxis)
      pen.drawLine(xaxisv,0, xaxisv, 1000); // FIXME
    if (yaxis)
      pen.drawLine(0, yaxisv, 1000, yaxisv); // FIXME
    pen.setLineStyle(TPen::SOLID);
  }
  
  figure->paintSelection(pen, -1);
  pen.pop();
  return true;
}
