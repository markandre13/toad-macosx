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

#include "selectiontool.hh"

using namespace fischland;

TSelectionTool*
TSelectionTool::getTool()
{
  static TSelectionTool* tool = 0;
  if (!tool)
    tool = new TSelectionTool();
  return tool;
}

void
TSelectionTool::stop(TFigureEditor *fe)
{
  fe->getWindow()->setAllMouseMoveEvents(true);
  fe->getWindow()->setCursor(0);
  fe->state = TFigureEditor::STATE_NONE;
}

void
TSelectionTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TFigure *figure;
  TCoord x, y;
  TRectangle r;

  switch(me.type) {
    case TMouseEvent::LDOWN:
      if (fe->state == TFigureEditor::STATE_NONE) {
        fe->state = TFigureEditor::STATE_CREATE;
        fe->getWindow()->setAllMouseMoveEvents(true);
      }
      if (!fe->selection.empty()) {
        // origin is already applied by scroll pane?
        TCoord x = me.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
        TCoord y = me.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;
//        cout << "down at " << x << ", " << y << endl;
        for(unsigned i=0; i<8; ++i) {
          getBoundingHandle(i, &r);
//          cout << "  check " << r.x << ", " << r.y << endl;
          if (r.isInside(x, y)) {
            handle = i;
            ox0 = x0;
            oy0 = y0;
            ox1 = x1;
            oy1 = y1;
            hndl = true;
            oldmat.clear();
            for(TFigureSet::const_iterator p = fe->selection.begin();
                p != fe->selection.end();
                ++p)
            {
              if ( (*p)->mat || (*p)->cmat ) {
                if (scale_strokes_and_fx)
                  oldmat.push_back(*(*p)->mat);
                else
                  oldmat.push_back(*(*p)->cmat);
              } else {
                oldmat.push_back(TMatrix2D());
              }
            }
//            cout << "its inside handle " << i << endl;
            return;
          }
        }
      }
      fe->mouse2sheet(me.x, me.y, &x, &y);
      figure = fe->findFigureAt(x, y);
      if (fe->selection.find(figure) != fe->selection.end()) {
        fe->sheet2grid(x, y, &last_x, &last_y);
        last_sx = me.x;
        last_sy = me.y;
        grab = true;
        break;
      }
      
      if (!(me.modifier() & MK_SHIFT)) {
        fe->clearSelection();
      }
      if (!selection.empty()) {
        fe->invalidateWindow();
      }
      selection.clear();
      if (figure) {
        selection.insert(figure);
        fe->invalidateWindow();
      }
      down = true;
      rx0 = rx1 = me.x;
      ry0 = ry1 = me.y;
      break;
    case TMouseEvent::MOVE:
      if (!hndl) {
        if (!fe->selection.empty()) {
          // origin is already applied by scroll pane?
          int x = me.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
          int y = me.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;
          TCursor::EType cursor = TCursor::DEFAULT;
          for(unsigned i=0; i<8; ++i) {
            getBoundingHandle(i, &r);
            if (r.isInside(x, y)) {
              static const TCursor::EType wind[8] = {
               TCursor::NW_RESIZE,
               TCursor::N_RESIZE,
               TCursor::NE_RESIZE,
               TCursor::E_RESIZE,
               TCursor::SE_RESIZE,
               TCursor::S_RESIZE,
               TCursor::SW_RESIZE,
               TCursor::W_RESIZE
              };
              cursor = wind[i];
              break;
            }
          }
          fe->getWindow()->setCursor(cursor);
        }
      } else {
        // mouse is holding a handle, scale the selection
        invalidateBounding(fe);
        TCoord x, y;
        x = me.x - fe->getVisible().x;
        y = me.y - fe->getVisible().y;
        switch(handle) {
          case 0:
            x0 = x;
            y0 = y;
            break;
          case 1:
            y0 = y;
            break;
          case 2:
            x1 = x;
            y0 = y;
            break;
          case 3:
            x1 = x;
            break;
          case 4:
            x1 = x;
            y1 = y;
            break;
          case 5:
            y1 = y;
            break;
          case 6:
            x0 = x;
            y1 = y;
            break;
          case 7:
            x0 = x;
            break;
        }

        TCoord sx = (x1-x0)/(ox1 - ox0);
        TCoord sy = (y1-y0)/(oy1 - oy0);
        TCoord X0, OX0, Y0, OY0;
        if (fe->getMatrix()) {
          TMatrix2D m(*fe->getMatrix());
          m.invert();
          m.map(x0, y0, &X0, &Y0);
          m.map(ox0, oy0, &OX0, &OY0);
        } else {
          X0 = x0; Y0 = y0;
          OX0 = ox0; OY0 = oy0;
        }
        TMatrix2D m;
        m.translate(X0, Y0);
        m.scale(sx, sy);
        m.translate(-OX0, -OY0);

        unsigned i=0;
        for(TFigureSet::iterator p = fe->selection.begin();
            p != fe->selection.end();
            ++p, ++i)
        {
          if (scale_strokes_and_fx) {
            if ((*p)->mat) {
              *(*p)->mat = m * oldmat[i];
            } else {
              (*p)->mat = new TMatrix2D(m);
            }
          } else {
            if ((*p)->cmat) {
              *(*p)->cmat = m * oldmat[i];
            } else {
              (*p)->cmat = new TMatrix2D(m);
            }
          }
        }

        invalidateBounding(fe);
        break;
      }
      if (grab) {
        TCoord x, y;
        fe->mouse2sheet(me.x, me.y, &x, &y);
        fe->sheet2grid(x, y, &x, &y);
        TCoord dx = x-last_x;
        TCoord dy = y-last_y;
        last_x=x;
        last_y=y;
        
        invalidateBounding(fe);

        fe->getModel()->translate(fe->selection, dx, dy);
        #if 0
        dx = me.x - last_sx;
        dy = me.y - last_sy;
        last_sx = me.x;
        last_sy = me.y;
        x0 += dx;
        y0 += dy;
        x1 += dx;
        y1 += dy;
        #else
        calcSelectionsBoundingRectangle(fe);
        #endif
        invalidateBounding(fe);
        break;
      }
      if (down) {
        if (!rect) {
          if (me.x < rx0-2 || me.x > rx0+2 || me.y < ry0-2 || me.y > ry0+2) {
            rect = true;
            selection.clear();
          } else {
            break;
          }
        }
        fe->getWindow()->invalidateWindow(
          rx0 + fe->getWindow()->getOriginX(),
          ry0 + fe->getWindow()->getOriginY(),
          rx1-rx0+3,ry1-ry0+2);
        rx1 = me.x;
        ry1 = me.y;
        fe->getWindow()->invalidateWindow(
          rx0 + fe->getWindow()->getOriginX(),
          ry0 + fe->getWindow()->getOriginY(),
          rx1-rx0+3,ry1-ry0+2);
        selection.clear();
        TFigureModel::const_iterator p, e;
        p = fe->getModel()->begin();
        e = fe->getModel()->end();
        TPoint p0, p1;
        fe->mouse2sheet(rx0, ry0, &p0.x, &p0.y);
        fe->mouse2sheet(rx1, ry1, &p1.x, &p1.y);
        TRectangle r0(p1, p0), r1;
        while(p!=e) {
#if 1
          fe->getFigureShape(*p, &r1, NULL);
#else
          (*p)->getShape(&r1);
          
          #warning "why isn't the following already done by getShape?"
          if ( (*p)->mat ) {
            TPoint p0, p1;
            (*p)->mat->map(r1.x       , r1.y       , &p0.x, &p0.y);
            (*p)->mat->map(r1.x+r1.w-1, r1.y+r1.h-1, &p1.x, &p1.y);
            r1.set(p0, p1);
          }
#endif
          if (r0.isInside( r1.x, r1.y ) &&
              r0.isInside( r1.x+r1.w, r1.y+r1.h ) )
          {
            selection.insert(*p);
          }
          ++p;
        }
      }
      break;
    case TMouseEvent::LUP:
      if (hndl) {
        hndl = false;
        // oldmat should be used for undo
        oldmat.clear();
        break;
      }
      if (grab) {
        grab = false;
        break;
      }
      down = rect = false;
      
      // add new selection to figure editors selection
      fe->selection.insert(selection.begin(), selection.end());
      selection.clear();

      // calculate the selections bounding rectangle
      calcSelectionsBoundingRectangle(fe);
      fe->invalidateWindow();
      break;
    default:
      ;
  }
}

