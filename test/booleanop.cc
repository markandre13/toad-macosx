#include <toad/geometry.hh>
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
