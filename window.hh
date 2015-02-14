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

#ifndef __TOAD_WINDOW_HH
#define __TOAD_WINDOW_HH 1

#include <toad/interactor.hh>
#include <toad/cursor.hh>
#include <toad/color.hh>
#include <toad/region.hh>
#include <string>

@class NSEvent, NSView, NSNotification, toadWindow, toadView;

#import <appkit/NSEvent.h>

namespace toad {

using namespace std;

// see /Developer/SDKs/MacOSX10.3.9.sdk/Developer/Headers/CFMCarbon/Events.h

#define MK_SHIFT     NSShiftKeyMask
#define MK_COMMAND   NSCommandKeyMask
#define MK_CONTROL   NSControlKeyMask
#define MK_ALT       NSAlternateKeyMask

#define MK_ALTGR   (1<<24)
#define MK_LBUTTON (1<<25)
#define MK_MBUTTON (1<<26)
#define MK_RBUTTON (1<<27)
#define MK_DOUBLE  (1<<28)

#define TK_SHIFT_L 56
#define TK_SHIFT_R 56

typedef unsigned short TKey;
#if 0
#define TK_BACKSPACE 8
#define TK_TAB       9
#define TK_RETURN    13
//#define TK_KP_RETURN 13
#define TK_ESCAPE    27
#define TK_DELETE    127
#define TK_INSERT    
#define TK_SPACE     32

#define TK_HOME      1
#define TK_LEFT      28
#define TK_UP        30
#define TK_RIGHT     29
#define TK_DOWN      31
#define TK_PAGEUP    11
#define TK_PAGEDOWN  12
#define TK_END       4
#define TK_BEGIN     0
#else
#define TK_RETURN    36
#define TK_KP_RETURN 0xffff
#define TK_TAB       48
#define TK_LEFT_TAB  0xffff
#define TK_INSERT    0xffff
#define TK_SPACE     49
#define TK_BACKSPACE 51
#define TK_ESCAPE    53
#define TK_SHIFT     56
#define TK_CONTROL_L   0xffff
#define TK_CONTROL_R   0xffff
#define TK_HOME      115
#define TK_PAGEUP    116
#define TK_DELETE    117
#define TK_END       119
#define TK_PAGEDOWN  121
#define TK_LEFT      123
#define TK_RIGHT     124
#define TK_DOWN      125
#define TK_UP        126
#define TK_F1        122
#define TK_F2        120
#define TK_F3        99
#define TK_F4        118
#define TK_F5        96
#define TK_F6        97
#define TK_F7        98
#define TK_F8        100
#define TK_F9        101
#define TK_F10       109
#define TK_F11       103
#define TK_F12       111
#define TK_F13       0xffff
#define TK_F14       0xffff
#define TK_F15       0xffff
#define TK_F16       0xffff
#define TK_F17       0xffff
#define TK_F18       0xffff
#define TK_F19       0xffff
#define TK_F20       0xffff
#endif

class TLayout;

class TMouseEvent
{
  public:
    NSEvent *nsevent;
    TMouseEvent(NSEvent *ne, TWindow *window);
    TMouseEvent(const TMouseEvent &me, TPoint p) {
      nsevent = me.nsevent;
      pos = p;
      _modifier = me._modifier;
      dblClick = me.dblClick;
      window = me.window;
    };

    enum EType {
      MOVE, ENTER, LEAVE,
      LDOWN, MDOWN, RDOWN, LUP, MUP, RUP,
      ROLL_UP, ROLL_DOWN
    };
    EType type;
    const char* name() const;
  
    TWindow *window; // information for the mouse event filter
    TPoint pos;
    TCoord pressure();
    TCoord rotation();
    TCoord tilt();
    unsigned modifier() const;
    enum EPointingDeviceType {
      // unknown, pen, cursor, eraser
    };
    // true when double click
    bool dblClick;
    static unsigned _modifier;
};

class TKeyEvent {
  public:
    TKeyEvent(NSEvent *ns) {
      nsevent = ns;
      _modifier = 0;
      _has_modifier = false;
    }
    enum EType {
      DOWN, UP
    } type;

    NSEvent *nsevent;
    TWindow *window;
    
    unsigned _modifier;
    bool _has_modifier:1;

