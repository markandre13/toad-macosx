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
//  destroyWindow();
}

#if 0
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
#endif

//    0   4   8   12  16
//  0 +   +   +   +   +
// 
//  4 +   +   +   +   +
// 
//  8 +   +---+---+   +
//        |       |
// 12 +   +---+---+   +
// 
// 16 +   +   +   +   +
// 
// 20 +   +   +   +   +
//

struct TGridRange
{
  TGridRange() { x0=x1=y0=y1; }
  TGridRange(TCoord inX0, TCoord inX1, TCoord inY0, TCoord inY1): x0(inX0), x1(inX1), y0(inY0), y1(inY1) {}
  TCoord x0, x1, y0, y1;
  bool operator==(const TGridRange &rhs) const {
    return x0==rhs.x0 && x1==rhs.x1 && y0==rhs.y0 && y1==rhs.y1;
  }
};
ostream& operator<<(ostream &out, const TGridRange& gr)
{
  out << "(x=" << gr.x0 << " - " << gr.x1 << ", y=" << gr.y0 << " - " << gr.y1 << ")";
  return out;
}

/**
 * \param area		area in screen coordinates
 * \param mat		null or scaling matrix
 * \param origin	origin in picture coordinates coordinates
 * \param gridsize	grid size in picture coordinates
 */
TGridRange
gridrange(const TRectangle &inArea,
          const TMatrix2D *mat,
          const TPoint &origin,
          TCoord gridsize)
{
//  cout << "------------------------------------------------------------------------------------" << endl;
  TRectangle area(inArea);
  TGridRange range;

  double f, gridsizeScaled;

  if (mat) {
    gridsizeScaled = gridsize * 2;
  
    TMatrix2D m(*mat);
    m.invert();
    TPoint p0 = m.map(TPoint(area.x, area.y));
    TPoint p1 = m.map(TPoint(area.x+area.w, area.y+area.h));

    area.x = p0.x;
    area.y = p0.y;
    area.w = p1.x - p0.x;
    area.h = p1.y - p0.y;
  } else {
    gridsizeScaled = gridsize;
  }
  
  range.x1 = area.x + area.w;
  range.y1 = area.y + area.h;

  // Y
  // origin correction
  f = fmod(origin.y, gridsize);
  if (f<0)
    f+=gridsize;

  // area jump
  while(f-gridsize-area.y>=0)
    f -= gridsize;
  while(f-gridsize-area.y<=-5)
    f += gridsize;
  range.y0 = f;

  // X
  // origin correction
  f = fmod(origin.x, gridsize);
  if (f<0)
    f+=gridsize;

  // area jump
#if 0  
  if (false) {
  }

//  else if (f0==0 && area.x==-5) // f0-gridsize-1 == area.x
//    f0 -= gridsize;
//  else if (f0==0 && area.x==-4) // f0-gridsize == area.x
//    f0 -= gridsize;

//  else if (f0==2 && area.x==-5) // f0-gridsize-3 == area.x
//    f0 -= gridsize;
//  else if (f0==2 && area.x==-4) // f0-gridsize-2 == area.x
//    f0 -= gridsize;
//  else if (f0==2 && area.x==-3) // f0-gridsize-1 == area.x
//    f0 -= gridsize;
//  else if (f0==2 && area.x==-2) // f0-gridsize == area.x
//    f0 -= gridsize;

//  else if (f0==3 && area.x==-5) // f0-gridsize-4 == area.x
//    f0 -= gridsize*2;

//  else if (f0==3 && area.x==-4) // f0-gridsize-3 == area.x
//    f0 -= gridsize;
//  else if (f0==3 && area.x==-3) // f0-gridsize-2 == area.x
//    f0 -= gridsize;
//  else if (f0==3 && area.x==-2) // f0-gridsize-1 == area.x
//    f0 -= gridsize;
//  else if (f0==3 && area.x==-1) // f0-gridsize == area.x
//    f0 -= gridsize;

  else if (f0-gridsize-area.x==0)
    f0 -= gridsize;
  else if (f0-gridsize-area.x==1)
    f0 -= gridsize;
  else if (f0-gridsize-area.x==2)
    f0 -= gridsize;
  else if (f0-gridsize-area.x==3)
    f0 -= gridsize;
  else if (f0-gridsize-area.x==4)
    f0 -= gridsize*2;

//  else if (f0==0 && area.x== 1) // f0-gridsize+5 == area.x
//    f0 += gridsize;
//  else if (f0==0 && area.x== 2) // f0-gridsize+6 == area.x
//    f0 += gridsize;
//  else if (f0==0 && area.x== 3)
//    f0 += gridsize;
//  else if (f0==0 && area.x== 4)
//    f0 += gridsize;
//  else if (f0==0 && area.x== 5)
//    f0 += gridsize * 2;
//  else if (f0==0 && area.x== 6)
//    f0 += gridsize * 2;

//  else if (f0==2 && area.x== 3) // f0-gridsize+5 == area.x
//    f0 += gridsize;
//  else if (f0==2 && area.x== 4) // f0-gridsize+6 == area.x
//    f0 += gridsize;
//  else if (f0==2 && area.x== 5)
//    f0 += gridsize;
//  else if (f0==2 && area.x== 6)
//    f0 += gridsize;

//  else if (f0==3 && area.x== 4) // f0-gridsize+5 == area.x
//    f0 += gridsize;
//  else if (f0==3 && area.x== 5) // f0-gridsize+6 == area.x
//    f0 += gridsize;
//  else if (f0==3 && area.x== 6)
//    f0 += gridsize;
    
  else if (f0-gridsize-area.x==-5)
    f0 += gridsize;
  else if (f0-gridsize-area.x==-6)
    f0 += gridsize;
  else if (f0-gridsize-area.x==-7)
    f0 += gridsize;
  else if (f0-gridsize-area.x==-8)
    f0 += gridsize;
  else if (f0-gridsize-area.x==-9)
    f0 += gridsize * 2;
  else if (f0-gridsize-area.x==-10)
    f0 += gridsize * 2;
#else
  while(f-gridsize-area.x>=0)
    f -= gridsize;
  while(f-gridsize-area.x<=-5)
    f += gridsize;
#endif

  range.x0 = f;
  
  return range;
}

