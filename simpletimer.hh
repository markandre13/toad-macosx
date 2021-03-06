/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2003 by Mark-André Hopf <mhopf@mark13.org>
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

// implementation is in `select.cc'

#ifndef TSimpleTimer
#define TSimpleTimer TSimpleTimer

#include <toad/types.hh>
#include <sys/time.h>

@class toadTimerListener;
@class NSTimer;

namespace toad {

class TOADBase;

class TSimpleTimer
{
  private:
    toadTimerListener *listener;
    NSTimer *nstimer;
  public:
    TSimpleTimer();
    virtual ~TSimpleTimer();
    virtual void tick()=0;
    void startTimer(ulong sec, ulong usec=0, bool skip_first=false);
    void stopTimer();
};

} // namespace toad

#endif
