/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2003 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/layout.hh>
#include <toad/io/urlstream.hh>

/**
 * \class toad::TLayout
 *
 * TLayout is the base class for layout managers, which can be plugged
 * into all TWindow based objects.
 *
 * \sa toad::TDialogLayout, toad::TTabbedLayout, toad::TFormLayout
 * \sa toad::TWindow::loadLayout, toad::TWindow::setLayout.
 *
 * \todo
 *   don't destroy the control window when the dialog editor is just
 *   deactivated. the generic text layout editor is loosing all its data
 *   at this moment
 */

using namespace toad;

TLayout::TLayout()
{
  window = NULL;
  modified = false;
}

TLayout::TLayout(const TLayout &)
{
  window = NULL;
  modified = false;
}

void
TLayout::toFile()
{
  if (isModified() && filename.size()!=0) {
    try {
      cerr << "storing dialog layout in '" << filename << "'" << endl;
      ourlstream out(filename.c_str());
      out << "// this is a generated file" << endl;
      TOutObjectStream oout(&out);
      oout.store(this);
      setModified(false);
    }
    catch(exception &e) {
      cerr << "TLayout::toFile: exception while storing dialog layout: " << e.what() << endl;
    }
  }
}

/**
 * Arrange the children of window 'where' according to the
 * layout.
 */
void
TLayout::arrange()
{
}

/**
 * Some layouts may contain additional graphic besides the
 * layout of the children.
 */
void
TLayout::paint()
{
}

bool
TLayout::restore(TInObjectStream &in)
{
  return super::restore(in);
}

/**
 * Create a new layout editor control for this layout.
 *
 * The layout editor returned by TLayout is a simple text editor to
 * manipulate the serialized layout. Other classes may provide graphical
 * editors, for example like TDialogLayout.
 *
 * \param inWindow   Parent for the new window.
 * \param forWindow  Window to be edited.
 * \return           A layout editor.
 */
TLayoutEditor *
TLayout::createEditor(TWindow *inWindow, TWindow *forWindow)
{
  return 0;
//  return new TLayoutEditGeneric(inWindow, "TLayout.editor", this, forWindow);
}
