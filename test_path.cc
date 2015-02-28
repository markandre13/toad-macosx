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
#include <toad/figure.hh>
#include <toad/connect.hh>

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
    
    void paint() override;
};

void
TMyWindow::paint()
{
  TPen pen(this);
  TPath p;
  
  cout << "elements: " << p.size() << endl;
  
  p.move(TPoint(120, 20));
  cout << "elements: " << p.size() << endl;
  p.line(TPoint(300, 50));
  cout << "elements: " << p.size() << endl;
  p.curve(TPoint(310,190), TPoint(10,190), TPoint(50, 30));
  cout << "elements: " << p.size() << endl;
  p.close();
  cout << "elements: " << p.size() << endl;
  
  p.move(TPoint(1,2));
  
  for(int i=0; i<p.size(); ++i) {
    TPoint pts[3];
    TPath::EType type = p.point(i, pts);
    cout << i << ": " << type << " ";
    switch(p.point(i, pts)) {
      case TPath::CURVE:
        cout << pts[0] << ", " << pts[1] << ", " << pts[2] << endl;
        break;
      default:
        cout << pts[0] << endl;
        break;
    }
  }
  
  p.stroke();
  
  pen.setColor(1,0,0);
  pen.drawRectangle(p.bounds());
  pen.setColor(0,0,1);
  pen.drawRectangle(p.editBounds());
}

} // unnamed namespace

void
test_path()
{
  TMyWindow wnd(NULL, "test path");
  toad::mainLoop();
}
