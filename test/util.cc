#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>

using namespace toad;

string
testname() {
  const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  return string(test_info->test_case_name())+'.'+string(test_info->name());
}

CGImageRef
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

void
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

inline NSUInteger
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
void
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
