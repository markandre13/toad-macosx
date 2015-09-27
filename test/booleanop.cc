#include <toad/geometry.hh>
#include <toad/booleanop.hh>
#include "gtest.h"

using namespace toad;

TEST(BooleanOp, OverlapUnion) {
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

  TVectorPath result;
  boolean(p0, p1, &result, UNION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, OutsideInsideUnion) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,40));
  ex.line(TPoint(10,40));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, UNION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, InsideOutsideUnion) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,40));
  ex.line(TPoint(10,40));
  ex.close();
  
  TVectorPath result;
  boolean(p1, p0, &result, UNION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, DisjunctUnion) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(20,10));
  p0.line(TPoint(20,20));
  p0.line(TPoint(10,20));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(30,10));
  p1.line(TPoint(40,10));
  p1.line(TPoint(40,20));
  p1.line(TPoint(40,20));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(20,10));
  ex.line(TPoint(20,20));
  ex.line(TPoint(10,20));
  ex.close();
  ex.move(TPoint(30,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,20));
  ex.line(TPoint(40,20));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, UNION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

#if 0
TEST(BooleanOp, UnionFreak1) {
  TVectorPath p0;
  p0.move(TPoint( 10, 60));
  p0.line(TPoint( 50, 20));
  p0.line(TPoint( 60, 10));
  p0.line(TPoint( 70,  0));
  p0.line(TPoint(110 ,40));
  p0.line(TPoint( 50,100));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint( 20, 50));
  p1.line(TPoint( 30, 40));
  p1.line(TPoint( 40, 30));
  p1.line(TPoint( 50, 20));
  p1.line(TPoint( 80, 30));
  p1.line(TPoint( 40, 70));
  p1.close();

  TVectorPath ex;
  ex.move(TPoint(10,60));
  ex.line(TPoint(70,  0));
  ex.line(TPoint(110 ,40));
  ex.line(TPoint( 50,100));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, UNION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}
#endif

extern bool booleanop_gap_error;

// the relevant subset of backup-hang005.txt
TEST(BooleanOp, BackupHang005) {
  TVectorPath p0;
  p0.move(TPoint(9.421740538377911, 4.972124044991261));
  p0.line(TPoint(9.421740538377911, 3.397403910076214));
  p0.line(TPoint(9.598534829304640, 3.267035718302495));
  p0.line(TPoint(9.743758705572354, 3.207232154070539));
  p0.line(TPoint(9.853836273376928, 3.219465779608377));
  p0.line(TPoint(9.926057057672502, 3.303435362454202));
  p0.line(TPoint(9.902695050879458, 3.956599578975180));
  p0.line(TPoint(9.815539291167347, 4.290187939807659));
  p0.line(TPoint(9.778990403122265, 4.401823325525271));
  p0.line(TPoint(9.778990403122265, 4.972124044991261));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(9.920236273376929, 2.848365779608372));
  p1.line(TPoint(9.902695050879458, 3.956599578975180));
  p1.line(TPoint(9.815539291167347, 4.290187939807659));
  p1.line(TPoint(9.691469746748638, 4.669147524840902));
  p1.close();

  TVectorPath ex;
  ex.move(TPoint(9.4217405383779109,3.3974039100762141));
  ex.line(TPoint(9.5985348293046400,3.2670357183024952));
  ex.line(TPoint(9.7437587055723540,3.2072321540705389));
  ex.line(TPoint(9.8538362733769276,3.2194657796083770));
  ex.line(TPoint(9.8710900950692189,3.2395264342264243));
  ex.line(TPoint(9.9202362733769291,2.8483657796083719));
  ex.line(TPoint(9.9132687351130198,3.2885666495055479));
  ex.line(TPoint(9.9260570576725016,3.3034353624542021));
  ex.line(TPoint(9.9026950508794584,3.9565995789751800));
  ex.line(TPoint(9.8155392911673474,4.2901879398076588));
  ex.line(TPoint(9.7789904031222648,4.4018233255252710));
  ex.line(TPoint(9.7789904031222648,4.9721240449912614));
  ex.line(TPoint(9.4217405383779109,4.9721240449912614));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, UNION);
  
  ASSERT_EQ(false, booleanop_gap_error);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}


