#include <toad/connect.hh>

#include "gtest.h"

using namespace toad;

class TReceiver:  
  public TSlot
{
};
   
class TGiver
{
  public:
    TSignal signal;
};

TEST(Signal, SingleSlot)
{
  TGiver giver;
  ASSERT_EQ(0, giver.signal.size());
  {  
    TReceiver receiver;
    connect(giver.signal, &receiver, [] {
    });
    ASSERT_EQ(1, giver.signal.size());
  }
  ASSERT_EQ(0, giver.signal.size());
}

/*
static inline TSignalLink* connect(TSignal &signal, std::vector<TSlot*> slots, std::function<void()> closure) {     
  ...?
}          

TEST(Signal, ListOfSlots)
{
  TGiver giver;
  ASSERT_EQ(0, giver.signal.size());
  {  
    TReceiver receiver1;
    {
      TReceiver receiver2;
      connect(giver.signal, { &receiver1, &receiver2 }, [] {
      });
      ASSERT_EQ(1, giver.signal.size());
      ASSERT_EQ(1, receiver1.slots.size());
      ASSERT_EQ(1, receiver2.slots.size());
    }
    ASSERT_EQ(0, giver.signal.size());
    ASSERT_EQ(0, receiver1.slots.size());
  }
  ASSERT_EQ(0, giver.signal.size());
}
*/
