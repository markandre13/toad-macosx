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

#include <toad/figure/connecttool.hh>
#include <toad/figure/connectfigure.hh>
#include <toad/figureeditor.hh>
#include <toad/vector.hh>

using namespace toad;

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
  fconnect = nullptr;
}

void 
TConnectTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint p;
  TFigure *figure;
  TFText *text;

  switch(me.type) {
    case TMouseEvent::LDOWN:
      fe->clearSelection();
      fe->mouse2sheet(me.pos, &p);
      figure = fe->findFigureAt(p);
      if (figure) {
        text = dynamic_cast<TFText*>(figure);
        if (text)
          figure = text->relation;
        fconnect = new TFConnection(figure, nullptr);
        fe->getAttributes()->setAllReasons();
        fconnect->setAttributes(fe->getAttributes());
        fconnect->p[1] = p;
        fconnect->updatePoints();
        firstFigure = figure;
        fe->invalidateFigure(fconnect);
        fe->getWindow()->grabMouse(true);
      }
      break;
    case TMouseEvent::MOVE:
      if (!firstFigure)
        break;
      fe->mouse2sheet(me.pos, &p);
      figure = fe->findFigureAt(p);
      text = dynamic_cast<TFText*>(figure);
      if (text)
        figure = text->relation;
      overFigure = figure;
      if (figure == firstFigure) {
        fconnect->p[1] = p;
      }
      fconnect->end = figure;
      if (!figure)
        fconnect->p[1] = p;
      fconnect->updatePoints();
      fe->invalidateFigure(fconnect);
      break;
    case TMouseEvent::LUP:
      if (!firstFigure)
        break;
      firstFigure = overFigure = nullptr;
      fe->addFigure(fconnect);
      if (fconnect->start)
        TFigureEditor::relatedTo[fconnect->start].insert(fconnect);
      if (fconnect->end)
        TFigureEditor::relatedTo[fconnect->end  ].insert(fconnect);
      fe->invalidateFigure(fconnect);
      fconnect = nullptr;
      fe->getWindow()->ungrabMouse();
      break;
  }
}


bool
TConnectTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  pen.push();
  pen.setColor(TColor::FIGURE_SELECTION);
  pen.setAlpha(1.0);
  pen.setScreenLineWidth(1.0);

  for(int i=0; i<2; ++i) {
    const TFigure *figure;
    switch(i) {
      case 0: figure = firstFigure; break;
      case 1: figure = overFigure; break;
    }
    if (!figure)
      continue;

    TVectorGraphic *graphic = figure->getPath();
    if (graphic) {
      for(auto &painter: *graphic) {
        painter->path->apply(pen);
        pen.stroke();
      }
      delete graphic;
    }
  }

  if (fconnect)
    fconnect->paint(pen, TFigure::EDIT);

  pen.pop();
  return true;
}

void
TConnectTool::stop(TFigureEditor *fe)
{
}

void
TConnectTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
}
