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

#include <execinfo.h>

#include <toad/core.hh>
#include <toad/layout.hh>
#include <toad/cursor.hh>
#include <toad/focusmanager.hh>
#include <toad/io/urlstream.hh>
#include <toad/command.hh>
#include <toad/stacktrace.hh>
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

typedef vector<TWindow*> TVectorParentless;
static TVectorParentless parentless;

typedef map<TWindow*,string> TTextMap;
static TTextMap tooltipmap;

static TWindow *lastMouse = 0;

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
TWindow::keyEvent(const TKeyEvent &ke)
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
//cout << "TWindow::setFocus: " << getTitle() << endl;
  TFocusManager::setFocusWindow(this);
  return TFocusManager::getFocusWindow()==this;
}

void
TWindow::_setFocusHelper(TInteractor *parent, bool b)
{
//cout << "TWindow::setFocusHelper: " << getTitle() << endl;
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
//cout << "TWindow::_setFocus: " << getTitle() << " from " << _bOwnsFocus << " to " << b << endl;
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
TWindow::closeRequest()
{
//  cout << "TWindow::closeRequest for " << getTitle() << endl;
  destroyWindow();
}

void
TWindow::grabMouse(bool allmove, TWindow *confine, TCursor::EType type)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

static TWindow *grabPopupWindow = 0;

void
TWindow::grabPopupMouse(bool allmove, TCursor::EType type)
{
  if (grabPopupWindow) {
    if (grabPopupWindow==this)
      return;
    ungrabMouse();
  }
  grabPopupWindow = this;
  lastMouse = 0;
}

void
TWindow::ungrabMouse()
{
  if (grabPopupWindow) {
    TWindow *wnd = grabPopupWindow;
    grabPopupWindow = 0;
    wnd->closeRequest();
    return;
  }
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
      NSRect nr;
      nr.origin.x = *x;
      nr.origin.y = p->h-*y;
      nr = [p->nswindow convertRectToScreen: nr];
//cout << "nswindow " << p->getTitle() << " on screen " << np.x << ", " << np.y << endl;
      *x = nr.origin.x;
      *y = nr.origin.y;
      return;
    }
//cout << "toad window " << p->getTitle() << " on screen " << p->x << ", " << p->y << endl;
  }
}

void
TWindow::placeWindow(EWindowPlacement how, TWindow *parent, TCoord dx, TCoord dy)
{
  TCoord px, py;
  TCoord sw, sh;

  TRectangle where;
  switch(how) {
    case PLACE_PARENT_RANDOM:
    case PLACE_PARENT_CENTER: {
      if (!parent)
        return;
      parent->getShape(&where);
      NSScreen *mainScreen = [parent->nswindow screen];
      NSRect r = [mainScreen visibleFrame];
      sw = r.size.width;
      sh = r.size.height;
    } break;
    case PLACE_SCREEN_RANDOM:
    case PLACE_SCREEN_CENTER: {
      NSScreen *mainScreen;
      if (nswindow)
        mainScreen = [nswindow screen];
      else
        mainScreen = [NSScreen mainScreen];
      NSRect r = [mainScreen visibleFrame];
      where.set(0,0,r.size.width,r.size.height);
      sw = r.size.width;
      sh = r.size.height;
    } break;
  }
  
  switch(how) {
    case PLACE_SCREEN_CENTER:
    case PLACE_PARENT_CENTER: {
      if (nswindow) {
        [nswindow center];
        return;
      }
      x = where.x + (where.w/2) - (w/2);
      y = where.y + (where.h/2) - (h/2);
    } break;
    case PLACE_SCREEN_RANDOM:
    case PLACE_PARENT_RANDOM: {
      x = where.x + (where.w/2) - (w/2);
      y = where.y + (where.h/2) - (h/2);
      int xr = (int) (0.5*where.w*rand()/(RAND_MAX+1.0));
      xr-=(where.w/4);
      x+=xr;
      int yr = (int) (0.5*where.h*rand()/(RAND_MAX+1.0));
      yr-=(where.h/4);
      y+=yr;
      
      // keep window inside the screen boundarys
      static const TCoord dist = 32;
      if (x+w>sw-dist)
	x = sw-w-dist;
      if (y+h>sh-dist)
        y = sh-h-dist;
      if (x<dist)
        x=dist;   
      if (y<dist)
        y=dist;
    } break;
    case PLACE_MOUSE_POINTER: {
      NSPoint p = [NSEvent mouseLocation];
      x = p.x - w/2;
      y = p.y - h/2;
      } break;
    case PLACE_CORNER_MOUSE_POINTER: {
      NSPoint p = [NSEvent mouseLocation];
      x = p.x;
      y = p.y - h;
    } break;
    case PLACE_PULLDOWN: {
      int rx, ry;
      parent->getRootPos(&rx, &ry);
      x=rx; y=ry;
      y-=parent->h; // below parent
      y-=h;
      // FIXME: should update the window object now, only done in setMapped for now
    } break;
    case PLACE_TOOLTIP:
      break;
  }
  
  x+=dx;
  y-=dy; // inverted Y-coord on Cocoa
}

