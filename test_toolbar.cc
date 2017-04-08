/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 2015-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/springlayout.hh>
#include <toad/menubar.hh>
#include <toad/action.hh>
#include <toad/command.hh>

#include <toad/pushbutton.hh>

#include <cassert>

using namespace toad;

class TTestToolbar:
  public TWindow
{
  public:
    TTestToolbar(TWindow *parent, const string &title);
};

TTestToolbar::TTestToolbar(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  setShape(640,1000,320,200);
  
  TMenuBar *menu = new TMenuBar(this, "menubar");
  menu->loadLayout("test_toolbar_menu.atv");
  
  TSpringLayout *layout = new TSpringLayout;
  layout->attach("menubar", TSpringLayout::TOP | TSpringLayout::RIGHT | TSpringLayout::BOTTOM);

  TAction *action;  
  action = new TAction(this, "file|quit");
  connect(action->sigClicked, [&] {
    sendMessageDeleteWindow(this);
  });
  
  // action = new TAction(this, "view|grid");
  // action->type = TAction::CHECKBUTTON;

  GChoice<int> *m = new GChoice<int>(this, "tool|toolbox");
  m->add("select", 1);
  m->add("zoom",   2);
  connect(m->sigClicked, [=] {
    cout << "radio button " << m->getValue() << endl;
  });

  setLayout(layout);
}

// FIXME: this should be TFatRadioButton, but TRadioStateModel is not compatible with TAbstractChoice
class TToolButton:
  public TButtonBase
{
    typedef TButtonBase super;
  public:
    TToolButton(TWindow*, 
                const string &,
                TAbstractChoice *choice,
                size_t index);
  protected:
    TAbstractChoice *choice;
    size_t index;
    void paint();
};

TToolButton::TToolButton(TWindow *parent, const string &title, TAbstractChoice *aChoice, size_t anIndex)
  :TButtonBase(parent, title), choice(aChoice), index(anIndex)
{
  connect(choice->getModel()->sigChanged, [=] {
    this->invalidateWindow();
  });
}

void
TToolButton::paint()
{
  TPen pen(this);

  TChoiceModel *model = choice->getModel();
  bool down = isDown() || model->getSelection()==index;
  
//  cout << "this: '" << getTitle() << ", model:" << model->getSelection() << endl;
  
  drawLabel(pen, getLabel(), down);
  drawShadow(pen, down);
}

class TToolBar:
  public TWindow
{
  public:
    TToolBar(TWindow *parent, const string &title);
  protected:
    void actionsChanged();
    void addAction(const string &title, TAction*);
    void addChoice(const string &title, TAbstractChoice *, size_t index);
};

TToolBar::TToolBar(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  connect(TAction::actions.sigChanged, [=] {
    this->actionsChanged();
  });
  setShape(640, 900, 320, 32);
  
  actionsChanged();
}

void
TToolBar::actionsChanged()
{
  for(TActionStorage::iterator i = TAction::actions.begin();
      i != TAction::actions.end();
      ++i)
  {
    TAction *action = *i;
    switch(action->type) {
      case TAction::BUTTON:
        addAction(action->getTitle(), action);
        break;
      case TAction::CHECKBUTTON:
        break;
      case TAction::RADIOBUTTON:
        TAbstractChoice *choice = dynamic_cast<TAbstractChoice*>(action);
        if (!choice)
          break;
        for(size_t i=0; i<choice->getSize(); ++i) {
          addChoice(choice->getTitle() + '|' + choice->getID(i), choice, i);
        }
        break;
    }
  }
}

void
TToolBar::addAction(const string &title, TAction*)
{
}

void
TToolBar::addChoice(const string &title, TAbstractChoice *choice, size_t index)
{
  TCoord x = 0;
  for(TInteractor *child = getFirstChild(); child; child=child->getNextSibling()) {
    if (child->getTitle() == title)
      return;
    x += 32;
  }

  auto&& button = new TToolButton(this, title, choice, index);
  if (title=="tool|toolbox|select")
    button->loadBitmap("resource/tool_select.png");
  else
  if (title=="tool|toolbox|zoom") {
    button->loadBitmap("resource/tool_pick.png");
  }
  button->setShape(x,0,32,32);
  connect(button->sigClicked, [=] {
    choice->trigger(index);
  });
}

int 
test_toolbar()
{
  new TToolBar(nullptr, "TToolBar");
  new TTestToolbar(nullptr, "TTestToolbar");
  toad::mainLoop();
  return 0;
}
