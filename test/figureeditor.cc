/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>

#include <toad/scrollpane.hh>

#include <toad/figureeditor.hh>


using namespace toad;

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

/*
TEST_F(FigureEditor, Rectangle) {
  TFigureEditor wnd(NULL, testname());
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.doModalLoop();
}
*/

/*
TEST_F(FigureEditor, Origin) {
  TFigureEditor wnd(NULL, testname());
  wnd.origin.x = -4;
  wnd.origin.y = -12;
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.doModalLoop();
}
*/

/*
TEST_F(FigureEditor, Translate) {
  TFigureEditor wnd(NULL, testname());
  wnd.translate(-4, -12);
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.doModalLoop();
}
*/

TEST_F(FigureEditor, Scale) {
  TFigureEditor wnd(NULL, testname());
  wnd.scale(2.0, 2.0);
  wnd.addFigure(new TFRectangle(4.5, 8.5, 8, 4));
  wnd.doModalLoop();
}
