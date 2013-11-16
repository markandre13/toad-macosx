/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2006 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef __TOAD_COLOR_HH
#define __TOAD_COLOR_HH 1

#include <toad/types.hh>
#include <toad/io/serializable.hh>

namespace toad {

using namespace atv;

struct TRGB;

class TColor {
  public:
    enum EColor {
      BLACK=0,
      RED=1,  
      GREEN=2,
      YELLOW=3,
      BLUE=4,  
      MAGENTA=5,
      CYAN=6,   
      GREY=7, GRAY=7,
      LIGHTGREY=8, LIGHTGRAY=8,
      LIGHTRED,
      LIGHTGREEN,
      LIGHTYELLOW,
      LIGHTBLUE,  
      LIGHTMAGENTA,
      LIGHTCYAN,   
      WHITE,

      BTNTEXT,              // buttonbase
      BTNSHADOW,
      BTNFACE,  
      BTNLIGHT, 
      MENU,                 // menubar
      MENUTEXT,
      TEXTEDIT,
      MDIAREA,              // mdi stuff
      CAPTION,
      CAPTIONTEXT,
      INACTIVECAPTION,
      INACTIVECAPTIONTEXT,
      DIALOG,               // dialog
      DIALOGTEXT,
      SLIDER_FACE,          // scrollbar slider
      SLIDER_SHADOW,
      SLIDER_LIGHT, 
      SELECTED,             // background for selected text when focus
      SELECTED_2,
      SELECTED_TEXT,        // color for selected text
      SELECTED_GRAY,        // background for selected text when no focus
      SELECTED_GRAY_2,
      TABLE_CELL,           // table cell color
      TABLE_CELL_2,
      FIGURE_SELECTION,     // used by figureeditor to mark selections
      MAX
    };

  static const TRGB* lookup(EColor);
};

struct TRGB
{
    TCoord r,g,b;
    TRGB() {
      r=g=b=0;
    }
    TRGB(TCoord ir, TCoord ig, TCoord ib) {
      r=ir; g=ig; b=ib;
    }

    bool operator ==(const TRGB &c) const {
      return (r==c.r && g==c.g && b==c.b);
    }
    bool operator !=(const TRGB &c) const {
      return (r!=c.r || g!=c.g || b!=c.b);
    }
    void operator() (TCoord rn, TCoord gn, TCoord bn) {
      r=rn;g=gn;b=bn;
    }
    void set(TCoord rn, TCoord gn, TCoord bn) {
      r=rn;g=gn;b=bn;
    }
    void set(TColor::EColor c) {
      *this = *TColor::lookup(c);
    }
};

struct TRGBA {
  TRGBA() {
    r = g = b = 0.0;
    a = 1.0;
  }
  TCoord r, g, b, a;
};

struct TSerializableRGB:
  public TRGB, public TSerializable
{
    typedef TSerializable super;

    TSerializableRGB() {}
 
    TSerializableRGB(TCoord ir, TCoord ig, TCoord ib) {
      r=ir; g=ig; b=ib;
    }
    TSerializableRGB& operator= (const TRGB &c) {
      r=c.r; g=c.g; b=c.b;
      return *this;
    }
    SERIALIZABLE_INTERFACE(toad::, TSerializableRGB)
};

bool restore(TInObjectStream &p, TSerializableRGB *value);

} // namespace toad

#endif
