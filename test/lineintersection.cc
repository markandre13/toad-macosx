#include <toad/geometry.hh>
#include "gtest.h"

using namespace toad;

//  p0
//  |
//  |q0
//  |  \
//  |   \
//  p1   q1
TEST(LineLineIntersectionWithOverlap, OnePoint) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,20);
  pt[3].set(20,90);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,20), il[0].seg0.pt);
}

// p0
// |
// |q0
// |
// |q1
// |
// p1
TEST(LineLineIntersectionWithOverlap, TwoPointInside) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,20);
  pt[3].set(10,80);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,20), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,80), il[1].seg0.pt);
}

// p0
// |
// |q0
// |
// |p1
// |
// q1
TEST(LineLineIntersectionWithOverlap, TwoPointBefore) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,50);
  pt[2].set(10,20);
  pt[3].set(10,60);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,20), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,50), il[1].seg0.pt);
}

// q0
// |
// |p0
// |
// |q1
// |
// p1
TEST(LineLineIntersectionWithOverlap, TwoPointAfter) {
  TPoint pt[4];
  pt[0].set(10,20);
  pt[1].set(10,60);
  pt[2].set(10,10);
  pt[3].set(10,50);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,20), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,50), il[1].seg0.pt);
}

//  p0q0
//  |  \
//  |   \
//  p1   q1
TEST(LineLineIntersectionWithOverlap, OneSamePointDown) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,10);
  pt[3].set(20,90);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
}

//  p1q0
//  |  \
//  |   \
//  p0   q1
TEST(LineLineIntersectionWithOverlap, OneSamePointUp) {
  TPoint pt[4];
  pt[0].set(10,90);
  pt[1].set(10,10);
  pt[2].set(10,10);
  pt[3].set(20,90);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
}

//  p0q1
//  |  \
//  |   \
//  p1   q0
TEST(LineLineIntersectionWithOverlap, OneSamePointUp2) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(20,90);
  pt[3].set(10,10);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
}

//  p0q0
//  |
//  |
//  p1q1
TEST(LineLineIntersectionWithOverlap, TwoSamePointDown) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,10);
  pt[3].set(10,90);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,90), il[1].seg0.pt);
}

//  p0q1
//  |
//  |
//  p1q0
TEST(LineLineIntersectionWithOverlap, TwoSamePointUp) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,90);
  pt[3].set(10,10);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,90), il[1].seg0.pt);
}

//  p0q0
//  |
//  |q1
//  |
//  p1
TEST(LineLineIntersectionWithOverlap, OneSamePointInsideDown) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,10);
  pt[3].set(10,80);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,80), il[1].seg0.pt);
}

//  p1q0
//  |
//  |q1
//  |
//  p0
TEST(LineLineIntersectionWithOverlap, OneSamePointInsideUp) {
  TPoint pt[4];
  pt[0].set(10,90);
  pt[1].set(10,10);
  pt[2].set(10,10);
  pt[3].set(10,80);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,80), il[1].seg0.pt);
}

//  p0q1
//  |
//  |q0
//  |
//  p1
TEST(LineLineIntersectionWithOverlap, OneSamePointInsideUp2) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,80);
  pt[3].set(10,10);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0].seg0.pt);
  ASSERT_EQ(TPoint(10,80), il[1].seg0.pt);
}

//  p0q1
//  |
//  |q0
//  |
//  p1
TEST(LineLineIntersectionWithOverlap, Crossing) {
  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(90,90);
  pt[2].set(90,10);
  pt[3].set(10,90);
  
  TIntersectionList il;
  intersectLineLine2(&il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(50,50), il[0].seg0.pt);
}
