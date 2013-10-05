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

#ifndef __TOAD_FOCUSMANGER_HH
#define __TOAD_FOCUSMANGER_HH 1

#include <toad/window.hh>
#include <toad/eventfilter.hh>

namespace toad {

class TFocusManager
{
  public:
    static void newWindow(TWindow *wnd);
    static void destroyWindow(TWindow* wnd);
    
    static void domainToWindow(TWindow *wnd);
    
    static void handleEvent(TKeyEvent &);
    
    static void setFocusWindow(TWindow* wnd);
    static TWindow* getFocusWindow();
    
    static void insertEventFilter(TEventFilter *flt, TWindow *wnd, EEventFilterPos pos);
    static void removeEventFilter(TEventFilter *flt);
};

} // namespace toad

#endif
