/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/figureeditor.hh>
#include <toad/figure/selectiontool.hh>
#include <toad/figure/nodetool.hh>
#include <toad/figure/shapetool.hh>
#include <toad/figure/connectfigure.hh>
#include <toad/figure/connecttool.hh>
#include <toad/action.hh>
#include <toad/vector.hh>
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

TEST_F(FigureEditor, TranslateOneFigure)
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

TEST_F(FigureEditor, RotateTwoRectangles)
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

  sendMouseEvent(fe, TMouseEvent::LDOWN, 15, 15);
  sendMouseEvent(fe, TMouseEvent::LUP  , 25, 35);
  
  ASSERT_EQ(2, model.size());

  // select all
  choice->setValue(TSelectionTool::getTool());
  sendMouseEvent(fe, TMouseEvent::LDOWN, 0, 0);
  sendMouseEvent(fe, TMouseEvent::LUP  , 90, 90);

  ASSERT_EQ(2, fe->selection.size());
  
  // rotate (rectangles can't be rotated so they are prefixed with TFTransform)
  sendMouseEvent(fe, TMouseEvent::LDOWN, 5, 5);
  sendMouseEvent(fe, TMouseEvent::LUP  , 200, 5);
  
  ASSERT_EQ(2, fe->selection.size());
  
  // check that the selection has been updated
  auto &&s = fe->selection.begin();
  ASSERT_STREQ("toad::TFTransform", (*s)->getClassName());
  ++s;
  ASSERT_STREQ("toad::TFTransform", (*s)->getClassName());
  
  ASSERT_STREQ("toad::TFTransform", model[0]->getClassName());
  ASSERT_STREQ("toad::TFTransform", model[1]->getClassName());
  
  // undo rotation
  ASSERT_EQ(true, undoManager->canUndo());
  undoManager->doUndo();

  ASSERT_EQ(2, fe->selection.size());
  s = fe->selection.begin();
  ASSERT_STREQ("toad::TFRectangle", (*s)->getClassName());
  ++s;
  ASSERT_STREQ("toad::TFRectangle", (*s)->getClassName());

  
  ASSERT_STREQ("toad::TFRectangle", model[0]->getClassName());
  ASSERT_STREQ("toad::TFRectangle", model[1]->getClassName());

  // undo adding a figure
  ASSERT_EQ(true, undoManager->canUndo());
  undoManager->doUndo();
  ASSERT_EQ(1, model.size());

  // undo adding a figure
  ASSERT_EQ(true, undoManager->canUndo());
  undoManager->doUndo();
  ASSERT_EQ(0, model.size());

  ASSERT_EQ(false, undoManager->canUndo());
}