/**
 * I am not quite sure why I did add this method... [MAH]
 */
void
TWindow::windowEvent(const TWindowEvent &we)
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
TWindow::mouseEvent(const TMouseEvent &me)
{
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
void TWindow::mouseMove(const TMouseEvent &){}
void TWindow::mouseEnter(const TMouseEvent &){}
void TWindow::mouseLeave(const TMouseEvent &){}

//! Called when the left mouse button is pressed. Since X11 performs an
//! automatic mouse grab you will receive a mouseLUp message afterwards
//! unless you call UngrabMouse().
void TWindow::mouseLDown(const TMouseEvent &){}
//! Same as mouseLDown for the middle mouse button.
void TWindow::mouseMDown(const TMouseEvent &){}
//! Same as mouseLDown for the right mouse button.
void TWindow::mouseRDown(const TMouseEvent &){}
void TWindow::mouseLUp(const TMouseEvent &){}
void TWindow::mouseMUp(const TMouseEvent &){}
void TWindow::mouseRUp(const TMouseEvent &){}
                                                                                                                                                                                                 
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
  TOAD_DBG_ENTER
  if (twindow) {
    cout << "dealloc toadWindow " << twindow->getTitle() << endl;
    delete twindow;
  }
  [super dealloc];
  TOAD_DBG_LEAVE
}
- (void)setWindow: (TWindow*)aWindow {
  twindow = aWindow;
}

- (void)becomeKeyWindow {
  TOAD_DBG_ENTER
  [super becomeKeyWindow];
//  printf("%s\n", __FUNCTION__);
  TFocusManager::domainToWindow(twindow);
  TOAD_DBG_LEAVE
}
- (void)resignKeyWindow {
  TOAD_DBG_ENTER
  [super resignKeyWindow];
//  printf("%s\n", __FUNCTION__);
  if (grabPopupWindow)
    TWindow::ungrabMouse();
  TFocusManager::domainToWindow(0);
  TOAD_DBG_LEAVE
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

@interface toadView : NSView <NSTextInputClient>
{
  @public
    TWindow *twindow;
//    NSTrackingRectTag trackAll;
}
@end

@implementation toadView : NSView

/*
 * To properly handle dead keys (ie. [alt]+[n], [ ] -> '~') our NSView has
 * to implement the NSTextInputClient protocol.
 *
 * Here implement all methods for this protocol but they actually do nothing.
 * Just having them available and calling [self interpretKeyEvents] will
 * take care that the event's methods will deliver the requested dead keys.
 */
- (BOOL) hasMarkedText { return FALSE; }
- (NSRange)markedRange { return {NSNotFound, 0}; }
- (NSRange)selectedRange { return {NSNotFound, 0}; }
- (void)setMarkedText: (id)str selectedRange:(NSRange)selected replacementRange:(NSRange)replacement {}
- (void)unmarkText {}
- (NSArray*) validAttributesForMarkedText { return [NSArray array]; }
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange { return nil; }

static string ndkstr;

- (void)insertText: str replacementRange:(NSRange) replacement {
  ndkstr =  [str UTF8String];
}
- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint { return NSNotFound; }
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange { return NSMakeRect(0,0,0,0); }
- (void)doCommandBySelector:(SEL)aSelector {}

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
/*
- (void) initTrackAll:(NSRect)frame {
  trackAll = [self addTrackingRect: frame
              owner: self
              userData: NULL
              assumeInside: NO];
  // [self translateOriginToPoint: NSMakePoint(-0.5, -0.5)];
  // [self setBoundsOrigin: NSMakePoint(-0.5, -0.5)];
}
*/
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
  TOAD_DBG_ENTER
//  printf("%s: (%f,%f)\n", __FUNCTION__, newSize.width, newSize.height);
  [super setFrameSize:newSize];
/*
  [self removeTrackingRect: trackAll];
  trackAll = [self addTrackingRect: NSMakeRect(0, 0, newSize.width, newSize.height)
              owner: self
              userData: NULL
              assumeInside: NO];
*/
  twindow->w = newSize.width;
  twindow->h = newSize.height;
  twindow->doResize();
  TOAD_DBG_LEAVE
}

