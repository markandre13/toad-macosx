/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2014 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _FISCHLAND_SELECTIONTOOL_HH
#define _FISCHLAND_SELECTIONTOOL_HH 1

#include <toad/figureeditor.hh>
#include <toad/figuretool.hh>
#include <toad/boolmodel.hh>
#include <toad/undo.hh>
#include <toad/undomanager.hh>
#include <map>

namespace fischland {

using namespace std;
using namespace toad;

class TSelectionTool:
  public TFigureTool
{
    // by default strokes and effects aren't scaled
    TBoolModel scale_strokes_and_fx; // Preferences > General

    bool down;                // is the mouse button down?
    bool rect;                // rectangle selection?
    bool grab;                // grabbed selection for moving
    bool hndl;                // grabbed handle
    unsigned handle;
    TCoord rx0, ry0, rx1, ry1;  // rectangle for rectangle selection
    TCoord x0, y0, x1, y1;      // bounding rectangle
    TCoord ox0, oy0, ox1, oy1;  // bounding rectangle before resizing it
    TCoord last_x, last_y;      // last mouse position in figure coordinates when moving selection
    TCoord last_sx, last_sy;    // last mouse position in screen coordinates when moving selection
    TFigureSet tmpsel;          // objects to be added on next mouseLUp
    vector<TMatrix2D> oldmat;
  public:
    TSelectionTool() {
      scale_strokes_and_fx = true;
      down = false;
      rect = false;
      grab = false;
      hndl = false;
    }
    static TSelectionTool* getTool();
  
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
    
    void setCursorForHandle(TFigureEditor *fe, const TMouseEvent &me);
    bool downHandle(TFigureEditor *fe, const TMouseEvent &me);
    void moveHandle(TFigureEditor *fe, const TMouseEvent &me);
    
    void moveGrab(TFigureEditor *fe, const TMouseEvent &me);
    void moveSelect(TFigureEditor *fe, const TMouseEvent &me);
    
    bool paintSelection(TFigureEditor*, TPenBase &pen);
    void stop(TFigureEditor *fe);
    
    void invalidateBounding(TFigureEditor *fe);
    void getBoundingHandle(unsigned i, TRectangle *r);
    void calcSelectionsBoundingRectangle(TFigureEditor *fe);
};

} // namespace toad

#endif
