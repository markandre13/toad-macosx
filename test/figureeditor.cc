/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>
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

class TTest:
  public TFigureEditor, public TSimpleTimer
{
  public:
    using TFigureEditor::TFigureEditor;
  protected:
    void tick() override;
};

void
TTest::tick()
{
  CGImageRef image = grabImage(this);
  if (!image)
    return;
  saveImage(image, "test/"+getTitle()+".tmp.png");
  CFRelease(image);
  stopTimer();
  compareImageFile("test/"+getTitle()+".tmp.png", "test/"+getTitle()+".png");
  destroyWindow();
}

TEST_F(FigureEditor, Rectangle) {
  TTest wnd(NULL, testname());
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

TEST_F(FigureEditor, Origin) {
  TTest wnd(NULL, testname());
  wnd.setOrigin(TPoint(-4, -12));
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

TEST_F(FigureEditor, Translate) {
  TTest wnd(NULL, testname());
  wnd.setOrigin(TPoint(-4, -12));
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

TEST_F(FigureEditor, Scale) {
  TTest wnd(NULL, testname());
  wnd.scale(2.0, 2.0);
  wnd.addFigure(new TFRectangle(4.5, 8.5, 8, 4));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}
