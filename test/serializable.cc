#include <toad/io/serializable.hh>
#include <sstream>
#include "gtest.h"

using namespace std;
using namespace toad;

struct TestPointer: public TSerializable {
  typedef TSerializable super;
  string name;
  unsigned x, y;
  TestPointer *relation;
  void print() {
    cout << "TestPointer { name=\"" << name << "\", x=" << x << ", y=" << y;
    if (relation) {
      cout << ", relation=";
      relation->print();
    }
    cout << " }";
  }
  
  SERIALIZABLE_INTERFACE(, TestPointer);
};

void TestPointer::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "name",     name);
  ::store(out, "x",        x);
  ::store(out, "y",        y);
  ::storePointer(out, "relation", relation);
}

bool
TestPointer::restore(TInObjectStream &in)
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

TEST(Serializeable, Pointer) {
  TestPointer a0, a1;
  a0.name     = "Wirsing";
  a0.x        = 10;
  a0.y        = 20;
  a0.relation = &a1;

  a1.name     = "Kohl";
  a1.x        = 30;
  a1.y        = 40;
  a1.relation = nullptr;
  
  toad::getDefaultStore().registerObject(new TestPointer());

  // write
  ostringstream out;
  TOutObjectStream os(&out);
  EXPECT_NO_THROW({os.store(&a0);});
  EXPECT_NO_THROW({os.store(&a1);});
  os.close();
  
//  cout << out.str() << endl;

  // read
  istringstream in(out.str());
  TInObjectStream is(&in);
  
  vector<TestPointer*> c;
  TSerializable *s;
  while( (s = is.restore()) ) {
    if (s) {
      TestPointer *a = dynamic_cast<TestPointer*>(s);
      if (a) {
        c.push_back(a);
      }
    }
  }
  EXPECT_NO_THROW({is.close();});
  
//  c[0]->print();
//  cout << endl;
  
  ASSERT_EQ(c[1], c[0]->relation);
}

struct ReservedAttributeId: public TSerializable {
  typedef TSerializable super;
  string name;
  unsigned x, y;
  ReservedAttributeId *relation;
  void print() {
    cout << "ReservedAttributeId { name=\"" << name << "\", x=" << x << ", y=" << y;
    if (relation) {
      cout << ", relation=";
      relation->print();
    }
    cout << " }";
  }
  
  SERIALIZABLE_INTERFACE(, ReservedAttributeId);
};

void ReservedAttributeId::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "id", "hello");
}

bool
ReservedAttributeId::restore(TInObjectStream &in)
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



TEST(Serializeable, ReservedAttributeId)
{
  ReservedAttributeId a;

  ostringstream out;
  TOutObjectStream os(&out);
  ASSERT_THROW(os.store(&a), std::invalid_argument);
  os.close();
}
