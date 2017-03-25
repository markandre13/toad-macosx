#include <toad/io/serializable.hh>
#include <toad/types.hh>
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

// check store/restore of vector<TPoint>
// o store derivates from the atv format a bit and writes { x0 y0 x1 y1 ... }
//   to save space.
// o to parse this, restore sets interpreter to null and parses the list on
//   its own. afterwards, the previous interpreter must be able to resume
//   its work.
struct TestList: public TSerializable {
  typedef TSerializable super;
  string name;
  unsigned x, y;
  vector<TPoint> p;
  
  void print() {
    cout << "TestList { name=\"" << name << "\", x=" << x << ", y=" << y << ", p={"<<endl;
    for(auto &&pt: p)
      cout << "  " << pt << endl;
    cout << "}" << endl;
  }
  
  SERIALIZABLE_INTERFACE(, TestList);
};

void TestList::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "name", name);
  ::store(out, "p",    p);
  ::store(out, "x",    x);
  ::store(out, "y",    y);
}

bool
TestList::restore(TInObjectStream &in)
{
  if (
    super::restore(in) ||
    ::restore(in, "name",     &name) ||
    ::restore(in, "p",        &p) ||
    ::restore(in, "x",        &x) ||
    ::restore(in, "y",        &y)
  ) return true;
  ATV_FAILED(in)
  return false;
}

struct TestListContainer: public TSerializable {
  typedef TSerializable super;
  string name0, name1;
  TestList *l0, *l1;
  
  void print() {
    cout << "TestListContainer { name0=\"" << name0 << "\", name1=\"" << name1 << "\", " << endl;
    if (l0)
      l0->print();
    if (l1)
      l1->print();
    cout << "}" << endl;
  }
  
  SERIALIZABLE_INTERFACE(, TestListContainer);
};

void TestListContainer::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "name0", name0);
  ::store(out, "l0", l0);
  ::store(out, "l1", l1);
  ::store(out, "name1", name1);
}

bool
TestListContainer::restore(TInObjectStream &in)
{
  if (
    super::restore(in) ||
    ::restore(in, "name0",   &name0) ||
    ::restore(in, "name1",   &name1) ||
    ::restoreObject(in, "l0", &l0) ||
    ::restoreObject(in, "l1", &l1)
  ) return true;
  ATV_FAILED(in)
  return false;
}

TEST(Serializeable, List) {
  TestList l0;
  l0.name     = "Wirsing";
  l0.x        = 10;
  l0.y        = 20;
  l0.p.push_back(TPoint(1,2));
  l0.p.push_back(TPoint(3.1415,4));
  l0.p.push_back(TPoint(5,6));

  TestList l1;
  l1.name     = "Kohl";
  l1.x        = 30;
  l1.y        = 40;
  l1.p.push_back(TPoint(8,9));
  
  TestListContainer c;
  c.name0 = "Kartoffel";
  c.name1 = "Brei";
  c.l0 = &l0;
  c.l1 = &l1;
  
  toad::getDefaultStore().registerObject(new TestListContainer());
  toad::getDefaultStore().registerObject(new TestList());

  // write
  ostringstream out;
  TOutObjectStream os(&out);
  EXPECT_NO_THROW({os.store(&c);});
  os.close();
  
//  cout << out.str() << endl;

  // read
  istringstream in(out.str());
  TInObjectStream is(&in);
  
  TSerializable *s = is.restore();
  EXPECT_NO_THROW({is.close();});

  EXPECT_NE(nullptr, s);
  TestListContainer *a = dynamic_cast<TestListContainer*>(s);
  EXPECT_NE(nullptr, a);

  // a->print();
  
  // check value before list
  EXPECT_EQ("Kartoffel", a->name0);
  
  ASSERT_EQ("Wirsing", a->l0->name);
  ASSERT_DOUBLE_EQ(10, a->l0->x);
  ASSERT_DOUBLE_EQ(20, a->l0->y);
  ASSERT_EQ(3, a->l0->p.size());
  ASSERT_DOUBLE_EQ(1, a->l0->p[0].x);
  ASSERT_DOUBLE_EQ(2, a->l0->p[0].y);
  ASSERT_DOUBLE_EQ(3.1415, a->l0->p[1].x);
  ASSERT_DOUBLE_EQ(4, a->l0->p[1].y);
  ASSERT_DOUBLE_EQ(5, a->l0->p[2].x);
  ASSERT_DOUBLE_EQ(6, a->l0->p[2].y);

  ASSERT_EQ("Kohl", a->l1->name);
  ASSERT_DOUBLE_EQ(30, a->l1->x);
  ASSERT_DOUBLE_EQ(40, a->l1->y);
  ASSERT_EQ(1, a->l1->p.size());
  ASSERT_DOUBLE_EQ(8, a->l1->p[0].x);
  ASSERT_DOUBLE_EQ(9, a->l1->p[0].y);
  
  // check value after list
  EXPECT_EQ("Brei", a->name1);
  
  delete s;
  
//  c[0]->print();
}
