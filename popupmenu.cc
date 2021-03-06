/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/popupmenu.hh>
#include <toad/focusmanager.hh>

using namespace toad;

/**
 * \ingroup control
 * \class toad::TPopupMenu
 *
 */

class TPopupMenu::TMenuFilter:
  public TEventFilter
{ 
    TWindow *window;
    TPopupMenu *menu;
  public:
    TMenuFilter(TPopupMenu *menu, TWindow *window) {
      this->menu   = menu;
      this->window = window;
    }
  protected:
    bool mouseEvent(TMouseEvent &evt) {
      if (evt.window == window &&
          evt.type == TMouseEvent::RDOWN)
      {
        menu->open(evt);
        return true;
      }
      return false;
    }
};

TPopupMenu::TPopupMenu(TWindow *parent, const string &title):
  TMenuHelper(parent, title)
{
  flt = 0;
  vertical = true;
  flagExplicitCreate = true;
  flagPopup = true;
  setLayout(new TMenuLayout()); // i require a layout
}

TPopupMenu::~TPopupMenu()
{
  if (flt) {
    TFocusManager::removeEventFilter(flt);
  }
}

/**
 * Open the popup menu.
 */
void
TPopupMenu::open(TMouseEvent &event)
{
  pos = event.pos;
  modifier = event.modifier();
  placeWindow(PLACE_CORNER_MOUSE_POINTER, 0);
  createWindow();
  grabPopupMouse(true);
}

void
TPopupMenu::open(TPoint pos, unsigned modifier)
{
  this->pos = pos;
  this->modifier = modifier;
  placeWindow(PLACE_CORNER_MOUSE_POINTER, 0);
  createWindow();
  grabPopupMouse(true);
}

/**
 * Add a event filter, which catches the right mouse button being pressed
 * inside the popup menu's parent window and invokes 'open()'.
 */
void
TPopupMenu::addFilter()
{
  if (flt)
    return;
  flt = new TMenuFilter(this, getParent());
  TFocusManager::insertEventFilter(flt, 0, KF_GLOBAL);
}
