/*
 * TOAD -- A Simple and Powerfull C++ GUI Toolkit for X-Windows
 * Copyright (C) 1996-2017 by Mark-André Hopf <mhopf@mark13.org>
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

// UNDER CONSTRUCTION

#ifndef TGLWindow
#define TGLWindow TGLWindow

#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#include <toad/window.hh>

namespace toad {

class TGLWindow:
  public TWindow
{
  public:
    TGLWindow(TWindow*,const string &);

  private:
    void paint() override;
    void createCocoaView() override;

  protected:
    virtual void glPaint();
};

} // namespace toad

#endif