TEST_F(FigureEditor, RotateAConnectedFigure)
{
  TFigureModel model;
  TRectangle bounds;
  
  TFigureEditor *fe = new TFigureEditor(nullptr, "TFigureEditor");
  
  TUndoManager *undoManager = new TUndoManager(fe, "undomanager", "edit|undo", "edit|redo");
  
  fe->setModel(&model);
  fe->enableGrid(false);

  TToolBox *tb = TToolBox::getToolBox();
  auto *choice = new GChoice<TFigureTool*>(fe, "tool|toolbox", tb);
  tb->add("selection"      , TSelectionTool::getTool());
  tb->add("directselection", TNodeTool::getTool());
  tb->add("rectangle"      , TFRectangle::getTool());
  tb->add("connect"        , TConnectTool::getTool());
  
  fe->setToolBox(tb);
  
  ASSERT_EQ(choice->getValue(), TSelectionTool::getTool());
  choice->setValue(TFRectangle::getTool());

  ASSERT_EQ(choice->getValue(), TFRectangle::getTool());

  ASSERT_EQ(false, undoManager->canUndo());

  // create rect(10, 10, 20, 20)
  sendMouseEvent(fe, TMouseEvent::LDOWN, 10, 10);
  sendMouseEvent(fe, TMouseEvent::LUP  , 30, 30);

  // create rect(50, 10, 20, 20)
  sendMouseEvent(fe, TMouseEvent::LDOWN, 50, 10);
  sendMouseEvent(fe, TMouseEvent::LUP  , 70, 30);
  
  // check the result
  ASSERT_EQ(2, model.size());
  TFRectangle  *rectangle0 = dynamic_cast<TFRectangle*>(model[0]);
  TFRectangle  *rectangle1 = dynamic_cast<TFRectangle*>(model[1]);
  ASSERT_EQ(TRectangle(10,10,20,20), model[0]->bounds());
  ASSERT_EQ(TRectangle(50,10,20,20), model[1]->bounds());
  
  // connect both rectangles with TFConnection
  choice->setValue(TConnectTool::getTool());
  sendMouseEvent(fe, TMouseEvent::LDOWN, 15, 10);
  sendMouseEvent(fe, TMouseEvent::MOVE , 55, 10); // FIXME: should work without the move
  sendMouseEvent(fe, TMouseEvent::LUP  , 55, 10);

  // check the result
  ASSERT_EQ(3, model.size());
  TFConnection *connection = dynamic_cast<TFConnection*>(model[2]);
  ASSERT_NE(nullptr, rectangle0);
  ASSERT_NE(nullptr, rectangle1);
  ASSERT_NE(nullptr, connection);
  ASSERT_EQ(rectangle0, connection->start);
  ASSERT_EQ(rectangle1, connection->end);
  
  ASSERT_EQ(2, connection->p.size());
  ASSERT_EQ(TPoint(30, 20), connection->p[0]);
  ASSERT_EQ(TPoint(50, 20), connection->p[1]);
  
  ASSERT_EQ(2, TFigureEditor::relatedTo.size());
  ASSERT_NE(TFigureEditor::relatedTo.end(), TFigureEditor::relatedTo.find(rectangle0));
  ASSERT_NE(TFigureEditor::relatedTo.end(), TFigureEditor::relatedTo.find(rectangle1));
  ASSERT_NE(TFigureEditor::relatedTo[rectangle0].end(), TFigureEditor::relatedTo[rectangle0].find(connection));
  ASSERT_NE(TFigureEditor::relatedTo[rectangle1].end(), TFigureEditor::relatedTo[rectangle1].find(connection));

  choice->setValue(TSelectionTool::getTool());

  // select 2nd rectangle
  sendMouseEvent(fe, TMouseEvent::LDOWN, 50, 10);
  sendMouseEvent(fe, TMouseEvent::LUP, 50, 10);

  // move 2nd rectangle
  sendMouseEvent(fe, TMouseEvent::LDOWN, 55, 10);
  sendMouseEvent(fe, TMouseEvent::LUP, 75, 10);
  ASSERT_EQ(rectangle1, model[1]);
  ASSERT_EQ(TRectangle(70,10,20,20), model[1]->bounds());

  ASSERT_EQ(2, TFigureEditor::relatedTo.size());

  ASSERT_EQ(2, connection->p.size());
  ASSERT_EQ(TPoint(30, 20), connection->p[0]);
  ASSERT_EQ(TPoint(70, 20), connection->p[1]);

  // rotate 2nd rectangle 45° clockwise
  sendMouseEvent(fe, TMouseEvent::LDOWN, 95, 5);
  sendMouseEvent(fe, TMouseEvent::LUP, 95, 20);

  TFTransform *transform = dynamic_cast<TFTransform*>(model[1]);
  ASSERT_NE(nullptr, transform);

  ASSERT_EQ(2, TFigureEditor::relatedTo.size());
  ASSERT_EQ(2, TFigureEditor::relatedTo.size());
  ASSERT_NE(TFigureEditor::relatedTo.end(), TFigureEditor::relatedTo.find(rectangle0));
  ASSERT_NE(TFigureEditor::relatedTo.end(), TFigureEditor::relatedTo.find(transform));
  ASSERT_NE(TFigureEditor::relatedTo[rectangle0].end(), TFigureEditor::relatedTo[rectangle0].find(connection));
  ASSERT_NE(TFigureEditor::relatedTo[transform].end(), TFigureEditor::relatedTo[transform].find(connection));

  ASSERT_EQ(rectangle1, transform->figure);
  ASSERT_EQ(rectangle0, connection->start);
  ASSERT_EQ(transform, connection->end);

  ASSERT_DOUBLE_EQ(65.857864376269049, connection->p[1].x);
  ASSERT_DOUBLE_EQ(20.0              , connection->p[1].y);
  
  // undo rotation
  undoManager->doUndo();

  ASSERT_EQ(rectangle1, connection->end);

  ASSERT_EQ(rectangle1, model[1]);
  ASSERT_EQ(TRectangle(70,10,20,20), model[1]->bounds());

  ASSERT_EQ(2, TFigureEditor::relatedTo.size());
  ASSERT_NE(TFigureEditor::relatedTo.end(), TFigureEditor::relatedTo.find(rectangle0));
  ASSERT_NE(TFigureEditor::relatedTo.end(), TFigureEditor::relatedTo.find(rectangle1));
  ASSERT_NE(TFigureEditor::relatedTo[rectangle0].end(), TFigureEditor::relatedTo[rectangle0].find(connection));
  ASSERT_NE(TFigureEditor::relatedTo[rectangle1].end(), TFigureEditor::relatedTo[rectangle1].find(connection));

  ASSERT_EQ(2, connection->p.size());
  ASSERT_EQ(TPoint(30, 20), connection->p[0]);
  ASSERT_EQ(TPoint(70, 20), connection->p[1]);

  // FIXME: could now test removing a connected figure

}

} // namespace
