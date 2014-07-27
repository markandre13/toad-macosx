#include <toad/window.hh>
#include <toad/simpletimer.hh>

using namespace toad;

class TMyTimer:
  public TSimpleTimer
{
  public:
    void tick() {
      cout << "tick" << endl;
    }
};

int
test_timer()
{
cout << "test_timer" << endl;
  TWindow wnd(0, "TOAD Timer");
  TMyTimer t0;
  t0.startTimer(1);
  toad::mainLoop();
  return 0;
}
