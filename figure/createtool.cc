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

#include <toad/figureeditor.hh>
#include <toad/figure/createtool.hh>

using namespace toad;

void
TShapeTool::stop(TFigureEditor *fe)
{
//cout << "TShapeTool::stop" << endl;
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
    fe->getWindow()->ungrabMouse();
  }
}

void
TShapeTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint point;
  switch(me.type) {
    case TMouseEvent::LDOWN:
      fe->mouse2sheet(me.pos, &point);
      fe->sheet2grid(point, &point);
      fe->clearSelection();
      figure = static_cast<TFigure*>(tmpl->clone());
      fe->getAttributes()->setAllReasons();
      figure->setAttributes(fe->getAttributes());
      figure->startCreate(point);
      break;
    case TMouseEvent::MOVE:
    case TMouseEvent::LUP:
      fe->mouse2sheet(me.pos, &point);
      fe->sheet2grid(point, &point);
      fe->invalidateFigure(figure);
      figure->dragCreate(point);
      fe->invalidateFigure(figure);
      if (me.type == TMouseEvent::LUP) {
        TRectangle r = figure->bounds();
        if (r.size.width == 0 && r.size.height == 0) {
          delete figure;
        } else {
          fe->addFigure(figure);
          fe->selection.insert(figure);
        }
        figure = nullptr;
      }
      break;
  }
}

void
TShapeTool::setAttributes(TFigureAttributeModel *a)
{
  // FIXME: what about the selection? and does this really belong into the tool?
  if (figure)
    figure->setAttributes(a);
}

bool
TShapeTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (!figure)
    return false;
  figure->paint(pen, TFigure::EDIT);
  return true;
}
