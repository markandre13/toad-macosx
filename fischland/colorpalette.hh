/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2005 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_COLORPALETTE_HH
#define _TOAD_COLORPALETTE_HH 1

#include <toad/window.hh>
#include <toad/combobox.hh>
#include <toad/textfield.hh>
#include <toad/dnd/color.hh>
#include <toad/figureeditor.hh>

namespace toad {

class TColorPalette:
  public TWindow
{
    typedef TColorPalette TThis;
  public:
    TColorPalette(TWindow *parent, const string &title, TFigureAttributeModel *preferences = 0);
  protected:
    PFigureAttributes preferences;
    struct TNamedColor:
      public TRGB
    {
      TNamedColor(TCoord r, TCoord g, TCoord b, const string &aName):
        TRGB(r, g, b), name(aName) {}
      string name;
    };
    typedef vector<TNamedColor> TPalette;
    struct TNamedPalette
    {
      TPalette rgb;    // the colors of the palette
      bool modified;   // 'true' in case the palette was modified
      string filename; // full path of the palette file
      string name;     // name presented to the user
    };
    typedef vector<TNamedPalette*> TPaletteList;
    class TPaletteListAdapter:
      public TSimpleTableAdapter
    {
        TPaletteList *model;
      public:
        TPaletteListAdapter(TPaletteList *model) {
          this->model = model;
        }
        size_t getRows() { 
          return model->size();
        }
        void tableEvent(TTableEvent &te) {
          switch(te.type) {
            case TTableEvent::GET_COL_SIZE: 
              te.w = 200;
              break;
            case TTableEvent::GET_ROW_SIZE:
              te.h = getDefaultFont().getHeight()+2;
              break;
            case TTableEvent::PAINT:
              renderBackground(te);
              te.pen->drawString(0,(te.h-te.pen->getHeight())/2,model->at(te.row)->name);
              renderCursor(te);
              break;
	    default:
              break;
          }
        }
    };
    TPaletteListAdapter *palettelistrenderer;
    TPaletteList palettes;

    void paletteSelected(TComboBox *cb);
    void selectPalette(int n);

    //! pointer to current palette
    TNamedPalette *ppalette;

    //! position within current palette
    TNamedColor *pcolor;
    unsigned npos;

    // color field width and height
    int fw, fh;
    
    void paint();
    void mouseLDown(const TMouseEvent &);
    void mouseMDown(const TMouseEvent &);

    TNamedColor* mouse2color(TPoint mousepos);
    void dropColor(const PDnDColor&);
    
    void assignLineColor();
    void assignFillColor();

    bool loadPalette(const string &filename);

    TTextModel colorname;
    TIntegerModel position;
    TRectangle rectCurrent;
    TRectangle rectColors;
    TBoolModel doFillColor, doLineColor;
};

} // namespace toad

#endif