TEST(BooleanOp, OverlapIntersection) {
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
  ex.move(TPoint(20,20));
  ex.line(TPoint(30,20));
  ex.line(TPoint(30,30));
  ex.line(TPoint(20,30));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, INTERSECTION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, OutsideInsideIntersection) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(20,20));
  ex.line(TPoint(30,20));
  ex.line(TPoint(30,30));
  ex.line(TPoint(20,30));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, INTERSECTION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, InsideOutsideIntersection) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(20,20));
  ex.line(TPoint(30,20));
  ex.line(TPoint(30,30));
  ex.line(TPoint(20,30));
  ex.close();
  
  TVectorPath result;
  boolean(p1, p0, &result, INTERSECTION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, DisjunctIntersection) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(20,10));
  p0.line(TPoint(20,20));
  p0.line(TPoint(10,20));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(30,10));
  p1.line(TPoint(40,10));
  p1.line(TPoint(40,20));
  p1.line(TPoint(40,20));
  p1.close();
  
  TVectorPath ex;
  
  TVectorPath result;
  boolean(p0, p1, &result, INTERSECTION);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, OverlapDifference) {
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
  ex.line(TPoint(20,20));
  ex.line(TPoint(20,30));
  ex.line(TPoint(10,30));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, DIFFERENCE);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, OutsideInsideDifference) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,40));
  ex.line(TPoint(10,40));
  ex.close();
  ex.move(TPoint(20,20));
  ex.line(TPoint(30,20));
  ex.line(TPoint(30,30));
  ex.line(TPoint(20,30));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, DIFFERENCE);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, InsideOutsideDifference) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  
  TVectorPath result;
  boolean(p1, p0, &result, DIFFERENCE);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, DisjunctDifference) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(20,10));
  p0.line(TPoint(20,20));
  p0.line(TPoint(10,20));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(30,10));
  p1.line(TPoint(40,10));
  p1.line(TPoint(40,20));
  p1.line(TPoint(40,20));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(20,10));
  ex.line(TPoint(20,20));
  ex.line(TPoint(10,20));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, DIFFERENCE);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, OverlapXor) {
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
  ex.line(TPoint(20,20));
  ex.line(TPoint(20,30));
  ex.line(TPoint(10,30));
  ex.close();
  ex.move(TPoint(20,30));
  ex.line(TPoint(30,30));
  ex.line(TPoint(30,20));
  ex.line(TPoint(40,20));
  ex.line(TPoint(40,40));
  ex.line(TPoint(20,40));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, XOR);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, OutsideInsideXor) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,40));
  ex.line(TPoint(10,40));
  ex.close();
  ex.move(TPoint(20,20));
  ex.line(TPoint(30,20));
  ex.line(TPoint(30,30));
  ex.line(TPoint(20,30));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, XOR);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, InsideOutsideXor) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(40,10));
  p0.line(TPoint(40,40));
  p0.line(TPoint(10,40));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(20,20));
  p1.line(TPoint(30,20));
  p1.line(TPoint(30,30));
  p1.line(TPoint(20,30));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,40));
  ex.line(TPoint(10,40));
  ex.close();
  ex.move(TPoint(20,20));
  ex.line(TPoint(30,20));
  ex.line(TPoint(30,30));
  ex.line(TPoint(20,30));
  ex.close();
  
  TVectorPath result;
  boolean(p1, p0, &result, XOR);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

TEST(BooleanOp, DisjunctXor) {
  TVectorPath p0;
  p0.move(TPoint(10,10));
  p0.line(TPoint(20,10));
  p0.line(TPoint(20,20));
  p0.line(TPoint(10,20));
  p0.close();

  TVectorPath p1;
  p1.move(TPoint(30,10));
  p1.line(TPoint(40,10));
  p1.line(TPoint(40,20));
  p1.line(TPoint(40,20));
  p1.close();
  
  TVectorPath ex;
  ex.move(TPoint(10,10));
  ex.line(TPoint(20,10));
  ex.line(TPoint(20,20));
  ex.line(TPoint(10,20));
  ex.close();
  ex.move(TPoint(30,10));
  ex.line(TPoint(40,10));
  ex.line(TPoint(40,20));
  ex.line(TPoint(40,20));
  ex.close();
  
  TVectorPath result;
  boolean(p0, p1, &result, XOR);
  
  if (result==ex) {
    SUCCEED();
  } else {
    FAIL() << "expected " << ex << "but got " << result;
  }
}

SweepEvent*
createSweep(TCoord x0, TCoord y0, TCoord x1, TCoord y1)
{
  SweepEvent *l = new SweepEvent(true,  TPoint(x0, y0), nullptr, SUBJECT);
  SweepEvent *r = new SweepEvent(false, TPoint(x1, y1), l, SUBJECT);
  l->otherEvent = r;
  return l;
}

TEST(BooleanOpSweepLineBufferComp, Simple) {
  auto s0 = createSweep(10,10, 20,10);
  auto s1 = createSweep(10,20, 20,20);
  SegmentComp cmp;
  ASSERT_EQ(true, cmp(s0, s1));
}

TEST(BooleanOpSweepLineBufferComp, TrickyBeforeSplit) {
  auto s0 = createSweep(9.7789904031222648,4.401823325525271, 9.8155392911673474,4.2901879398076588); // 15
  auto s1 = createSweep(9.6914697467486377,4.669147524840902, 9.8155392911673474,4.2901879398076588); // 25 before split
  SegmentComp cmp;
  ASSERT_EQ(false, cmp(s0, s1));
}

TEST(BooleanOpSweepLineBufferComp, TrickyAfterSplit) {
  auto s0 = createSweep(9.7789904031222648,4.401823325525271, 9.8155392911673474,4.2901879398076588); // 15
  auto s1 = createSweep(9.6914697467486377,4.669147524840902, 9.7789904031222648,4.4018233255252710); // 25 after split
  SegmentComp cmp;
  ASSERT_EQ(true, cmp(s0, s1));
}

