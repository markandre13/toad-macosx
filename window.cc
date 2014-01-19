/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2006 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307,  USA
 */

#include <toad/core.hh>
#include <toad/layout.hh>
#include <toad/focusmanager.hh>
#include <toad/io/urlstream.hh>
#include <toad/command.hh>

#include <set>

/**
 * \class toad::TWindow
 * \extends toad::TInteractor
 * \extends toad::TRectangle
 *
 * The rectangular frame which can be drawn into and which also receives
 * input from the mouse pointer, keyboard, etc.
 */

using namespace toad;

TWindow*
TWindow::getParent() const
{
  TInteractor *p = TInteractor::getParent();
  if (!p)
    return NULL;
  TWindow *w = dynamic_cast<TWindow*>(p);
  if (!w) {
    std::cerr << "fatal: window '"<<getTitle()<<"' has parent '"<<p->getTitle()<<"' ("<<p<<") which is not of type TWindow"<<endl;
  }
  return w;
}


typedef vector<TWindow*> TVectorParentless;
static TVectorParentless parentless;

void TWindow::paint()
{
}

void
TWindow::doResize()
{
  if (flag_wm_resize)
    return;
  flag_wm_resize = true;
  if (layout) {
    layout->arrange();
  }
  resize();
//  if (getParent()) {
//     _childNotify(TCHILD_RESIZE);
//  }
  flag_wm_resize = false;
}

void TWindow::resize() {}
void
TWindow::keyEvent(TKeyEvent &ke)
{
  switch(ke.type) {
    case TKeyEvent::DOWN:
//printf("key down in %s (%u)\n", getTitle().c_str(), (unsigned)ke.getKey());
      keyDown(ke.getKey(), const_cast<char*>(ke.getString()), ke.modifier());
      break;
    case TKeyEvent::UP:
//printf("key up in %s\n", getTitle().c_str());
      keyUp(ke.getKey(), const_cast<char*>(ke.getString()), ke.modifier());
      break;
  }
}  
   
void TWindow::keyDown(TKey,char*,unsigned){}
void TWindow::keyUp(TKey,char*,unsigned){}  

bool
TWindow::isRealized() const
{
  return nsview!=nil;
}

bool
TWindow::setFocus()
{
  TFocusManager::setFocusWindow(this);
  return TFocusManager::getFocusWindow()==this;
}

void
TWindow::_setFocusHelper(TInteractor *parent, bool b)
{
  parent->focus(b);
  TInteractor *p = parent->getFirstChild();
  while(p) {
    p->focus(b);
    p = p->getNextSibling();
  }
}
   
/**
 * Toggles the <VAR>_bOwnsFocus</VAR> flag.
 */ 
void
TWindow::_setFocus(bool b)
{
  if (b==_bOwnsFocus)
    return;
  _bOwnsFocus = b;
  _setFocusHelper(this, b);
/*
  if (getParent())
    getParent()->childNotify(this, TCHILD_FOCUS);
*/
}

void
TWindow::grabMouse(bool allmove, TWindow *confine, TCursor::EType type)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TWindow::grabPopupMouse(bool allmove, TCursor::EType type)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}
void
TWindow::ungrabMouse()
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TWindow::getRootPos(int *x,int *y)
{
  TWindow *p = this;
  *x = 0; // this->x;
  *y = 0; // this->y;
  while(p) {
    *x += p->x;
    *y += p->y;
    p = p->getParent();
    if (!p)
      break;
    if (p->nswindow) {
      NSPoint np;
      np.x = *x;
      np.y = p->h-*y;
      np = [p->nswindow convertBaseToScreen: np];
cout << "nswindow " << p->getTitle() << " on screen " << np.x << ", " << np.y << endl;
      *x = np.x;
      *y = np.y;
      return;
    }
cout << "toad window " << p->getTitle() << " on screen " << p->x << ", " << p->y << endl;
  }
}

