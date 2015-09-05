#include "gtest.h"
#include "booleanop.hh"

using namespace toad;

static TVectorPath
boolean(const TVectorPath &a, const TVectorPath &b, cbop::BooleanOpType op)
{
  TVectorPath c;
  cbop::compute(a, b, c, op);
  return c;
}

TEST(MathTest, TwoPlusTwoEqualsFour) {
  EXPECT_EQ(2 + 2, 4);
}

TEST(BooleanOp, Union) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(30,10));
  p0.line(TPoint(30,30));
  p0.line(TPoint(10,30));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(40,20));
  p1.line(TPoint(40,40));
  p1.line(TPoint(20,40));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(30,10));
  ex.line(TPoint(30,20));
  ex.line(TPoint(40,20));
  ex.line(TPoint(40,40));
  ex.line(TPoint(20,40));
  ex.line(TPoint(20,30));
  ex.line(TPoint(10,30));
  ex.close();
  
  TVectorPath result = boolean(p0, p1, cbop::UNION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
