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

unsigned outPass = 0;
unsigned outId = 0;
map<const TSerializable*, unsigned> outIdMap;

void
storeRef(TOutObjectStream &out, const char *attribute, const TSerializable *obj)
{
  switch(outPass) {
    case 0: {
      map<const TSerializable*, unsigned>::const_iterator p = outIdMap.find(obj);
      if (p==outIdMap.end())
        outIdMap[obj] = ++outId;
    } break;
    case 1: {
      out.indent();
      if (obj) {
        out << attribute << " = " << outIdMap[obj];
      } else {
        out << attribute << " = " << "null";
      }
    } break;
  }
}

void A::store(TOutObjectStream &out) const
{
  if (outPass==1) {
    map<const TSerializable*, unsigned>::const_iterator p = outIdMap.find(this);
    if (p!=outIdMap.end()) {
      out.indent();
      out << "id = " << p->second;
    }
  }
  ::store(out, "name",     name);
  ::store(out, "x",        x);
  ::store(out, "y",        y);
  ::storeRef(out, "relation", relation);
}

// list of ids and their objects
map<unsigned, const TSerializable*> inIdMap;

// list of ids and pointer toward their objects
map<unsigned, vector<TSerializable**>> inRefMap;

template <class T> bool
restoreRef(TInObjectStream &in, const char *attribute, T **ptr)
{
  unsigned id;
  if (!::restore(in, attribute, &id))
    return false;
  inRefMap[id].push_back(reinterpret_cast<TSerializable**>(ptr));
  return true;
}

void
resolveRef()
{
  for(auto &p: inRefMap) {
    for(auto &q: p.second) {
      if (p.first) {
        *q = const_cast<TSerializable*>(inIdMap[p.first]);
      } else {
        *q = nullptr;
      }
    }
  }
}

bool
A::restore(TInObjectStream &in)
{
  unsigned id;
  if (::restore(in, "id", &id)) {
    inIdMap[id] = this;
    return true;
  }
  
  if (
    ::restore(in, "name",     &name) ||
    ::restore(in, "x",        &x) ||
    ::restore(in, "y",        &y) ||
    ::restoreRef(in, "relation", &relation) ||
    super::restore(in)
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
  outPass = 0;
  nullstream null;
  TOutObjectStream ns(&null);
  ns.store(&a0);
  ns.store(&a1);

  // write
  outPass = 1;
  ostringstream out;
  TOutObjectStream os(&out);
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
  resolveRef();
  
  c[0]->print();
  cout << endl;
}