static TRegion *updateRegion = 0;

- (void) drawRect:(NSRect)rect
{
  TOAD_DBG_ENTER
  if (updateRegion) {
    updateRegion = NULL;
  }
/*
  if ([self inLiveResize]) {
    drawQuick()...
    return;
  }
*/
/*
  // rect is the union of all clipping rects, which can be retrieved with this:
  const NSRect *rects;
  int count;
  [self getRectsBeingDrawn:&rects count:&count];
*/
//printf("print draw rect (%f,%f,%f,%f)\n",rect.origin.x,rect.origin.y,
//                                         rect.size.width,rect.size.height);
  if (!twindow) {
    TOAD_DBG_LEAVE
    return;
  }

  if (!twindow->flagNoBackground) {
    CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    CGContextSetRGBFillColor(ctx, twindow->_bg.r, twindow->_bg.g, twindow->_bg.b, 1);
    CGContextAddRect(ctx, NSRectToCGRect(rect));
    CGContextDrawPath(ctx, kCGPathFill);
  }
  if (twindow->layout)
    twindow->layout->paint();
  twindow->paint();
  TOAD_DBG_LEAVE
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
//  printf("lost focus: %s %s\n", __FUNCTION__, twindow->getTitle().c_str());
  return YES;
}
 
- (void) keyDown:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
//cout << "key down" << endl;
  [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];

  TKeyEvent ke(theEvent);
  ke.type = TKeyEvent::DOWN;
  //twindow->keyEvent(ke);
  TFocusManager::handleEvent(ke);
  TOAD_DBG_LEAVE
}

- (void) keyUp:(NSEvent*)theEvent
{
//printf("key up\n");
  TOAD_DBG_ENTER
  TKeyEvent ke(theEvent);
  ke.type = TKeyEvent::UP;
  //twindow->keyEvent(ke);
  TFocusManager::handleEvent(ke);
  TOAD_DBG_LEAVE
}

// handle layout and global event filter
static void _doMouse2(TWindow *twindow, TMouseEvent &me)
{
  if (me.type == TMouseEvent::ENTER) {
    if (twindow->cursor && twindow->cursor->cursor)
      [twindow->cursor->cursor set];
    else
      [[NSCursor arrowCursor] set];
  }
  
  if (me.type == TMouseEvent::MOVE &&
      !(me.modifier() & (MK_LBUTTON|MK_MBUTTON|MK_RBUTTON)) &&
      !twindow->_allMouseMoveEvents)
    return;

  me.window = twindow;
  TEventFilter *flt = toad::global_evt_filter;
  while(flt) {
    if (flt->mouseEvent(me))
      return;
    flt = flt->next;
  }

  if (twindow->layout && twindow->layout->mouseEvent(me))
    return;

  twindow->mouseEvent(me);
}

const char*
TMouseEvent::name() const
{
  static const char *name[13] = {
    "MOVE", "ENTER", "LEAVE",
    "LDOWN", "MDOWN", "RDOWN",
    "LUP", "MUP", "RUP",
    "ROLL_UP", "ROLL_DOWN",
    "TABLET_POINT", "TABLET_PROXIMITY"
  };
  if (type>=0 && type<=12)
    return name[type];
  return "?";
}

float
TMouseEvent::pressure() const
{
  float result = 0.0;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity)
    result = [nsevent pressure];
  return result;
}

float
TMouseEvent::tangentialPressure() const
{
  float result = 0.0;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity &&
      ([nsevent type] == NSTabletPoint ||
       [nsevent subtype]==NSTabletPointEventSubtype))
    result = [nsevent tangentialPressure];
  return result;
}

float
TMouseEvent::rotation() const
{
  float result = 0.0;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity &&
      ([nsevent type] == NSTabletPoint ||
       [nsevent subtype]==NSTabletPointEventSubtype))
    result = [nsevent rotation];
  return result;
}

