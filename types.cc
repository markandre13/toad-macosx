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
    TATVInterpreter *interpreter = in.getInterpreter();
    in.setInterpreter(nullptr);
    while(true) {
      TCoord x, y;

      in.parse();
      if (in.what==ATV_FINISHED)
        break;
      if (in.what!=ATV_VALUE || !in.attribute.empty() || !in.type.empty()) {
        ATV_FAILED(in)
        in.setInterpreter(interpreter);
        return false;
      }
      ::restore(in, &x);

      in.parse();
      if (in.what!=ATV_VALUE || !in.attribute.empty() || !in.type.empty()) {
        ATV_FAILED(in)
        in.setInterpreter(interpreter);
        return false;
      }
      ::restore(in, &y);

      p->push_back(TPoint(x, y));
    }
    in.setInterpreter(interpreter);
    return true;
  }
  return false;
}
