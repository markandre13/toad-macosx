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

#include <toad/window.hh>
#include <toad/pen.hh>

using namespace toad;

namespace {

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint();
};

void
TMyWindow::paint()
{
  TPen pen(this);

  pen.drawRectangle(50,50,100,50);
  
  pen.translate(100,75);
  pen.rotate(2.0 * M_PI / 4);
  pen.translate(-100,-75);
  
  pen.drawRectangle(50,50,100,50);
  
  // via pen
  pen.setColor(1,0,0);
  pen.identity();
  pen.drawRectangle(50,50,100,50);

  pen.translate(50, 50);
pen.drawRectangle(-2,-2,5,5);
  pen.rotate(2.0 * M_PI / 16);
  pen.translate(-50, -50);

  pen.translate(100,75);
  pen.rotate(2.0 * M_PI / 4);
  pen.translate(-100,-75);

  pen.setColor(0,0,1);
  pen.drawRectangle(50,50,100,50);

  // via matrix
  TMatrix2D m;
  pen.identity();
  pen.translate(50, 50);
  pen.rotate(2.0 * M_PI / 16);
  pen.translate(-50, -50);

  TMatrix2D m1;
  m1.translate(100,75);
  m1.rotate(2.0 * M_PI / 4);
  m1.translate(-100,-75);

  TMatrix2D m2;
  m2 = m1;
  
  pen.multiply(&m2);

  pen.setColor(0,1,0);
  pen.drawRectangle(50,50,100,50);
}

} // unnamed namespace

void
test_grab()
{
  TMyWindow wnd(NULL, "my test");
  toad::mainLoop();
}
