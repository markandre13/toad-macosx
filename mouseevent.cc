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

TMouseEvent::TMouseEvent(NSEvent *anEvent, TWindow *aWindow) {
  nsevent = anEvent;
  if (aWindow) {
    NSPoint p = [aWindow->nsview convertPoint:[anEvent locationInWindow] fromView:nil];
    pos.x = p.x;
    pos.y = p.y;
// cerr << "TMouseEvent::TMouseEvent: pos=("<<x<<","<<y<<"), origin=("<<aWindow->getOriginX()<<","<<aWindow->getOriginY()<<")\n";
    pos -= aWindow->getOrigin();
  }
  window = aWindow;
  dblClick = false;
  __modifier = [nsevent modifierFlags] 
           | _modifier;
}


// _doMouse helper: handle layout and global event filter
static void 
_doMouse2(TWindow *twindow, TMouseEvent &me)
{
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

  if (twindow->layout && twindow->layout->mouseEvent(me))
    return;

  twindow->mouseEvent(me);
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
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity)
    result = [nsevent pressure];
  return result;
}

float
TMouseEvent::tangentialPressure() const
{
  float result = 0.0;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity &&
      ([nsevent type] == NSTabletPoint ||
       [nsevent subtype]==NSTabletPointEventSubtype))
    result = [nsevent tangentialPressure];
  return result;
}

float
TMouseEvent::rotation() const
{
  float result = 0.0;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity &&
      ([nsevent type] == NSTabletPoint ||
       [nsevent subtype]==NSTabletPointEventSubtype))
    result = [nsevent rotation];
  return result;
}

TPoint
TMouseEvent::tilt() const
{
  TPoint result;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity &&
      ([nsevent type] == NSTabletPoint ||
       [nsevent subtype]==NSTabletPointEventSubtype))
  {
    NSPoint p = [nsevent tilt];
    result.set(p.x, p.y);
  }
  return result;
}

// handle grabPopUp mouse and enter/leave event generation
void
TMouseEvent::_doMouse(TWindow *twindow, TMouseEvent &me)
{
/*
if (me.type == TMouseEvent::LUP)
{
  cerr << "_doMouse: LUP -----------------------------------------------------" << endl;
}
if (me.type == TMouseEvent::LDOWN)
{
  cerr << "_doMouse: LDOWN ---------------------------------------------------" << endl;
}
if (me.type == TMouseEvent::LUP || me.type == TMouseEvent::LDOWN) {
  cerr << "_doMouse: TWindow='"<<twindow->getTitle()<<"', layout=" << twindow->layout << endl;
  cerr << "twindow=" << twindow << ", TWindow::grabPopupWindow=" << TWindow::grabPopupWindow << endl;
}
*/
  TWindow *mouseOver = 0;
  
  NSPoint p;
  p = [NSEvent mouseLocation];
  NSInteger wn = [NSWindow windowNumberAtPoint: p belowWindowWithWindowNumber: 0];
  NSWindow *wnd = [NSApp windowWithWindowNumber: wn];
    
  if (wnd && [wnd isKindOfClass:[toadWindow class]]) {
    toadWindow *t = (toadWindow*)wnd;
      
    NSRect r;
    r.origin = p;
    r = [wnd convertRectFromScreen: r]; // convert screen to window coordinates
      
    NSView *view = [t->twindow->nsview hitTest: r.origin];
    if (view && [view isKindOfClass:[toadView class]]) {
      toadView *v = (toadView*)view;
      mouseOver = v->twindow;
    }
  }

//cout << "_doMouse " << twindow << " " << twindow->getTitle() << " " << me.name() << ", over=" << (mouseOver?mouseOver->getTitle():"NULL") << endl;

  if (mouseOver != TWindow::lastMouse) {
    if (TWindow::lastMouse && TWindow::lastMouse->_inside) {
//cout << "set "<<TWindow::lastMouse<<" " << lastMouse->getTitle() << "->inside=false (grab)"<<endl;
      TWindow::lastMouse->_inside = false;
      TMouseEvent me2(me.nsevent, TWindow::lastMouse);
      me2.type = TMouseEvent::LEAVE;
      _doMouse2(TWindow::lastMouse, me2);
    }
    if (mouseOver && !mouseOver->_inside) {
//cout << "set "<<mouseOver<<" " << mouseOver->getTitle() << "->inside=true (grab)"<<endl;
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
    if (mouseOver) {
//      cout << "  mouse over title=" << mouseOver->getTitle() << endl;
      twindow = mouseOver;
//    } else {
//      cout << "  not a toad window" << endl;
    }
//    cout << "  twindow="<<twindow<<", TWindow::lastMouse="<<lastMouse<<", mouseOver="<<mouseOver<<endl;
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
/*
if (me.type == TMouseEvent::LUP || me.type == TMouseEvent::LDOWN) {
  cerr << "twindow=" << twindow << ", TWindow::grabPopupWindow=" << TWindow::grabPopupWindow << endl;
}
if (me.type == TMouseEvent::LDOWN)
  cerr << "_doMouse: LDOWN done -----------------------------------------------" << endl;
if (me.type == TMouseEvent::LUP)
  cerr << "_doMouse: LUP done -------------------------------------------------" << endl;
*/
}
