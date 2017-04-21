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
    enum {
      CURSOR_SELECT           = 0,
      CURSOR_SELECT_HOVER     = 1, // Illustrator uses two variants: hover stroke & hover fill
      CURSOR_SELECT_MOVE      = 2,

      CURSOR_SELECT_RESIZE_N  = 3,
      CURSOR_SELECT_RESIZE_NE = 4,
      CURSOR_SELECT_RESIZE_E  = 5,
      CURSOR_SELECT_RESIZE_SE = 6,
      CURSOR_SELECT_RESIZE_S  = CURSOR_SELECT_RESIZE_N,
      CURSOR_SELECT_RESIZE_SW = CURSOR_SELECT_RESIZE_NE,
      CURSOR_SELECT_RESIZE_W  = CURSOR_SELECT_RESIZE_E,
      CURSOR_SELECT_RESIZE_NW = CURSOR_SELECT_RESIZE_SE,

      CURSOR_SELECT_ROTATE_N  = 7,
      CURSOR_SELECT_ROTATE_NE = 8,
      CURSOR_SELECT_ROTATE_E  = 9,
      CURSOR_SELECT_ROTATE_SE = 10,
      CURSOR_SELECT_ROTATE_S  = 11,
      CURSOR_SELECT_ROTATE_SW = 12,
      CURSOR_SELECT_ROTATE_W  = 13,
      CURSOR_SELECT_ROTATE_NW = 14,
    };
    static TCursor* cursor[15];
    
    enum {
      STATE_NONE,
      /**
       * "Marquee" is Adobe's name for the rectangular frame one drags
       * around objects to be selected.  Originally the frame was drawn
       * with a dotted line, hence that name derives from
       * https://en.wikipedia.org/wiki/Marquee_(sign)
       */
      STATE_DRAG_MARQUEE,
      STATE_MOVE_SELECTION,
      STATE_ROTATE_SELECTION
    } state;
    
    // marquee
    bool marqueeDraggedOpen;
    TPoint marqueeStart, marqueeEnd;
    void startMarquee(const TMouseEvent &me);
    void dragMarquee(TFigureEditor *fe, const TMouseEvent &me);
    void paintMarquee(TFigureEditor *fe, TPenBase &pen);
    void stopMarquee(TFigureEditor *fe);
    TFigureSet temporarySelection;
    
    // handle
    bool setCursorForHandle(TFigureEditor *fe, const TMouseEvent &me);
    bool downHandle(TFigureEditor *fe, const TMouseEvent &me);
    void moveHandle(TFigureEditor *fe, const TMouseEvent &me);

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
    TPoint last;                // last mouse position in figure coordinates when moving selection
    TPoint last_s;              // last mouse position in screen coordinates when moving selection
    TFigureSet tmpsel;          // objects to be added on next mouseLUp
    vector<TMatrix2D> oldmat;
  public:
    TSelectionTool() {
      state = STATE_NONE;
      scale_strokes_and_fx = true;
      down = false;
      rect = false;
      grab = false;
      hndl = false;
    }
    static TSelectionTool* getTool();
  
    void keyEvent(TFigureEditor *fe, const TKeyEvent &ke);
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
    
    
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
