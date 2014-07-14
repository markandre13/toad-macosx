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

#include <toad/core.hh>

#include <toad/buttonbase.hh>

using namespace toad;

/**
 * \\class toad::TButtonBase
 *
 * The base class for TPushButton and TRadioButtonBase.
 */

TButtonBase::TButtonBase(TWindow *parent, const string &title)
  :super(parent,title)
{
  bitmap = NULL;
  bDown=false;
  bInside=false;
  setBorder(false);
  setBackground(TColor::BTNFACE);
  setSize(100, getDefaultFont().getHeight() + 8);
//  setSize(100, 22);
}

TButtonBase::~TButtonBase()
{
  if (bitmap)
    delete bitmap;
}

/**
 * `true' will move the button down and trigger `sigArm', `false' will release
 * the button and trigger `sigDisarm'.
 */
void TButtonBase::setDown(bool down)
{
  if (down==bDown) return;
  bDown=down;
  invalidateWindow();
  if (down) {
    sigArm();
  } else {
    sigDisarm();
//    sigClicked();
  }
}

/**
 * TButtonBase objects can display either a text label or a bitmap. When
 * this method succeeds the bitmap is used and `true' is returned.
 */
bool TButtonBase::loadBitmap(const string& name)
{
  if (isRealized())
    invalidateWindow();
  if (!bitmap)
    bitmap = new TBitmap;
  try {
    bitmap->load(name.c_str());
  }
  catch(...) {
    delete bitmap;
    bitmap = NULL;
    return false;
  }
  return true;
}

/**
 * Draws the labels text when no bitmap was loaded or the bitmap.
 */
void 
TButtonBase::drawLabel(TPen &pen,const string &text, bool bDown, bool bEnabled)
{
//printf("%s\n", __FUNCTION__);
  int n=bDown?1:0;

  if (bitmap) {
    pen.drawBitmap(
      (getWidth()-bitmap->width) / 2 + n,
      (getHeight()-bitmap->height) / 2 + n,
      *bitmap);
    return;
  }

  int x = (getWidth()-pen.getTextWidth(text)) / 2;
  int y = (getHeight()-pen.getHeight()) / 2;
  if(isEnabled() && bEnabled) {
    pen.setColor(TColor::BTNTEXT);
    pen.drawString(x+n, y+n, text);
    if (isFocus()) {
      pen.setLineStyle(TPen::DOT);
      pen.drawRectanglePC(x+n-3,y+n-1,pen.getTextWidth(text)+6,pen.getHeight()+1);
      pen.setLineStyle(TPen::SOLID);
    }
  } else {
    pen.setColor(TColor::BTNLIGHT);
    pen.drawString(x+1,y+1,text);
    pen.setColor(TColor::BTNSHADOW);
    pen.drawString(x,y,text);   
  }

}
  
/**
 * Draws the buttons shadow.
 */
void
TButtonBase::drawShadow(TPen &pen, bool down, bool onwhite)
{
//printf("%s\n", __FUNCTION__);
  int a=isFocus()?1:0;

  TPoint p[6];

  if (down) {
    pen.setColor(TColor::BTNSHADOW);
    pen.drawRectanglePC(a,a,getWidth()-a*2, getHeight()-a*2);
  } else {
    if (!onwhite) {
      pen.setColor(0,0,0);
      p[0].set(a             , getHeight()-a-1);
      p[1].set(getWidth()-a-1, getHeight()-a-1);
      p[2].set(getWidth()-a-1, a);
      pen.drawLines(p, 3);
      pen.setColor(TColor::BTNSHADOW);
      p[0].set(a+2           , getHeight()-a-2);
      p[1].set(getWidth()-a-2, getHeight()-a-2);
      p[2].set(getWidth()-a-2, a+1);
      pen.drawLines(p, 3);
    } else {
      pen.setColor(TColor::BTNSHADOW);
      p[0].set(a,getHeight()-a-1);
      p[1].set(getWidth()-a-1, getHeight()-a-1  );
      p[2].set(getWidth()-a-1,  a           );
      p[3].set(getWidth()-a-2, a+1        );
      p[4].set(getWidth()-a-2, getHeight()-a-2  );
      p[5].set(a+2        , getHeight()-a-2 );
      pen.drawLines(p, 6);
    }

    if (!onwhite) {
      pen.setColor(TColor::BTNLIGHT);
    } else {
      pen.setColor(TColor::BTNFACE);
    }
    p[0].set(a          , getHeight()-a-1 );
    p[1].set(a          , a           );
    p[2].set(getWidth()-a-2,  a           );
    pen.drawLines(p, 3);
    
    if (onwhite) {
      pen.setColor(TColor::BTNLIGHT);
      p[0].set(getWidth()-a-2, a+1          );
      p[1].set(a+1        , a+1         );
      p[2].set(a+1        , getHeight()-a-2 );
      pen.drawLines(p, 3);
    }
  }

  if (isFocus()) {
    pen.setColor(0,0,0);
    pen.drawRectanglePC(0,0, getWidth(), getHeight());
  }
}

void
TButtonBase::mouseLDown(const TMouseEvent &)
{
//cout << getTitle() << " " << __PRETTY_FUNCTION__ << endl;
  if (!isEnabled() || !sigClicked.isConnected())
    return;
  bDown=true;
  if (isFocus()) {
    invalidateWindow();
  } else {
    setFocus();
//    if (!setFocus())  // 'setFocus' calls 'focus', which already updates the window
      invalidateWindow(true);
  }
//printf("%s\n", __FUNCTION__);
  sigArm();
}

void
TButtonBase::mouseLUp(const TMouseEvent &)
{
//cout << getTitle() << " " << __PRETTY_FUNCTION__ << endl;
//printf("%s\n", __FUNCTION__);
  if (bDown) {
    bDown=false;
//cout << "************** inside " << getTitle() << " " << bInside << endl;
    if (bInside) {
      invalidateWindow(true);
      sigDisarm();
      sigClicked();
    }
  }
}

void
TButtonBase::keyDown(TKey key, char* str, unsigned modifier)
{
  if (!isEnabled() || !sigClicked.isConnected())
    return;

  if (!bDown && modifier==0 && (key==TK_RETURN || *str==' ')) {
    sigArm();
    sigDisarm();
    sigClicked();
  }
}

void
TButtonBase::mouseEnter(const TMouseEvent &)
{
//cout << getTitle() << " " << __PRETTY_FUNCTION__ << endl;
  bInside = true;
  if(bDown) {
    invalidateWindow();
    sigArm();
  }
}

void
TButtonBase::mouseLeave(const TMouseEvent &)
{
//cout << getTitle() << " " << __PRETTY_FUNCTION__ << endl;
  bInside = false;
  if(bDown) {
    invalidateWindow();
    sigDisarm();
  }
}
