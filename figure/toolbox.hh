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

#ifndef _TOAD_FIGURE_TOOLBOX_HH
#define _TOAD_FIGURE_TOOLBOX_HH 1

#include <toad/window.hh>
#include <toad/action.hh>

namespace toad {

using namespace std;

class TFigureTool;

/**
 * \ingroup figure
 * \class toad::TToolBox
 *
 * TToolBox provides a collection of TFigureTools to TFigureEditor.
 *
 * A TFigureEditor has one TToolBox, while a TToolBox may be assigned to
 * many TFigureEditors.
 *
 * One TFigureTool within TToolBox is the active tool, to which
 * TFigureEditor will delegate all edit events.
 *
 * When the pointing device provides an ID, as is the case with some graphic
 * tablet pens, TToolBox will also maintain a separate active tool per pen.
 */
class TToolBox:
  public GChoiceModel<TFigureTool*>
{
    typedef GChoiceModel<TFigureTool*> super;

    // each pointer has it's own active tool
    map<TMouseEvent::TPointerID, TFigureTool*> toolForPointer;
    TMouseEvent::TPointerID activePointer;
  public:
    TToolBox();
    TFigureTool* getTool() const { return getValue(); }
    static TToolBox *getToolBox();
    void selectPointer(TMouseEvent::TPointerID pointerID);
};

} // namespace toad

#endif
