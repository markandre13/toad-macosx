#include <toad/types.hh>
#include <iostream>

#include "gtest.h"

using namespace toad;
using namespace std;

class TRectangle2
{
  public:
    TPoint origin;
    TSize size;
    
    TRectangle2(const TRectangle2 &rectangle): origin(rectangle.origin), size(rectangle.size) {
      cout << "copy constructed rectangle" << endl;
    }
    TRectangle2(const TRectangle2 &&rectangle): origin(rectangle.origin), size(rectangle.size) {
      cout << "move constructed rectangle" << endl;
    }
    TRectangle2& operator=(TRectangle2 &&rectangle) {
      cout << "move assigned rectangle" << endl;
      return *this;
    }
    
    TRectangle2(const TPoint &p0, const TPoint &p1) {
      cout << "construct rectangle from two points" << endl;
      if (p0.x < p1.x) {
        origin.x = p0.x;
        size.width = p1.x - p0.x;
      }
      if (p0.y < p1.y) {
        origin.y = p0.y;
        size.height = p1.y - p0.y;
      }
    }
    TRectangle2& translate(const TPoint point) {
      cout << "translate rectangle" << endl;
      origin += point;
      return *this;
    }
    TRectangle2& expand(TCoord expansion) {
      cout << "expand rectangle" << endl;
      origin.x -= expansion;
      origin.y -= expansion;
      expansion *= 2.0;
      size.width  += expansion;
      size.height += expansion;
      return *this;
    }
};

inline ostream& operator<<(ostream &s, const TRectangle2& r) {
  return s<<'('<<r.origin.x<<','<<r.origin.y<<','<<r.size.width<<','<<r.size.height<<')';
}

TEST(Rectangle, foo)
{
  cout << TRectangle2(TPoint(10, 20), TPoint(20, 40)).translate(TPoint(1, 2)).expand(4) << endl;
}
