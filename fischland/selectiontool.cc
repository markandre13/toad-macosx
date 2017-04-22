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

#include "selectiontool.hh"
#include <toad/vector.hh>

using namespace fischland;

void
TSelectionTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint pos;
  TFigure *figure;
  fe->mouse2sheet(me.pos, &pos);
  figure = fe->findFigureAt(pos);

  // FIXME: use state for outer switch to improve readability?
  switch(me.type) {
    case TMouseEvent::ENTER:
      fe->getWindow()->setAllMouseMoveEvents(true);
    case TMouseEvent::MOVE:
      switch(state) {
        case STATE_NONE:
          if (setCursorForHandle(fe, me))
            break;
          if (!figure)
            fe->getWindow()->setCursor(cursor[CURSOR_SELECT]);
          else
          if (fe->selection.contains(figure))
            fe->getWindow()->setCursor(cursor[CURSOR_SELECT_MOVE]);
          else
            fe->getWindow()->setCursor(cursor[CURSOR_SELECT_HOVER]);
          break;
        case STATE_DRAG_MARQUEE:
          dragMarquee(fe, me);
          break;
        case STATE_MOVE_HANDLE:
          moveHandle(fe, me);
          break;
      }
      break;
    case TMouseEvent::LEAVE:
      fe->getWindow()->setCursor(nullptr);
      fe->getWindow()->setAllMouseMoveEvents(false);
      break;
    case TMouseEvent::LDOWN:
      if (downHandle(fe, me))
        return;
      if (!fe->selection.contains(figure) &&
          !(me.modifier() & MK_SHIFT))
      {
        invalidateBounding(fe);
        fe->clearSelection();
      }

      // on a figure, start grab (prepare to move the selected figures)
      if (figure) {
        fe->selection.insert(figure);
        fe->sheet2grid(pos, &last);
        last_s = me.pos;
        state = STATE_MOVE_SELECTION;
        calcSelectionsBoundingRectangle(fe);
        invalidateBounding(fe);
        fe->getWindow()->setCursor(cursor[CURSOR_SELECT_MOVE]);
        break;
      }
      
      // invalidate old tempory selection
      if (!tmpsel.empty() &&
          !(me.modifier() & MK_SHIFT))
      {
        invalidateBounding(fe);
        tmpsel.clear();
      }
      startMarquee(me);
      break;
    case TMouseEvent::LUP:
      switch(state) {
        case STATE_DRAG_MARQUEE:
          stopMarquee(fe);
          break;
        case STATE_MOVE_HANDLE:
          state = STATE_NONE;
          fe->getWindow()->invalidateWindow();
          break;
        case STATE_MOVE_SELECTION:
          state = STATE_NONE;
          break;
      }
      break;
  }
