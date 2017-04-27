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

#include <toad/scrollpane.hh>
#include <toad/scrollbar.hh>
#include <toad/region.hh>

#define DBM(X)

/**
 * \class toad::TScrollPane
 *
 * A window with two scrollbars by which the window contents can be scrolled.
 *
 * 'pane' is the area managed by TScrollPane
 * 'visible' is the visible part of the 'pane'
 *
 * When visible.x or visible.y are not 0, then the area left/above to 0 will
 * be scrolled also/still be scrolled. This is to be used for horizontal and
 * vertical headers.
 * 
 */

using namespace toad;

// constructor
//-------------------------------------
TScrollPane::TScrollPane(TWindow *p, const string &t):
  TWindow(p,t)
{
  vscroll = hscroll = 0;
  uix = uiy = 1;
  resetScrollPane();
}

void
TScrollPane::mouseEvent(const TMouseEvent &me)
{
  if (vscroll && (
        me.type == TMouseEvent::ROLL_UP ||
        me.type == TMouseEvent::ROLL_DOWN ))
  {
    vscroll->mouseEvent(me);
    return;
  }
}

void
TScrollPane::resetScrollPane()
{
  lx = ly = 0;
  if (vscroll)
    vscroll->setValue(0);
  if (hscroll)
    hscroll->setValue(0);
}

#if 1
void
TScrollPane::setOrigin(const TPoint &p)
{
  paneOrigin = p;
}

TPoint
TScrollPane::getOrigin() const
{
  return paneOrigin;
}
#else
void
TScrollPane::setOrigin(const TPoint &p)
{
  if (vscroll)
    vscroll->setRangeProperties(
      p.y,
      visible.h,
      p.y<0 ? p.y : 0,
      p.y>visible.h ? p.y : visible.h, true);
  if (hscroll)
    hscroll->setRangeProperties(
      p.x,
      visible.w,
      p.x<0 ? p.x : 0,
      p.x>visible.w ? p.x : visible.w, true);
}

TPoint
TScrollPane::getOrigin() const
{
  return TPoint(
    hscroll ? hscroll->getValue() : 0,
    vscroll ? vscroll->getValue() : 0);
}
#endif

/**
 * The method being invoked by the scrollbars
 */
void
TScrollPane::_scrolled()
{
  // calculate delta (dx, dy) between last position (lx, ly) and new one
  // and store a new last position in (lx, ly)
  TCoord dx, dy;
  dx = dy = 0;
  if (hscroll) {
    TCoord n = hscroll->getValue();
    dx = lx - n;
    lx = n;
  }
  if (vscroll) {
    TCoord n = vscroll->getValue();  
    dy = ly - n;
    ly = n;
  }
  // scroll screen
  scrollRectangle(visible, dx, dy, true);
  
  // IMPROVE: the two scroll rectangle commands could be merged into the
  // above scroll screen command
  if (visible.origin.x) {
    TRectangle r(0,visible.origin.y,visible.origin.x,visible.size.height);
    scrollRectangle(r, 0, dy, true);
  }
  
  if (visible.origin.y) {
    TRectangle r(visible.origin.x,0,visible.size.width,visible.origin.y);
    scrollRectangle(r, dx, 0, true);
  } 

//#warning "invalidating the whole window to avoid off-by-one error in scrollRectangle"
//invalidateWindow();

  scrolled(dx, dy);
}

/**
 * This virtual method is called when one of the scrollbars was moved.
 *
 * The method getPanePos will deliver the absolute position, the two
 * parameters will deliver the relative movement in relation to the
 * previous position.
 *
 * The default action is to set the windows origin to the new pane position.
 *
 * \param dx
 *   delta movement on x-axis
 * \param dy
 *   delta movement on y-axis
 */
void
TScrollPane::scrolled(TCoord dx, TCoord dy)
{
  TCoord x, y;
  getPanePos(&x, &y);
  setOrigin(TPoint(-x, -y));
}

// avoid calling resize() when no window exists
void
TScrollPane::adjust()
{
}

void
TScrollPane::created()
{
  if (isRealized())
    doLayout();
  else
    cerr << "TScrollPane::created: not realized" << endl;
}

void
TScrollPane::resize()
{
  if (isRealized())
    doLayout();
  else
    cerr << "TScrollPane::resize: not realized" << endl;
}

/**
 * Configure the scrollbars according the window and pane size.
 */
