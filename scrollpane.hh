/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2007 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_SCROLLPANE_HH
#define _TOAD_SCROLLPANE_HH 1

#include <toad/window.hh>
#include <toad/penbase.hh>

namespace toad {

class TScrollBar;

class TScrollPane:
  public TWindow
{
    TPoint paneOrigin;
  public:
    TScrollPane(TWindow *p, const string &t);
    const TRectangle& getVisible() const { return visible; }
    void mouseEvent(const TMouseEvent &me);

    void scrollTo(TCoord x, TCoord y);

    virtual void setOrigin(const TPoint&);
    virtual TPoint getOrigin() const;
    
  protected:
    //! the visible part of the pane (better: pane?)
    TRectangle visible;

    //! pane location and size in pixels (better: area, world?)
    TRectangle pane;
    
    void resetScrollPane();
    
    virtual void scrolled(TCoord x, TCoord y);
    
    void adjust();
    void created();
    void resize();
    void doLayout();
    virtual void adjustPane() = 0;
    
    void getPanePos(TCoord *x, TCoord *y, bool setall=true) const;
    void setPanePos(TCoord x, TCoord y);
    void setUnitIncrement(TCoord uix, TCoord uiy);
    void paintCorner(TPenBase&);
    
    void pageUp();
    void pageDown();

  private:
    //! unit increment as set by setUnitIncrement
    TCoord uix, uiy;
  
    //! last scrollbar position (so we know how much to scroll)
    TCoord lx, ly;

    TScrollBar *vscroll, *hscroll;

    void _scrolled();
};

} // namespace toad

#endif
