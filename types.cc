#include <toad/types.hh>

#include <limits>

using namespace toad;

// distance of line (p0, p1) to point q
TCoord
toad::distance(const TPoint &q, const TPoint &p0, const TPoint &p1)
{
  TPoint b(p1-p0);
  TPoint a(q-p0);
  
  TCoord lb = squaredLength(b);
  TCoord t = dot(a, b) / lb;
  if (t<0.0 || t>1.0)
    return numeric_limits<TCoord>::infinity();
  return fabs(b.y * a.x - b.x * a.y) / sqrt(lb);
}


void
toad::store(TOutObjectStream &out, const vector<TPoint> &p)
{
  out << "{ ";
  for(auto &&pt: p)
    out << pt.x << " " << pt.y << " ";
  out << "}";
}

bool
toad::restore(TInObjectStream &in, vector<TPoint> *p)
{
  if (in.what == ATV_GROUP &&
      in.type.empty())
  {
    in.setInterpreter(nullptr);
    p->clear();
    while(true) {
      TCoord x, y;

      in.parse();
      if (in.what==ATV_FINISHED)
        break;
      if (in.what!=ATV_VALUE || !in.attribute.empty() || !in.type.empty()) {
        ATV_FAILED(in)
        return false;
      }
      ::restore(in, &x);

      in.parse();
      if (in.what!=ATV_VALUE || !in.attribute.empty() || !in.type.empty()) {
        ATV_FAILED(in)
        return false;
      }
      ::restore(in, &y);

      p->push_back(TPoint(x, y));
    }
    return true;
  }
  return false;
}
