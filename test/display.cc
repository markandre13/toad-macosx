/*
 * Tests for the Display Output Routines in TPen and TWindow
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>

#include <toad/scrollpane.hh>

using namespace toad;

class Display:
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

static void
scrollRectangleTest(TCoord dx, TCoord dy, unsigned times=1)
{
  class TScrollRectangleWindow: public TWindow, TSimpleTimer
  {
      unsigned state;
      TCoord dx, dy;
      unsigned times;
    public:
      TScrollRectangleWindow(TWindow *parent, const string &title, TCoord dx, TCoord dy, unsigned times): TWindow(parent, title) {
        flagNoBackground = true;
        state = 0;
        this->dx = dx;
        this->dy = dy;
        this->times = times;
        startTimer(0, 1000);
      }
      void tick() override { event(true); }
      void paint() override { event(false); }
      
      void event(bool tick) {
//cout << getTitle() << " state " << state << endl;
        switch(state) {
          case 0: {
            if (tick)
              return;
            TPen pen(this);
            pen.setColor(1,1,1);
            pen.fillRectangle(0,0,320,200);

            pen.setColor(0,0,0);
      
            pen.setColor(0,0.5,1);
            pen.drawRectangle(1.5,1.5,317,197);
      
            pen.drawRectangle(10.5,10.5,10,10);
            pen.setColor(1,0,0);
            pen.drawRectangle(11.5, 11.5, 8, 8);
            pen.setColor(0,1,0);
            pen.drawRectangle(12.5, 12.5, 6, 6);
            state = 1;
          } break;
          case 1: {
            if (!tick)
              return;
            CGImageRef image = grabImage(this);
            if (!image)
              return;
            for(unsigned i=0; i<times; ++i)
              scrollRectangle(TRectangle(11, 11, 9, 9), dx, dy, true);
            CFRelease(image);
            state = 2;
          } break;
          case 2: {
            if (tick)
              return;
            TPen pen(this);
            pen.setColor(1,0.5,0);
            pen.fillRectangle(0,0,320,200);
            state = 3;
          } break;
          case 3: {
            if (!tick)
              return;
            CGImageRef image = grabImage(this);
            if (!image)
              return;
            saveImage(image, "test/"+getTitle()+".tmp.png");
            CFRelease(image);
//            toad::running = false;
            stopTimer();
            compareImageFile("test/"+getTitle()+".tmp.png", "test/"+getTitle()+".png");
            destroyWindow();
          } break;
        }
      }
  };
  
  TScrollRectangleWindow wnd(NULL, testname(), dx, dy, times);
  wnd.doModalLoop();
}

// scroll by 2 pixels
TEST_F(Display, ScrollRectangleRight) { scrollRectangleTest(2, 0); }
TEST_F(Display, ScrollRectangleLeft) { scrollRectangleTest(-2, 0); }
TEST_F(Display, ScrollRectangleDown) { scrollRectangleTest(0, 2); }
TEST_F(Display, ScrollRectangleUp) { scrollRectangleTest(0, -2); }
TEST_F(Display, ScrollRectangleRightDown) { scrollRectangleTest(2, 2); }
TEST_F(Display, ScrollRectangleRightUp) { scrollRectangleTest(2, -2); }
TEST_F(Display, ScrollRectangleLeftDown) { scrollRectangleTest(-2, 2); }
TEST_F(Display, ScrollRectangleLeftUp) { scrollRectangleTest(-2, -2); }

// scroll by 1 pixel two times to check that information about the
// invalidated area from the 1st scroll is also scrolled
TEST_F(Display, ScrollRectangleRight2) { scrollRectangleTest(1, 0, 2); }
TEST_F(Display, ScrollRectangleLeft2) { scrollRectangleTest(-1, 0, 2); }
TEST_F(Display, ScrollRectangleDown2) { scrollRectangleTest(0, 1, 2); }
TEST_F(Display, ScrollRectangleUp2) { scrollRectangleTest(0, -1, 2); }
TEST_F(Display, ScrollRectangleRightDown2) { scrollRectangleTest(1, 1, 2); }
TEST_F(Display, ScrollRectangleRightUp2) { scrollRectangleTest(1, -1, 2); }
TEST_F(Display, ScrollRectangleLeftDown2) { scrollRectangleTest(-1, 1, 2); }
TEST_F(Display, ScrollRectangleLeftUp2) { scrollRectangleTest(-1, -1, 2); }

#if 0

static void
scrollPaneTest(TCoord dx, TCoord dy)
{
  class TScrollPaneWindow: public TScrollPane, TSimpleTimer
  {
      unsigned state;
      TCoord px, py;
    public:
      TScrollPaneWindow(TWindow *parent, const string &title, TCoord dx, TCoord dy): TScrollPane(parent, title) {
        state = 0;
//        this->dx = dx;
//        this->dy = dy;
        lx = ly = 0;
      }
/*      
      void scrolled(TCoord dx, TCoord dy) override {
        px=py=0;
        if (hscroll)
          px =  hscroll->getValue();
        if (vscroll)
          py =  vscroll->getValue();
      }
*/      
      void tick() override {
      }
      
      void adjustPane() override {
        pane.set(0,0,480,180);
      }
      
      void paint() override {
static TCoord c = 1.0;
        TPen pen(this);
        TCoord x, y;
        getPanePos(&x, &y);
        pen.setColor(c,0,1-c);
        c-=0.1;
cout << "paint: x:"<<x<<endl;

const NSRect *rects;
NSInteger count;
[nsview getRectsBeingDrawn:&rects count:&count];
for(int i=0; i<count; ++i)
  cout << "  " << rects[i].origin.x << ", " << rects[i].origin.y << ", " << rects[i].size.width << ", " << rects[i].size.height << endl;

        pen.translate(-lx, -ly);
        pen.drawLine(320-100,0, 320+100, 200);
        
//        CGContextFlush(pen.ctx);
//        [nswindow flushWindow];
      }
  };
  
  TScrollPaneWindow wnd(NULL, testname(), dx, dy);
  wnd.doModalLoop();
}

TEST_F(Display, ScrollPaneRight) {
  scrollPaneTest(0, 2);
}

#endif
