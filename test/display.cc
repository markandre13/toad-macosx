/*
 * Tests for the Display Output Routines in TPen and TWindow
 *
 */

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>

#include <toad/scrollpane.hh>

#include "gtest.h"

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

static string
testname() {
  const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  return string(test_info->test_case_name())+'.'+string(test_info->name());
}

static CGImageRef
grabImage(TWindow *window)
{
  CGImageRef image = CGWindowListCreateImage(
    CGRectNull,
    kCGWindowListOptionIncludingWindow,
    (CGWindowID)[window->nswindow windowNumber],
    kCGWindowImageBoundsIgnoreFraming);

  // image too small yet, return
  if (CGImageGetHeight(image) <= 1) {
    CFRelease(image);
    return 0;
  }

  size_t width = CGImageGetWidth(image);
  size_t height = CGImageGetHeight(image);

  // clip 22 pixels from the top
  CGColorSpaceRef colorspace = CGImageGetColorSpace(image);
  CGContextRef context = CGBitmapContextCreate(
    NULL,
    width, height-22,
    CGImageGetBitsPerComponent(image),
    CGImageGetBytesPerRow(image),
    colorspace,
    CGImageGetAlphaInfo(image));
  CGColorSpaceRelease(colorspace);
  assert(context);
  
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
  CFRelease(image);
  image = CGBitmapContextCreateImage(context);
  CGContextRelease(context);

  return image;
}

static void
saveImage(CGImageRef image, const string &filename)
{
  NSString *nsFilename = [NSString stringWithUTF8String: filename.c_str()];

  CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath: nsFilename];
  CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
  if (!destination) {
    NSLog(@"Failed to create CGImageDestination for %@", nsFilename);
    exit(1);
  }

  CGImageDestinationAddImage(destination, image, nil);

  if (!CGImageDestinationFinalize(destination)) {
    NSLog(@"Failed to write image to %@", nsFilename);
    CFRelease(destination);
    exit(1);
  }

  CFRelease(destination);
//  [nsFilename release];
}

static inline NSUInteger
distance(NSUInteger c0, NSUInteger c1)
{
  return c0<c1 ? c1-c0 : c0-c1;
} 

struct RGBA {
  NSUInteger data[4];
  bool operator<(const RGBA &rhs) const {
    for (int i=0; i<3; ++i) {
      if (data[i] < rhs.data[i])
        return true;
      if (data[i] > rhs.data[i])
        return false;
    }
    return false;
  }
  bool operator!=(const RGBA &rhs) const {
    for (int i=0; i<3; ++i) {
      if (distance(data[i], rhs.data[i]) > 1)
        return true;
    }
    return false;
  }
};
inline ostream& operator<<(ostream &s, const RGBA& c) {
  return s<<'('<<c.data[0] << ", " << c.data[1] << ", " << c.data[2] << ')';
}

/**
 * Actual pixel values do depend on the display's colorspace.
 *
 * To be independent off the displays colorspace the algorithm only compares
 * the pattern, independent of it's color. Tests should therefore avoid using
 * a color only once.
 */
static void
compareImageFile(const string &file0, const string &file1)
{
  TBitmap bmp0, bmp1;
  bmp0.load(file0);
  bmp1.load(file1);

#if 0
  if (bmp0.getWidth() != bmp1.getWidth() ||
      bmp0.getHeight() != bmp1.getHeight() )
  {
    FAIL() << "size mismatch between " 
           << file0 << " (" << bmp0.getWidth() << ", " << bmp0.getHeight() << ") and "
           << file1 << " (" << bmp1.getWidth() << ", " << bmp1.getHeight() << ")" << endl;
    return;
  }
#endif

  map<RGBA, RGBA> colormap;

  for(size_t x=1; x<bmp0.getWidth()-1; ++x) {
    for(size_t y=1; y<bmp0.getHeight()-2; ++y) {
      RGBA c0, c1;
      [bmp0.img getPixel: c0.data atX: x y: y];
      [bmp1.img getPixel: c1.data atX: x y: y];
      auto ptr = colormap.find(c0);
      if (ptr==colormap.end()) {
        colormap[c0] = c1;
      } else
      if (ptr->second != c1) {
        cout << "---------- file: " << file0 << endl;
        for(size_t y=0; y<20; ++y) {
          for(size_t x=0; x<20; ++x) {
            NSUInteger data[4];
            [bmp0.img getPixel: data atX: x y: y];
            cout << "["<<x<<","<<y<<"]("<< data[0] << ", " << data[1] << ", " << data[2] << ")\t";
          }
          cout << endl;
        }
        cout << "---------- file: " << file1 << endl;
        for(size_t y=0; y<20; ++y) {
          for(size_t x=0; x<20; ++x) {
            NSUInteger data[4];
            [bmp1.img getPixel: data atX: x y: y];
            cout << "["<<x<<","<<y<<"]("<< data[0] << ", " << data[1] << ", " << data[2] << ")\t";
          }
          cout << endl;
        }
        FAIL() << "pixel mismatch between " << file0 << " and " << file1 << " at " << x << ", " << y
               << c0 << "->" << c1 << ", expected " << ptr->second << endl;
      }
    }
  }
}

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