#if 0
  TFigure *figure;
  TPoint p;
  TRectangle r;

  if (fe->state == TFigureEditor::STATE_EDIT) {
cout << "TSelectionTool::mouseEvent: rudimentary edit mode" << endl;
    if (me.type == TMouseEvent::LDOWN) {
      figure = *tmpsel.begin();
//cout << "selection = " << figure << endl;
      fe->mouse2sheet(me.pos, &p);
      TFigure *f = fe->findFigureAt(p);
//cout << "at mouse " << f << endl;
      if (f!=figure) {
//cout << "stop" << endl;
        if (figure->stop(fe)==TFigure::DELETE) {
          cout << "TSelectionTool::mouseEvent: should delete figure" << endl;
        }
        fe->state = TFigureEditor::STATE_NONE;
      }
    }
//    unsigned r = figure->mouseEvent(fe, ke.getKey(), const_cast<char*>(ke.getString()), ke.modifier());
    return;
  }

  switch(me.type) {
    case TMouseEvent::LDOWN:
      // get mouse move events
      if (fe->state == TFigureEditor::STATE_NONE) {
        fe->start();
        fe->state = TFigureEditor::STATE_CREATE;
        fe->getWindow()->setAllMouseMoveEvents(true);
      }
      
      if (downHandle(fe, me)) {
        break;
      }

      // find figure under mouse
      fe->mouse2sheet(me.pos, &p);
      figure = fe->findFigureAt(p);

      if (figure &&
          me.dblClick &&
          figure->startInPlace())
      {
        // FIXME: selection- & createtool should now delegate to edittool?
        cout << "TSelectionTool: figure shall be edited" << endl;
        fe->state = TFigureEditor::STATE_EDIT;
        fe->clearSelection();
        // invalidateBounding(fe);
        tmpsel.clear();
        tmpsel.insert(figure);
        fe->invalidateWindow();
        break;
      }
      
#if 0
        TFigureEditEvent ee;
        ee.model = fe->getModel(); // why???
        ee.editor = fe;
        ee.type = TFigureEditEvent::START_IN_PLACE;
        if (figure->editEvent(ee)) {
          fe->clearSelection();
#endif
#if 0
          fe->clearSelection();
          sigSelectionChanged();
          gadget = g;
          invalidateFigure(gadget);
          state = STATE_EDIT;
          goto redo;
#endif

      // if figure not part of selection and not shift
      if (fe->selection.find(figure) == fe->selection.end() &&
          !(me.modifier() & MK_SHIFT))
      {
// invalidate???
        fe->clearSelection();
      }

      // on a figure, start grab
      if (figure) {
        fe->selection.insert(figure);
        fe->sheet2grid(p, &last);
        last_s = me.pos;
        grab = true;
        calcSelectionsBoundingRectangle(fe);
        invalidateBounding(fe);
        break;
      }
      
      // invalidate old selection
      if (!tmpsel.empty() &&
          !(me.modifier() & MK_SHIFT))
      {
        // fe->invalidateWindow();
        invalidateBounding(fe);
        tmpsel.clear();
      }
      
      down = true;
      rx0 = rx1 = me.pos.x;
      ry0 = ry1 = me.pos.y;
      break;

    case TMouseEvent::MOVE:
      if (hndl) {
        moveHandle(fe, me);
        break;
      }
      if (grab) {
        moveGrab(fe, me);
        break;
      }
      if (down) {
        moveSelect(fe, me);
        break;
      }
//      setCursorForHandle(fe, me);
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
      fe->selection.insert(tmpsel.begin(), tmpsel.end());
      tmpsel.clear();

      // calculate the selections bounding rectangle
      calcSelectionsBoundingRectangle(fe);
      fe->stop();
      fe->invalidateWindow();
      fe->getWindow()->setAllMouseMoveEvents(false);
      break;
    default:
      ;
  }
#endif
}

void
TSelectionTool::stop(TFigureEditor *fe)
{
  fe->getWindow()->setAllMouseMoveEvents(true);
  fe->getWindow()->setCursor(nullptr);
  fe->state = TFigureEditor::STATE_NONE;
}

void
TSelectionTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
  if (fe->state != TFigureEditor::STATE_EDIT) {
    cout << "no editing" << endl;
    return;
  }
  if (tmpsel.begin()==tmpsel.end()) {
    cout << "no keyevent to figure" << endl;
    return;
  }
  cout << "key event to figure" << endl;
  TFigure *figure = *tmpsel.begin();
  switch(ke.type) {
    case TKeyEvent::DOWN:
      unsigned r = figure->keyDown(fe, ke.key, const_cast<char*>(ke.string.c_str()), ke.modifier);
      if (r & TFigure::STOP) {
        fe->state = TFigureEditor::STATE_NONE;
        fe->invalidateWindow();
      }
      break;
  }
}

/*
 * Handles
 */
void
TSelectionTool::getBoundingHandle(unsigned i, TRectangle *r)
{
  const TCoord x0(boundary.x1), y0(boundary.y1),
               x1(boundary.x2), y1(boundary.y2);

  static const TCoord s = 5.0; // size
  const TCoord w = x1 - x0;
  const TCoord h = y1 - y0;
  switch(i) {
    case  0: r->set(x0    +1.5, y0+1   , s, s); break;
    case  1: r->set(x0+w/2    , y0+1   , s, s); break;
    case  2: r->set(x0+w  -0.5, y0+1   , s, s); break;
    case  3: r->set(x0+w  -0.5, y0+h/2 , s, s); break;
    case  4: r->set(x0+w  -0.5, y0+h   , s, s); break;
    case  5: r->set(x0+w/2    , y0+h   , s, s); break;
    case  6: r->set(x0    +1.5, y0+h   , s, s); break;
    case  7: r->set(x0    +1.5, y0+h/2 , s, s); break;

    case  8: r->set(x0    -s, y0    -s, s, s); break;
    case  9: r->set(x0+w/2  , y0    -s, s, s); break;
    case 10: r->set(x0+w  +s, y0    -s, s, s); break;
    case 11: r->set(x0+w  +s, y0+h/2  , s, s); break;
    case 12: r->set(x0+w  +s, y0+h  +s, s, s); break;
    case 13: r->set(x0+w/2  , y0+h  +s, s, s); break;
    case 14: r->set(x0    -s, y0+h  +s, s, s); break;
    case 15: r->set(x0    -s, y0+h/2  , s, s); break;
  }
  if (state==STATE_MOVE_HANDLE)
    m.map(r->x, r->y, &r->x, &r->y);
  r->translate(TPoint(-s/2.0, -s/2.0));
}