void
TScrollPane::doLayout()
{
  visible.set(0,0,getWidth(), getHeight());
  adjustPane(); // adjust 'visible'

  bool need_hscroll = false;
  bool need_vscroll = false;

  if (pane.size.width > visible.size.width || pane.origin.x < 0) {
    need_hscroll = true;  
    visible.size.height -= TScrollBar::getFixedSize();
  }
   
  if (pane.size.height > visible.size.height || pane.origin.y < 0) {
    need_vscroll = true;  
    visible.size.width -= TScrollBar::getFixedSize();
  }
   
  if (!need_hscroll && pane.size.width > visible.size.width) {
    need_hscroll = true;
    visible.size.height -= TScrollBar::getFixedSize();
  }
/*  
cerr << "TScrollPane("<<getTitle()<<"::doLayout:" << endl
     << "visible  = "<<visible<<endl
     << "pane     = "<<pane<<endl
     << "need h,v = "<<need_hscroll<<", "<<need_vscroll<<endl
     << "vscroll = " << vscroll << endl;
*/
  if (need_vscroll) {
    if (!vscroll) {  
      vscroll = new TScrollBar(this, "vertical");
      connect(vscroll->getModel()->sigChanged, this, &TScrollPane::_scrolled);
      vscroll->createWindow();
    }
    vscroll->flagNoFocus=true;
    vscroll->setShape(
      visible.origin.x+visible.size.width,
      visible.origin.y,
      TScrollBar::getFixedSize(),
      visible.size.height);
    vscroll->setExtent(visible.size.height);
    vscroll->setMinimum(pane.origin.y);
    if (pane.origin.y+pane.size.height < visible.origin.y+visible.size.height)
      vscroll->setMaximum(visible.origin.y+visible.size.height);
    else    
      vscroll->setMaximum(pane.origin.y+pane.size.height);
    vscroll->setMapped(true);  
    vscroll->setUnitIncrement(uiy);
  } else {
    if (vscroll) {
      vscroll->setMapped(false);
      vscroll->setValue(0);
    }
  }  
     
  if (need_hscroll) {
    if (!hscroll) {  
      hscroll = new TScrollBar(this, "horizontal");
      connect(hscroll->getModel()->sigChanged, this, &TScrollPane::_scrolled);
      hscroll->createWindow();
    }
    hscroll->flagNoFocus=true;
    hscroll->setShape(
      visible.origin.x,
      visible.origin.y+visible.size.height,
      visible.size.width,
      TScrollBar::getFixedSize());
    hscroll->setExtent(visible.size.width);
    hscroll->setMinimum(pane.origin.x);
    if (pane.origin.x+pane.size.width < visible.origin.x+visible.size.width)
      hscroll->setMaximum(visible.origin.x+visible.size.width);
    else    
      hscroll->setMaximum(pane.origin.x+pane.size.width);
    hscroll->setMapped(true);  
    hscroll->setUnitIncrement(uix);
  } else {
    if (hscroll) {
      hscroll->setMapped(false);
      hscroll->setValue(0);
    }
  }  
}

/**
 * draw the litle gray box between the two scrollbars (when we have
 * two of 'em)
 */
void
TScrollPane::paintCorner(TPenBase &pen)
{
#if 0 
  if (hscroll && vscroll &&
      hscroll->isMapped() &&
      vscroll->isMapped())
  {
    TRectangle r(getWidth() - TScrollBar::getFixedSize(),
                 getHeight()- TScrollBar::getFixedSize(),
                 TScrollBar::getFixedSize(), TScrollBar::getFixedSize());
    pen.setColor(TColor::LIGHTGRAY);
    pen.identity();
    pen|=r;
    pen.fillRectanglePC(r);
  }
#else
  pen.setColor(TColor::DIALOG);
  pen.identity();
  TRectangle r(0,0,getWidth(),getHeight());
  if (visible.origin.y>0) {
    if (visible.origin.x>0) 
      pen.fillRectanglePC(0, 0, visible.origin.x, visible.origin.y);
    if (visible.origin.x+visible.size.width < getWidth())
      pen.fillRectanglePC(visible.origin.x+visible.size.width, 0, getWidth()-visible.origin.x-visible.size.width, visible.origin.y);
  }
  if (visible.origin.y+visible.size.height < getHeight()) {
    if (visible.origin.x>0) 
      pen.fillRectanglePC(0, visible.origin.y+visible.size.height, visible.origin.x, getHeight()-visible.origin.y-visible.size.height);
    if (visible.origin.x+visible.size.width < getWidth())
      pen.fillRectanglePC(visible.origin.x+visible.size.width, visible.origin.y+visible.size.height, getWidth()-visible.origin.x-visible.size.width, getHeight()-visible.origin.y-visible.size.height);
  }
  
#endif
}

/** 
 *
 * IMPROVE: we have pointer here! drop the 'setall' flag
 */
void
TScrollPane::getPanePos(TCoord *x, TCoord *y, bool setall) const {
  if (setall) {
    *x = *y = 0;
  }
  if (hscroll) *x = hscroll->getValue();
  if (vscroll) *y = vscroll->getValue();
}

void
TScrollPane::setPanePos(TCoord x, TCoord y) {
  if (hscroll) hscroll->setValue(x);
  if (vscroll) vscroll->setValue(y);
}

void
TScrollPane::setUnitIncrement(TCoord uix, TCoord uiy)
{
  this->uix = uix;
  this->uiy = uiy;
  
  if (hscroll)
    hscroll->setUnitIncrement(uix);
  if (vscroll)
    vscroll->setUnitIncrement(uiy);
}

void
TScrollPane::pageUp()
{
  if (vscroll) {
    vscroll->pageUp();
  }
}

void
TScrollPane::pageDown()
{
  if (vscroll) {
    vscroll->pageDown();
  }
}
