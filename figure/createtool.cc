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
    if (r & TFigure::DELETE) {
      delete figure;
    } else {
      fe->addFigure(figure);
      fe->selection.insert(figure);
      fe->getWindow()->invalidateWindow();
    }
    figure = 0;
    fe->setCurrent(0);
    fe->getWindow()->ungrabMouse();
  }
  fe->state = TFigureEditor::STATE_NONE;
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

const char *statename(unsigned state)
{
  static const char* name0to6[7] = {
    "STATE_NONE",
    "STATE_MOVE",
    "STATE_MOVE_HANDLE",
    "STATE_SELECT_RECT",
    "STATE_EDIT",
    "STATE_ROTATE",
    "STATE_MOVE_ROTATE"
  };
  if (state<=6)
    return name0to6[state];
  
  static const char* name20to21[2] = {
    "STATE_START_CREATE",
    "STATE_CREATE"
  };
  if (state>=20 && state<=21)
    return name20to21[state-20];
  return "unknown";
}

void
TFCreateTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
//cout << "TFCreateTool::mouseEvent " << mousename(me.type) << " at " << statename(fe->state) << endl;
  TPoint p0, p1;
  unsigned r;

redo:

  switch(fe->state) {
    case TFigureEditor::STATE_NONE:
      switch(me.type) {
        case TMouseEvent::LDOWN: {
//          cout << "TFCreateTool: LDOWN" << endl;
//          cout << "TFCreateTool: start create" << endl;
          fe->mouse2sheet(me.pos, &p0);
          fe->sheet2grid(p0, &p1);
          fe->clearSelection();
          figure = static_cast<TFigure*>(tmpl->clone());
          fe->setCurrent(figure);
//cout << "  new figure " << figure << endl;
          figure->removeable = true;
          fe->getAttributes()->setAllReasons();
          figure->setAttributes(fe->getAttributes());
          figure->startCreate();
          fe->state = TFigureEditor::STATE_START_CREATE;
          TMouseEvent me2(me, p1);
          r = figure->mouseLDown(fe, me2);
          fe->state = TFigureEditor::STATE_CREATE;
          if (r & TFigure::DELETE) {
//            cout << "  delete" << endl;
            delete figure;
            figure = 0;
          }
          if (r & TFigure::STOP) {
//cout << "  stop" << endl;
            fe->state = TFigureEditor::STATE_NONE;
            fe->getWindow()->ungrabMouse();
            fe->setCurrent(0);
            if (figure) {
              fe->addFigure(figure);
          fe->selection.insert(figure);
//cout << __FILE__ << ":" << __LINE__ << endl;
            }
          }
          if (fe->state != TFigureEditor::STATE_NONE &&
              !(r & TFigure::NOGRAB) )
          {
            fe->getWindow()->grabMouse(true);
          }
          if (r & TFigure::REPEAT) {
//            cout << "  repeat" << endl;
            goto redo;
          }
        } break;
        default:
//          cout << "TFCreateTool: unhandled mouse event in state 0" << endl;
          return;
      }
      break;
    
    case TFigureEditor::STATE_CREATE:
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
        fe->state = TFigureEditor::STATE_NONE;
        fe->setCurrent(0);
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
        if (me.modifier() & MK_DOUBLE) {
//          cerr << "TFigureEditor: kludge: avoiding endless loop bug\n";
          break;
        }
        goto redo;
      }
      break;
  }
  if (figure)
    fe->invalidateFigure(figure);
}

void
TFCreateTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
  if (fe->state == TFigureEditor::STATE_NONE && ke.type == TKeyEvent::DOWN) {
    fe->clearSelection();
  }
  if (!figure || ke.type != TKeyEvent::DOWN)
    return;
  if (ke.getKey() == TK_ESCAPE) {
    fe->deleteFigure(figure);
    fe->state = TFigureEditor::STATE_NONE;
    fe->getWindow()->ungrabMouse();
    figure = 0;
    return;
  }

  unsigned r = figure->keyDown(fe, ke.getKey(), const_cast<char*>(ke.getString()), ke.modifier());
  if (r & TFigure::DELETE) {
//        cout << "  delete figure" << endl;
    fe->deleteFigure(figure);
    figure = 0;
  }
  if (r & TFigure::STOP) {
    fe->getWindow()->ungrabMouse();
    fe->state = TFigureEditor::STATE_NONE;
    if (figure) {
      fe->addFigure(figure);
      figure = 0;
    }
  }
  if (figure)
    fe->invalidateFigure(figure);
}

void
TFCreateTool::setAttributes(TFigureAttributes *a)
{
  if (figure)
    figure->setAttributes(a);
}

bool
TFCreateTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (!figure)
    return false;
  pen.push();
  if (figure->mat)
    pen.multiply(figure->mat);
  if (figure->cmat)
    pen.multiply(figure->cmat);
  figure->paint(pen, TFigure::EDIT);
  pen.pop();
  return true;
}
