/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>
#include <toad/geometry.hh>

namespace {

using namespace toad;

class WordWrap:
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
  public TWindow
{
    TPoint p[8];
    int handle;
  public:
    TTest(TWindow *parent, const string &title);
    void paint() override;
    void mouseEvent(const TMouseEvent&) override;
};

TTest::TTest(TWindow *parent, const string &title):TWindow(parent, title) {
  handle = -1;

  p[0].set(50, 25);
  p[1].set(75, 50);
  p[2].set(50, 75);
  p[3].set(25, 75);

  p[4].set(125, 25);
  p[5].set(100, 50);
  p[6].set(125, 75);
  p[7].set(150, 75);
}

void
TTest::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      for(int i=0; i<8; ++i) {
        if (TRectangle(p[i].x-2.5, p[i].y-2.5, 5,5).isInside(me.pos)) {
          handle = i;
          break;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
      p[handle] = me.pos;
      invalidateWindow();
      break;
    case TMouseEvent::LUP:
      if (handle==-1)
        break;
      p[handle] = me.pos;
      handle = -1;
      invalidateWindow();
      break;
  }
}

/*
o convert the path into a list of non-overlapping segments
o 
*/

void
TTest::paint()
{
  TPen pen(this);
  
  TVectorPath path;

/*  
  path.move(10,10);
  path.curve(80,10, 310, 150, 310,190);
  path.line(10,150);
  path.curve(10,10, 150,10, 310, 150);
  path.close();
*/  
/*
  path.move(160,75);
  path.curve(180,10, 250,55, 260,100); // 20 90 10 45
  path.curve(270,145, 160, 190, 160,190);
  path.curve(160,190, 10,180, 10,100);
  path.curve(10,10, 50,10, 160,75);
*/

  path.move(160, 40);
  path.curve(320,10, 50, 100, 310,130);
  path.line(130, 190);
  path.line(10,70);
  path.close();
  
/*
  BooleanOpImp thing(UNION);
  thing.path2events(path, SUBJECT);
  cout << "sweep events: " << thing.eq.size() << endl;
*/  
  path.apply(pen);
  pen.stroke();
  
#if 0
  pen.drawBezier(p, 4);
  pen.drawBezier(p+4, 4);
  for(int i=0; i<8; ++i)
    pen.drawRectangle(p[i].x-2.5, p[i].y-2.5, 5,5);
  
  pen.setColor(1,0,0);
  for(TCoord u=0.0; u<=1.0; u+=0.1) {
    TPoint a = p[0]*(1-u)*(1-u)*(1-u) + p[1]*3*(1-u)*(1-u)*u + p[2]*3*(1-u)*u*u + p[3]*u*u*u;
    pen.drawRectangle(a.x-0.5,a.y-0.5,1,1);
  }
  pen.setColor(0,0,1);
  for(TCoord u=0.0; u<=1.0; u+=0.01) {
    TPoint a, b;
    // a = p[0]*(1 - 3*u + 3*u*u - u*u*u) + p[1]*(3*u - 6*u*u + 3*u*u*u) + p[2]*(3*u*u - 3*u*u*u) + p[3]*u*u*u;
    // a = p[0] - p[0]*3*u + p[0]*3*u*u - p[0]*u*u*u + p[1]*3*u - p[1]*6*u*u + p[1]*3*u*u*u + p[2]*3*u*u -  p[2]*3*u*u*u + p[3]*u*u*u;
    a = 
      p[0] 
      + u * (-p[0]*3 + p[1]*3 )
      + u*u * (p[0]*3 - p[1]*6 + p[2]*3 )
      + u*u*u * (-p[0] + p[1]*3 - p[2]*3 + p[3]);

    b = 
        (-p[0]*3 + p[1]*3 )
      + 2*u * (p[0]*3 - p[1]*6 + p[2]*3 )
      + 3*u*u * (-p[0] + p[1]*3 - p[2]*3 + p[3]);

    pen.drawRectangle(50+(b.x)/20.0-0.5,a.y-0.5,1,1);
  }
  pen.drawLine(50,0,50,200);

  TCoord roots[3];
  int n = solveQuadratic(
    3 * (-p[0].x + p[1].x*3 - p[2].x*3 + p[3].x),
    2 * (p[0].x*3 - p[1].x*6 + p[2].x*3),
    (-p[0].x*3 + p[1].x*3),
    roots, 0, 1);
  
  pen.setColor(0,1,0);
  for(int i=0; i<n; ++i) {
    TPoint pt = bez2point(p, roots[i]);
    TCoord u = roots[i];
    pen.drawRectangle(pt.x-0.5, pt.y-0.5, 1, 1);
  }
  
/*
  (1-u)^2 = 1 - 2u + u^2
  (1-u)^3 = 1 - 3u + 3u^2 - u^3
  
    a0*(1-u)^3 + a1*(1-u)^2*u + a2*(1-u)*u^2 + a3*u^3
  ⇔ a0*(1 - 3u + 3u^2 - u^3) + a1*(u - 3u + u^3) + a2*(u^2-u^3) + a3*u^3
  ⇔ a0 - a0*3u + a0*3u^2 - a0*u^3 + a1*u - a1*3u + a1*u^3 + a2*u^2-a2*u^3 + a3*u^3
*/
#endif
}

TEST_F(WordWrap, foo) {
  TTest wnd(NULL, testname());
  wnd.doModalLoop();
}

} // namespace