void
TWindow::placeWindow(EWindowPlacement how, TWindow *parent)
{
  int px, py;
  switch(how) {
    case PLACE_SCREEN_CENTER:
      break;
    case PLACE_SCREEN_RANDOM:
      break;
    case PLACE_PARENT_CENTER:
      break;
    case PLACE_PARENT_RANDOM: 
      break;
    case PLACE_MOUSE_POINTER:
      break;
    case PLACE_CORNER_MOUSE_POINTER:
      break;
    case PLACE_PULLDOWN: {
      int rx, ry;
      parent->getRootPos(&rx, &ry);
      x=rx; y=ry;
      y-=parent->h; // below parent
      y-=h;
    } break;
    case PLACE_TOOLTIP:
      break;
  }
}

/**
 * I am not quite sure why I did add this method... [MAH]
 */
void
TWindow::windowEvent(TWindowEvent &we)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
  switch(we.type) {
    case TWindowEvent::NEW:
      break;
    case TWindowEvent::DELETE:
      break;
    case TWindowEvent::CREATE:
      // create();
    case TWindowEvent::CREATED:
      // created();
      break;
    case TWindowEvent::DESTROY:
      // destroy();
      break;
    case TWindowEvent::MAPPED:
      break;
    case TWindowEvent::UNMAPPED:
      break;
    case TWindowEvent::PAINT:
      paint();
      break;  
    case TWindowEvent::ADJUST:
      // adjust();
      break;   
    case TWindowEvent::RESIZE:
      doResize();
      break;
    case TWindowEvent::FOCUS:
      focus(isFocus());
      break;
  }
}  

void
TWindow::mouseEvent(TMouseEvent &me)
{
  if (layout && layout->mouseEvent(me))
    return;

  switch(me.type) {
    case TMouseEvent::MOVE:
      mouseMove(me);
      break;
    case TMouseEvent::ENTER:
      mouseEnter(me);
      break;
    case TMouseEvent::LEAVE:
      mouseLeave(me);
      break;
    case TMouseEvent::LDOWN:
      mouseLDown(me);
      break;
    case TMouseEvent::MDOWN:
      mouseMDown(me);
      break;
    case TMouseEvent::RDOWN:
      mouseRDown(me);
      break;
    case TMouseEvent::LUP:
      mouseLUp(me);
      break;
    case TMouseEvent::MUP:
      mouseMUp(me);
      break;
    case TMouseEvent::RUP:
      mouseRUp(me);
      break;
  }
}  

//! See etMouseMoveMessages' when you need mouseMove.
void TWindow::mouseMove(TMouseEvent &){}
void TWindow::mouseEnter(TMouseEvent &){}
void TWindow::mouseLeave(TMouseEvent &){}

//! Called when the left mouse button is pressed. Since X11 performs an
//! automatic mouse grab you will receive a mouseLUp message afterwards
//! unless you call UngrabMouse().
void TWindow::mouseLDown(TMouseEvent &){}
//! Same as mouseLDown for the middle mouse button.
void TWindow::mouseMDown(TMouseEvent &){}
//! Same as mouseLDown for the right mouse button.
void TWindow::mouseRDown(TMouseEvent &){}
void TWindow::mouseLUp(TMouseEvent &){}
void TWindow::mouseMUp(TMouseEvent &){}
void TWindow::mouseRUp(TMouseEvent &){}
                                                                                                                                                                                                 
unsigned
TWindow::getParentlessCount()
{
  return parentless.size();
}

TWindow*    
TWindow::getParentless(unsigned i)
{
  if (i>parentless.size())
    return NULL;
  return parentless[i];
}

/**
 * Create all parentless windows.
 *
 * Windows with bExplicitCreate == true won't be created.
 *
 * This message is invoked by TOADBase::runApp.
 *
 * \return 'false' when there are no parentless windows.
 */