TPoint
TMouseEvent::tilt() const
{
  TPoint result;
  if ([nsevent type] != NSScrollWheel &&
      [nsevent type] != NSTabletProximity &&
      ([nsevent type] == NSTabletPoint ||
       [nsevent subtype]==NSTabletPointEventSubtype))
  {
    NSPoint p = [nsevent tilt];
    result.set(p.x, p.y);
  }
  return result;
}

// handle grabPopUp mouse and enter/leave event generation
static void _doMouse(TWindow *twindow, TMouseEvent &me)
{
/*
if (me.type == TMouseEvent::LUP)
{
  cerr << "_doMouse: LUP -----------------------------------------------------" << endl;
}
if (me.type == TMouseEvent::LDOWN)
{
  cerr << "_doMouse: LDOWN ---------------------------------------------------" << endl;
}
if (me.type == TMouseEvent::LUP || me.type == TMouseEvent::LDOWN) {
  cerr << "_doMouse: TWindow='"<<twindow->getTitle()<<"', layout=" << twindow->layout << endl;
  cerr << "twindow=" << twindow << ", grabPopupWindow=" << grabPopupWindow << endl;
}
*/
  TWindow *mouseOver = 0;
  
  NSPoint p;
  p = [NSEvent mouseLocation];
  NSInteger wn = [NSWindow windowNumberAtPoint: p belowWindowWithWindowNumber: 0];
  NSWindow *wnd = [NSApp windowWithWindowNumber: wn];
    
  if (wnd && [wnd isKindOfClass:[toadWindow class]]) {
    toadWindow *t = (toadWindow*)wnd;
      
    NSRect r;
    r.origin = p;
    r = [wnd convertRectFromScreen: r]; // convert screen to window coordinates
      
    NSView *view = [t->twindow->nsview hitTest: r.origin];
    if (view && [view isKindOfClass:[toadView class]]) {
      toadView *v = (toadView*)view;
      mouseOver = v->twindow;
    }
  }

//cout << "_doMouse " << twindow << " " << twindow->getTitle() << " " << me.name() << ", over=" << (mouseOver?mouseOver->getTitle():"NULL") << endl;

  if (mouseOver != lastMouse) {
    if (lastMouse && lastMouse->_inside) {
//cout << "set "<<lastMouse<<" " << lastMouse->getTitle() << "->inside=false (grab)"<<endl;
      lastMouse->_inside = false;
      TMouseEvent me2(me.nsevent, lastMouse);
      me2.type = TMouseEvent::LEAVE;
      _doMouse2(lastMouse, me2);
    }
    if (mouseOver && !mouseOver->_inside) {
//cout << "set "<<mouseOver<<" " << mouseOver->getTitle() << "->inside=true (grab)"<<endl;
      mouseOver->_inside = true;
      TMouseEvent me2(me.nsevent, mouseOver);
      me2.type = TMouseEvent::ENTER;
      if (me.type == TMouseEvent::LDOWN ||
          me.type == TMouseEvent::MDOWN ||
          me.type == TMouseEvent::RDOWN )
      {
        // mouse enter & down at the same time -> remove the button
        // information from the modifier
        me2.__modifier &= ~(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON);
      }
      _doMouse2(mouseOver, me2);
    }
    lastMouse = mouseOver;
  }
  
  if (me.type == TMouseEvent::ENTER ||
      me.type == TMouseEvent::LEAVE)
    return;

  if (grabPopupWindow) {
    if (mouseOver) {
//      cout << "  mouse over title=" << mouseOver->getTitle() << endl;
      twindow = mouseOver;
//    } else {
//      cout << "  not a toad window" << endl;
    }
//    cout << "  twindow="<<twindow<<", lastMouse="<<lastMouse<<", mouseOver="<<mouseOver<<endl;
  }

  _doMouse2(twindow, me);

  if ( grabPopupWindow &&
       twindow != grabPopupWindow &&
       ( me.type == TMouseEvent::LDOWN ||
         me.type == TMouseEvent::MDOWN ||
         me.type == TMouseEvent::RDOWN ) &&
       !twindow->isChildOf(grabPopupWindow) )
  {
    TWindow::ungrabMouse();
  }
/*
if (me.type == TMouseEvent::LUP || me.type == TMouseEvent::LDOWN) {
  cerr << "twindow=" << twindow << ", grabPopupWindow=" << grabPopupWindow << endl;
}
if (me.type == TMouseEvent::LDOWN)
  cerr << "_doMouse: LDOWN done -----------------------------------------------" << endl;
if (me.type == TMouseEvent::LUP)
  cerr << "_doMouse: LUP done -------------------------------------------------" << endl;
*/
}

