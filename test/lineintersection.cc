#include <toad/geometry.hh>
#include "gtest.h"

using namespace toad;

void intersectLineLine2(vector<TPoint> &ilist, const TPoint *l0, TPoint *l1);

//  p0
//  |
//  |q0
//  |  \
//  |   \
//  p1   q1
TEST(LineLineIntersectionWithOvelap, OnePoint) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,20);
  pt[3].set(20,90);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,20), il[0]);
}

// p0
// |
// |q0
// |
// |q1
// |
// p1
TEST(LineLineIntersectionWithOvelap, TwoPointInside) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,20);
  pt[3].set(10,80);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,20), il[0]);
  ASSERT_EQ(TPoint(10,80), il[1]);
}

// p0
// |
// |q0
// |
// |p1
// |
// q1
TEST(LineLineIntersectionWithOvelap, TwoPointBefore) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,50);
  pt[2].set(10,20);
  pt[3].set(10,60);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,20), il[0]);
  ASSERT_EQ(TPoint(10,50), il[1]);
}

// q0
// |
// |p0
// |
// |q1
// |
// p1
TEST(LineLineIntersectionWithOvelap, TwoPointAfter) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,20);
  pt[1].set(10,60);
  pt[2].set(10,10);
  pt[3].set(10,50);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,20), il[0]);
  ASSERT_EQ(TPoint(10,50), il[1]);
}

//  p0q0
//  |  \
//  |   \
//  p1   q1
TEST(LineLineIntersectionWithOvelap, OneSamePointDown) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,10);
  pt[3].set(20,90);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
}

//  p1q0
//  |  \
//  |   \
//  p0   q1
TEST(LineLineIntersectionWithOvelap, OneSamePointUp) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,90);
  pt[1].set(10,10);
  pt[2].set(10,10);
  pt[3].set(20,90);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
}

//  p0q1
//  |  \
//  |   \
//  p1   q0
TEST(LineLineIntersectionWithOvelap, OneSamePointUp2) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(20,90);
  pt[3].set(10,10);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
}

//  p0q0
//  |
//  |
//  p1q1
TEST(LineLineIntersectionWithOvelap, TwoSamePointDown) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,10);
  pt[3].set(10,90);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
  ASSERT_EQ(TPoint(10,90), il[1]);
}

//  p0q1
//  |
//  |
//  p1q0
TEST(LineLineIntersectionWithOvelap, TwoSamePointUp) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,90);
  pt[3].set(10,10);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
  ASSERT_EQ(TPoint(10,90), il[1]);
}

//  p0q0
//  |
//  |q1
//  |
//  p1
TEST(LineLineIntersectionWithOvelap, OneSamePointInsideDown) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,10);
  pt[3].set(10,80);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
  ASSERT_EQ(TPoint(10,80), il[1]);
}

//  p1q0
//  |
//  |q1
//  |
//  p0
TEST(LineLineIntersectionWithOvelap, OneSamePointInsideUp) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,90);
  pt[1].set(10,10);
  pt[2].set(10,10);
  pt[3].set(10,80);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
  ASSERT_EQ(TPoint(10,80), il[1]);
}

//  p0q1
//  |
//  |q0
//  |
//  p1
TEST(LineLineIntersectionWithOvelap, OneSamePointInsideUp2) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(10,90);
  pt[2].set(10,80);
  pt[3].set(10,10);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(2, il.size());
  ASSERT_EQ(TPoint(10,10), il[0]);
  ASSERT_EQ(TPoint(10,80), il[1]);
}

//  p0q1
//  |
//  |q0
//  |
//  p1
TEST(LineLineIntersectionWithOvelap, Crossing) {
  vector<TPoint> il;

  TPoint pt[4];
  pt[0].set(10,10);
  pt[1].set(90,90);
  pt[2].set(90,10);
  pt[3].set(10,90);
  
  intersectLineLine2(il, pt, pt+2);
  
  ASSERT_EQ(1, il.size());
  ASSERT_EQ(TPoint(50,50), il[0]);
}