bool
TWindow::createParentless()
{
  TVectorParentless::iterator p = parentless.begin();
  TVectorParentless::iterator e = parentless.end();  
  if (p==e)
    return false;
  
  while(p!=e) {
    assert(*p != 0);
    if (!(*p)->flagExplicitCreate) {
      (*p)->createWindow();
    }
    ++p;
  }
  return true;
}
 
/**
 * Destroy all parentless windows.
 *
 * This message is invoked by TOADBase::runApp.
 */
void
TWindow::destroyParentless()
{
  TVectorParentless::iterator p = parentless.begin();
  TVectorParentless::iterator e = parentless.end();  
  while(p!=e) {
//    printf("%s: %08x\n", __PRETTY_FUNCTION__, *p);
//    cout << "destroying window" << (*p)->getTitle() << endl;
    if ((*p)->isRealized()) {
      (*p)->destroyWindow(); 
      p = parentless.begin();
      e = parentless.end();  
      continue;
    }
    p++;
  }
}  

/////////////// Cocoa Section

@interface toadWindow : NSWindow
{
  @public
    TWindow *twindow;
}
@end

@implementation toadWindow : NSWindow
- (void)dealloc {
  if (twindow) {
    cout << "dealloc toadWindow " << twindow->getTitle() << endl;
    delete twindow;
  }
  [super dealloc];
}
- (void)setWindow: (TWindow*)aWindow {
  twindow = aWindow;
}
- (void)becomeKeyWindow {
  [super becomeKeyWindow];
//  printf("%s\n", __FUNCTION__);
  TFocusManager::domainToWindow(twindow);
}
- (void)resignKeyWindow {
  [super resignKeyWindow];
//  printf("%s\n", __FUNCTION__);
  TFocusManager::domainToWindow(0);
}

- (BOOL)canBecomeMainWindow
{
  return twindow->flagParentlessAssistant ? NO : YES;
}

/*
- (BOOL)canBecomeKeyWindow
{
  return twindow->flagParentlessAssistant ? NO : YES;
}
*/
/*
- (void)becomeMainWindow {
  printf("%s\n", __FUNCTION__);
}
- (void)resignMainWindow {
  printf("%s\n", __FUNCTION__);
}
*/
@end

/*
 *
 * Cocoa toadView
 *
 */

@interface toadView : NSView
{
  @public
    TWindow *twindow;
    NSTrackingRectTag trackAll;
}
@end

@implementation toadView : NSView
/*
- (id)initWithFrame:(NSRect)frame {
  printf("initWithFrame\n");
  self = [super initWithFrame:frame];
  if (self) {
    // [[self window] setAcceptsMouseMovedEvents:YES];
    printf("  add tracking rectangle (0,0,%f,%f)\n", frame.size.width, frame.size.height);

    trackAll = [self addTrackingRect: NSMakeRect(0, 0, frame.size.width, frame.size.height)
               owner: self
               userData: NULL
                assumeInside: NO];
//    [self setPostsBoundsChangedNotifications: YES];
//    [self setPostsFrameChangedNotifications: YES];
  }
  return self;
}
*/
- (void)dealloc {
  if (twindow) {
    cout << "dealloc toadView " << twindow->getTitle() << endl;
  }
  [super dealloc];
}

- (void)setWindow: (TWindow*)aWindow {
  twindow = aWindow;
}

- (void) initTrackAll:(NSRect)frame {
  trackAll = [self addTrackingRect: frame
              owner: self
              userData: NULL
              assumeInside: NO];
  // [self translateOriginToPoint: NSMakePoint(-0.5, -0.5)];
  // [self setBoundsOrigin: NSMakePoint(-0.5, -0.5)];
}
// TRUE: (0,0) is in the upper-left-corner
- (BOOL)isFlipped {
  return TRUE;
}
- (BOOL)isOpaque {
  return TRUE;
}
/*
- (void) resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
  printf("%s\n", __FUNCTION__);
}
*/
- (void) setFrameSize:(NSSize)newSize
{
//  printf("%s: (%f,%f)\n", __FUNCTION__, newSize.width, newSize.height);
  [super setFrameSize:newSize];
  [self removeTrackingRect: trackAll];
  trackAll = [self addTrackingRect: NSMakeRect(0, 0, newSize.width, newSize.height)
              owner: self
              userData: NULL
              assumeInside: NO];
  twindow->w = newSize.width;
  twindow->h = newSize.height;
  twindow->doResize();
}

