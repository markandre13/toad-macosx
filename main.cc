#include <toad/core.hh>
#include <toad/pushbutton.hh>
#include <toad/scrollbar.hh>
#include <toad/checkbox.hh>
#include <toad/textarea.hh>

using namespace toad;

@interface MyDelegate : NSObject
{
}
- (void) createWindow;
- (void) createMenu;  
- (void) applicationWillFinishLaunching:(NSNotification *)notification;
- (void) applicationDidFinishLaunching:(NSNotification *)notification;
@end

@implementation MyDelegate : NSObject
- (void) dealloc
{
  [super dealloc];
  // RELEASE (myWindow);
}

- (void) createWindow
{
  TWindow::createParentless();
}

// this one creates a simple custom menu without NIB files
- (void) createMenu
{
  printf("create menu\n");
  NSMenu *m0, *m1;

  // menu = AUTORELEASE ([NSMenu new]);
  // m0 = [NSApp mainMenu];
  m0 = [NSMenu new];
  
  m1 = [NSMenu new];
  [m1 setTitle: @"myMenu"];
  [m1 setAutoenablesItems:YES];
  
  [m0 addItemWithTitle: @"myMenu" action: NULL keyEquivalent: @""];
  [m0 setSubmenu: m1 forItem: [m0 itemWithTitle: @"myMenu"]];
      
  [m1 addItemWithTitle: @"Quit" action: @selector (terminate:) keyEquivalent: @"q"];

  [NSApp setMainMenu: m0];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification;
{
  [self createWindow];
}

- (void) applicationDidFinishLaunching: (NSNotification *)notification;
{
  [self createMenu];  
}
@end

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *parent, const string &title);
    void paint();
};

class TMyWindow2:
  public TWindow
{
  public:
  TMyWindow2(TWindow *parent, const string &title):
    TWindow(parent, title)
  {}
  void paint() {
    TPen pen(this);

    pen.setColor(1,0,0);
    pen.drawRectanglePC(0,0,30,30);
    pen.setColor(0,0,1);
    TPoint p[5];
    p[0].set( 1, 1);
    p[1].set(28, 1);
    p[2].set(28,28);
    p[3].set( 1,28);
    p[4].set( 1, 1);
    pen.drawLines(p,5);

    pen.setColor(0,1,0);
/*
    TRectangle r(40,40,10,10);
    pen&=r;
    r.set(50,50,10,10);
    pen|=r;
*/
    pen.fillRectangle(-10,-10,320,200);
  }
};

void foo() {}

TMyWindow::TMyWindow(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  TPushButton *btn = new TPushButton(this, "  OK  ");
  btn->setShape(5,170,100,25);
  connect(btn->sigClicked, &foo);
  
  TWindow *w = new TMyWindow2(this, "x");
  w->setShape(20,100,30,30);
  
  w = new TScrollBar(this, "sb");
  w->setShape(300,5,17,190);
  
  w = new TCheckBox(this, "Check it!");
  w->setShape(100,50,200,40);

  w = new TTextArea(this, "text");
  w->setShape(110, 100, 160, 95);
}

void
TMyWindow::paint()
{
#if 0
  TPen pen(this);

  printf("TMyWindow::paint\n");
  TCoord x,y,w,h;
  x=5; y=20; w=50; h=10;

  NSRect r = NSMakeRect(x+0.5,y+0.5,w,h);
  [NSBezierPath strokeRect: r];
              
#else
  TPen pen(this);
  pen.drawRectanglePC(19,99,32,32);
  pen.setColor(1,0,0);
  pen.drawRectangle(0,0,5,5);
  pen.setColor(0,1,0);
  pen.drawRectangle(5,5,50,50);
  pen.setColor(0,0,1);
  pen.drawString(55,55,"Hello Guys...!");
#endif
}

int 
main(int argc, char *argv[])
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];

  printf("Application started\n");
  // setup NSApp
  [NSApplication sharedApplication];

  // add a delegat to NSApp to customize the application
  [NSApp setDelegate: [MyDelegate new]];

//  TWindow wnd0(NULL, "Cocoa TOAD Button Test: Gemüse");
//  TPushButton wnd(NULL, "Cocoa TOAD Button Test: Äpfel");
  TMyWindow wnd(0, "Cocoa TOAD Test");
/*
  TWindow wnd(0, "Cocoa TOAD Test");
  // wnd.setSize(340,240);
  TWindow *w = new TMyWindow2(&wnd, "clip");
  w->setShape(10,10,300,180);
*/
//  connect(wnd.sigClicked, &foo);
  
  int r = NSApplicationMain(argc,  (const char **) argv);
  
  [pool release];
  
  return r;
}
