/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2014 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figuretool.hh>

using namespace toad;

TFigureTool::~TFigureTool()
{
}

void
TFigureTool::stop(TFigureEditor*)
{
}

void
TFigureTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
}

void
TFigureTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
}

void
TFigureTool::setAttributes(TFigureAttributes *p)
{
}

/**
 * Let the figure tool draw a selection.
 * \return 'true' when the figure editor shall not paint its own selection
 */
bool
TFigureTool::paintSelection(TFigureEditor *fe, TPenBase &)
{
  return false;
}

/**
 * This virtual method is called each time TFigureEditor's TFigureModel
 * reported a change.
 */
void
TFigureTool::modelChanged(TFigureEditor *fe)
{
}

/**
 * Create a window to configure additional tool options while the tool is
 * active.
 */
TWindow*
TFigureTool::createEditor(TWindow *inWindow)
{
  return 0;
}