- (void)tabletPoint:(NSEvent *)theEvent
{
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::TABLET_POINT;
  twindow->mouseEvent(me);
}

- (void)tabletProximity:(NSEvent *)theEvent
{
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::TABLET_PROXIMITY;
  twindow->mouseEvent(me);
}

- (void) mouseEntered:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::ENTER;
  _doMouse(twindow, me);
  TOAD_DBG_LEAVE
}

- (void) mouseExited:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::LEAVE;
  _doMouse(twindow, me);
  TOAD_DBG_LEAVE
}

- (void) mouseDown:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent::_modifier |= MK_LBUTTON;
  twindow->_down(TMouseEvent::LDOWN, theEvent);
  TOAD_DBG_LEAVE
}

- (void) rightMouseDown:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent::_modifier |= MK_RBUTTON;
  twindow->_down(TMouseEvent::RDOWN, theEvent);
  TOAD_DBG_LEAVE
}

- (void) otherMouseDown:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent::_modifier |= MK_MBUTTON;
  twindow->_down(TMouseEvent::MDOWN, theEvent);
  TOAD_DBG_LEAVE
}

void
TWindow::_down(TMouseEvent::EType type, NSEvent *theEvent)
{
  TMouseEvent me(theEvent, this);
  me.type = type;
  me.dblClick = (type!=TMouseEvent::ROLL_UP && type!=TMouseEvent::ROLL_DOWN) ? [theEvent clickCount]==2 : false;
  _doMouse(this, me);
}

- (void) mouseUp:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent::_modifier &= ~MK_LBUTTON;
  twindow->_up(TMouseEvent::LUP, theEvent);
  TOAD_DBG_LEAVE
}

- (void) rightMouseUp:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent::_modifier &= ~MK_RBUTTON;
  twindow->_up(TMouseEvent::RUP, theEvent);
  TOAD_DBG_LEAVE
}

- (void) otherMouseUp:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent::_modifier &= ~MK_MBUTTON;
  twindow->_up(TMouseEvent::MUP, theEvent);
  TOAD_DBG_LEAVE
}

void
TWindow::_up(TMouseEvent::EType type, NSEvent *theEvent)
{
  TMouseEvent me(theEvent, this);
  me.type = type;
  me.dblClick = (type!=TMouseEvent::ROLL_UP && type!=TMouseEvent::ROLL_DOWN) ? [theEvent clickCount]==2 : false;
  _doMouse(this, me);
}

- (void) mouseDragged:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::MOVE;
  _doMouse(twindow, me);
  TOAD_DBG_LEAVE
}

- (void) rightMouseDragged:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::MOVE;
  _doMouse(twindow, me);
  TOAD_DBG_LEAVE
}
- (void) otherMouseDragged:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::MOVE;
  _doMouse(twindow, me);
  TOAD_DBG_LEAVE
}

- (void) mouseMoved:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  TMouseEvent me(theEvent, twindow);
  me.type = TMouseEvent::MOVE;
  _doMouse(twindow, me);
  TOAD_DBG_LEAVE
}

- (void) scrollWheel:(NSEvent*)theEvent
{
  TOAD_DBG_ENTER
  if ([theEvent deltaY] > 0.0) {
    twindow->_up(TMouseEvent::ROLL_UP, theEvent);
  } else {
    twindow->_down(TMouseEvent::ROLL_DOWN, theEvent);
  }
  TOAD_DBG_LEAVE
}
@end

TMouseEvent::TMouseEvent(NSEvent *anEvent, TWindow *aWindow) {
  nsevent = anEvent;
  if (aWindow) {
    NSPoint p = [aWindow->nsview convertPoint:[anEvent locationInWindow] fromView:nil];
    pos.x = p.x;
    pos.y = p.y;
// cerr << "TMouseEvent::TMouseEvent: pos=("<<x<<","<<y<<"), origin=("<<aWindow->getOriginX()<<","<<aWindow->getOriginY()<<")\n";
    pos -= aWindow->getOrigin();
  }
  window = aWindow;
  dblClick = false;
  __modifier = [nsevent modifierFlags] 
           | _modifier;
}

