/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2014 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/pen.hh>
#include <toad/window.hh>
#include <toad/bitmap.hh>

using namespace toad;

void
TWindow::setCursor(TCursor::EType type)
{
  if (cursor) {
    delete cursor;
    cursor = 0;
  }
  if (type!=TCursor::DEFAULT)
    cursor = new TCursor(type);
}

void
TWindow::setCursor(const TCursor *c)
{
  if (c && c->cursor) {
    // FIXME: only if mouse is inside this window
    [c->cursor set];
  } else {
    [[NSCursor arrowCursor] set];
  }
}

TCursor::TCursor(TCursor::EType type)
{
  NSCursor *c = 0;
  switch(type) {
    case TCursor::DEFAULT:    break;
    case TCursor::CROSSHAIR:  c = [NSCursor crosshairCursor]; break;
    case TCursor::HAND:       c = [NSCursor pointingHandCursor]; break;
    case TCursor::TEXT:       c = [NSCursor IBeamCursor]; break;
    case TCursor::WAIT:       break; // displayed by Cocoa on it's own
    case TCursor::MOVE:       c = [NSCursor openHandCursor]; break;
    case TCursor::N_RESIZE:   c = [NSCursor resizeDownCursor]; break;
    case TCursor::NE_RESIZE:  break;
    case TCursor::NW_RESIZE:  break;
    case TCursor::S_RESIZE:   c = [NSCursor resizeUpCursor]; break;
    case TCursor::SE_RESIZE:  break;
    case TCursor::SW_RESIZE:  break;
    case TCursor::W_RESIZE:   c = [NSCursor resizeRightCursor]; break;
    case TCursor::E_RESIZE:   c = [NSCursor resizeLeftCursor]; break;
    case TCursor::QUESTION:   break;
    case TCursor::EXCHANGE:   break;
    case TCursor::PIRATE:     break;
    case TCursor::MOUSE:      break;
    case TCursor::PENCIL:     break;
    case TCursor::SPRAY:      break;
    case TCursor::HORIZONTAL: c = [NSCursor resizeLeftRightCursor]; break;
    case TCursor::VERTICAL:   c = [NSCursor resizeUpDownCursor]; break;
    case TCursor::TARGET:     break;
    case TCursor::DOT:        break;
    case TCursor::CIRCLE:     break;
    case TCursor::PARENT:     break;
  }
  cursor = c;
}

TCursor::TCursor(const char shape[32][32+1], unsigned ox, unsigned oy)
{
  TBitmap bmp(32, 32);
  for(int y=0; y<32; y++) {
    for(int x=0; x<32; x++) {
      switch(shape[y][x]) {
        case ' ': // opaque
          // bmp.setPixel(x, y, 1,0,0);
          break;
        case '.': // black
          bmp.setPixel(x, y, 0,0,0);
          break;
        default: // white
          bmp.setPixel(x, y, 1,1,1);
      }
    }
  }

  NSImage * image = [[NSImage alloc] initWithSize:[bmp.img size]];
  [image addRepresentation: bmp.img];
  cursor = [[NSCursor alloc] initWithImage: image hotSpot: NSMakePoint(ox,oy)];
}

TCursor::~TCursor()
{
  if (cursor)
    [cursor dealloc];
}
