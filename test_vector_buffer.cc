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

#include <toad/vector.hh>
#include <toad/figure.hh>
#include <toad/window.hh>
#include <toad/pen.hh>

#include <cstdarg>

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
  pen.translate(-0.5, -0.5);

  TFRectangle r0(50,50,100,50);
  r0.setFillColor(TRGB(1,0.5,0));
  r0.setLineColor(TRGB(0,0,0));

  TVectorGraphic *vb = r0.getPath();

  TFRectangle r1(100,80,200,50);
  r1.setFillColor(TRGB(0,0.5,1));
  r1.setLineColor(TRGB(0,0,0));

  TVectorGraphic *vb2 = r1.getPath();
  vb->push_back(vb2->front());
  delete vb2;

  vb->paint(pen);
  
  delete vb;
}

} // unnamed namespace

void
test_vector_buffer()
{
  TMyWindow wnd(NULL, "test vector buffer");
  toad::mainLoop();
}
