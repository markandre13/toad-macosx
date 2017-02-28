#include <toad/io/serializable.hh>
#include <sstream>
#include "gtest.h"

using namespace std;
using namespace toad;

struct A: public TSerializable {
  typedef TSerializable super;
  string name;
  unsigned x, y;
  A *relation;
  void print() {
    cout << "A { name=\"" << name << "\", x=" << x << ", y=" << y;
    if (relation) {
      cout << ", relation=";
      relation->print();
    }
    cout << " }";
  }
  
  SERIALIZABLE_INTERFACE(, A);
};

class nullbuffer: public std::streambuf
{
  public:
    int overflow(int c) { return c; }
};

class nullstream: public std::ostream
{
  public:
    nullstream() : std::ostream(&buffer) {}
  private:
    nullbuffer buffer;
};

void A::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "name",     name);
  ::store(out, "x",        x);
  ::store(out, "y",        y);
  ::storePointer(out, "relation", relation);
}

bool
A::restore(TInObjectStream &in)
{
  if (
    super::restore(in) ||
    ::restore(in, "name",     &name) ||
    ::restore(in, "x",        &x) ||
    ::restore(in, "y",        &y) ||
    ::restorePointer(in, "relation", &relation)
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

  // prepare
  nullstream null;
  TOutObjectStream ns(&null);
  ns.setPass(0);
  ns.store(&a0);
  ns.store(&a1);

  // write
  ostringstream out;
  TOutObjectStream os(&out);
  os.setPass(1);
  os.store(&a0);
  os.store(&a1);
  
  cout << out.str() << endl;

  // read
  istringstream in(out.str());
  TInObjectStream is(&in);
//  is.setVerbose(true);
//  is.setDebug(true);
  
  vector<A*> c;
  TSerializable *s;
  while( (s = is.restore()) ) {
    if (s) {
      A *a = dynamic_cast<A*>(s);
      if (a) {
        c.push_back(a);
      }
    }
  }
  is.resolve();
  
  c[0]->print();
  cout << endl;
}