bool
TSelectionTool::setCursorForHandle(TFigureEditor *fe, const TMouseEvent &me)
{
  if (fe->selection.empty())
    return false;
  
  // origin is already applied by scroll pane?
  
  TCoord x = me.pos.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
  TCoord y = me.pos.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;
  
  for(unsigned i=0; i<16; ++i) {
    TRectangle r;
    getBoundingHandle(i, &r);
    if (r.isInside(x, y)) {
      static const int wind[16] = {
        CURSOR_SELECT_RESIZE_NW,
        CURSOR_SELECT_RESIZE_N,
        CURSOR_SELECT_RESIZE_NE,
        CURSOR_SELECT_RESIZE_E,
        CURSOR_SELECT_RESIZE_SE,
        CURSOR_SELECT_RESIZE_S,
        CURSOR_SELECT_RESIZE_SW,
        CURSOR_SELECT_RESIZE_W,
        CURSOR_SELECT_ROTATE_NW,
        CURSOR_SELECT_ROTATE_N,
        CURSOR_SELECT_ROTATE_NE,
        CURSOR_SELECT_ROTATE_E,
        CURSOR_SELECT_ROTATE_SE,
        CURSOR_SELECT_ROTATE_S,
        CURSOR_SELECT_ROTATE_SW,
        CURSOR_SELECT_ROTATE_W
      };
      fe->getWindow()->setCursor(cursor[wind[i]]);
      return true;
    }
  }
  return false;
}

bool
TSelectionTool::downHandle(TFigureEditor *fe, const TMouseEvent &me)
{      
  if (fe->selection.empty())
    return false;
  // origin is already applied by scroll pane?
  TCoord x = me.pos.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
  TCoord y = me.pos.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;
// cout << "down at " << x << ", " << y << endl;
  for(unsigned i=0; i<16; ++i) {
    TRectangle r;
    getBoundingHandle(i, &r);
// cout << "  check " << r.x << ", " << r.y << endl;
    if (r.isInside(x, y)) {
      state = STATE_MOVE_HANDLE;
      selectedHandle = i;
      handleStart = me.pos;
      m.identity();
      oldBoundary = boundary;
      if (selectedHandle>=8) {
        rotationCenter = boundary.center();
        rotationStartDirection = atan2(y - rotationCenter.y, x - rotationCenter.x);
      }
      return true;
    }
  }
  
  return false;
}

void
TSelectionTool::moveHandle(TFigureEditor *fe, const TMouseEvent &me)
{
  if (selectedHandle<8)
    moveHandle2Scale(fe, me);
  else
    moveHandle2Rotate(fe, me);
}

