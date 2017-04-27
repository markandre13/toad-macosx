/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2005 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figure.hh>
#include <toad/figureeditor.hh>
#include <toad/filedialog.hh>

using namespace toad;

TFImage::TFImage()
{
  x = y = 0;
}

TFImage::TFImage(const string &filename)
{
  x = y = 0;
  this->filename = filename;
  bitmap = new TBitmap();
  bitmap->load(filename);
}

void 
TFImage::paint(TPenBase &pen, EPaintType)
{
  if (bitmap) {
    pen.drawBitmap(x, y, bitmap);
  } else {
    pen.setFillColor(0.75, 0.75, 0.75);
    pen.setStrokeColor(0, 0, 0);
    pen.fillRectangle(x, y, 32, 32);
    pen.drawLine(x,y,x+32,y+32);
    pen.drawLine(x+32,y,x,y+32);
  }
}

bool
TFImage::startInPlace()
{
  TFileDialog dlg(NULL, "Open Image");
  dlg.doModalLoop();
  filename = dlg.getFilename();
  bitmap = new TBitmap();
  bitmap->load(filename);
  
  return false;
}

TRectangle
TFImage::bounds() const
{
  TRectangle r;
  r.origin.x = x;
  r.origin.y = y;
  if (bitmap) {
    r.size.width  = bitmap->getWidth();
    r.size.height = bitmap->getHeight();
  } else {
    r.size.width  = 32;
    r.size.height = 32;
  }
  return r;
}

bool
TFImage::editEvent(TFigureEditEvent &ee)
{
  switch(ee.type) {
    case TFigureEditEvent::TRANSLATE:
      x+=ee.x;
      y+=ee.y;
      return true;
    default:
      ;
  }
  return false;
}

void
TFImage::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "filename", filename);
  ::store(out, "x", x);
  ::store(out, "y", y);
}

bool
TFImage::restore(TInObjectStream &in)
{
  if (in.what == ATV_FINISHED) {
    bitmap = new TBitmap();
    if (!bitmap->load(filename)) {
      cout << "failed to load bitmap" << endl;
    }
  }
  if (
    ::restore(in, "filename", &filename) ||
    ::restore(in, "x", &x) ||
    ::restore(in, "y", &y) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}
