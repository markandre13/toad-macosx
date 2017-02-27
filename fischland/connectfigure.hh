/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_CONNECTFIGURE_HH
#define _TOAD_CONNECTFIGURE_HH 1

#include <toad/figure.hh>

using namespace toad;

class TFConnection:
  public TColoredFigure
{
    typedef TColoredFigure super;
    const TFigure *start, *end;
  public:
    TFConnection(const TFigure *inStart, const TFigure *inEnd): start(inStart), end(inEnd) {}
    void paint(TPenBase &pen, EPaintType type=NORMAL) override;
    TRectangle bounds() const override;
/*
    TCoord distance(const TPoint &pos) override;
    void translate(TCoord dx, TCoord dy) override;
    bool getHandle(unsigned n, TPoint *p) override;
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override;
*/
    SERIALIZABLE_INTERFACE(toad::, TFConnection);
};

#endif
