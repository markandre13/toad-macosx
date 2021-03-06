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
#include <toad/labelowner.hh>

// avoid problems on AIX, IRIX, ...
#define exception c_exception
#include <cmath>
#undef exception

using namespace toad;

TFWindow::TFWindow()
{
  window = NULL;
  taborder = 0;
  filled = true; // for super::distance()
}

void 
TFWindow::paint(TPenBase&, EPaintType)
{
}

TCoord
TFWindow::distance(const TPoint &pos)
{
  return super::distance(pos);
}

bool
TFWindow::transform(const TMatrix2D &transform)
{
  if (!transform.isOnlyTranslateAndScale())
    return true; // ignore illegal transformation
  super::transform(transform);
  if (!window) {
    cerr << "toad warning: TFWindow.window==nullptr : " << title << endl;
    return true;
  }
  window->setShape(bounds());
  return true;
}

void
TFWindow::translateHandle(unsigned handle, TCoord mx, TCoord my, unsigned m)
{
  if (!window) {
    cerr << "toad warning: TFWindow.window==nullptr : " << title << endl;
    return;
  }
  super::translateHandle(handle, mx, my, m);
  window->setShape(bounds());
}

void 
TFWindow::store(TOutObjectStream &out) const
{
  if (!window)
    cerr << "toad warning: TFWindow.window==nullptr : " << title << endl;
    
  TRectangle r(p1,p2);
  ::store(out, "x", r.origin.x);
  ::store(out, "y", r.origin.y);
  ::store(out, "w", r.size.width);
  ::store(out, "h", r.size.height);
  ::store(out, "title", title);

  if (window) {
    TLabelOwner *lo = dynamic_cast<TLabelOwner*>(window);
    if (lo) {
      ::store(out, "label", lo->_label);
    }
  }
  
  ::store(out, "taborder", taborder);
}

/**
 * Restore TFWindow from stream.
 *
 * The data fetched during restore will be copied by `arrangeHelper(...)'
 * in `dialog.cc' to the window.
 */
bool
TFWindow::restore(TInObjectStream &in)
{
  static TRectangle r;
  
  if (::finished(in)) {
    setShape(r);
    return true;
  }
  
  if (
    ::restore(in, "x", &r.origin.x) ||
    ::restore(in, "y", &r.origin.y) ||
    ::restore(in, "w", &r.size.width) ||
    ::restore(in, "h", &r.size.height) ||
    ::restore(in, "title", &title) ||
    ::restore(in, "label", &label) ||
    ::restore(in, "taborder", &taborder) ||
    super::restore(in)
  ) {
    filled = true; // override super::restore
    return true;
  }
  ATV_FAILED(in)
  return false;
}