- (void) drawRect:(NSRect)rect
{
  // [self inLiveResize]
//printf("print draw rect (%f,%f,%f,%f)\n",rect.origin.x,rect.origin.y,
//                                         rect.size.width,rect.size.height);
  if (!twindow)
    return;

  {
  TPen pen(twindow);
  pen.setColor(twindow->_bg.r/255.0, twindow->_bg.g/255.0, twindow->_bg.b/255.0);
  pen.fillRectangle(0,0,twindow->w,twindow->h);
  }

  twindow->paint();
}

// We always return YES here to simulate FocusFollowsMouse behaviour.
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
  return YES;
}

/*
 * NSView: 
 *   acceptsFirstResponder
 *     return YES when we can handle key events and action messages
 *   becomeFirstResponder
 *   needsPanelToBecomeKey
 *   becomesKeyOnlyIfNeeded(NSPanel)
 */

/**
 * return YES when we can handle key events and action messages
 */
- (BOOL)acceptsFirstResponder
{
//  printf("accept focus: %s %s\n", __FUNCTION__, twindow->getTitle().c_str());
  return YES;
}

- (BOOL)becomeFirstResponder
{
//  printf("became focus: %s %s\n", __FUNCTION__, twindow->getTitle().c_str());
  return YES;
}

- (BOOL)resignFirstResponder
{
  printf("lost focus: %s %s\n", __FUNCTION__, twindow->getTitle().c_str());
  return YES;
}
 
- (void) keyDown:(NSEvent*)theEvent
{
//printf("key down\n");
  TKeyEvent ke(theEvent);
  ke.type = TKeyEvent::DOWN;
  //twindow->keyEvent(ke);
  TFocusManager::handleEvent(ke);
  executeMessages();
}

- (void) keyUp:(NSEvent*)theEvent
{
//printf("key up\n");
  TKeyEvent ke(theEvent);
  ke.type = TKeyEvent::UP;
  //twindow->keyEvent(ke);
  TFocusManager::handleEvent(ke);
  executeMessages();
}

- (void) mouseEntered:(NSEvent*)theEvent
{
//printf("%s: %s\n",__FUNCTION__, twindow->getTitle().c_str());
  twindow->_inside = true;
  TMouseEvent me(theEvent, self, twindow);
  me.type = TMouseEvent::ENTER;
  twindow->mouseEvent(me);
  executeMessages();
}

- (void) mouseExited:(NSEvent*)theEvent
{
//printf("%s: %s\n",__FUNCTION__, twindow->getTitle().c_str());
  twindow->_inside = false;
  TMouseEvent me(theEvent, self, twindow);
  me.type = TMouseEvent::LEAVE;
  twindow->mouseEvent(me);
  executeMessages();
}

- (void) mouseDown:(NSEvent*)theEvent
{
cout << "mouse down in " << twindow->getTitle() << endl;
  TMouseEvent::_modifier |= MK_LBUTTON;
  twindow->_down(TMouseEvent::LDOWN, theEvent);
}

- (void) rightMouseDown:(NSEvent*)theEvent
{
  TMouseEvent::_modifier |= MK_RBUTTON;
  twindow->_down(TMouseEvent::RDOWN, theEvent);
}

- (void) otherMouseDown:(NSEvent*)theEvent
{
  TMouseEvent::_modifier |= MK_MBUTTON;
  twindow->_down(TMouseEvent::MDOWN, theEvent);
}

