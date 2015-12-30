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
  // this replaces NSApplicationMain(global_argc,  (const char **) global_argv);

// sometimes we got the busy caret after the application was drawn
// on the screen. inspector couldn't locate the source so this is
// another try to hunt it down. (doesn't hang so far thus...)
//cerr << __FILE__ << ":" << __LINE__ << endl;
  NSApplication *app = [NSApplication sharedApplication];
//cerr << __FILE__ << ":" << __LINE__ << endl;
  [[NSBundle mainBundle] loadNibNamed:@"myMain" owner:app topLevelObjects:nil];

//cerr << __FILE__ << ":" << __LINE__ << endl;

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//cerr << __FILE__ << ":" << __LINE__ << endl;
 
  [app finishLaunching];
//cerr << __FILE__ << ":" << __LINE__ << endl;
 
  do {
    [pool release];
    pool = [[NSAutoreleasePool alloc] init];
 
//cerr << __FILE__ << ":" << __LINE__ << endl;
    NSEvent *event =
      [app
         nextEventMatchingMask:NSAnyEventMask
         untilDate:[NSDate distantFuture]
         inMode:NSDefaultRunLoopMode
         dequeue:YES];
//cerr << __FILE__ << ":" << __LINE__ << endl;
 
    [app sendEvent:event];
//cerr << __FILE__ << ":" << __LINE__ << endl;
    [app updateWindows];
//cerr << __FILE__ << ":" << __LINE__ << endl;
    executeMessages();
  } while(true);
//cerr << __FILE__ << ":" << __LINE__ << endl;
  [pool release];
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