unsigned TMouseEvent::_modifier = 0;

void
TWindow::_windowWillMove(NSNotification *notification)
{
  if (grabPopupWindow)
    ungrabMouse();
}

void
TWindow::_windowDidMove(NSNotification *notification)
{
  NSObject *obj = [notification object];
  if (![obj isKindOfClass:[toadWindow class]]) {
    cerr << "TWindow::_windowDidMove: it's not a window" << endl;
    return;
  }
  toadWindow *t = (toadWindow*)obj;
  if (!t->twindow) {
    cerr << "TWindow::_windowDidMove: not a toad window" << endl;
    return;
  }
  NSPoint pt = [t frame].origin;
  t->twindow->x = pt.x;
  t->twindow->y = t->twindow->h - pt.y;
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
  set(0,0,320,200);
  if (!parent)
    placeWindow(PLACE_SCREEN_CENTER);
  _bg.set(1, 1, 1);
  layout = 0;
  cursor = 0;
  
  flagExplicitCreate = false;
  flagTabKey = false;
  flagNoBackground = false;
  _inside = false;
  _mapped = true;
  _allMouseMoveEvents = false;
  _bOwnsFocus = false;
  _bToolTipAvailable = false;
  flagNoFocus = false;
  flagPopup = false;

  flag_wm_resize = false;
}

TWindow::~TWindow()
{
//cerr << "enter TWindow::~TWindow: title="<<title<<", this="<<this<<endl;
  if (lastMouse==this)
    lastMouse = 0;
    
  if (grabPopupWindow==this)
    ungrabMouse();
  
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

  setToolTip("");

//cerr << "leave TWindow::~TWindow: title="<<title<<", this="<<this<<endl;
}

void
TWindow::createWindow()
{
  // if we already have a window, return
  if (nsview)
    return;
    
  // wait for parent
  if (getParent() && !flagShell && !flagPopup && !getParent()->nsview)
    return;

//cout << "TWindow::createWindow()" << endl;

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
    if (flagPopup)
      [nswindow setLevel: NSPopUpMenuWindowLevel];
  }
  // we must create the tracking window after the view was added to it's
  // parent, otherwise tracking does not work
//  [nsview initTrackAll: NSMakeRect(0,0,w,h)];

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
  
  if (_bToolTipAvailable)
    [nsview setToolTip: [NSString stringWithUTF8String: tooltipmap[this].c_str()]];

  doResize();
}

static TWindow* runningAsModal = 0;

void
TWindow::destroyWindow()
{
//cerr << "TWindow::destroyWindow: title=\"" << getTitle() << "\"" << endl;

  if (this==runningAsModal) {
//cerr << "  modal stop" << endl;
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
  if (runningAsModal) {
    cerr << "error: TWindow::doModalLoop: FIXME: nesting not yet supported" << endl;
    return;
  }
  createWindow();
  if (!nswindow) {
    cerr << "error: TWindow::doModalLoop requires '" << getTitle() << "' to be top level window" << endl;
    exit(0);
  }
  runningAsModal = this;
  // two reason on why we can use
  //   [NSApp runModalForWindow: nswindow];
/*    
  if (false && !toad::layouteditor) {
    [NSApp runModalForWindow: nswindow];
  } else {
    // we also need to receive events for the layout editor control window
    // hence we run a modeless loop
*/
  while(runningAsModal == this) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSEvent *event =
        [NSApp
            nextEventMatchingMask:NSAnyEventMask
            untilDate:[NSDate distantFuture]
            inMode:NSDefaultRunLoopMode
            dequeue:YES];
    NSWindow *wnd = [event window];
    if ([wnd isKindOfClass:[toadWindow class]]) {
      toadWindow *t = (toadWindow*)wnd;
      if (nswindow == wnd || t->twindow->isChildOf(this) ) // ||
//          (toad::layouteditor && t->twindow->isChildOf(toad::layouteditor)))
      {
        [NSApp sendEvent:event];
        [NSApp updateWindows];
      }
    } else {
      [NSApp sendEvent:event];
      [NSApp updateWindows];
    }
    [pool release];
  }
  destroyWindow();
}

void
TWindow::setTitle(const string &title) {
  TInteractor::setTitle(title);
  if (nswindow)
    [nswindow setTitle: [NSString stringWithUTF8String: title.c_str()]];
}

