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

#ifndef _TOAD_SELECTIONTOOL_HH
#define _TOAD_SELECTIONTOOL_HH 1

#include <toad/figureeditor.hh>
#include <toad/figuretool.hh>
#include <toad/boolmodel.hh>
#include <toad/undo.hh>
#include <toad/undomanager.hh>
#include <map>

namespace toad {

using namespace std;

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
      STATE_MOVE_HANDLE,
      STATE_MOVE_SELECTION,
    } state;

    void keyEvent(TFigureEditor *fe, const TKeyEvent &ke);
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);

    
    // marquee
    bool marqueeDraggedOpen;
    TPoint marqueeStart, marqueeEnd;
    void startMarquee(const TMouseEvent &me);
    void dragMarquee(TFigureEditor *fe, const TMouseEvent &me);
    void paintMarquee(TFigureEditor *fe, TPenBase &pen);
    void stopMarquee(TFigureEditor *fe);
    TFigureSet temporarySelection;
    
    // handle
    void getBoundaryHandle(unsigned i, TRectangle *r);
    bool setCursorForHandle(TFigureEditor *fe, const TMouseEvent &me);
    unsigned selectedHandle;
    TPoint handleStart;
    TMatrix2D m;
    TBoundary oldBoundary;
    TPoint rotationCenter;
    double rotationStartDirection;
    bool downHandle(TFigureEditor *fe, const TMouseEvent &me);
    void moveHandle(TFigureEditor *fe, const TMouseEvent &me);
    void moveHandle2Scale(TFigureEditor *fe, const TMouseEvent &me);
    void moveHandle2Rotate(TFigureEditor *fe, const TMouseEvent &me);
    void stopHandle(TFigureEditor *fe);

    void moveSelection(TFigureEditor *fe, const TMouseEvent &me);

    TBoundary boundary;
    void calcBoundary(TFigureEditor *fe);
    void invalidateBoundary(TFigureEditor *fe);
    void paintOutline(const TFigureSet &figures, TPenBase &pen);
    bool paintSelection(TFigureEditor*, TPenBase &pen);

    // by default strokes and effects aren't scaled
    TBoolModel scale_strokes_and_fx; // Preferences > General

  public:
    TSelectionTool() {
      state = STATE_NONE;
      scale_strokes_and_fx = true;
    }
    static TSelectionTool* getTool();
  
    void stop(TFigureEditor *fe);
};

} // namespace toad

#endif
