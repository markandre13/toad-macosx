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

#ifndef _TOAD_DIRECTSELECTIONTOOL_HH
#define _TOAD_DIRECTSELECTIONTOOL_HH 1

#include <toad/figureeditor.hh>
#include <toad/boolmodel.hh>
#include <toad/undo.hh>
#include <toad/undomanager.hh>
#include <map>

namespace toad {

using namespace std;

class TDirectSelectionTool:
  public TFigureTool
{
    enum {
      STATE_NONE,
      STATE_FIGURE_HANDLES_MOUSE
    } state;

    set<TCoord> xalign, yalign;
    bool xaxis, yaxis;
    TCoord xaxisv, yaxisv;
    
    bool grab;                // grabbed selection for moving
    bool hndl;                // grabbed handle
    unsigned handle;          // the handle number
    TFigure *figure;          // selected figure
    TRectangle oldshape;
    
    bool tht;                 // do translate handle transformation

    TPoint memo_pt;

  public:
    TDirectSelectionTool() {
      grab = false;
      hndl = false;
      xaxis = yaxis = false;
      figure = nullptr;
      state = STATE_NONE;
    }
    static TDirectSelectionTool* getTool();

  protected:
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
    bool handleLDown(TFigureEditor *fe, const TPoint &pos);
    bool paintSelection(TFigureEditor*, TPenBase &pen);
    void stop(TFigureEditor *fe);
};

} // namespace toad

#endif
