/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>
#include <toad/vector.hh>
#include <toad/geometry.hh>

using namespace toad;

TVectorPath
computeBoolean(const TVectorPath &path1, const TVectorPath &path2, int operation)
{
  return TVectorPath();
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

  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(310,10));
  p0.line(TPoint(160,190));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(10,10));
  p1.curve(TPoint(160,190), TPoint(310,10), TPoint(10,10));
/*
  p1.line(TPoint(160,190));
  p1.line(TPoint(310,10));
  p1.close();
*/
  
  p0.apply(pen);
  pen.stroke();

  p1.apply(pen);
  pen.stroke();
}

} // unnamed namespace

void
test_path_bool()
{
  TMyWindow wnd(NULL, "test path bool");
  toad::mainLoop();
}