void
TWindow::_down(TMouseEvent::EType type, NSEvent *theEvent)
{
  TMouseEvent me(theEvent, nsview, this);
printf("%s: %s: %f, %f\n",__FUNCTION__, getTitle().c_str(), me.x, me.y);
  if (!_inside) {
//printf("  flip inside\n");
    _inside = true;
    TMouseEvent me(theEvent, nsview, this);
    me.type = TMouseEvent::ENTER;
    mouseEvent(me);
  }
  me.type = type;
  me.dblClick = [theEvent clickCount]==2;
  _inside = true;
  mouseEvent(me);
  executeMessages();
}

- (void) mouseUp:(NSEvent*)theEvent
{
  TMouseEvent::_modifier &= ~MK_LBUTTON;
  twindow->_up(TMouseEvent::LUP, theEvent);
}
- (void) rightMouseUp:(NSEvent*)theEvent
{
  TMouseEvent::_modifier &= ~MK_RBUTTON;
  twindow->_up(TMouseEvent::RUP, theEvent);
}
- (void) otherMouseUp:(NSEvent*)theEvent
{
  TMouseEvent::_modifier &= ~MK_MBUTTON;
  twindow->_up(TMouseEvent::MUP, theEvent);
}
void
TWindow::_up(TMouseEvent::EType type, NSEvent *theEvent)
{
//printf("%s: %s\n",__FUNCTION__, twindow->getTitle().c_str());
  TMouseEvent me(theEvent, nsview, this);
  if (!_inside) {
//printf("  flip outside\n");
    _inside = false;
    TMouseEvent me(theEvent, nsview, this);
    me.type = TMouseEvent::LEAVE;
    mouseEvent(me);
  }
  me.type = type;
  mouseEvent(me);
  executeMessages();
}
- (void) mouseDragged:(NSEvent*)theEvent
{
//printf("%s: %s _inside=%i\n",__FUNCTION__, twindow->getTitle().c_str(),twindow->_inside);
  TMouseEvent me(theEvent, self, twindow);
  TRectangle r(0,0,twindow->w,twindow->h);
  if (twindow->_inside != r.isInside(me.x, me.y)) {
//printf("  flip inside/outide\n");
    twindow->_inside = !twindow->_inside;
    TMouseEvent me(theEvent, self, twindow);
    me.type = twindow->_inside ? TMouseEvent::ENTER : TMouseEvent::LEAVE;
    twindow->mouseEvent(me);
  }
  me.type = TMouseEvent::MOVE;
  twindow->mouseEvent(me);
  executeMessages();
}

- (void) mouseMoved:(NSEvent*)theEvent
{
  if (!twindow->_allMouseMoveEvents)
    return;
//printf("%s: %s _inside=%i\n",__FUNCTION__, twindow->getTitle().c_str(),twindow->_inside);
  TMouseEvent me(theEvent, self, twindow);
  TRectangle r(0,0,twindow->w,twindow->h);
  if (twindow->_inside != r.isInside(me.x, me.y)) {
//printf("  flip inside/outide\n");
    twindow->_inside = !twindow->_inside;
    TMouseEvent me(theEvent, self, twindow);
    me.type = twindow->_inside ? TMouseEvent::ENTER : TMouseEvent::LEAVE;
    twindow->mouseEvent(me);
  }
  me.type = TMouseEvent::MOVE;
  twindow->mouseEvent(me);
  executeMessages();
}
@end

TMouseEvent::TMouseEvent(NSEvent *anEvent, NSView *aView, TWindow *aWindow) {
  nsevent = anEvent;
  NSPoint pt = [aView convertPoint:[anEvent locationInWindow] fromView:nil];
  x = pt.x;
  y = pt.y;
cerr << "TMouseEvent::TMouseEvent: pos=("<<x<<","<<y<<"), origin=("<<aWindow->getOriginX()<<","<<aWindow->getOriginY()<<")\n";
  x -= aWindow->getOriginX();
  y -= aWindow->getOriginY();
  window = aWindow;
  dblClick = false;
}

unsigned TMouseEvent::_modifier = 0;

