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

#include <toad/color.hh>

using namespace toad;

static TRGB sysrgb[TColor::MAX]=
{
                        // IBM PC CGA     Amstrad CPC   HTML
  TRGB(  0,  0,  0),  //  0 black       0             Black
  TRGB(128,  0,  0),  //  1 red         3             Maroon
  TRGB(  0,128,  0),  //  2 green       9             Green 
  TRGB(128,128,  0),  //  3 yellow      12            Olive 
  TRGB(  0,  0,128),  //  4 blue        1             Navy  
  TRGB(128,  0,128),  //  5 violett     4             Purple
  TRGB(  0,128,128),  //  6 cyan        10            Teal  
  TRGB(128,128,128),  //  7 gray        13            Gray  
  TRGB(192,192,192),  //  8 lightgray                 Silver
  TRGB(255,  0,  0),  //  9 lightred    6             Red   
  TRGB(  0,255,  0),  // 10 lightgreen  18            Lime  
  TRGB(255,255,  0),  // 11 yellow      24            Yellow
  TRGB(  0,  0,255),  // 12 lightblue   2             Blue  
  TRGB(255,  0,255),  // 13 magenta     8             Fuchsia
  TRGB(  0,255,255),  // 14 lightcyan   20            Aqua   
  TRGB(255,255,255),  // 15 white       26            White  

  // SystemColors
  TRGB(  0,  0,  0),  // BTNTEXT
  TRGB(150,150,150),  // BTNSHADOW
  TRGB(215,215,215),  // BTNFACE
  TRGB(255,255,255),  // BTNLIGHT
  TRGB(215,215,215),  // MENU
  TRGB(  0,  0,  0),  // MENUTEXT
  TRGB(255,255,255),  // TEXTEDIT
  TRGB(128,  0,  0),  // MDIAREA
  TRGB(  0,  0,128),  // CAPTION
  TRGB(255,255,255),  // CAPTIONTEXT
  TRGB(215,215,215),  // INACTIVECAPTION
  TRGB(  0,  0,  0),  // INACTIVECAPTIONTEXT
  TRGB(215,215,215),  // DIALOG
  TRGB(  0,  0,  0),  // DIALOGTEXT
  TRGB(127,127,191),  // SLIDER_FACE
  TRGB( 63, 63,127),  // SLIDER_SHADOW
  TRGB(191,191,255),  // SLIDER_LIGHT
  TRGB(137,137,215),  // SELECTED
  TRGB(127,127,191),  // SELECTED_2
  TRGB(  0,  0,  0),  // SELECTED_TEXT
  TRGB(222,222,222),  // SELECTED_GRAY
  TRGB(215,215,215),  // SELECTED_GRAY_2
  TRGB(255,255,255),  // TABLE_CELL   
  TRGB(231,231,255),  // TABLE_CELL_2
  TRGB( 79,128,255),  // FIGURE_SELECTION
};

const TRGB*
TColor::lookup(EColor n)
{
  if (n>=MAX)
    n = BLACK;
  return &sysrgb[n];
}

bool
toad::restore(atv::TInObjectStream &p, toad::TSerializableRGB *value)
{
  if (p.what != ATV_GROUP)
    return false;
//  if (!p.type.empty())
//    return false;
  p.setInterpreter(value);
  return true;
}

void
TSerializableRGB::store(TOutObjectStream &out) const
{
  ::store(out, (int)(r*255));
  ::store(out, (int)(g*255));
  ::store(out, (int)(b*255)); 
}

bool
TSerializableRGB::restore(TInObjectStream &in)
{
  int i;
  if (super::restore(in))
    return true;
  if (::restore(in, 0, &i)) {
    r = i / 255.0;
    return true;
  }
  if (::restore(in, 1, &i)) {
    g = i / 255.0;
    return true;
  }
  if (::restore(in, 2, &i)) {
    b = i / 255.0;
    return true;
  }
  ATV_FAILED(in) 
  return false;
}
