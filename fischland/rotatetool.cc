/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 2014 by Mark-André Hopf <mhopf@mark13.org>
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

#include "rotatetool.hh"
#include "fischland.hh"

TRotateTool*
TRotateTool::getTool()
{
  static TRotateTool* tool = 0;
  if (!tool)
    tool = new TRotateTool();
  return tool;
}

void
TRotateTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
//cout << "TRotateTool::mouseEvent" << endl;

  TCoord x, y;
  
  switch(me.type) {
    case TMouseEvent::LDOWN: {
      fe->mouse2sheet(me.x, me.y, &x, &y);
      if (figure) {
        if (rotx - fe->fuzziness*2 <= x && x <= rotx + fe->fuzziness*2 &&
            roty - fe->fuzziness*2 <= y && y <= roty + fe->fuzziness*2)
        {
          cerr << "going to move rotation center" << endl;
          state = MOVE_CENTER;
          break;
        }
      }

      TFigure *f = fe->findFigureAt(x, y);
      if (f) {
//cout << "got figure" << endl;
        if (figure!=f) {
          // a new figure was selected, setup a new rotation center
          TRectangle r;
          f->getShape(&r);
          rotx = r.x + r.w/2;
          roty = r.y + r.h/2;
          if (f->mat) {
            f->mat->map(rotx, roty, &rotx, &roty);
          }
          figure = f;
        }
        rotd0=atan2(static_cast<TCoord>(y - roty), 
                    static_cast<TCoord>(x - rotx));
        rotd = 0.0;
        if (f->mat)
          oldmatrix = *f->mat;
        else
          oldmatrix.identity();
        state = ROTATE;
        fe->invalidateWindow();
      }
    }  break;
    
    case TMouseEvent::LUP:
      state = NONE;
      break;

    case TMouseEvent::MOVE:
      if (!figure)
        break;
        fe->mouse2sheet(me.x, me.y, &x, &y);
        // fe->sheet2grid(x, y, &x, &y);

        switch(state) {
          case ROTATE:
            rotd=atan2(y - roty, x - rotx);
//      cerr << "rotd="<<rotd<<", rotd0="<<rotd0<<" -> " << (rotd-rotd0) << "\n";
            rotd-=rotd0;
            if (!figure->mat)
              figure->mat = new TMatrix2D(oldmatrix);
            else
              *figure->mat = oldmatrix;
            figure->mat->translate(rotx, roty);
            figure->mat->rotate(rotd);
            figure->mat->translate(-rotx, -roty);
        
            fe->invalidateWindow();
            break;
          case MOVE_CENTER:
            rotx = x;
            roty = y;
            fe->invalidateWindow();
            break;
      }
      break;
  }
}

bool
TRotateTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (!figure)
    return true;

#if 0
  pen.push();
  pen.translate(rotx, roty);
  pen.rotate(rotd);
  pen.translate(-rotx, -roty);
  figure->paint(pen);
  pen.pop();
#endif

  // draw center of rotation
  pen.setLineWidth(1);
  pen.setLineColor(TColor::FIGURE_SELECTION);
  pen.setFillColor(TColor::WHITE);

  TCoord x, y;
  if (pen.getMatrix()) {
    pen.getMatrix()->map(rotx, roty, &x, &y);
    pen.push();
    pen.identity();
  } else {
    x = rotx;
    y = roty;
  }

  pen.drawCircle(x-3.5,y-3.5,6,6);
  pen.drawLine(x-0.5, y+2,   x-0.5, y+5);
  pen.drawLine(x+2  , y-0.5, x+5,   y-0.5);
  pen.drawLine(x-0.5, y-3,   x-0.5, y-6);
  pen.drawLine(x-3  , y-0.5, x-6,   y-0.5);
  if (pen.getMatrix())
    pen.pop();
  
  // draw handles for rotated figure
  pen.push();
/*
  pen.translate(rotx, roty);
  pen.rotate(rotd);
  pen.translate(-rotx, -roty);
*/
  if (figure->mat)
    pen.multiply(figure->mat);

  TRectangle r;
  figure->getShape(&r);
  for(int i=0; i<=4; ++i) {
    switch(i) {
      case 0: x = r.x;       y = r.y;       break;
      case 1: x = r.x+r.w-1; y = r.y;       break;
      case 2: x = r.x+r.w-1; y = r.y+r.h-1; break;
      case 3: x = r.x;       y = r.y+r.h-1; break;
      case 4: x = r.x+r.w/2; y = r.y+r.h/2; break;
    }
    if (pen.getMatrix()) {
      pen.getMatrix()->map(x, y, &x, &y);
      pen.push();
      pen.identity();
    }
    if (i!=4) {
      pen.fillRectangle(x-2,y-2,4,4);
      pen.drawRectangle(x-2,y-2,4,4);
    } else {
      pen.drawLine(x-2.5, y-2.5, x+1.5, y+1.5);
      pen.drawLine(x-2.5, y+1.5, x+1.5, y-2.5);
    }
    if (pen.getMatrix())
      pen.pop();
  }
  pen.pop();
  
  return true;
}
