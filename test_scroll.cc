// #include <toad/toad.hh>
#include <toad/table.hh>
#include <toad/checkbox.hh>
#include <toad/radiobutton.hh>
#include <toad/dialog.hh>
#include <toad/stl/vector.hh>
#include <toad/utf8.hh>

using namespace toad;

class TTestScrollWindow:
  public TWindow
{
  public:
    TTestScrollWindow(TWindow *parent, const string &title);
    void paint();
    void mouseLDown(const TMouseEvent&);
};

TTestScrollWindow::TTestScrollWindow(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  flagNoBackground = true;
}

void
TTestScrollWindow::mouseLDown(const TMouseEvent &me)
{
cout << "mouseLDown" << endl;
  scrollRectangle(TRectangle(20,20,150,150), 10, 30);
}

void
TTestScrollWindow::paint()
{
cout << "paint" << endl;
  TPen pen(this);
  pen.fillRectangle(5,5,100,50);
}

int
test_scroll()
{
cout << "test_scroll" << endl;
  TTestScrollWindow wnd(0, "TOAD Scroll");
  toad::mainLoop();
  return 0;
}
