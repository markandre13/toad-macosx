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

#if 0

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

  TPoint pos;
  
  switch(me.type) {
    case TMouseEvent::LDOWN: {
      fe->mouse2sheet(me.pos, &pos);
      if (figure) {
        if (rotx - fe->fuzziness*2 <= pos.x && pos.x <= rotx + fe->fuzziness*2 &&
            roty - fe->fuzziness*2 <= pos.y && pos.y <= roty + fe->fuzziness*2)
        {
          cerr << "going to move rotation center" << endl;
          state = MOVE_CENTER;
          break;
        }
      }

      TFigure *f = fe->findFigureAt(pos);
      if (!f) {
        figure = 0;
        fe->invalidateWindow();
        break;
      }
//cout << "got figure" << endl;
      if (figure!=f) {
        // a new figure was selected, setup a new rotation center
        TRectangle r = f->bounds();
        rotx = r.x + r.w/2;
        roty = r.y + r.h/2;

        if (f->mat) {
          TCoord x0, x1, y0, y1;
          f->mat->map(r.x, r.y, &x0, &y0);
          f->mat->map(r.x+r.w, r.y+r.h, &x1, &y1);
cout << "mapped rectangle "
     << r.x << ", " << r.y << " - " << (r.x+r.w) << ", " << (r.y+r.h) << " to "
     << x0  << ", " << y0  << " - " << x1        << ", " << y1 << endl;
          rotx = x0 + (x1-x0)/2;
          roty = y0 + (y1-y0)/2; 
          // f->mat->map(rotx, roty, &rotx, &roty);
        } else {
cout << "figure " << f << " has no matrix" << endl;
        }
        figure = f;
      }
      rotd0=atan2(pos.y - roty, pos.x - rotx);
      if (f->mat) {
//        cout << "save old matrix" << endl;
        oldmatrix = *f->mat;
      } else {
        oldmatrix.identity();
      }
      state = ROTATE;
      fe->invalidateWindow();
    }  break;
    
    case TMouseEvent::LUP:
      state = NONE;
      break;

    case TMouseEvent::MOVE:
      if (!figure || state==NONE)
        break;
      fe->mouse2sheet(me.pos, &pos);
      // fe->sheet2grid(x, y, &x, &y);

      switch(state) {
        case ROTATE: {
          rotd=atan2(pos.y - roty, pos.x - rotx);
//          cerr << "rotd="<<rotd<<", rotd0="<<rotd0<<" -> " << (rotd-rotd0) << "\n";
          rotd-=rotd0;
          if (!figure->mat)
            figure->mat = new TMatrix2D();
          figure->mat->identity();

//cout << "rotate around " << rotx << ", " << roty << endl;
          figure->mat->translate(rotx, roty);
          figure->mat->rotate(rotd);
          figure->mat->translate(-rotx, -roty);
            
          figure->mat->multiply(&oldmatrix);

          fe->invalidateWindow();
        } break;
        case MOVE_CENTER:
//cout << "center is now at " << x << ", " << y << endl;
          rotx = pos.x;
          roty = pos.y;
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

  // draw center of rotation
  pen.setLineWidth(1);
  pen.setStrokeColor(TColor::FIGURE_SELECTION);
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

  if (figure->mat)
    pen.multiply(figure->mat);

  TRectangle r = figure->bounds();
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

#endif
