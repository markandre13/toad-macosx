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

#include <toad/focusmanager.hh>

#include <toad/figure/createtool.hh>
#include <toad/figure/selectiontool.hh>
#include <toad/figure/directselectiontool.hh>
#include <toad/figure/texttool.hh>

#include <cassert>
#include <set>

using namespace toad;

TToolBox*
TToolBox::getToolBox() {
  static TToolBox *tool = nullptr;
  if (!tool) tool = new TToolBox();
  return tool;
}

TToolBox::TToolBox()
{
  activePointer = 0;
/*
*/  
  // FIXME: this would be nicer with closures
  class TMyEventFilter: public TEventFilter {
      TToolBox *toolbox;
    public:
      TMyEventFilter(TToolBox *aToolBox):toolbox(aToolBox) {}
    protected:
      bool mouseEvent(TMouseEvent &me) override {
        if (me.type==TMouseEvent::TABLET_PROXIMITY) {
          toolbox->selectPointer(me.pointerID());
        }
        return false;
      }
  };
  TFocusManager::insertEventFilter(new TMyEventFilter(this), NULL, KF_GLOBAL);
}

void
TToolBox::selectPointer(TMouseEvent::TPointerID pointerID)
{
  if (activePointer == pointerID)
    return;

  // new pointing device, save old one
  toolForPointer[activePointer] = getValue();

  // switch to the previously known tool for the new pointing device
  auto &&knownTool = toolForPointer.find(pointerID);
  if (knownTool != toolForPointer.end()) {
    setValue(knownTool->second);
  }
  
  activePointer = pointerID;
}


class TTestToolbar:
  public TWindow
{
    unique_ptr<GChoice<TFigureTool*>> choice;
  public:
    TTestToolbar(TWindow *parent, const string &title);
};

TTestToolbar::TTestToolbar(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  setShape(640,1000,320,200);
  
  // create children
  TMenuBar *menu = new TMenuBar(this, "menubar");
  menu->loadLayout("test_toolbar_menu.atv");

  TFigureEditor *editor = new TFigureEditor(this, "editor");
//  editor->scale(0.25, 0.25);

  // this way, we would have to remove the editor from the toolbox again:
  //
  // TToolBox::getToolBox()->addEditor(editor);
  //
  // this way, we can give a fuck about cleaning up resources and calling
  // setTool in all connected figure editors:
  //
  editor->setToolBox(TToolBox::getToolBox());

  // create actions
  TAction *action;  
  action = new TAction(this, "file|quit");
  connect(action->sigClicked, [&] {
    sendMessageDeleteWindow(this);
  });
  
  // action = new TAction(this, "view|grid");
  // action->type = TAction::CHECKBUTTON;

  choice = make_unique<GChoice<TFigureTool*>>(this, "tool|toolbox", TToolBox::getToolBox());
  
  // layout children
  TSpringLayout *layout = new TSpringLayout;
  layout->attach("menubar", TSpringLayout::TOP | TSpringLayout::LEFT | TSpringLayout::RIGHT);
  layout->attach("editor", TSpringLayout::LEFT | TSpringLayout::RIGHT | TSpringLayout::BOTTOM);
  layout->attach("editor", TSpringLayout::TOP, "menubar");
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
                TChoiceModel *choice,
                size_t index);
    // TAbstractChoice *choice;
    TChoiceModel *choice;
  protected:
    size_t index;
    void paint() override;
};

TToolButton::TToolButton(TWindow *parent, const string &title, TChoiceModel *aChoice, size_t anIndex)
  :TButtonBase(parent, title), choice(aChoice), index(anIndex)
{
  connect(choice->sigChanged, this, [=] {
    this->invalidateWindow();
  });
/*
  // FIXME: an alternative to TSlot?
  disconnect(choice->getModel()->sigChanged, this, [=] {
    this.choices.remove(choice);
    this->choice = nullptr;
  });
*/
}

void
TToolButton::paint()
{
  TPen pen(this);

  TChoiceModel *model = choice;
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
    void addChoice(const string &title, TChoiceModel*, size_t index);
};