unsigned
TMouseEvent::modifier() const
{
  if (dblClick)
    return _modifier | MK_DOUBLE;
  return _modifier;
}

TWindow::TWindow(TWindow *parent, const string &title):
  TInteractor(parent, title)
{
//cerr << "TWindow::TWindow: title="<<title<<", this="<<this<<endl;
  if (parent==NULL) {
    flagShell = true;
    parentless.push_back(this);
  }
  nsview = nil;
  nswindow = nil;
  x = y = 0;
  w = 320;
  h = 200;
  _bg.set(255,255,255);
  layout = 0;
  
  flagExplicitCreate = false;
  flagTabKey = false;
  _inside = false;
  _mapped = true;
  _allMouseMoveEvents = false;
  _bOwnsFocus = false;
  flagNoFocus = false;
  flagPopup = false;

  flag_wm_resize = false;
}

TWindow::~TWindow()
{
//cerr << "enter TWindow::~TWindow: title="<<title<<", this="<<this<<endl;
  
  if (layout) {
    // layout->toFile();
    delete layout;
    layout = NULL;
  }

  // deleteChildren is also called in TInteractor::~TInteractor but
  // then this object is not a TWindow anymore, thus we do it here
  deleteChildren();
  destroyWindow();
  
  // remove from parentless list (if applicable)
  if (getParent()==NULL) {
    for(TVectorParentless::iterator p=parentless.begin(); p!=parentless.end(); ++p) {
      if (*p==this) {
        parentless.erase(p);
        break;
      }
    }
    
    // when all parentless windows without flagParentlessAssistant==true are
    // gone, quit application
    bool quitApplication = true;
    for(TVectorParentless::iterator p=parentless.begin(); p!=parentless.end(); ++p) {
      if (!(*p)->flagParentlessAssistant && (*p)->isRealized()) {
        quitApplication = false;
        break;
      }
    }
    if (quitApplication) {
      [NSApp terminate: nil];
    }
  }
//cerr << "leave TWindow::~TWindow: title="<<title<<", this="<<this<<endl;
}


void
TWindow::createWindow()
{
  // if we already have a window, return
  if (nsview)
    return;
//cerr << "TWindow::createWindow: title=\"" << getTitle() << "\", pos="<<x<<", "<<y<<endl;
  nsview = [[toadView alloc] initWithFrame: NSMakeRect(x,y,w,h)];
  nsview->twindow = this;
  if (getParent() && !flagShell && !flagPopup) {
    [getParent()->nsview addSubview: nsview];
  } else {
    nswindow = [toadWindow alloc];
    [nswindow setReleasedWhenClosed: YES];
    nswindow->twindow = this;
    unsigned int styleMask = 0;
    if (flagPopup) {
      styleMask = NSBorderlessWindowMask;
    } else
    if (flagParentlessAssistant) {
      styleMask = NSTitledWindowMask
                | NSResizableWindowMask
                ;
                
    } else {
      styleMask = NSTitledWindowMask 
                | NSMiniaturizableWindowMask
                | NSClosableWindowMask
                | NSResizableWindowMask;
    }
    [nswindow initWithContentRect: NSMakeRect(x, y, w, h)
         styleMask: styleMask
         backing: NSBackingStoreBuffered
         defer: NO];
    
    [nswindow setTitle: [NSString stringWithUTF8String: getTitle().c_str()]];
    [nswindow setContentView: nsview];
    [nswindow makeKeyAndOrderFront: nil];
    [nswindow setAcceptsMouseMovedEvents: true];
    [nswindow setHasShadow: true];
  }
  // we must create the tracking window after the view was added to it's
  // parent, otherwise tracking does not work
  [nsview initTrackAll: NSMakeRect(0,0,w,h)];

  TFocusManager::newWindow(this);
  if (flagShell)
    TFocusManager::domainToWindow(this);

  if (layout)
    layout->arrange();

  // create children
  TInteractor *ptr = getFirstChild();
  while(ptr) {
    TWindow *p = dynamic_cast<TWindow*>(ptr);
    if (p && !p->flagExplicitCreate)
      p->createWindow();
    ptr = getNextSibling(ptr);
  }

  doResize();
}