void
TWindow::setMapped(bool b)
{
  if (!nsview)
    return;
  if (_mapped==b)
    return;
  _mapped = b;

//cout << "TWindow::setMapped("<<(b?"true":"false")<<")"<<endl;

  if (b && nswindow) {
    // FIXME: only the size, see also placeWindow
    NSRect frame = [nswindow frame];
    frame.origin.x = x;
    frame.origin.y = y;
    [nswindow setFrame: frame display: false];
  }

  if (!nswindow) {
    [nsview setHidden: !b];
    return;
  }

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
TWindow::setSize(TCoord w, TCoord h)
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
TWindow::setPosition(TCoord x, TCoord y)
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
TWindow::setShape(TCoord x, TCoord y, TCoord w, TCoord h)
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
TWindow::invalidateWindow(TCoord x, TCoord y ,TCoord w, TCoord h, bool clearbg)
{
  if (nsview) {
    [nsview setNeedsDisplayInRect: NSMakeRect(x, y, w, h)];
  }
}

void
TWindow::invalidateWindow(const TRectangle &r, bool clearbg)
{
  if (nsview) {
    [nsview setNeedsDisplayInRect: NSMakeRect(r.x, r.y, r.w, r.h)];
  }
}

/*
 * Returns the region to be updated during paint.
 */
TRegion*
TWindow::getUpdateRegion() const
{
  if (!nsview)
    return NULL;
  if (updateRegion)
    return updateRegion;
  static TRegion r;
  r.clear();
  const NSRect *rects;
  NSInteger count;
  [nsview getRectsBeingDrawn:&rects count:&count];
  for(int i=0; i<count; ++i)
    r|=TRectangle(rects[i].origin.x - origin.x,
                  rects[i].origin.y - origin.y,//                  h - (rects[i].size.height - rects[i].origin.y) - origin.y,
                  rects[i].size.width,
                  rects[i].size.height);
  updateRegion = &r;

//  TRectangle e;
//  r.getBoundary(&e);
// cout << getTitle() << ": update region extend " << e.x << ", " << e.y << ", " << e.w << ", " << e.h << endl;

  return &r;
}

void
TWindow::scrollRectangle(const TRectangle &r, TCoord x, TCoord y, bool bClrBG)
{
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
//  invalidateWindow();
  CGRect cgr = CGRectMake(r.x, r.y, r.w, r.h);
  
  [nsview scrollRect: cgr by: NSMakeSize(x, y)];
  // note: his overlaps with the destination of the scroll
  //       toad's x11 implementation took more care of this
  [nsview setNeedsDisplayInRect: cgr];
}

/**
 * Set the origin for all drawing operations and scroll the windows content
 * to the new position.
 */
void
TWindow::scrollTo(TCoord nx, TCoord ny)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
  
  
  TCoord dx = nx - origin.x;
  TCoord dy = ny - origin.y;
  CGRect cgr = CGRectMake(0,0,w,h);
  [nsview scrollRect: cgr by: NSMakeSize(dx, dy)];
  origin.set(nx, ny);
  // note: his overlaps with the destination of the scroll
  //       toad's x11 implementation took more care of this
  [nsview setNeedsDisplayInRect: cgr];
}

void
TWindow::setOrigin(TCoord x,TCoord y)
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
#if 0
  NSString *ns = [nsevent charactersIgnoringModifiers];
  return [ns UTF8String];
#endif
  return ndkstr.c_str();
}

TKey 
TKeyEvent::getKey() const
{
  return [nsevent keyCode];
}

unsigned 
TKeyEvent::modifier() const
{
  if (!_has_modifier) {
    TKeyEvent *t = const_cast<TKeyEvent*>(this);
    t->_modifier = [nsevent modifierFlags];
    t->_has_modifier = true;
  }
  return _modifier;
}

void
TKeyEvent::setModifier(unsigned m)
{
  _modifier = m;
  _has_modifier = true;
}

void
TWindow::setToolTip(const string &text)
{
  if (!text.empty()) {
    tooltipmap[this] = text;
    _bToolTipAvailable = true;
  } else {
    if (_bToolTipAvailable) {
      TTextMap::iterator p = tooltipmap.find(this);
      tooltipmap.erase(p);
    }
    _bToolTipAvailable = false;
  }
  
  if (nsview)
    [nsview setToolTip: text.empty() ? nil : [NSString stringWithUTF8String: text.c_str()]];
}

void
TWindow::paintNow()
{
  invalidateWindow();
//  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
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