    const char* getString() const;
    TKey getKey() const;
    unsigned modifier() const;
    void setModifier(unsigned);
};

class TWindowEvent {
  public:
    enum {
      NEW,
      DELETE,
      CREATE,
      CREATED,
      DESTROY,
      MAPPED,
      UNMAPPED,
      PAINT,
      ADJUST,
      RESIZE,
      FOCUS
    } type;
    TWindow *window;
};

class TWindow:
  public TInteractor, public TRectangle
{
    // FIXME?: the origin is nice for scrolling but fails for rotation... can we live with that?
    TPoint origin;
  public:
    bool _inside:1; // helper to emulate mouseEnter, mouseLeave on Cocoa
    bool _mapped:1;
    bool _allMouseMoveEvents:1;
    void _down(TMouseEvent::EType type, NSEvent *theEvent);
    void _up(TMouseEvent::EType type, NSEvent *theEvent);
    static void _windowWillMove(NSNotification *theNotification);
    static void _windowDidMove(NSNotification *theNotification);

    //! don't create window when parent is created
    bool flagExplicitCreate:1;
    bool flagNoBackground:1;
    bool flagDoubleBuffer:1;
    bool flagTabKey:1;
    bool flagPopup:1;
    bool flagSaveUnder:1;
    bool flagStaticFrame:1;
    bool flagDrawFocus:1;
    bool flagCompressMotion:1;
    bool flagParentlessAssistant:1;
    bool flagNoMenu:1;
    
    bool flag_wm_resize:1;
    bool _bOwnsFocus:1;
    bool _bToolTipAvailable:1;

    int _b; // border
    TRGB _bg;
    TLayout *layout;
    TCursor *cursor;

    toadView *nsview;
    toadWindow *nswindow;
  // public:
    TWindow(TWindow *parent, const string &title);
    ~TWindow();

    //! Returns the parent window of this window.
    TWindow* getParent() const;
    
    static bool createParentless();
    static void destroyParentless();
    static unsigned getParentlessCount();
    static TWindow* getParentless(unsigned);

    virtual void closeRequest();
    void createWindow();
    void destroyWindow();
    void doModalLoop();
    
    void setMapped(bool);
    bool isMapped() const;
    void raiseWindow();

    void keyEvent(const TKeyEvent&);
    virtual void keyDown(TKey key, char *string, unsigned modifier);
    virtual void keyUp(TKey key, char *string, unsigned modifier);

    bool isRealized() const;
    bool setFocus();
    void _setFocus(bool);
    void _setFocusHelper(TInteractor *parent, bool b);

    void setAllMouseMoveEvents(bool all) { _allMouseMoveEvents = all; }

    void grabMouse(bool allmove=true, TWindow *confine=0, TCursor::EType type=TCursor::DEFAULT);
    void grabPopupMouse(bool allmove=true, TCursor::EType type=TCursor::DEFAULT);
    static void ungrabMouse();
    
    void getRootPos(int*,int*);
    enum EWindowPlacement {
      PLACE_SCREEN_CENTER,
      PLACE_SCREEN_RANDOM,
      PLACE_PARENT_CENTER,
      PLACE_PARENT_RANDOM, 
      PLACE_MOUSE_POINTER,
      PLACE_CORNER_MOUSE_POINTER,
      PLACE_PULLDOWN,
      PLACE_TOOLTIP
    };
    void placeWindow(EWindowPlacement how, TWindow *parent=NULL);
    void windowEvent(const TWindowEvent &we);
    virtual void mouseEvent(const TMouseEvent &);
    virtual void mouseMove(const TMouseEvent &);
    virtual void mouseEnter(const TMouseEvent &);
    virtual void mouseLeave(const TMouseEvent &);
    virtual void mouseLDown(const TMouseEvent &);
    virtual void mouseMDown(const TMouseEvent &);
    virtual void mouseRDown(const TMouseEvent &);
    virtual void mouseLUp(const TMouseEvent &);
    virtual void mouseMUp(const TMouseEvent &);
    virtual void mouseRUp(const TMouseEvent &);  

    enum EChildNotify {
      TCHILD_TITLE, TCHILD_POSITION, TCHILD_RESIZE, TCHILD_ADD,
      TCHILD_REMOVE, TCHILD_CREATE, TCHILD_DESTROY, TCHILD_ICON,
      TCHILD_FOCUS, TCHILD_DIALOG_EDIT_REQUEST, TCHILD_BEFORE_ADD,
      TCHILD_BEFORE_CREATE
    };
    //! this method should be removed in favour for event filters
    // virtual void childNotify(TWindow*, EChildNotify);


    virtual void paint();
    void doResize();
    virtual void resize();
    virtual void setPosition(TCoord x, TCoord y);
    virtual void setSize(TCoord w, TCoord h);
    virtual void setShape(TCoord x, TCoord y, TCoord w, TCoord h);
    void setShape(const TRectangle &r) {
      setShape(r.x, r.y, r.w, r.h);
    }
    virtual void getShape(TRectangle *r) const {
      *r = *this;
    }
    void invalidateWindow(bool clearbg=true);
    void invalidateWindow(TCoord x, TCoord y, TCoord w, TCoord h, bool clearbg=true);
    void invalidateWindow(const TRectangle&, bool clearbg=true);
    void invalidateWindow(const TRegion&, bool clearbg=true);
    TRegion* getUpdateRegion() const;
    
    void scrollWindow(TCoord x, TCoord y, bool bClrBG=true);
    void scrollRectangle(const TRectangle &rect, TCoord x, TCoord y, bool bClrBG=true);
    void scrollTo(TCoord x, TCoord y);
    void setOrigin(TCoord x, TCoord y);
    void getOrigin(int *x, int *y) const;
    void getOrigin(TCoord *x, TCoord *y) const;
    TCoord getOriginX() const { return origin.x; }
    TCoord getOriginY() const { return origin.y; }
    TPoint getOrigin() const { return origin; }
    
    void setBackground(TColor::EColor c) {
      const TRGB *a = TColor::lookup(c);
      _bg.r = a->r / 255.0;
      _bg.g = a->g / 255.0;
      _bg.b = a->b / 255.0;
      invalidateWindow();
    }
    void setBackground(TCoord r, TCoord g, TCoord b) {
      _bg.set(r, g, b);
    }
    const TRGB& getBackground() const { return _bg; }
    void setBorder(unsigned b){ _b=b?1:0; }
    unsigned getBorder() const {return _b;}   
    TCoord getXPos() const { return x; }
    TCoord getYPos() const { return y; }
    TCoord getWidth() const { return w; }
    TCoord getHeight() const { return h; }  

    void setCursor(TCursor::EType);
    void setCursor(const TCursor *cursor);
    
    void loadLayout(const string &filename);
    void setLayout(TLayout*);
    TLayout * getLayout() const { return layout; }

    void setToolTip(const string&);
    
    bool isFocus() { return _bOwnsFocus; }
    void paintNow();
};

void initialize(int argc, char **argv);
bool mainLoop();
bool modalLoop(TWindow*);
void terminate();


} // namespace toad

#endif
