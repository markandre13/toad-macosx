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

#include <toad/pen.hh>
#include <toad/window.hh>
#include <toad/radiobutton.hh>

using namespace toad;

/**
 * @ingroup control
 * \class toad::TRadioButton
 * 
 * See <code>TRadioStateModel</code> for code examples.
 * \sa toad::TRadioState
 */

TRadioButton::TRadioButton(TWindow *parent, 
                           const string &title, 
                           TRadioStateModel *state)
  :super(parent, title, state)
{
  setBackground(TColor::DIALOG);
}
           
void
TRadioButton::paint()
{
  TPen pen(this);
  // draw text
  int y = ( (getHeight()-pen.getHeight()) / 2 );

  pen.setColor(TColor::BTNSHADOW);
  pen.fillCirclePC(0,0,11,11);
  pen.setColor(TColor::BTNLIGHT);
  pen.fillCirclePC(1,1,12,12);
  pen.setColor(TColor::BTNSHADOW);
  pen.drawCirclePC(0,0,11,11);
  pen.drawCirclePC(1,1,10,10);
  pen.setColor(  0,  0,  0);
  pen.drawArcPC(1,1,11,11, 90, 180);
  if (!isEnabled()) {
    pen.setColor(TColor::DIALOG);
    pen.fillCircle(2,2, 8,8);
    pen.setColor(0,0,0);
  }
  if (isDown())
    pen.fillCircle(3,3, 6,6);

  if (isEnabled()) {    
    pen.drawString(20,y, getLabel());
  } else {
    pen.setColor(TColor::BTNLIGHT);
    pen.drawString(20+1,y+1, getLabel());
    pen.setColor(TColor::BTNSHADOW);
    pen.drawString(20,y, getLabel());   
  }
  
  if (isFocus()) {
    pen.setLineStyle(TPen::DOT);
    pen.drawRectanglePC(18,0,getWidth()-18,getHeight());
  }
}
