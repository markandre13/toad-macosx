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

#ifndef _TOAD_EVENTFILTER_HH
#define _TOAD_EVENTFILTER_HH 1

namespace toad {

enum EEventFilterPos {
  KF_GLOBAL,
  KF_TOP_DOMAIN,
  KF_DOMAIN,
  KF_WINDOW 
};
  
class TWindowEvent;
class TMouseEvent; 
class TKeyEvent;   

class TEventFilter
{
  public:
    virtual ~TEventFilter();
    virtual bool windowEvent(TWindowEvent&);
    virtual bool mouseEvent(TMouseEvent&);  
    virtual bool keyEvent(TKeyEvent&);
//  private:
    TEventFilter *next;
    EEventFilterPos pos;
    void *ptr;
};

extern TEventFilter *global_evt_filter;

} // namespace toad

#endif
