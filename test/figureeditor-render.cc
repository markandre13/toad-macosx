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

#include "../fischland/connectfigure.hh"

namespace {

using namespace toad;

class FigureEditorRender:
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

TEST_F(FigureEditorRender, Rectangle) {
  TTest wnd(NULL, testname());
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

TEST_F(FigureEditorRender, Origin) {
  TTest wnd(NULL, testname());
  wnd.setOrigin(TPoint(-4, -12));
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

TEST_F(FigureEditorRender, Translate) {
  TTest wnd(NULL, testname());
  wnd.setOrigin(TPoint(-4, -12));
  wnd.addFigure(new TFRectangle(8.5, 16.5, 32, 40));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

TEST_F(FigureEditorRender, Scale) {
  TTest wnd(NULL, testname());
  wnd.scale(2.0, 2.0);
  wnd.addFigure(new TFRectangle(4.5, 8.5, 8, 4));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}

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
    TPoint p0 = m.map(area.origin);
    TPoint p1 = m.map(area.origin+area.size);

    area.origin = p0;
    area.size   = p1 - p0;
  } else {
    gridsizeScaled = gridsize;
  }
  
  range.x1 = area.origin.x + area.size.width;
  range.y1 = area.origin.y + area.size.height;

  // Y
  // origin correction
  f = fmod(origin.y, gridsize);
  if (f<0)
    f+=gridsize;

  // area jump
  while(f-gridsize-area.origin.y>=0)
    f -= gridsize;
  while(f-gridsize-area.origin.y<=-5)
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
  while(f-gridsize-area.origin.x>=0)
    f -= gridsize;
  while(f-gridsize-area.origin.x<=-5)
    f += gridsize;
#endif

  range.x0 = f;
  
  return range;
}

TEST_F(FigureEditorRender, gridRange) {

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


#if 0
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


TEST_F(FigureEditorRender, Scroll) {
  TTest2 wnd(NULL, testname());
  wnd.scale(2,2);
//  wnd.addFigure(new TFRectangle(-64.5, -64.5, 320+128, 200+128));
  wnd.addFigure(new TFRectangle(4.5, 4.5, 312, 192));
  wnd.startTimer(0, 1000);
  wnd.doModalLoop();
}
#endif

// FIXME: this should be moved into a figuremodel test
TEST_F(FigureEditorRender, RelatedFigures)
{
  toad::getDefaultStore().registerObject(new TFConnection());
  istringstream in(R"(
    toad::TFigureModel {
      toad::TFRectangle {
        id = 1
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 160
        y = 140
        w = 91
        h = 51
      }
      toad::TFCircle {
        id = 2
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 360.5
        y = 356.5
        w = 109
        h = 101
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        linewidth = 1
        start = 1
        end = 2
      }
    }
  }
})");
  TInObjectStream is(&in);
//  in.setVerbose(true);
//  in.setDebug(true);

  TSerializable *s = is.restore();
  ASSERT_TRUE(is);
  ASSERT_NE(nullptr, s);

  is.close();
  TFigureEditor::restoreRelations();

  TFigureModel *m = dynamic_cast<TFigureModel*>(s);
  ASSERT_NE(nullptr, m);
  
  // check that we've loaded everything correctly
  ASSERT_EQ(3, m->size());
  ASSERT_EQ("toad::TFRectangle", m->at(0)->getClassName());
  ASSERT_EQ("toad::TFCircle", m->at(1)->getClassName());
  ASSERT_EQ("toad::TFConnection", m->at(2)->getClassName());

  ASSERT_EQ(m->at(0), dynamic_cast<TFConnection*>(m->at(2))->start);
  ASSERT_EQ(m->at(1), dynamic_cast<TFConnection*>(m->at(2))->end);
  
  // FIXME: should move relatedTo into TFigureModel because it is data which might not be edited
  // FIXME: TFigureModel also has the notification logic in case figures get removed to that related
  //        figures can update their pointers
  ASSERT_EQ(2, TFigureEditor::relatedTo.size());

  ASSERT_EQ(1, TFigureEditor::relatedTo[m->at(0)].size());
  ASSERT_EQ(1, TFigureEditor::relatedTo[m->at(1)].size());
  
  ASSERT_EQ(2, TFigureEditor::relatedTo.size());
  
  ASSERT_NE(TFigureEditor::relatedTo[m->at(0)].end(),
            TFigureEditor::relatedTo[m->at(0)].find(m->at(2)));
  ASSERT_NE(TFigureEditor::relatedTo[m->at(1)].end(),
            TFigureEditor::relatedTo[m->at(1)].find(m->at(2)));

  // delete a referenced figure
  m->erase(m->begin()); // FIXME: does this also delete the figure? shouldn't. how about eraseAndDelete()?
  
  ASSERT_EQ(2, m->size());
  ASSERT_EQ("toad::TFCircle", m->at(0)->getClassName());
  ASSERT_EQ("toad::TFConnection", m->at(1)->getClassName());
  
  ASSERT_EQ(1, TFigureEditor::relatedTo.size());
  
  ASSERT_EQ(nullptr,  dynamic_cast<TFConnection*>(m->at(1))->start);
  ASSERT_EQ(m->at(0), dynamic_cast<TFConnection*>(m->at(1))->end);
  
  // delete a referencing figure
cout << "---- delete a referencing figure ---" << endl;
  m->erase(m->at(1));
cout << "------------------------------------" << endl;
  ASSERT_EQ(1, m->size());
  ASSERT_EQ("toad::TFCircle", m->at(0)->getClassName());
  
  ASSERT_EQ(0, TFigureEditor::relatedTo.size());

/*
  for(auto &p0: TFigureEditor::relatedTo) {
    cout << p0.first << endl; // ->getClassName() << endl;
    for(auto &p1: p0.second) {
      cout << "  " << p1 << endl;
    }
  }
*/
  
}

} // namespace