TToolBar::TToolBar(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  flagParentlessAssistant = true;

  connect(TAction::actions.sigChanged, this, [=] {
    this->actionsChanged();
  });
  setShape(640, 900, 320, 32);
  
  actionsChanged();
}

void
TToolBar::actionsChanged()
{
//cout << endl;
//cout << "TToolBar::actionsChanged()" << endl;
  std::set<std::string> found;

  for(TActionStorage::iterator i = TAction::actions.begin();
      i != TAction::actions.end();
      ++i)
  {
    TAction *action = *i;
    switch(action->type) {
      case TAction::BUTTON:
//cout << "  button '" << action->getTitle() << "'" << endl;
        found.insert(action->getTitle());
        addAction(action->getTitle(), action);
        break;
      case TAction::CHECKBUTTON:
//cout << "  checkbox" << endl;
        break;
      case TAction::RADIOBUTTON:
//cout << "  radiobutton '" << action->getTitle() << "'" << endl;
        TAbstractChoice *choice = dynamic_cast<TAbstractChoice*>(action);
        if (!choice) {
//          cout << "    choice is not an abstract choice" << endl;
          break;
        }
        for(size_t i=0; i<choice->getSize(); ++i) {
          string title = choice->getTitle() + '|' + choice->getID(i);
//cout << "    '" << title << "'" << endl;
          found.insert(title);
          addChoice(title, choice->getModel(), i);
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
TToolBar::addChoice(const string &title, TChoiceModel *choice, size_t index)
{
//cout << "toolbar: addChoice(" << title << ", ...)" << endl;

  TCoord x = 0;
  for(TInteractor *child = getFirstChild(); child; child=child->getNextSibling()) {
    if (child->getTitle() == title)
      return;
    x += 32;
  }

  auto&& button = new TToolButton(this, title, choice, index);
  if (title=="tool|toolbox|selection")
    button->loadBitmap("resource/tool_select.png");
  else
  if (title=="tool|toolbox|directselection")
    button->loadBitmap("resource/tool_directselect.png");
  if (title=="tool|toolbox|pen")
    button->loadBitmap("resource/tool_pen.png");
  else
  if (title=="tool|toolbox|pencil")
    button->loadBitmap("resource/tool_pencil.png");
  else
  if (title=="tool|toolbox|text")
    button->loadBitmap("resource/tool_text.png");
  else
  if (title=="tool|toolbox|circle")
    button->loadBitmap("resource/tool_circ.png");
  else
  if (title=="tool|toolbox|rectangle")
    button->loadBitmap("resource/tool_rect.png");

  button->setShape(x,0,32,32);
  connect(button->sigClicked, [=] {
    choice->select(index);
  });
  
}

int 
test_toolbar()
{
#if 0
  class TMyWindow:
    public TWindow
  {
    public:
      TMyWindow(): TWindow(nullptr, "TMyWindow") {}
      void paint() {
        TPen pen(this);
        TMatrix2D m;
        m.rotate(0.1);
        m.scale(4,2);
        pen.transform(m);
        pen.drawString(32,32, "Hello");
      }
  };
  new TMyWindow();
#else
  TToolBox *tb = TToolBox::getToolBox();
  tb->add("selection",       TSelectionTool::getTool());
  tb->add("directselection", TDirectSelectionTool::getTool());
//  tb->add("pen",             TTextTool::getTool());
  tb->add("text",             TTextTool::getTool());
//  tb->add("pencil",          new TShapeTool(new TFRectangle));
  tb->add("rectangle",       new TShapeTool(new TFRectangle));
  tb->add("circle",          new TShapeTool(new TFCircle));

  new TToolBar(nullptr, "TToolBar");
  new TTestToolbar(nullptr, "TTestToolbar");

  connect(TToolBox::getToolBox()->sigChanged, [=] {
    cout << "TToolBox.sigChanged: selected tool" << endl;
//    cout << "selected tool '" << TToolBox::getToolBox()->choice->getValue()->name() << "'" << endl;
  });
#endif
  toad::mainLoop();
  return 0;
}
