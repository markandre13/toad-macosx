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

#ifndef __TOAD_CURSOR_HH
#define __TOAD_CURSOR_HH 1

//#include <toad/os.hh>
//#include <toad/toadbase.hh>

@class NSCursor;

namespace toad {

class TWindow;

class TCursor
{
  public:
    NSCursor *cursor;
    enum EType {
      // Java compatible cursor types
      DEFAULT,
      CROSSHAIR,
      HAND,
      TEXT,
      WAIT,
      MOVE,
      N_RESIZE,
      NE_RESIZE,
      NW_RESIZE,
      S_RESIZE,
      SE_RESIZE,
      SW_RESIZE,
      W_RESIZE,
      E_RESIZE,
      
      // other stuff
      QUESTION,
      EXCHANGE,
      RESIZE,
      PIRATE,
      MOUSE,
      PENCIL,
      SPRAY,
      HORIZONTAL,
      VERTICAL,
      TARGET,
      DOT,
      CIRCLE,
      PARENT,
      _MAX
    };

    TCursor(EType type);
    TCursor(const char shape[32][32+1], unsigned x=0, unsigned y=0);
    ~TCursor();
};

} // namespace toad

#endif