void
TSelectionTool::invalidateBounding(TFigureEditor *fe)
{
  fe->getWindow()->invalidateWindow(
    x0-4 + fe->getWindow()->getOriginX() + fe->getVisible().x,
    y0-4 + fe->getWindow()->getOriginY() + fe->getVisible().y,
    x1-x0+10,y1-y0+10);
#if 0
  cout << "invalidate bounding ("
       << x0-4 + fe->getWindow()->getOriginX() + fe->getVisible().x
       << ", "
       << y0-4 + fe->getWindow()->getOriginY() + fe->getVisible().y
       << ", "
       << x1-x0+9
       << ", "
       << y1-y0+9;
#endif
}


void
TSelectionTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (down) {
    // draw the selection marks over all figures
    for(TFigureSet::iterator sp = selection.begin();
        sp != selection.end();
        ++sp)
    {
      if ((*sp)->mat) {
        pen.push();
        pen.multiply( (*sp)->mat );
      }
      pen.setLineWidth(1);   
      (*sp)->paintSelection(pen, -1);
      if ((*sp)->mat)
        pen.pop(); 
    }
    
    // draw selection rectangle
    if (rect) {
      pen.push();
      double tx = 0.0, ty = 0.0;
      if (pen.getMatrix()) {
        tx = pen.getMatrix()->tx - fe->getVisible().x;
        ty = pen.getMatrix()->ty - fe->getVisible().y;
      }
      pen.identity();
      pen.translate(tx, ty);
      pen.setColor(TColor::FIGURE_SELECTION);
      pen.setLineWidth(1);
      pen.setAlpha(0.3);
      if (pen.getAlpha()!=1.0)
        pen.fillRectangle(rx0, ry0, rx1-rx0+1, ry1-ry0);
      else
        pen.drawRectangle(rx0, ry0, rx1-rx0+1, ry1-ry0);
      pen.setAlpha(1.0);
      pen.drawRectangle(rx0, ry0, rx1-rx0+1, ry1-ry0);
      pen.pop();
    }
  } else
  if (!fe->selection.empty()) {
//    cout << "draw bounding rectangle " << x0 << ", " << y0 << " to " << x1 << ", " << y1 << endl;
    const TMatrix2D *m = 0;
    if (pen.getMatrix()) {
      m = pen.getMatrix();
      double tx, ty;
      m->map(0.0, 0.0, &tx, &ty);
      pen.push();
      pen.identity();
      pen.translate(tx, ty);
    }
    pen.setColor(TColor::FIGURE_SELECTION);
    pen.setLineWidth(1);
    pen.setAlpha(1.0);
    pen.drawRectangle(x0, y0, x1-x0, y1-y0);

    pen.setFillColor(1,1,1);
    TRectangle r;
    for(unsigned i=0; i<8; ++i) {
      getBoundingHandle(i, &r);
      pen.fillRectangle(r);
    }
    
    if (pen.getMatrix()) {
      pen.pop();
    }
  }
}

