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

#include <toad/figure/toolbox.hh>
#include <toad/figure/toolpanel.hh>
#include <toad/figure/createtool.hh>
#include <toad/figure/selectiontool.hh>
#include <toad/figure/nodetool.hh>
#include <toad/figure/texttool.hh>


#include <cassert>
#include <set>

using namespace toad;

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
  tb->add("directselection", TNodeTool::getTool());
//  tb->add("pen",             TTextTool::getTool());
  tb->add("text",             TTextTool::getTool());
//  tb->add("pencil",          new TShapeTool(new TFRectangle));
  tb->add("rectangle",       new TShapeTool(new TFRectangle));
  tb->add("circle",          new TShapeTool(new TFCircle));

  new TToolPanel(nullptr, "TToolPanel");
  new TTestToolbar(nullptr, "TTestToolbar");

  connect(TToolBox::getToolBox()->sigChanged, [=] {
    cout << "TToolBox.sigChanged: selected tool" << endl;
//    cout << "selected tool '" << TToolBox::getToolBox()->choice->getValue()->name() << "'" << endl;
  });
#endif
  toad::mainLoop();
  return 0;
}
