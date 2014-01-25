/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2007 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _FISCHLAND_FPATH_HH
#define _FISCHLAND_FPATH_HH 1
#include <toad/figure.hh>

using namespace toad;

class TFPath:
  public TColoredFigure
{
    SERIALIZABLE_INTERFACE(toad::, TFPath);
  public:
    TFPath() {
      closed = false;
    }
    void paint(TPenBase&, EPaintType);
    void paintSelection(TPenBase &pen, int handle);
    void getShape(toad::TRectangle *r);

    void translate(int dx, int dy);
    bool getHandle(unsigned handle, TPoint *p);
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier);
    double _distance(TFigureEditor *fe, int x, int y);
    unsigned mouseRDown(TFigureEditor*, TMouseEvent &);
    
    void addPoint(const TPoint &p) { polygon.addPoint(p); }
    void addPoint(int x, int y) { polygon.addPoint(x,y); }
    void insertPointNear(int x, int y);
    void deletePoint(unsigned i);

    bool closed;
    TPolygon polygon;
    vector<byte> corner;
};

#endif
