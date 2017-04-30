/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2007 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figureeditor.hh>
#include <toad/figure/createtool.hh>

using namespace toad;

void
TFCreateTool::stop(TFigureEditor *fe)
{
//cout << "TFCreateTool::stop" << endl;
  if (figure) {
    unsigned r = figure->stop(fe);
    if (false /* size==(0,0) */) {
      delete figure;
    } else {
      fe->addFigure(figure);
      fe->selection.insert(figure);
      fe->getWindow()->invalidateWindow();
    }
    figure = 0;
//    fe->setCurrent(0);
    fe->getWindow()->ungrabMouse();
  }
  state = STATE_NONE;
}

const char *mousename(TMouseEvent::EType type)
{
  static const char* name0to10[11] = {
    "MOVE",
    "ENTER",
    "LEAVE",
    "LDOWN",
    "MDOWN",
    "RDOWN",
    "LUP",
    "MUP",
    "RUP",
    "ROLL_UP",
    "ROLL_DOWN"
  };
  if (type>=0 && type<=10)
    return name0to10[type];
  return "unknown";
}

void
TFCreateTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
#if 0
//cout << "TFCreateTool::mouseEvent " << mousename(me.type) << " at " << statename(fe->state) << endl;
  TPoint p0, p1;
  unsigned r;

redo:

  switch(state) {
    case STATE_NONE:
      switch(me.type) {
        case TMouseEvent::LDOWN: {
          fe->mouse2sheet(me.pos, &p0);
          fe->sheet2grid(p0, &p1);
          fe->clearSelection();
          figure = static_cast<TFigure*>(tmpl->clone());
          fe->getAttributes()->setAllReasons();
          figure->setAttributes(fe->getAttributes());
          figure->startCreate(p1);
          state = STATE_CREATE;
        } break;
        default:
//          cout << "TFCreateTool: unhandled mouse event in state 0" << endl;
          return;
      }
      break;
    
    case STATE_CREATE:
      fe->mouse2sheet(me.pos, &p0);
      fe->sheet2grid(p0, &p1);
      TMouseEvent me2(me, p1);
      switch(me.type) {
        case TMouseEvent::LDOWN:
//          cout << "TFCreateTool: mouseLDown during create" << endl;
          r = figure->mouseLDown(fe, me2);
          break;
        case TMouseEvent::MOVE:
//          cout << "TFCreateTool: mouseMove during create" << endl;
          r = figure->mouseMove(fe, me2);
          break;
        case TMouseEvent::LUP:
//          cout << "TFCreateTool: mouseLUp during create" << endl;
          r = figure->mouseLUp(fe, me2);
          break;
        case TMouseEvent::RDOWN:
//          cout << "TFCreateTool: mouseRDown during create" << endl;
          r = figure->mouseRDown(fe, me2);
          break;
        default:
//          cout << "TFCreateTool: unhandled mouse event in state 1" << endl;
          return;
      }

      if (r & TFigure::DELETE) {
//        cout << "  delete figure" << endl;
        fe->deleteFigure(figure);
        figure = 0;
      }
      if (r & TFigure::STOP) {
//        cout << "  stop" << endl;
        // fe->stopOperation();
        fe->getWindow()->ungrabMouse();
        state = STATE_NONE;
//        fe->setCurrent(0);
        if (figure) {
          fe->addFigure(figure);
          fe->selection.insert(figure);
          fe->invalidateFigure(figure);
//cout << __FILE__ << ":" << __LINE__ << endl;
          figure = 0;
        }
      }
      if (r & TFigure::REPEAT) {
//        cout << "  repeat" << endl;
        if (me.dblClick) {
//          cerr << "TFigureEditor: kludge: avoiding endless loop bug\n";
          break;
        }
        goto redo;
      }
      break;
  }
  if (figure)
    fe->invalidateFigure(figure);
#endif
}

void
TFCreateTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
#if 0
  if (state == STATE_NONE && ke.type == TKeyEvent::DOWN) {
    fe->clearSelection();
  }
  if (!figure || ke.type != TKeyEvent::DOWN)
    return;
  if (ke.key == TK_ESCAPE) {
    fe->deleteFigure(figure);
    state = STATE_NONE;
    fe->getWindow()->ungrabMouse();
    figure = 0;
    return;
  }

  unsigned r = figure->keyDown(fe, ke.key, const_cast<char*>(ke.string.c_str()), ke.modifier); // FIXME: const_cast, pass ke instead

  if (r & TFigure::DELETE) {
//        cout << "  delete figure" << endl;
    fe->deleteFigure(figure);
    figure = 0;
  }
  if (r & TFigure::STOP) {
    fe->getWindow()->ungrabMouse();
    state = STATE_NONE;
    if (figure) {
      fe->addFigure(figure);
      figure = 0;
    }
  }
  if (figure)
    fe->invalidateFigure(figure);
#endif
}

void
TFCreateTool::setAttributes(TFigureAttributeModel *a)
{
  if (figure)
    figure->setAttributes(a);
}

bool
TFCreateTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (!figure)
    return false;
  figure->paint(pen, TFigure::EDIT);
  return true;
}
