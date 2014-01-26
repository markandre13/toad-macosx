/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/combobox.hh>
#include <toad/buttonbase.hh>
#include <toad/scrollbar.hh>

using namespace toad;

class TComboBox::TComboButton:
  public TButtonBase
{
  public:
    TComboButton(TWindow *parent, const string &title)
      :TButtonBase(parent,title)
    {
      setBorder(false);
    };

  protected:
    void paint();
    void mouseLDown(const TMouseEvent &);
    void mouseLUp(const TMouseEvent &);
    void mouseEnter(const TMouseEvent &);
    void mouseLeave(const TMouseEvent &);
};   

TComboBox::TComboBox(TWindow * parent, const string &title):
  super(parent, title)
{
  setSize(320, 20);
  setBorder(false);

  btn = new TComboButton(this,"combobox.combobutton");
  connect(btn->sigArm, this, &TComboBox::button);
  connect(btn->sigDisarm, this, &TComboBox::button);
  
  table = new TTable(this, "table");
  table->selectionFollowsMouse = true;
  connect(table->sigClicked, this, &TComboBox::selected);
  connect(table->sigDoubleClicked, this, &TComboBox::selected);

  table->flagExplicitCreate = true;
  table->flagPopup = true;
  table->flagSaveUnder = true;
  // table->_faked_focus = true;
}

void
TComboBox::paint()
{
cout << "TComboBox::paint" << endl;
  TPen pen(this);

  pen.draw3DRectanglePC(0,0, getWidth(), getHeight());
  if (!table->getAdapter()) {
    pen.setColor(TColor::DIALOG);
    pen.fillRectanglePC(2,2, getWidth()-4, getHeight()-4);
cout << "  no table adapter" << endl;
    return;
  }
#if 1
  // checks which should already be handled in TTable...
  size_t col=0, row=0;
  TAbstractSelectionModel *sm = table->getSelectionModel();
cout << "  selection model = " << sm << endl;
if (sm && sm->empty())
  cout << "  selection model is empty" << endl;
  if (sm) {
    sm->getFirst(&col, &row);
  }
  if (row >= table->getAdapter()->getRows()) {
    cerr << "TComboBox '"<<getTitle()<<"': tables selection model was out of renderer range" << endl;
    pen.setColor(128,64,64);
    pen.fillRectanglePC(2,2, getWidth()-4, getHeight()-4);
    return;
  }
#endif
  pen.translate(2, 2);
  if (!sm || !sm->empty()) {
    TTableEvent te;
    te.type = TTableEvent::PAINT;
    te.pen = &pen;
    te.col = col;
    te.row = row;
    te.even= false;
    te.w = btn->getXPos()-2;
    te.h = getHeight()-4;
    te.cursor = false;
    te.selected = false;
    te.focus = true;
cout << "  call adapter for paint" << endl;
    table->getAdapter()->tableEvent(te);
  }

  if (isFocus()) {
    pen.setColor(0,0,0);
    pen.setLineStyle(TPen::SOLID);
    pen.setLineWidth(1);
    pen.fillRectanglePC(0,0, getWidth()-4-btn->getWidth(), getHeight()-4);
  }
}

void
TComboBox::resize()
{
cout << "TComboBox '"<<getTitle()<<"': resize " << w << ", " << h << endl;
  btn->setShape(
    getWidth()-TScrollBar::getFixedSize()+1 -2, 2,
    TScrollBar::getFixedSize(), getHeight()-4
  );
}

void
TComboBox::focus(bool b)
{
  // in case we're loosing the focus, close the combobox
/*
  if (b==false && table->isMapped()) {
    btn->setDown(false);
  }
  invalidateWindow();
*/
}

void
TComboBox::button()
{
cout << "TComboBox " << getTitle() << " button isDown=" << (btn->isDown()?"true":"false") << endl;
//return;

  if (btn->isDown()) {
cout << "*** TComboBox::button: is down" << endl;
    if (!isFocus())
      setFocus();
    table->setSize(getWidth(), (getDefaultFont().getHeight()+1)*8);
    table->placeWindow(PLACE_PULLDOWN, this);
    table->setMapped(true);
    if (table->isRealized()) {
cout << "raise combobox popup" << endl;
      table->raiseWindow();
    } else {
cout << "create combobox popup" << endl;
      table->createWindow();
    }
    table->setFocus();
    grabPopupMouse();
  } else {
cout << "*** TComboBox::button: is up" << endl;
    table->setMapped(false);
    ungrabMouse();
  }
}

void
TComboBox::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      btn->setDown(!btn->isDown());
      setFocus();
      break;
    case TMouseEvent::ROLL_UP:
      table->keyDown(TK_UP, const_cast<char*>(""), 0);
      table->keyUp(TK_UP, const_cast<char*>(""), 0);
      invalidateWindow();
      sigSelection();
      break;
    case TMouseEvent::ROLL_DOWN:
      table->keyDown(TK_DOWN, const_cast<char*>(""), 0);
      table->keyUp(TK_DOWN, const_cast<char*>(""), 0);
      invalidateWindow();
      sigSelection();
      break;
  }
}

void
TComboBox::keyDown(TKey key, char *string, unsigned modifier)
{
  // in case the combobox is open and the table popup window
  // is mapped (visible), delegate all keyboard events to the
  // table
  if (table->isMapped()) {
    // but not the SHIFT keys, as table interprets 'em as
    // select.. which I don't think is a good idea in respect
    // to the backward focus traversal using shift+tab keys...
    if (key==TK_SHIFT_L || key==TK_SHIFT_R)
      return;
    if (key==TK_ESCAPE) {
      btn->setDown(false);
      return;
    }
    table->keyDown(key, string, modifier);
    return;
  }

  switch(key) {
    case TK_PAGEDOWN:
    case TK_SPACE:
      btn->setDown(true);
      break;
    case TK_DOWN:
    case TK_UP:
      table->keyDown(key, string, modifier);
      table->keyDown(TK_RETURN, string, modifier);
      // table->selectAtCursor();
      invalidateWindow();
      break;
  }
}

void
TComboBox::closeRequest()
{
  btn->setDown(false);
}

void
TComboBox::selected()
{
  table->selectAtCursor();
  btn->setDown(false);
  invalidateWindow();
  sigSelection();
}

void
TComboBox::_rendererChanged()
{
//  cerr << "TComboBox '"<<getTitle()<<"': the tables renderer triggered sigChanged" << endl;
  invalidateWindow();
}

void
TComboBox::_selectionChanged()
{
//  cerr << "TComboBox '"<<getTitle()<<"': the tables selection model triggered sigChanged()" << endl;
  invalidateWindow();
}

//----------------

void
TComboBox::TComboButton::paint()
{
  TPen pen(this);
  drawShadow(pen, bDown, true);

  int n=bDown?1:0;
  int cx = (getWidth() >> 1)+n;
  int cy = (getHeight() >> 1)+n - 1;

  TPoint p[3];
  p[0].set(cx-4,cy-2);
  p[1].set(cx+4,cy-2);
  p[2].set(cx ,  cy+3);

  pen.setColor(TColor::BTNTEXT);
  pen.fillPolygon(p,3);
  
  pen.fillRectanglePC(cx-4, cy+4, 9, 3);
}

void
TComboBox::TComboButton::mouseLDown(const TMouseEvent &)
{
  setDown(!isDown());
}

void
TComboBox::TComboButton::mouseLUp(const TMouseEvent &)
{
}

void
TComboBox::TComboButton::mouseEnter(const TMouseEvent &)
{
}

void
TComboBox::TComboButton::mouseLeave(const TMouseEvent &)
{
}