void
TSelectionTool::moveHandle2Scale(TFigureEditor *fe, const TMouseEvent &me)
{
  fe->getWindow()->invalidateWindow();

  TCoord x0(boundary.x1), y0(boundary.y1),
         x1(boundary.x2), y1(boundary.y2),
         ox0(oldBoundary.x1), oy0(oldBoundary.y1),
         ox1(oldBoundary.x2), oy1(oldBoundary.y2);

  // mouse is holding a handle, scale the selection
  invalidateBounding(fe);
//TCoord x0, y0;
  TCoord x, y;
  x = me.pos.x - fe->getVisible().x;
  y = me.pos.y - fe->getVisible().y;
  switch(selectedHandle) {
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
  m.identity();
  m.translate(X0, Y0);
  m.scale(sx, sy);
  m.translate(-OX0, -OY0);

  invalidateBounding(fe);
}

void
TSelectionTool::moveHandle2Rotate(TFigureEditor *fe, const TMouseEvent &me)
{
  fe->getWindow()->invalidateWindow();

  TCoord x = me.pos.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
  TCoord y = me.pos.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;

  double rotd = atan2(y - rotationCenter.y, x - rotationCenter.x);
  rotd-=rotationStartDirection;
  
  m.identity();
  m.translate(rotationCenter);
  m.rotate(rotd);
  m.translate(-rotationCenter);
}

void
TSelectionTool::moveGrab(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint pos;
  fe->mouse2sheet(me.pos, &pos);
  fe->sheet2grid(pos, &pos);
  TPoint d = pos-last;
  last = pos;
  
  invalidateBounding(fe);

  fe->getModel()->translate(fe->selection, d.x, d.y);

  calcSelectionsBoundingRectangle(fe);
  invalidateBounding(fe);
}

void
TSelectionTool::startMarquee(const TMouseEvent &me)
{
  state = STATE_DRAG_MARQUEE;
  marqueeDraggedOpen = false;
  marqueeStart = marqueeEnd = me.pos;
}

void
TSelectionTool::dragMarquee(TFigureEditor *fe, const TMouseEvent &me)
{
  if (!marqueeDraggedOpen) {
    if (maxDistanceAlongAxis(me.pos, marqueeStart) < 2)
      return;
    marqueeDraggedOpen = true;
    temporarySelection.clear();
  }

  TPoint origin = fe->getWindow()->getOrigin();
  fe->getWindow()->invalidateWindow(
    TRectangle(marqueeStart, marqueeEnd).translate(origin).expand(3)
  );
  marqueeEnd = me.pos;
  fe->getWindow()->invalidateWindow(
    TRectangle(marqueeStart, marqueeEnd).translate(origin).expand(3)
  );

  TPoint p0, p1;
  fe->mouse2model(marqueeStart, &p0);
  fe->mouse2model(marqueeEnd, &p1);
  TRectangle marquee(p0, p1);
  
  temporarySelection.clear();
  for(auto &&figure: *fe->getModel()) {
    TRectangle shape;
    fe->getFigureShape(figure, &shape, nullptr);
    if (shape.isInside(marquee)) {
      temporarySelection.insert(figure);
    }
  }
}

void
TSelectionTool::paintMarquee(TFigureEditor *fe, TPenBase &pen)
{
  for(auto &&figure: temporarySelection) {
    pen.setLineWidth(1);   
    figure->paintSelection(pen, -1);
  }

  pen.push();
  TCoord tx = 0.0, ty = 0.0;
  if (pen.getMatrix()) {
    tx = pen.getMatrix()->tx - fe->getVisible().x;
    ty = pen.getMatrix()->ty - fe->getVisible().y;
  }
  pen.identity();
  pen.translate(tx, ty);
  pen.setColor(TColor::FIGURE_SELECTION);
  pen.setLineWidth(1);
  pen.setAlpha(0.3);
  
  TRectangle marquee(marqueeStart, marqueeEnd);
  
  if (pen.getAlpha()!=1.0)
    pen.fillRectangle(marquee);
  pen.setAlpha(1.0);
  pen.drawRectangle(marquee);
  pen.pop();
}

void
TSelectionTool::stopMarquee(TFigureEditor *fe)
{
  state = STATE_NONE;
  fe->selection.insert(temporarySelection.begin(), temporarySelection.end());
  calcSelectionsBoundingRectangle(fe);
  TPoint origin = fe->getWindow()->getOrigin();
  fe->getWindow()->invalidateWindow(
    TRectangle(marqueeStart, marqueeEnd).translate(origin).expand(3)
  );
}

void
TSelectionTool::moveSelect(TFigureEditor *fe, const TMouseEvent &me)
{
  if (!rect) {
    if (me.pos.x < rx0-2 || me.pos.x > rx0+2 || me.pos.y < ry0-2 || me.pos.y > ry0+2) {
      rect = true;
      tmpsel.clear();
    } else {
      return;
    }
  }
  TPoint origin = fe->getWindow()->getOrigin();
  fe->getWindow()->invalidateWindow(
    rx0 + origin.x,
    ry0 + origin.y,
    rx1-rx0+3,ry1-ry0+2);
  rx1 = me.pos.x;
  ry1 = me.pos.y;
  fe->getWindow()->invalidateWindow(
    rx0 + origin.x,
    ry0 + origin.y,
    rx1-rx0+3,ry1-ry0+2);
  tmpsel.clear();
  auto p = fe->getModel()->begin();
  auto e = fe->getModel()->end();
  TPoint p0, p1;
  fe->mouse2sheet(TPoint(rx0, ry0), &p0);
  fe->mouse2sheet(TPoint(rx1, ry1), &p1);
  TRectangle r0(p1, p0), r1;
  while(p!=e) {
    fe->getFigureShape(*p, &r1, NULL);
    if (r0.isInside( r1.x, r1.y ) &&
        r0.isInside( r1.x+r1.w, r1.y+r1.h ) )
    {
      tmpsel.insert(*p);
    }
    ++p;
  }
}

void
TSelectionTool::invalidateBounding(TFigureEditor *fe)
{
  TPoint origin = fe->getWindow()->getOrigin();
  TRectangle visible = fe->getVisible();
  origin.translate(visible.x, visible.y);
  fe->getWindow()->invalidateWindow(TRectangle(boundary).translate(origin).expand(5));
}

bool
TSelectionTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (fe->state == TFigureEditor::STATE_EDIT) {
    TFigure *figure = *tmpsel.begin();
    pen.push();
    figure->paint(pen, TFigure::EDIT);
    pen.pop();
    return true;
  }
/*
  if (state==STATE_MOVE_HANDLE) {
    pen.setColor(TColor::FIGURE_SELECTION);
    pen.setAlpha(1.0);
    pen.setScreenLineWidth(1.0);

    TVectorPath path;
    path.addRect(TRectangle(x0+1, y0+1, x1-x0-1, y1-y0-1));
    path.transform(m);
    path.apply(pen);
    pen.stroke();
  } else
*/
  if (state==STATE_DRAG_MARQUEE) {
    paintMarquee(fe, pen);
  } else
  if (!fe->selection.empty()) {
//    cout << "draw bounding rectangle " << x0 << ", " << y0 << " to " << x1 << ", " << y1 << endl;
    
    pen.setColor(TColor::FIGURE_SELECTION);
    pen.setAlpha(1.0);
    pen.setScreenLineWidth(1.0);

    // paint outline FIXME: scaling
    for(auto &f: fe->selection) {
      TVectorGraphic *graphic = f->getPath();
      if (!graphic)
        continue;
      for(auto &painter: *graphic) {
        if (state==STATE_MOVE_HANDLE)
          painter->path->transform(m);
        painter->path->apply(pen);
        pen.stroke();
      }
      delete graphic;
    }

    if (pen.getMatrix()) {
      TCoord tx, ty;
      pen.getMatrix()->map(0.0, 0.0, &tx, &ty);
      pen.push();
      pen.identity();
      pen.translate(tx, ty);
    }

    pen.setLineWidth(1);
    TVectorPath pr;
    pr.addRect(TRectangle(boundary));
    if (state==STATE_MOVE_HANDLE)
      pr.transform(m);
    pr.apply(pen);
    pen.stroke();
    pen.setFillColor(1,1,1);
    TRectangle r;
    for(unsigned i=0; i<8; ++i) {
      getBoundingHandle(i, &r);
      pen.fillRectangle(r);
      pen.drawRectangle(r);
    }
    
    if (pen.getMatrix()) {
      pen.pop();
    }
  }
  return true;
}

      
void
TSelectionTool::calcSelectionsBoundingRectangle(TFigureEditor *fe)
{
//cout << "TSelectionTool::calcSelectionsBoundingRectangle" << endl;
  boundary.clear();
  for(auto &&figure: fe->selection) {
    TRectangle r;
    fe->getFigureShape(figure, &r, nullptr);
    boundary.expand(r);
  }
#if 0
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
#endif
//cout << "-> " << x0 << ", " << y0 << " - " << x1 << ", " << y1 << endl;

}

