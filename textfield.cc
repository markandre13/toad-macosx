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

#include <toad/textfield.hh>

using namespace toad;

void
TTextField::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::ROLL_UP:
      keyEvent(TKeyEvent(TKeyEvent::DOWN, TK_UP, "", 0));
      keyEvent(TKeyEvent(TKeyEvent::UP, TK_UP, "", 0));
      break;
    case TMouseEvent::ROLL_DOWN:
      keyEvent(TKeyEvent(TKeyEvent::DOWN, TK_DOWN, "", 0));
      keyEvent(TKeyEvent(TKeyEvent::UP, TK_DOWN, "", 0));
      break;
    default:
      super::mouseEvent(me);
  }
}
