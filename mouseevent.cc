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

#include <toad/window.hh>
#include <toad/layout.hh>
#include <toad/eventfilter.hh>

#import <Cocoa/Cocoa.h>
#include "cocoa/toadwindow.h"
#include "cocoa/toadview.h"

using namespace toad;

unsigned TMouseEvent::_modifier = 0;

// the following values are only delivered to proximity events.
// these static variables are used to copy them into all events.
static TMouseEvent::EPointerType staticPointerType = TMouseEvent::UNKNOWN;
static TMouseEvent::TPointerID staticPointerID = 0;
static bool staticProximity = false;

void
TMouseEvent::init(NSEvent *anEvent, TWindow *aWindow)
{
  nsevent = anEvent;
  if (aWindow) {
    auto &&p = [aWindow->nsview convertPoint:[anEvent locationInWindow] fromView:nil];
    pos.x = p.x;
    pos.y = p.y;
    pos -= aWindow->getOrigin();
  }
  window = aWindow;
  dblClick = false;
  __modifier = [nsevent modifierFlags] 
           | _modifier;
  _pointerType = staticPointerType;
  _pointerID = staticPointerID;
  _proximity = staticProximity;
}

TMouseEvent::TMouseEvent(NSEvent *nsevent, TWindow *window, EType aType)
{
  type = aType;
  if (type==TABLET_PROXIMITY) {
    if ([nsevent isEnteringProximity]) {
      staticPointerType = static_cast<EPointerType>([nsevent pointingDeviceType]);
      staticPointerID = [nsevent uniqueID];
      staticProximity = true;
    } else {
      staticPointerType = UNKNOWN;
      staticPointerID = 0;
      staticProximity = false;
    }
  }
  init(nsevent, window);
}

const char*
TMouseEvent::name() const
{
  static const char *name[13] = {
    "MOVE", "ENTER", "LEAVE",
    "LDOWN", "MDOWN", "RDOWN",
    "LUP", "MUP", "RUP",
    "ROLL_UP", "ROLL_DOWN",
    "TABLET_POINT", "TABLET_PROXIMITY"
  };
  if (type>=0 && type<=12)
    return name[type];
  return "?";
}

float
TMouseEvent::pressure() const
{
  float result = 0.0;
  if ([nsevent type] != NSEventTypeScrollWheel &&
      [nsevent type] != NSEventTypeTabletProximity)
    result = [nsevent pressure];
  return result;
}

float
TMouseEvent::tangentialPressure() const
{
  float result = 0.0;
  if ([nsevent type] != NSEventTypeScrollWheel &&
      [nsevent type] != NSEventTypeTabletProximity &&
      ([nsevent type] == NSEventTypeTabletPoint ||
       [nsevent subtype]==NSEventSubtypeTabletPoint))
    result = [nsevent tangentialPressure];
  return result;
}

float
TMouseEvent::rotation() const
{
  float result = 0.0;
  if ([nsevent type] != NSEventTypeScrollWheel &&
      [nsevent type] != NSEventTypeTabletProximity &&
      ([nsevent type] == NSEventTypeTabletPoint ||
       [nsevent subtype]==NSEventSubtypeTabletPoint))
    result = [nsevent rotation];
  return result;
}

TPoint
TMouseEvent::tilt() const
{
  TPoint result;
  if ([nsevent type] != NSEventTypeScrollWheel &&
      [nsevent type] != NSEventTypeTabletProximity &&
      ([nsevent type] == NSEventTypeTabletPoint ||
       [nsevent subtype]==NSEventSubtypeTabletPoint))
  {
    NSPoint p = [nsevent tilt];
    result.set(p.x, p.y);
  }
  return result;
}

// _doMouse helper: handle layout and global event filter
static void 
_doMouse2(TWindow *twindow, TMouseEvent &me)
{
//cout << "_doMouse2: " << me.name() << " for window " << twindow->getTitle() << endl;
  if (TWindow::grabWindow) {
    cout << "call " << TWindow::grabWindow->getTitle() << " instead of " << twindow->getTitle() << endl;
//    NSPoint p = [aWindow->nsview convertPoint:[anEvent locationInWindow] fromView:nil];

    TWindow::grabWindow->mouseEvent(me);
    return;
  }

  if (me.type == TMouseEvent::ENTER) {
    if (twindow->cursor && twindow->cursor->cursor)
      [twindow->cursor->cursor set];
    else
      [[NSCursor arrowCursor] set];
  }
  
  if (me.type == TMouseEvent::MOVE &&
      !(me.modifier() & (MK_LBUTTON|MK_MBUTTON|MK_RBUTTON)) &&
      !twindow->_allMouseMoveEvents)
    return;

  me.window = twindow;
  TEventFilter *flt = toad::global_evt_filter;
  while(flt) {
    if (flt->mouseEvent(me))
      return;
    flt = flt->next;
  }

//  cout << "_doMouse2: " << me.name() << endl;
  if (twindow->layout && twindow->layout->mouseEvent(me))
    return;

  twindow->mouseEvent(me);
}

