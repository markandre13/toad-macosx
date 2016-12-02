/*
 * Tests for the Display Output Routines in TPen and TWindow
 *
 */

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>
#include "gtest.h"

using namespace toad;

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

static void
compareImageFile(const string &file0, const string &file1)
{
  TBitmap bmp0, bmp1;
  bmp0.load(file0);
  bmp1.load(file1);
  
  if (bmp0.getWidth() != bmp1.getWidth() ||
      bmp0.getHeight() != bmp1.getHeight() )
  {
    FAIL() << "size mismatch between " << file0 << " and " << file1 << "." << endl;
    return;
  }
        
  for(size_t x=1; x<bmp0.getWidth()-1; ++x) {
    for(size_t y=1; y<bmp0.getHeight()-2; ++y) {
      NSUInteger data0[4], data1[4];
      [bmp0.img getPixel: data0 atX: x y: y];
      [bmp1.img getPixel: data1 atX: x y: y];
      if (data0[0]!=data1[0] ||
          data0[1]!=data1[1] ||
          data0[2]!=data1[2])
      {
        FAIL() << "pixel mismatch between " << file0 << " and " << file1 << " at " << x << ", " << y << ".";
      }
    }
  }
}

static void
scrollRectangleTest(TCoord dx, TCoord dy)
{
  class TScrollRectangleWindow: public TWindow, TSimpleTimer
  {
      unsigned state;
      TCoord dx, dy;
    public:
      TScrollRectangleWindow(TWindow *parent, const string &title, TCoord dx, TCoord dy): TWindow(parent, title) {
        flagNoBackground = true;
        state = 0;
        this->dx = dx;
        this->dy = dy;
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
  
  TWindow *wnd = new TScrollRectangleWindow(NULL, testname(), dx, dy);
  wnd->doModalLoop();
  delete wnd;
}

class Display:
  public ::testing::Test
{
  protected:
    static void SetUpTestCase() {
//      cerr << "SETUP" << endl;
      toad::initialize(0, NULL);
//      toad::nonBlockingMainLoopKludge = true;
    }
    
    static void TearDownTestCase() {
//      cerr << "TEARDOWN" << endl;
      toad::terminate();
    }
};

TEST_F(Display, ScrollRectangleRight) {
  scrollRectangleTest(2, 0);
}

TEST_F(Display, ScrollRectangleLeft) {
  scrollRectangleTest(-2, 0);
}

TEST_F(Display, ScrollRectangleDown) {
  scrollRectangleTest(0, 2);
}

TEST_F(Display, ScrollRectangleUp) {
  scrollRectangleTest(0, -2);
}
