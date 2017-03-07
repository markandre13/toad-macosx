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

#include "connecttool.hh"
#include "connectfigure.hh"
#include "fischland.hh"

TConnectTool*
TConnectTool::getTool()
{
  static TConnectTool* tool = nullptr;
  if (!tool)
    tool = new TConnectTool();
  return tool;
}

TConnectTool::TConnectTool()
{
  firstFigure = overFigure = nullptr;
}

void 
TConnectTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint p;
  TFigure *figure;

  // find figure under mouse
  fe->mouse2sheet(me.pos, &p);
  figure = fe->findFigureAt(p);
  if (figure) {
    cout << "found figure" << endl;
  }

  switch(me.type) {
    case TMouseEvent::MOVE:
      if (figure==overFigure)
        break;
      overFigure = figure;
      fe->invalidateWindow(); // FIXME: invalidates too much
      break;
    case TMouseEvent::LDOWN:
      if (!firstFigure) {
        firstFigure = overFigure;
      } else {
        if (figure && figure!=firstFigure) {
          fe->addFigure(new TFConnection(firstFigure, figure));
        }
        firstFigure = nullptr;
      }
      break;
  }
}

bool
TConnectTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (overFigure) {
    pen.push();
    overFigure->paintSelection(pen);
    pen.pop();
  }
  if (firstFigure && overFigure != firstFigure) {
    pen.push();
    firstFigure->paintSelection(pen);
    pen.pop();
  }
}

void
TConnectTool::stop(TFigureEditor *fe)
{
}

void
TConnectTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
}
