#include <toad/io/serializable.hh>
#include <sstream>
#include "gtest.h"

using namespace std;
using namespace toad;

struct A: public TSerializable {
  string name;
  unsigned x, y;
  A *relation;
  SERIALIZABLE_INTERFACE(, A);
/*
 TCloneable* clone() const { return new A(*this); }
 const char * getClassName() const { return "A"; }
 void store(TOutObjectStream &out) const override;
 bool restore(TInObjectStream &in) override;
*/
};

void A::store(TOutObjectStream &out) const
{
  ::store(out, "name",     name);
  ::store(out, "x",        x);
  ::store(out, "y",        y);
//  ::store(out, "relation", relation);
}

bool
A::restore(TInObjectStream &in)
{
  if (
    ::restore(in, "name",     &name) ||
    ::restore(in, "x",        &x) ||
    ::restore(in, "y",        &y) ||
    ::restore(in, "relation", &x)
  ) return true;
  ATV_FAILED(in)
  return false;
}

TEST(Serializeable, References) {
  A a0, a1;
  a0.name     = "Wirsing";
  a0.x        = 10;
  a0.y        = 20;
  a0.relation = &a1;

  a1.name     = "Kohl";
  a1.x        = 30;
  a1.y        = 40;
  a1.relation = nullptr;
  
  toad::getDefaultStore().registerObject(new A());

  ostringstream out("test.atv");
  TOutObjectStream os(&out);
  os.store(&a0);
  os.store(&a1);
  
  cout << out.str() << endl;
}