static TWindow* runningAsModal = 0;

void
TWindow::destroyWindow()
{
  if (this==runningAsModal) {
    runningAsModal = 0;
    [NSApp stopModal];
    return; // doModalLoop will invoke destroyWindow again
  }

  if (!nsview)
    return;
  TFocusManager::destroyWindow(this);
  [nsview setWindow: nil];
  // [nsview setHidden: true];
  [nsview removeFromSuperview];
  nsview->twindow = NULL;
  nsview = nil;
  
  if (!nswindow)
    return;
  [nswindow setWindow: nil];
  [nswindow close];
  nswindow->twindow = NULL;
  nswindow = nil;
}

void
TWindow::doModalLoop()
{
  createWindow();
  if (nswindow) {
    runningAsModal = this;
    [NSApp runModalForWindow: nswindow];
  } else {
    cerr << "error: TWindow::doModalLoop requires '" << getTitle() << "' to be top level window" << endl;
    exit(0);
  }
  destroyWindow();
}

void
TWindow::setMapped(bool b)
{
  if (!nsview)
    return;
  if (_mapped==b)
    return;
  _mapped = b;
  [nsview setHidden: !b];
  if (!nswindow)
    return;
  //[nswindow setOpaque: !b];
  if (!b) {
    cout << getTitle() << " orderOut: hide" << endl;
    [nswindow orderOut: nswindow];
  } else {
    cout << getTitle() << " orderFront: show" << endl;
    [nswindow orderFront: nswindow];
  }
}

bool
TWindow::isMapped() const
{
  if (!nsview)
    return false;
  return _mapped;
}

void
TWindow::raiseWindow()
{
  if (!nswindow)
    return;
  [nswindow orderFront: nswindow];
}

void
TWindow::setSize(int w, int h)
{
  if (w<0)
    w = this->w;
  if (h<0)
    h = this->h;
  if (w==this->w && h==this->h)
    return;

#if 0
cerr << "TWindow::setSize("<<w<<","<<h<<"): title=\"" << getTitle() << "\", nswindow=" << nswindow << ", nsview=" << nsview << endl;
cerr << "  old pos="<<this->x<<","<<this->y<<", size="<<this->w<<","<<this->h<<endl;

  if (nswindow) {
    NSRect r = [nswindow frame];
    cerr << "  nswindow=("<<r.origin.x<<","<<r.origin.y<<","<<r.size.width<<","<<r.size.height<<")\n";
  }

  if (nsview) {
    NSRect r = [nsview frame];
    cerr << "  nsview=("<<r.origin.x<<","<<r.origin.y<<","<<r.size.width<<","<<r.size.height<<")\n";
  }
#endif
  
  if (nswindow) {
    NSRect r = [nswindow frame];
    // MacOS's screen origin is left-bottom, TOAD's is left-top so changing the
    // size also requires to adjust the windows position
    r.origin.y += this->h - h;
    r.size.width = w;
    r.size.height = h;
    [nswindow setFrame: r display: true];
  }

  if (nsview) {
    NSRect r = [nsview frame];
    r.size.width = w;
    r.size.height = h;
    [nsview setFrame: r];
  }

  this->w = w;
  this->h = h;
}

void
TWindow::setPosition(int x, int y)
{
  if (nsview) {
    if (this->x==x && this->y==y)
      return;
//    cerr << "change position of Cocoa window " << getTitle() << " from " << this->x << ", " << this->y << " to " << x << ", " << y << endl;
    NSPoint pt = NSMakePoint(x, y);
    [nsview setFrameOrigin: pt];
  }
  this->x = x;
  this->y = y;
}

