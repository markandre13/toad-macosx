#include <toad/geometry.hh>
#include "gtest.h"

using namespace toad;

TEST(FitCurve, NoCurve001) {
  TPoint a[] = {
    { 10, 10 },
    { 90, 30 },
    { 10, 50 },
    { 90, 70 },
    { 10, 90 },
  };
  
  TVectorPath path;
  path.move(a[0]);
  for(size_t i=1; i<5; ++i) {
    path.line(a[i]);
  }
  path.simplify();

  ASSERT_EQ(5, path.type.size());
  ASSERT_EQ(TVectorPath::MOVE, path.type[0]);
  ASSERT_EQ(TVectorPath::LINE, path.type[1]);
  ASSERT_EQ(TVectorPath::LINE, path.type[2]);
  ASSERT_EQ(TVectorPath::LINE, path.type[3]);
  ASSERT_EQ(TVectorPath::LINE, path.type[4]);
  
  ASSERT_EQ(5, path.points.size());
  ASSERT_EQ(a[0], path.points[0]);
  ASSERT_EQ(a[1], path.points[1]);
  ASSERT_EQ(a[2], path.points[2]);
  ASSERT_EQ(a[3], path.points[3]);
  ASSERT_EQ(a[4], path.points[4]);
}

TEST(FitCurve, NoCurve002) {
  TPoint a[] = {
    { 10, 10 },
    { 90, 30 },
    { 10, 50 },
    { 90, 70 },
    {170, 90 },
  };
  
  TVectorPath path;
  path.move(a[0]);
  for(size_t i=1; i<5; ++i) {
    path.line(a[i]);
  }
  path.simplify();

  ASSERT_EQ(4, path.type.size());
  ASSERT_EQ(TVectorPath::MOVE, path.type[0]);
  ASSERT_EQ(TVectorPath::LINE, path.type[1]);
  ASSERT_EQ(TVectorPath::LINE, path.type[2]);
  ASSERT_EQ(TVectorPath::CURVE, path.type[3]);
  
  ASSERT_EQ(6, path.points.size());
  ASSERT_EQ(a[0], path.points[0]);
  ASSERT_EQ(a[1], path.points[1]);
  ASSERT_EQ(a[2], path.points[2]);
//  ASSERT_EQ(TPoint(63.3333,63.3333), path.points[3]);
//  ASSERT_EQ(TPoint(116.667,76.6667), path.points[4]);
  ASSERT_EQ(a[4], path.points[5]);
}

TEST(FitCurve, NoCurve003) {
  TPoint a[] = {
    { 10, 10 },
    { 90, 30 },
    { 10, 50 },
    { 90, 70 },
    {170, 90 },
  };
  
  TVectorPath path;
  path.move(a[0]);
  for(size_t i=1; i<5; ++i) {
    path.line(a[i]);
  }
  path.simplify();

  ASSERT_EQ(4, path.type.size());
  ASSERT_EQ(TVectorPath::MOVE, path.type[0]);
  ASSERT_EQ(TVectorPath::LINE, path.type[1]);
  ASSERT_EQ(TVectorPath::LINE, path.type[2]);
  ASSERT_EQ(TVectorPath::CURVE, path.type[3]);
  
  ASSERT_EQ(6, path.points.size());
  ASSERT_EQ(a[0], path.points[0]);
  ASSERT_EQ(a[1], path.points[1]);
  ASSERT_EQ(a[2], path.points[2]);
//  ASSERT_EQ(TPoint(63.3333,63.3333), path.points[3]);
//  ASSERT_EQ(TPoint(116.667,76.6667), path.points[4]);
  ASSERT_EQ(a[4], path.points[5]);
}
