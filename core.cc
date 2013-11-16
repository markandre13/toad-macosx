#include <toad/core.hh>
#include <toad/figure.hh>
#include <toad/command.hh>

#include "fischland/fontdialog.hh"

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
  printf("createMenu: create simple NSMenu\n");
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

static NSAutoreleasePool *pool = 0;
static int global_argc = 0;
static char **global_argv = 0;

void 
toad::initialize(int argc, char *argv[])
{
  global_argc = argc;
  global_argv = argv;

  TFigure::initialize();

  pool = [NSAutoreleasePool new];

  printf("Application started.\n");
  // setup NSApp
  [NSApplication sharedApplication];

  // add a delegat to NSApp to customize the application
  [NSApp setDelegate: [MyDelegate new]];

//  TWindow wnd0(NULL, "Cocoa TOAD Button Test: Gemüse");
//  TPushButton wnd(NULL, "Cocoa TOAD Button Test: Äpfel");
//  TMyWindow wnd(0, "Cocoa TOAD Test");
/*
  TWindow wnd(0, "Cocoa TOAD Test");
  // wnd.setSize(340,240);
  TWindow *w = new TMyWindow2(&wnd, "clip");
  w->setShape(10,10,300,180);
*/
//  connect(wnd.sigClicked, &foo);
}

bool
toad::mainLoop()
{
  int r = NSApplicationMain(global_argc,  (const char **) global_argv);
  return r;
}

void
toad::terminate()
{
  [pool release];
}

class TCommandDelayedTrigger:
  public TCommand
{     
    TSignal *signal;
  public:
    TCommandDelayedTrigger(TSignal *s):signal(s){}
    void execute() {
#ifdef TOAD_SECURE
      signal->delayedtrigger--;
#endif
      signal->trigger();
    }
};

bool
TSignal::delayedTrigger()
{
  if (!_list) return false;
  sendMessage(new TCommandDelayedTrigger(this));
  return true;
}

bool
toad::modalLoop(toad::TWindow *wnd)
{
  wnd->doModalLoop();
  return true;
}

TFontDialog::TFontDialog(TWindow *parent, const string &title):
  TDialog(parent, title) {}
TFontDialog::~TFontDialog() {}
void TFontDialog::setFont(const string &name) {}
void TFontDialog::button(unsigned) {}
