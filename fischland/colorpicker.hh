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

#ifndef _FISCHLAND_COLORPICKER_HH
#define _FISCHLAND_COLORPICKER_HH 1

#include <toad/figureeditor.hh>

using namespace toad;

class TColorPicker:
  public TWindow
{
    PFigureAttributes attr;

    enum Active {
      FILL, LINE
    } active;
    enum Type {
      COLOR, GRADIENT, NONE
    } fill, line; // GRADIENT only applies to fill
    TRGB linecolor;
    TRGB fillcolor;

  public:
    TColorPicker(TWindow *p, const string &t, TFigureAttributeModel *a=0);
    ~TColorPicker();
    void preferencesChanged();
    
    void paint();
    void mouseLDown(const TMouseEvent&);
    bool click(const TMouseEvent&);

    void init();
    void paintLine(TPen&);
    void paintFill(TPen&);
};

#endif