void
TWindow::setShape(int x, int y, int w, int h)
{
  TRectangle::set(x, y, w, h);
  if (nsview) {
    NSRect nr = NSMakeRect(x, y, w, h);
    [nsview setFrame: nr];
  }
}

void
TWindow::invalidateWindow(bool)
{
  if (nsview) {
    [nsview setNeedsDisplay: YES];
  }
}

void
TWindow::invalidateWindow(int,int,int,int, bool clearbg)
{
  invalidateWindow();
}

void
TWindow::invalidateWindow(const TRectangle&, bool clearbg)
{
  invalidateWindow();
}

TRegion
TWindow::getUpdateRegion() const
{
  TRegion r;
  r |= TRectangle(0,0,w,h);
  return r;
}

void
TWindow::scrollRectangle(const TRectangle &rect, int x,int y, bool bClrBG)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
  invalidateWindow();
}

void
TWindow::scrollTo(int x, int y)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
  setOrigin(x,y);
  invalidateWindow();
}

void
TWindow::setOrigin(int x,int y)
{
  origin.x = x;
  origin.y = y;
}
void
TWindow::getOrigin(int *x, int *y) const
{
  *x = origin.x;
  *y = origin.y;
}

void
TWindow::getOrigin(TCoord *x, TCoord *y) const
{
  *x = origin.x;
  *y = origin.y;
}

const char*
TKeyEvent::getString() const
{
  NSString *ns = [nsevent characters];
  return [ns UTF8String];
}

TKey 
TKeyEvent::getKey() const
{
  return [nsevent keyCode];
}

unsigned 
TKeyEvent::modifier() const
{
  return [nsevent modifierFlags];
}

void
TKeyEvent::setModifier(unsigned)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

#if 0
void
TWindow::setCursor(TCursor::EType)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TWindow::setCursor(const TCursor*)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}
#endif

void
TWindow::setToolTip(const string &s)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TWindow::paintNow()
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

void
TWindow::loadLayout(const string &filename)
{
  TLayout * new_layout = NULL;
  try {
    iurlstream url(filename);
    TInObjectStream in(&url);
    TSerializable *s = in.restore();
    if (!s || !in) {
      cerr << "loading layout '" << filename << "' failed " << in.getErrorText() << endl;
    } else {
      new_layout = dynamic_cast<TLayout*>(s);
      if (!new_layout) {
        cerr << "loading layout '" << filename << "' failed: doesn't provide TLayout object, "
             << "  got '" << typeid(*s).name() << "'\n";
        delete s;
      }
    }
  }
  catch(exception &e) {
    cerr << "loading layout '" << filename << "' failed:\ncaught exception: " << e.what() << endl;
  }
  if (new_layout) {
    new_layout->setFilename(filename);
    setLayout(new_layout);
  } else {
    if (layout) {
      TLayout *l = layout;
      l->setFilename(filename);
      layout = 0;
      setLayout(l);
    }
  }
}

/**
 * Set a new layout. The previous layout is deleted.
 *
 * When the new layout doesn't have a filename, the filename of the
 * old layout will be applied to the new layout and be cleared.
 *
 * This way the old layout won't be stored on destruction and the new
 * layout will be stored into the old layouts file.
 */
void
TWindow::setLayout(TLayout *l)
{
  if (layout == l)
    return;
  string oldfilename;
  if (layout) {
    oldfilename = layout->getFilename();
    layout->setFilename("");
    delete layout;
  }
  /**
   * windows without a paint method, don't get paint events, but the
   * layout might wan't 'em so set the required events again
   * todo: only subscribe for paint events, when the layout's paint
   * method is overwritten.
   */
#ifdef __X11__
  if ( x11window && ((layout && !l) || (!layout && l)) )
    XSelectInput(x11display, x11window, _buildEventmask());
#endif
  layout = l;
  if (layout) {
    if (layout->getFilename().size()==0 && oldfilename.size()!=0) {
      layout->setFilename(oldfilename);
    }
    layout->window = this;
    layout->arrange();
  }
}