void
TSelectionTool::getBoundingHandle(unsigned i, TRectangle *r)
{
  int w = x1 - x0;
  int h = y1 - y0;
  switch(i) {
    case 0: r->set(x0-3  , y0-3  , 5, 5); break;
    case 1: r->set(x0+w/2, y0-3  , 5, 5); break;
    case 2: r->set(x0+w-1, y0-3  , 5, 5); break;
    case 3: r->set(x0+w-1, y0+h/2, 5, 5); break;
    case 4: r->set(x0+w-1, y0+h-1, 5, 5); break;
    case 5: r->set(x0+w/2, y0+h-1, 5, 5); break;
    case 6: r->set(x0-3,   y0+h-1, 5, 5); break;
    case 7: r->set(x0-3,   y0+h/2, 5, 5); break;
  }
}
      
void
TSelectionTool::calcSelectionsBoundingRectangle(TFigureEditor *fe)
{
  for(TFigureSet::const_iterator p = fe->selection.begin();
      p != fe->selection.end();
      ++p)
  {
    TRectangle r;
#if 1
    fe->getFigureShape(*p, &r, NULL);
#else
    (*p)->getShape(&r);
    if ( (*p)->mat ) {
      TPoint p0, p1;
      (*p)->mat->map(r.x, r.y,             &p0.x, &p0.y);
      (*p)->mat->map(r.x+r.w-1, r.y+r.h-1, &p1.x, &p1.y);
      r.set(p0, p1);
    }
#endif
    if (p==fe->selection.begin()) {
      x0 = r.x; 
      y0 = r.y;
      x1 = r.x + r.w;
      y1 = r.y + r.h;
    } else {
      if (x0 > r.x) x0 = r.x;
      if (y0 > r.y) y0 = r.y;
      if (x1 < r.x + r.w) x1 = r.x + r.w;
      if (y1 < r.y + r.h) y1 = r.y + r.h;
    }
  }

  // map figure coordinates to screen coordinates
  if (fe->getMatrix()) {
    fe->getMatrix()->map(x0, y0, &x0, &y0);
    fe->getMatrix()->map(x1, y1, &x1, &y1);
    --x0;
    --y0;
    --x1;
    --y1;
/*
      x0 += fe->getVisible().x;
      x1 += fe->getVisible().x;
      y0 += fe->getVisible().y;
      y1 += fe->getVisible().y;
*/
  }
}
