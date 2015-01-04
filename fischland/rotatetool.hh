/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 2014 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _FISCHLAND_ROTATETOOL_HH
#define _FISCHLAND_ROTATETOOL_HH 1

#include <toad/figureeditor.hh>

using namespace toad;

class TRotateTool:
  public TFigureTool
{
    TFigure *figure;   // figure to rotate (FIXME: should be selection)
    TMatrix2D oldmatrix;
    TCoord rotx, roty; // point to rotate around
    TCoord rotd, rotd0;
    
    enum {
      NONE,
      MOVE_CENTER,
      ROTATE
    } state;

  public:
    static TRotateTool* getTool();
/*
    void cursor(TFigureEditor *fe, int x, int y);
*/
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
/*
    void keyEvent(TFigureEditor *fe, const TKeyEvent &ke);
*/
    bool paintSelection(TFigureEditor *fe, TPenBase &pen);
/*
    void stop(TFigureEditor*);
*/
};

#endif