/**
 * return the TWindow at the current mouse location
 */
static inline TWindow*
toadWindowBeneathMouse()
{
  TWindow *result = nullptr;

  NSPoint mouseLocation = [NSEvent mouseLocation];
  NSInteger cocoaWindowNumber = [NSWindow windowNumberAtPoint: mouseLocation belowWindowWithWindowNumber: 0];
  NSWindow *cocoaWindow = [NSApp windowWithWindowNumber: cocoaWindowNumber];
  
  if (!cocoaWindow)
    return result;
  if (![cocoaWindow isKindOfClass:[toadWindow class]])
    return result;
    
  toadWindow *toadWindow = static_cast<::toadWindow*>(cocoaWindow);
    
  // convert mouse location from screen to window coordinates
  NSRect mouseInWindow;
  mouseInWindow.origin = mouseLocation;
  mouseInWindow = [cocoaWindow convertRectFromScreen: mouseInWindow];

  NSView *cocoaView = [toadWindow->twindow->nsview hitTest: mouseInWindow.origin];
  if (!cocoaView)
    return result;
  if (![cocoaView isKindOfClass:[toadView class]])
    return result;
    
  result = static_cast<toadView*>(cocoaView)->twindow;
  return result;
}

/**
 * handle grabPopUp mouse and enter/leave event generation
 */
void
TMouseEvent::_doMouse(TWindow *twindow, TMouseEvent &me)
{
  static bool automaticGrab = false;

  if (!TWindow::grabPopupWindow) {
    switch(me.type) {
      case TMouseEvent::LDOWN:
      case TMouseEvent::MDOWN:
      case TMouseEvent::RDOWN:
        automaticGrab = true;
        break;
    }
    
    bool flag = false;
    if (automaticGrab) {
      _doMouse2(twindow, me);
      flag = true;
    }
    
    switch(me.type) {
      case TMouseEvent::LUP:
      case TMouseEvent::MUP:
      case TMouseEvent::RUP:
        automaticGrab = false;
        break;
    }
    
    if (flag)
      return;
  } else {
    automaticGrab = false;
  }

  TWindow *mouseOver = toadWindowBeneathMouse();  

  if (mouseOver != TWindow::lastMouse) {
    if (TWindow::lastMouse && TWindow::lastMouse->_inside) {
      TWindow::lastMouse->_inside = false;
      TMouseEvent me2(me.nsevent, TWindow::lastMouse);
      me2.type = TMouseEvent::LEAVE;
      _doMouse2(TWindow::lastMouse, me2);
    }
    if (mouseOver && !mouseOver->_inside) {
      mouseOver->_inside = true;
      TMouseEvent me2(me.nsevent, mouseOver);
      me2.type = TMouseEvent::ENTER;
      if (me.type == TMouseEvent::LDOWN ||
          me.type == TMouseEvent::MDOWN ||
          me.type == TMouseEvent::RDOWN )
      {
        // mouse enter & down at the same time -> remove the button
        // information from the modifier
        me2.__modifier &= ~(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON);
      }
      _doMouse2(mouseOver, me2);
    }
    TWindow::lastMouse = mouseOver;
  }
  
  if (me.type == TMouseEvent::ENTER ||
      me.type == TMouseEvent::LEAVE)
    return;

  if (TWindow::grabPopupWindow) {
    twindow = TWindow::grabPopupWindow;
  }

  _doMouse2(twindow, me);

  if ( TWindow::grabPopupWindow &&
       twindow != TWindow::grabPopupWindow &&
       ( me.type == TMouseEvent::LDOWN ||
         me.type == TMouseEvent::MDOWN ||
         me.type == TMouseEvent::RDOWN ) &&
       !twindow->isChildOf(TWindow::grabPopupWindow) )
  {
    TWindow::ungrabMouse();
  }
}
