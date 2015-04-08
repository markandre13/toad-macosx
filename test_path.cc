/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.org>
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

//#import <AppKit/NSAttributedString.h>
//#import <AppKit/NSPasteboard.h>
//@import AppKit;

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>
#include <toad/vector.hh>

using namespace toad;

void
TVectorPath::apply(TPen &pen) const
{
  const TPoint *pt = points.data();
  for(auto p: type) {
    switch(p) {
      case MOVE: pen.move(pt); ++pt; break;
      case LINE: pen.line(pt); ++pt; break;
      case CURVE: pen.curve(pt); pt+=3; break;
      case CLOSE: pen.close(); break;
    }
  }
}

namespace {

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
};


void
TMyWindow::paint()
{
  TPen pen(this);
  TVectorPath p;
  
  p.move(TPoint(120, 20));
  p.line(TPoint(300, 50));
  p.curve(TPoint(310,190), TPoint(10,190), TPoint(50, 30));
  p.close();
  
  p.apply(pen);
  pen.stroke();
/*  
  pen.setColor(1,0,0);
  pen.drawRectangle(p.bounds());
  pen.setColor(0,0,1);
  pen.drawRectangle(p.editBounds());
*/
}

} // unnamed namespace

void
test_path()
{
  TMyWindow wnd(NULL, "test path");
  toad::mainLoop();
}
