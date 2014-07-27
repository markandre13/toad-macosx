/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public 
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
  Example for TOAD first data interface implementation:
*/

#include <toad/window.hh>

using namespace toad;

namespace {

class TWindowB:
  public TWindow
{
  public:
    TWindowA(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      setBackgroun(1,0.5,0);
    }
};

class TWindowA:
  public TDialog
{
  public:
    TWindowA(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      setSize(90,90);
      TWindow *w = new TWindowB(this, "window b");
      w->setShape(20,20,50,50);
    }
};

} // unnamed namespace

void
test_grab()
{
  TWindowA wnd(NULL, "mouse grab test");
  toad::mainLoop();
}