TEST_F(FigureEditor, gridRange) {

  // UNSCALED

  // origin.y = [-5, 6]
  ASSERT_EQ(TGridRange( 0,320, 3,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -5), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -4), 4));
  ASSERT_EQ(TGridRange( 0,320, 1,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -3), 4));
  ASSERT_EQ(TGridRange( 0,320, 2,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -2), 4));
  ASSERT_EQ(TGridRange( 0,320, 3,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -1), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 1,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  1), 4));
  ASSERT_EQ(TGridRange( 0,320, 2,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  2), 4));
  ASSERT_EQ(TGridRange( 0,320, 3,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  3), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  4), 4));
  ASSERT_EQ(TGridRange( 0,320, 1,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  5), 4));
  ASSERT_EQ(TGridRange( 0,320, 2,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  6), 4));

  // origin.x = [-5, 6]
  ASSERT_EQ(TGridRange( 3,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  -5, 0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  -4, 0), 4));
  ASSERT_EQ(TGridRange( 1,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  -3, 0), 4));
  ASSERT_EQ(TGridRange( 2,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  -2, 0), 4));
  ASSERT_EQ(TGridRange( 3,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  -1, 0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   0, 0), 4));
  ASSERT_EQ(TGridRange( 1,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   1, 0), 4));
  ASSERT_EQ(TGridRange( 2,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   2, 0), 4));
  ASSERT_EQ(TGridRange( 3,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   3, 0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   4, 0), 4));
  ASSERT_EQ(TGridRange( 1,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   5, 0), 4));
  ASSERT_EQ(TGridRange( 2,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(   6, 0), 4));

  // view.y = [-5, 6]
  ASSERT_EQ(TGridRange( 0,320,-4,195), gridrange(TRectangle(0, -5,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320,-4,196), gridrange(TRectangle(0, -4,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,197), gridrange(TRectangle(0, -3,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,198), gridrange(TRectangle(0, -2,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,199), gridrange(TRectangle(0, -1,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,  0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 4,201), gridrange(TRectangle(0,  1,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 4,202), gridrange(TRectangle(0,  2,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 4,203), gridrange(TRectangle(0,  3,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 4,204), gridrange(TRectangle(0,  4,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 8,205), gridrange(TRectangle(0,  5,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 8,206), gridrange(TRectangle(0,  6,320,200), nullptr, TPoint(  0,  0), 4));

  // view.x = [-5, 6]
  ASSERT_EQ(TGridRange(-4,315, 0,200), gridrange(TRectangle( -5,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange(-4,316, 0,200), gridrange(TRectangle( -4,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,317, 0,200), gridrange(TRectangle( -3,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,318, 0,200), gridrange(TRectangle( -2,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,319, 0,200), gridrange(TRectangle( -1,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(  0,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 4,321, 0,200), gridrange(TRectangle(  1,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 4,322, 0,200), gridrange(TRectangle(  2,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 4,323, 0,200), gridrange(TRectangle(  3,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 4,324, 0,200), gridrange(TRectangle(  4,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 8,325, 0,200), gridrange(TRectangle(  5,0,320,200), nullptr, TPoint(  0,  0), 4));
  ASSERT_EQ(TGridRange( 8,326, 0,200), gridrange(TRectangle(  6,0,320,200), nullptr, TPoint(  0,  0), 4));

  // view.x = [-5, 6], origin.x=-1
  ASSERT_EQ(TGridRange(-5,315, 0,200), gridrange(TRectangle( -5,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange(-1,316, 0,200), gridrange(TRectangle( -4,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange(-1,317, 0,200), gridrange(TRectangle( -3,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange(-1,318, 0,200), gridrange(TRectangle( -2,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange(-1,319, 0,200), gridrange(TRectangle( -1,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 3,320, 0,200), gridrange(TRectangle(  0,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 3,321, 0,200), gridrange(TRectangle(  1,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 3,322, 0,200), gridrange(TRectangle(  2,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 3,323, 0,200), gridrange(TRectangle(  3,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 7,324, 0,200), gridrange(TRectangle(  4,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 7,325, 0,200), gridrange(TRectangle(  5,0,320,200), nullptr, TPoint( -1,  0), 4));
  ASSERT_EQ(TGridRange( 7,326, 0,200), gridrange(TRectangle(  6,0,320,200), nullptr, TPoint( -1,  0), 4));

  // view.x = [-5, 6], origin.x=-2
  ASSERT_EQ(TGridRange(-2,315, 0,200), gridrange(TRectangle( -5,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange(-2,316, 0,200), gridrange(TRectangle( -4,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange(-2,317, 0,200), gridrange(TRectangle( -3,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange(-2,318, 0,200), gridrange(TRectangle( -2,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 2,319, 0,200), gridrange(TRectangle( -1,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 2,320, 0,200), gridrange(TRectangle(  0,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 2,321, 0,200), gridrange(TRectangle(  1,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 2,322, 0,200), gridrange(TRectangle(  2,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 6,323, 0,200), gridrange(TRectangle(  3,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 6,324, 0,200), gridrange(TRectangle(  4,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 6,325, 0,200), gridrange(TRectangle(  5,0,320,200), nullptr, TPoint( -2,  0), 4));
  ASSERT_EQ(TGridRange( 6,326, 0,200), gridrange(TRectangle(  6,0,320,200), nullptr, TPoint( -2,  0), 4));

  // SCALED BY TWO

  TMatrix2D m;
  m.scale(2, 2);

  // origin.y = [-5, 6]
/*
  ASSERT_EQ(TGridRange( 0,320, 3,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -5), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -4), 4));
  ASSERT_EQ(TGridRange( 0,320, 1,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -3), 4));
  ASSERT_EQ(TGridRange( 0,320, 2,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -2), 4));
  ASSERT_EQ(TGridRange( 0,320, 3,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0, -1), 4));
*/
  ASSERT_EQ(TGridRange( 0,160, 0,100), gridrange(TRectangle(0,0,320,200), &m, TPoint(  0,  0), 4));
//  ASSERT_EQ(TGridRange( 0,160, 1,100), gridrange(TRectangle(0,0,320,200), &m, TPoint(  0,  1), 4));
/*
  ASSERT_EQ(TGridRange( 0,320, 2,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  2), 4));
  ASSERT_EQ(TGridRange( 0,320, 3,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  3), 4));
  ASSERT_EQ(TGridRange( 0,320, 0,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  4), 4));
  ASSERT_EQ(TGridRange( 0,320, 1,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  5), 4));
  ASSERT_EQ(TGridRange( 0,320, 2,200), gridrange(TRectangle(0,0,320,200), nullptr, TPoint(  0,  6), 4));
*/

/*
  ASSERT_EQ(TGridRange(4,320, 0,200), gridrange(TRectangle(1,0,320,200), nullptr, TPoint(0, 0), 4));
  ASSERT_EQ(TGridRange(0,320, 4,200), gridrange(TRectangle(0,1,320,200), nullptr, TPoint(0, 0), 4));
  ASSERT_EQ(TGridRange(4,320, 4,200), gridrange(TRectangle(1,1,320,200), nullptr, TPoint(0, 0), 4));
*/  
/*
  TTest wnd(NULL, testname());
  wnd.setOrigin(TPoint(0, -8));
//  wnd.scale(2.0, 2.0);
  wnd.addFigure(new TFRectangle(4.5, 8.5, 8, 4));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
*/
}


#if 1
class TTest2:
  public TFigureEditor, public TSimpleTimer
{
    unsigned state;
  public:
    using TFigureEditor::TFigureEditor;
  protected:
    void tick() override;
};

void
TTest2::tick()
{
  switch(state) {
    case 0: {
      } break;
    case 1: {
      CGImageRef image = grabImage(this);
      if (!image)
        return;
      saveImage(image, "test/"+getTitle()+".tmp.png");
      CFRelease(image);
      stopTimer();
      compareImageFile("test/"+getTitle()+".tmp.png", "test/"+getTitle()+".png");
//      destroyWindow();
    } break;
  }
}


TEST_F(FigureEditor, Scroll) {
  TTest2 wnd(NULL, testname());
  wnd.scale(2,2);
  wnd.addFigure(new TFRectangle(-64.5, -64.5, 320+128, 200+128));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}
#endif
