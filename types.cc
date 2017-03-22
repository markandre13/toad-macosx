#include <toad/types.hh>

using namespace toad;

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
