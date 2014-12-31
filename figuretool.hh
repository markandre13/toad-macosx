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

#ifndef _TOAD_FIGURETOOL
#define _TOAD_FIGURETOOL 1

#include <toad/core.hh>
#include <toad/figure.hh>

namespace toad {

class TFigureEditor;
class TFigureAttributes;

class TFigureTool
{
  public:
    virtual ~TFigureTool();
    virtual void stop(TFigureEditor*);
    virtual void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
    virtual void keyEvent(TFigureEditor *fe, const TKeyEvent &ke);
    virtual void setAttributes(TFigureAttributes *p);
    virtual void paintSelection(TFigureEditor *fe, TPenBase &pen);
    virtual void modelChanged(TFigureEditor *fe);
    virtual TWindow* createEditor(TWindow *inWindow);
};

} // namespace toad

#endif