static const char cursorData[15][32][32+1] = {
  {
  // CURSOR_SELECT
  // 0        1         2         3
  // 12345678901234567890123456789012
    "##                              ",
    "#.#                             ",
    "#..#                            ",
    "#...#                           ",
    "#....#                          ",
    "#.....#                         ",
    "#......#                        ",
    "#.......#                       ",
    "#...#####                       ",
    "#..#                            ",
    "#.#                             ",
    "##                              ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_HOVER
  // 0        1         2         3
  // 12345678901234567890123456789012
    "##                              ",
    "#.#                             ",
    "#..#                            ",
    "#...#                           ",
    "#....#                          ",
    "#.....#                         ",
    "#......#                        ",
    "#.......#                       ",
    "#...#####                       ",
    "#..#    ########                ",
    "#.#     #......#                ",
    "##      #.####.#                ",
    "        #.#  #.#                ",
    "        #.#  #.#                ",
    "        #.####.#                ",
    "        #......#                ",
    "        ########                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_MOVE
  // 0        1         2         3
  // 12345678901234567890123456789012
    "##                              ",
    "#.#                             ",
    "#..#                            ",
    "#...#                           ",
    "#....#                          ",
    "#.....#                         ",
    "#......#    #                   ",
    "#.......#  #.#                  ",
    "#...##### #...#                 ",
    "#..#     #.....#                ",
    "#.#     ####.####               ",
    "##     #.# #.# #.#              ",
    "      #..###.###..#             ",
    "     #.............#            ",
    "      #..###.###..#             ",
    "       #.# #.# #.#              ",
    "        ####.####               ",
    "         #.....#                ",
    "          #...#                 ",
    "           #.#                  ",
    "            #                   ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_RESIZE_N
  // 0        1         2         3
  // 12345678901234567890123456789012
    "    #                           ",
    "   #.#                          ",
    "  #...#                         ",
    " #.....#                        ",
    "####.####                       ",
    "   #.#                          ",
    "   ###                          ",
    "  #...#                         ",
    "   ###                          ",
    "   #.#                          ",
    "####.####                       ",
    " #.....#                        ",
    "  #...#                         ",
    "   #.#                          ",
    "    #                           ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
  },{
  // CURSOR_SELECT_RESIZE_NE
  // 0        1         2         3
  // 12345678901234567890123456789012
    "        #######                 ",
    "         #....#                 ",
    "          #...#                 ",
    "          #...#                 ",
    "         #.##.#                 ",
    "      # #.#  ##                 ",
    "     #.# #    #                 ",
    "      #.#                       ",
    "#    # #.#                      ",
    "##  #.# #                       ",
    "#.##.#                          ",
    "#...#                           ",
    "#...#                           ",
    "#....#                          ",
    "#######                         ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_RESIZE_E
  // 0        1         2         3
  // 12345678901234567890123456789012
    "    #     #                     ",
    "   ##     ##                    ",
    "  #.#  #  #.#                   ",
    " #..###.###..#                  ",
    "#.....#.#.....#                 ",
    " #..###.###..#                  ",
    "  #.#  #  #.#                   ",
    "   ##     ##                    ",
    "    #     #                     ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_RESIZE_SE
  // 0        1         2         3
  // 12345678901234567890123456789012
    "#######                         ",
    "#....#                          ",
    "#...#                           ",
    "#...#                           ",
    "#.##.#                          ",
    "##  #.# #                       ",
    "#    # #.#                      ",
    "      #.#                       ",
    "     #.# #    #                 ",
    "      # #.#  ##                 ",
    "         #.##.#                 ",
    "          #...#                 ",
    "          #...#                 ",
    "         #....#                 ",
    "        #######                 ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_N
  // 0        1         2         3
  // 12345678901234567890123456789012
    "#     ###     #                 ",
    "##  ##...##  ##                 ",
    "#.##..###..##.#                 ",
    "#...##   ##...#                 ",
    "#...#     #...#                 ",
    "#....#   #....#                 ",
    "####### #######                 ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_NE
  // 0        1         2         3
  // 12345678901234567890123456789012
    "     #                          ",
    "    ##                          ",
    "   #.#                          ",
    "  #..###                        ",
    " #......##                      ",
    "  #..###..#                     ",
    "   #.#  #.#                     ",
    "    ##   #.#                    ",
    "     #   #.#                    ",
    "      ####.####                 ",
    "       #.....#                  ",
    "        #...#                   ",
    "         #.#                    ",
    "          #                     ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_E
  // 0        1         2         3
  // 12345678901234567890123456789012
    "#######                         ",
    "#....#                          ",
    "#...#                           ",
    "#...#                           ",
    "#.##.#                          ",
    "## #.#                          ",
    "#   #.#                         ",
    "    #.#                         ",
    "#   #.#                         ",
    "##  .#                          ",
    "#.##.#                          ",
    "#...#                           ",
    "#...#                           ",
    "#....#                          ",
    "#######                         ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_SE
  // 0        1         2         3
  // 12345678901234567890123456789012
    "          #                     ",
    "         #.#                    ",
    "        #...#                   ",
    "       #.....#                  ",
    "      ####.####                 ",
    "     #   #.#                    ",
    "    ##   #.#                    ",
    "   #.#  #.#                     ",
    "  #..###..#                     ",
    " #......##                      ",
    "  #..###                        ",
    "   #.#                          ",
    "    ##                          ",
    "     #                          ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_S
  // 0        1         2         3
  // 12345678901234567890123456789012
    "####### #######                 ",
    "#....#   #....#                 ",
    "#...#     #...#                 ",
    "#...##   ##...#                 ",
    "#.##..###..##.#                 ",
    "##  ##...##  ##                 ",
    "#     ###     #                 ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_SW
  // 0        1         2         3
  // 12345678901234567890123456789012
    "    #                           ",
    "   #.#                          ",
    "  #...#                         ",
    " #.....#                        ",
    "####.####                       ",
    "   #.#   #                      ",
    "   #.#   ##                     ",
    "    #.#  #.#                    ",
    "    #..###..#                   ",
    "     ##......#                  ",
    "       ###..#                   ",
    "         #.#                    ",
    "         ##                     ",
    "         #                      ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_W
  // 0        1         2         3
  // 12345678901234567890123456789012
    "#######                         ",
    " #....#                         ",
    "  #...#                         ",
    "  #...#                         ",
    " #.##.#                         ",
    " #.# ##                         ",
    "#.#   #                         ",
    "#.#                             ",
    "#.#   #                         ",
    " #.# ##                         ",
    " #. #.#                         ",
    "  #...#                         ",
    "  #...#                         ",
    " #....#                         ",
    "#######                         ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_SELECT_ROTATE_NW
  // 0        1         2         3
  // 12345678901234567890123456789012
    "         #                      ",
    "         ##                     ",
    "         #.#                    ",
    "       ###..#                   ",
    "     ##......#                  ",
    "    #..###..#                   ",
    "    #.#  #.#                    ",
    "   #.#   ##                     ",
    "   #.#   #                      ",
    "####.####                       ",
    " #.....#                        ",
    "  #...#                         ",
    "   #.#                          ",
    "    #                           ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  }
};

TCursor* TSelectionTool::cursor[15];

TSelectionTool*
TSelectionTool::getTool()
{
  static TSelectionTool* tool = nullptr;
  if (!tool) {
    cursor[CURSOR_SELECT]           = new TCursor(cursorData[ 0], 1, 1);
    cursor[CURSOR_SELECT_HOVER]     = new TCursor(cursorData[ 1], 1, 1);
    cursor[CURSOR_SELECT_MOVE]      = new TCursor(cursorData[ 2], 1, 1);
    cursor[CURSOR_SELECT_RESIZE_N]  = new TCursor(cursorData[ 3], 5, 5);
    cursor[CURSOR_SELECT_RESIZE_NE] = new TCursor(cursorData[ 4], 8, 8);
    cursor[CURSOR_SELECT_RESIZE_E]  = new TCursor(cursorData[ 5], 8, 5);
    cursor[CURSOR_SELECT_RESIZE_SE] = new TCursor(cursorData[ 6], 8, 8);

    cursor[CURSOR_SELECT_ROTATE_N]  = new TCursor(cursorData[ 7], 8, 2);
    cursor[CURSOR_SELECT_ROTATE_NE] = new TCursor(cursorData[ 8],10, 6);
    cursor[CURSOR_SELECT_ROTATE_E]  = new TCursor(cursorData[ 9], 6, 8);
    cursor[CURSOR_SELECT_ROTATE_SE] = new TCursor(cursorData[10],10, 9);
    cursor[CURSOR_SELECT_ROTATE_S]  = new TCursor(cursorData[11], 8, 6-2);
    cursor[CURSOR_SELECT_ROTATE_SW] = new TCursor(cursorData[12], 6, 9);
    cursor[CURSOR_SELECT_ROTATE_W]  = new TCursor(cursorData[13], 2, 8);
    cursor[CURSOR_SELECT_ROTATE_NW] = new TCursor(cursorData[14], 6, 6);
    tool = new TSelectionTool();
  }
  return tool;
}
