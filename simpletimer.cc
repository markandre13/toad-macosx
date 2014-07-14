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

#import <Foundation/NSTimer.h>
#import <Foundation/NSRunLoop.h>

#include <toad/simpletimer.hh>

#include <iostream>
using namespace std;

using namespace toad;

@interface toadTimerListener : NSObject
{
  @public
    TSimpleTimer *tsimpletimer;
}
@end

@implementation toadTimerListener : NSObject

- (void)setSimpleTimer: (TSimpleTimer*)aTimer {
  tsimpletimer = aTimer;
}  
  

- (void)timerFireMethod:(NSTimer*)timer {
  tsimpletimer->tick();
}

@end

TSimpleTimer::TSimpleTimer() {
  listener = 0;
  nstimer = 0;
}

TSimpleTimer::~TSimpleTimer()
{
  stopTimer();
  if (listener)
    [listener release];
}

void
TSimpleTimer::startTimer(unsigned long sec, unsigned long usec, bool skip_first)
{
  NSTimeInterval s = sec + usec/1000000.0;
  if (!listener) {
    listener = [toadTimerListener alloc];
    [listener setSimpleTimer: this];
  }
  NSDate *fireDate = [NSDate dateWithTimeIntervalSinceNow: skip_first ? sec : 0.0];
  nstimer = [[NSTimer alloc] initWithFireDate:fireDate
                                      interval:s
                                      target:listener
                                      selector:@selector(timerFireMethod:)
                                      userInfo:nil
                                      repeats:YES];
  NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
  [runLoop addTimer:nstimer forMode:NSDefaultRunLoopMode];
}

void
TSimpleTimer::stopTimer()
{
  if (!nstimer)
    return;
  [nstimer invalidate];
  [nstimer release];
  nstimer = 0;
}
