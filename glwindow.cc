/*
 * TOAD -- A Simple and Powerfull C++ GUI Toolkit for X-Windows
 * Copyright (C) 2016-2017 by Mark-André Hopf <mhopf@mark13.org>
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

// OpenGL support for TOAD

// UNDER CONSTRUCTION

#include <toad/glwindow.hh>
#include <toad/core.hh>
#include <toad/layout.hh>
#include <toad/focusmanager.hh>

#import <Cocoa/Cocoa.h>

@interface toadGLView : NSOpenGLView <NSTextInputClient>
{
  @public
    toad::TWindow *twindow;
}
@end

using namespace toad;

@implementation toadGLView : NSOpenGLView
#include "cocoa/toadview.impl"
@end


TGLWindow::TGLWindow(TWindow *p,const string &t)
  :TWindow(p,t)
{
}

void
TGLWindow::createCocoaView()
{
  NSOpenGLPixelFormatAttribute attrs[] = {
    // NSOpenGLPFADoubleBuffer,
    NSOpenGLPFADepthSize, 32,
    0
  };

  NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  if(pixFmt == nil) {
    cerr << "TGLWindow::paint(): failed to create NSOpenGLPixelFormat" << endl;
    return;
  }

  toadGLView *gl = [[toadGLView alloc]
    initWithFrame: NSMakeRect(x,y,w,h)
    pixelFormat: pixFmt
  ];
  gl->twindow = this;
  nsview = gl;
}

void
TGLWindow::paint()
{
  glViewport(0,0,w,h);
  glPaint();
  glFlush();
}

void TGLWindow::glPaint()
{
  glClearColor( 0.0, 0.0, 0.5, 0.0 );
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0f, 0.85f, 0.35f);
  glBegin(GL_TRIANGLES);
  glVertex3f(  0.0,  0.6, 0.0);
  glVertex3f( -0.2, -0.3, 0.0);
  glVertex3f(  0.2, -0.3 ,0.0);
  glEnd();
}
