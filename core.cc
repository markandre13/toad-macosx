#include <toad/core.hh>
#include <toad/figure.hh>
#include <toad/command.hh>
#include <toad/dialogeditor.hh>

#include "fischland/fontdialog.hh"

bool toad::layouteditor = false;

using namespace toad;

@interface ToadDelegate : NSObject <NSFileManagerDelegate>
{
}
- (void) createWindow;
- (void) createMenu;  
- (void) applicationWillFinishLaunching:(NSNotification *)notification;
- (void) applicationDidFinishLaunching:(NSNotification *)notification;
- (void) windowWillMove:(NSNotification *)notification;
- (void) windowDidMove:(NSNotification *)notification;
@end

@implementation ToadDelegate : NSObject
- (void) dealloc
{
  TOAD_DBG_ENTER
  [super dealloc];
  // RELEASE (myWindow);
  TOAD_DBG_LEAVE
}

- (void) createWindow
{
  TOAD_DBG_ENTER
  TWindow::createParentless();
  TOAD_DBG_LEAVE
}

// this one creates a simple custom menu without NIB files
- (void) createMenu
{
  TOAD_DBG_ENTER
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
  TOAD_DBG_LEAVE
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification;
{
  TOAD_DBG_ENTER
  [self createWindow];
  TOAD_DBG_LEAVE
}

- (void) applicationDidFinishLaunching: (NSNotification *)notification;
{
  TOAD_DBG_ENTER
  [self createMenu];
  TOAD_DBG_LEAVE
}

- (void) windowWillMove: (NSNotification *)notification;
{
  TOAD_DBG_ENTER
  TWindow::_windowWillMove(notification);
  TOAD_DBG_LEAVE
}

- (void) windowDidMove: (NSNotification *)notification;
{
  TOAD_DBG_ENTER
  TWindow::_windowDidMove(notification);
  TOAD_DBG_LEAVE
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

  // setup NSApp
  [NSApplication sharedApplication];

  // add a delegate to NSApp to customize the application
  ToadDelegate *delegate = [ToadDelegate new];
  [NSApp setDelegate: delegate];
  
  [[NSNotificationCenter defaultCenter] addObserver:delegate
      selector:@selector(windowWillMove:)
          name:NSWindowWillMoveNotification object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:delegate
      selector:@selector(windowDidMove:)
          name:NSWindowDidMoveNotification object:nil];

  bool layouteditor = false;
  for(int i=1; i<argc; i++) {
    if (strcmp(argv[i], "--layout-editor")==0) {
      layouteditor = true;
    } else {
      cerr << "unknown option " << argv[i] << endl;
    }
  }

  if (layouteditor)
    new TDialogEditor();
}

bool
toad::mainLoop()
{
  bool r = NSApplicationMain(global_argc,  (const char **) global_argv);
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
