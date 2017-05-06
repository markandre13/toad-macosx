/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/figureeditor.hh>
#include <toad/figure/selectiontool.hh>
#include <toad/figure/nodetool.hh>
#include <toad/figure/shapetool.hh>
#include <toad/action.hh>
#include <toad/undomanager.hh>

using namespace toad;
using namespace std;

namespace {

class FigureEditor:
  public ::testing::Test
{
  protected:
    static void SetUpTestCase() {
      toad::initialize(0, NULL);
    }
    
    static void TearDownTestCase() {
      toad::terminate();
    }
};

void
sendMouseEvent(TWindow *window, TMouseEvent::EType type, TCoord x, TCoord y)
{
  TMouseEvent me;
  me.window = window;
  me.type = type;
  me.pos.set(x, y);
  window->mouseEvent(me);
}

TEST_F(FigureEditor, Foo)
{
  TFigureModel model;
  
  TFigureEditor *fe = new TFigureEditor(nullptr, "TFigureEditor");
  
  TUndoManager *undoManager = new TUndoManager(fe, "undomanager", "edit|undo", "edit|redo");
  
  fe->setModel(&model);
  fe->enableGrid(false);

  TToolBox *tb = TToolBox::getToolBox();
  auto *choice = new GChoice<TFigureTool*>(fe, "tool|toolbox", tb);
  tb->add("selection"      , TSelectionTool::getTool());
  tb->add("directselection", TNodeTool::getTool());
  tb->add("rectangle"      , TFRectangle::getTool());
  
  fe->setToolBox(tb);
  
  ASSERT_EQ(choice->getValue(), TSelectionTool::getTool());
  choice->setValue(TFRectangle::getTool());

  ASSERT_EQ(choice->getValue(), TFRectangle::getTool());

  ASSERT_EQ(false, undoManager->canUndo());

  // create as rect(10, 10, 10, 20)
  sendMouseEvent(fe, TMouseEvent::LDOWN, 10, 10);
  sendMouseEvent(fe, TMouseEvent::LUP  , 20, 30);
  
  ASSERT_EQ(1, model.size());
  TFRectangle *rectangle = dynamic_cast<TFRectangle*>(model[0]);
  
  TRectangle bounds = rectangle->bounds();
  ASSERT_EQ(TPoint(10, 10), bounds.origin);
  ASSERT_EQ(TSize(10, 20), bounds.size);

  // translate to rect(20, 15, 10, 20)
  choice->setValue(TSelectionTool::getTool());
  sendMouseEvent(fe, TMouseEvent::LDOWN, 15, 10);
  sendMouseEvent(fe, TMouseEvent::LUP  , 25, 15);
  
  bounds = rectangle->bounds();
  ASSERT_EQ(TPoint(20, 15), bounds.origin);
  ASSERT_EQ(TSize(10, 20), bounds.size);

  // undo to rect(10, 10, 10, 20)
  ASSERT_EQ(true, undoManager->canUndo());
  undoManager->doUndo();

  bounds = rectangle->bounds();
  ASSERT_EQ(TPoint(10, 10), bounds.origin);
  ASSERT_EQ(TSize(10, 20), bounds.size);

  // remove
  ASSERT_EQ(true, undoManager->canUndo());
  undoManager->doUndo();
  
  ASSERT_EQ(0, model.size());

  ASSERT_EQ(false, undoManager->canUndo());
}

} // namespace
