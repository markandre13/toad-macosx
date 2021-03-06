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

#define DBM(CMD)

// debugging code added to implement popup menus
#define DBM2(X)

#define DBM_FEEL(A)

#include <toad/menubutton.hh>
#include <toad/pen.hh>
#include <toad/popup.hh>
#include <toad/action.hh>
#include <toad/command.hh>
#include <toad/bitmap.hh>
#include <toad/focusmanager.hh>

namespace toad {
  bool debug_menubutton = false;
}

using namespace toad;

namespace {
  TMenuButton * stopat = 0;
  TMenuButton * inside = 0;
}

/**
 *
 * \param p  The menubutton's parent
 */
TMenuButton::TMenuButton(TMenuHelper *p, TMenuHelper::TNode *n):
  TWindow(p, n->getTitle()), node(n), master(p)
{
  assert(master!=NULL);
//cout << "MENUBUTTON: CREATE  " << this << endl;
//  bNoBackground = true;
  setBackground(TColor::MENU);
  _down = false;
  popup = NULL;
  idx = 0;
#warning "TMenuButton doesn't see when node was changed"
//  CONNECT(node->sigChanged, this, adjust);
}

TMenuButton::~TMenuButton()
{
//cout << "MENUBUTTON: DESTROY " << this << endl;
//if (Title()=="open") {
//  cout << "UPSI!" << endl;
//}

//  printf("%s\n", __PRETTY_FUNCTION__);
//  DISCONNECT(node->sigChanged, this, adjust);
}

void TMenuButton::adjustButton()
{
//cout << "TMenuButton::adjustButton: title=\""<<getTitle()<<"\""<<endl;
  if (node->type==TMenuHelper::TNode::SEPARATOR) {
    setSize(1,4);
    return;
  }

  int w,h, n;
  
  w=0;
  h = getDefaultFont().getHeight();
  
  n = drawIcon();
  master->menu_width_icon = max(master->menu_width_icon, n);
  w+=n;
  
  n = getDefaultFont().getTextWidth(node->getLabel(idx));
  master->menu_width_text = max(master->menu_width_text, n);
  w+=n;
 
  if (node->getShortcut().size()!=0) {
    n = getDefaultFont().getTextWidth(node->getShortcut());
    master->menu_width_short = max(master->menu_width_short, n);
    w+=n;
  }

  if (master->vertical) {
    w += 12+2;
    h += 2;
  } else {
    w += 12+8;
    h += 2+8;
  }

  setSize(w,h);
}

/**
 * Draws an icon when <code>pen<code> isn't NULL and return the size of
 * the icon.
 *
 * \param pen NULL or a pen.
 * \param x   The x-coordinate.
 * \param y   The y-coordinate.
 *
 * \todo
 *   The interface isn't perfect yet:
 *   \li How about one parameter being a rectangle within the function
 *       has to place the icon?
 */
unsigned
TMenuButton::drawIcon(TPen *pen, int x, int y)
{
  if (pen && node->getIcon()) {
    pen->drawBitmap(x, y, node->getIcon());
  }
  
  if (node->getIcon())
    return node->getIcon()->getWidth();
  return 0;
}

void 
TMenuButton::paint()
{
  TPen pen(this);

  bool mark = false;
  if (master->active==this) {
    switch(master->state) {
      case MHS_DOWN:
      case MHS_UP_N_HOLD:
      case MHS_DOWN_N_HOLD:
      case 4:
      case MHS_DOWN_N_INSIDE_AGAIN:
        mark=true;
        break;
      case MHS_DOWN_N_OUTSIDE:
        mark=node->down && node->isEnabled(); // same cond as in openPopup
    }
  }

  TCoord x, y;
  if (master->vertical) {
    x = 3;
    y = 1;
  } else {
    x = 6+4-8;
    y = 1+4;
  }

#if 0
  // debug code: check `isAvailable()'
  if (!node->isAvailable()) {
    pen.setColor(0,128,0);
    pen.fillRectanglePC(0,0, getWidth(), getHeight());
    return;
  }
#endif

  const string& label = node->getLabel(idx);
  const string& shortcut = node->getShortcut();

  if (node->type==TMenuHelper::TNode::SEPARATOR) {
//    pen.setColor(TColor::MENU);
//    pen.fillRectanglePC(0,0,getWidth(), getHeight());
    TCoord y = getHeight()/2;
    pen.setColor(TColor::BTNSHADOW);
    pen.drawLine(0, y, getWidth(), y);
    pen.setColor(TColor::BTNLIGHT);
    y++;
    pen.drawLine(0, y, getWidth(), y);
    return;
  }

  TCoord x_icon = x;
  TCoord x_text = x_icon + master->menu_width_icon+8;
  TCoord x_short= x_text + master->menu_width_text+8;
  
  if (node->isEnabled()) {
#if 1
    if (mark) {
      pen.setStrokeColor(TColor::MENUTEXT);
      pen.setFillColor(TColor::SELECTED);
      pen.fillRectanglePC(1,1,getWidth()-2,getHeight()-2);
    }
    pen.setColor(TColor::MENUTEXT);
#else
    pen.setColor(mark ? TColor::MENUTEXT : TColor::MENU);
    pen.fillRectanglePC(0,0, getWidth(),getHeight());
    pen.setColor(mark ? TColor::MENU : TColor::MENUTEXT);
#endif
    drawIcon(&pen, x_icon, y);
    pen.drawString(x_text, y, label);
    if (!shortcut.empty())
      pen.drawString(x_short, y, shortcut);
  } else {
//    pen.setColor(TColor::MENU);
//    pen.fillRectanglePC(0,0,getWidth(), getHeight());

    pen.setColor(TColor::BTNLIGHT);
    pen.drawString(x_text+1, y+1, label);
    if (!shortcut.empty())
      pen.drawString(x_short+1, 1+y, shortcut);

    pen.setColor(TColor::BTNSHADOW);
    drawIcon(&pen, x_icon, y);
    pen.drawString(x_text, y, label);
    if (shortcut.size()!=0)
      pen.drawString(x_short, y, shortcut);
  }
  
  if(node->down && master->vertical &&
      (node->actions.empty() || 
        (*node->actions.begin())->type!=TAction::RADIOBUTTON
      )
    )
  {
    TCoord y=size.height/2;
    TPoint tri[3];
    tri[0].x=size.width-7; tri[0].y=y-4;
    tri[1].x=size.width-3; tri[1].y=y;
    tri[2].x=size.width-7; tri[2].y=y+4;
    pen.fillPolygon(tri, 3);
  }
}

void 
TMenuButton::closeRequest()
{
/*
  The meaning of '!inside':
  We will receive closeRequest when a mouse button is pressed outside
  the current popup window and our parent, which is this popup, is
  closing.
  When 'inside' isn't set, we received it, because the mouse was pressed
  outside the menubar and must close everything.
*/
  if (!inside) {
    DBM(cerr << "+ closeRequest " << this << "\n";)
    deactivate();
    collapse();
    dropKeyboard();
    master->state=MHS_WAIT;
    DBM(cerr << "- closeRequest " << this << "\n";)
  }
}

namespace toad {
  class TMenuKeyFilter:
    public TEventFilter
  {
    public:
      TMenuButton *active;
      bool keyEvent(TKeyEvent&);
  };
}

bool 
TMenuKeyFilter::keyEvent(TKeyEvent &ke)
{
  TKey key = ke.key;
  
  if (!active) {
    if (toad::debug_menubutton) {
      cout << "KEYDOWN (active=NULL)" << endl;
    }
    return true;
  }
  
  if (toad::debug_menubutton) {
    cout << "KEYDOWN (active=" << active->getTitle() << ")" << endl;
  }

  #warning "better to make sure that mouse button isn't pressed?"

  if(active->master->vertical) {
    switch(key) {
      case TK_RIGHT:
        key = TK_DOWN;
        break;
      case TK_LEFT:
        key = TK_UP;
        break;
      case TK_UP:
        key = TK_LEFT;
        break;
      case TK_DOWN:
        key = TK_RIGHT;
        break;
    }
  }

  TMenuButton *i=NULL;
  TMenuButton *m;
  TMenuHelper::TNode *ptr;
  
  switch(key) {
    case TK_ESCAPE:
      i = active; // 
      i->deactivate();
      i->collapse();
      i->dropKeyboard();
      return true;
      break;
    case TK_SPACE:
    case TK_RETURN:
      i = active;
      i->trigger();
      i->dropKeyboard();
      return true;
    case TK_RIGHT:
      DBM(cerr << "want to go right\n";)
      // use the order of nodes to find the next window
      // (make a subroutine out of it and add a helper function to
      // node as a node can contain multiple windows in the future!)
      ptr = active->node->next;
      while(ptr) {
        TMenuButton *window = 0;
        if (ptr->winarray && ptr->nwinarray>0)
          window = ptr->winarray[0];
        if (window && ptr->isEnabled()) {
          i = window;
          break;
        }
        ptr=ptr->next;
      }
      break;
    case TK_LEFT:
      DBM(
        cerr << "want to go left\n";
        ptr = active->node;
        cerr << "  currently at node " << ptr->getLabel(0) << endl;
        if (ptr->parent) {
          cerr << "  its parent is " << ptr->parent->getLabel(0) << endl;
        } else {
          cerr << "  it has no parent\n";
        }
        cerr << "the master of the active node is " << active->master->getTitle() << endl;
      )
      ptr = 0;
      if (active->master->root.down) {
        DBM(cerr << "  it has a root.down: " << active->master->root.down->getLabel(0) << endl;)
        ptr = active->master->root.down;
      } else {
        DBM(cerr << "  is has no root.down" << endl;)
      }
      DBM(cerr << "traversing nodes...\n";)
      while(ptr->next) {
        DBM(cerr << "  next node is " << ptr->next->getLabel(0) << endl;)
        TMenuButton *window = 0;
        if (ptr->winarray && ptr->nwinarray>0)
          window = ptr->winarray[0];
        if (window == active) {
          DBM(cerr << "  it's the active one, stop here!" << endl;)
          break;
        }
        if (window && ptr->isEnabled()) {
          DBM(cerr << "  it's enabled, so its a candidate for the next window\n";)
          i = window;
        }
        ptr = ptr->next;
      }
      break;
  }

  DBM(
    cerr << "keyboard navigation: found menubutton " <<
      (i ? i->getTitle().c_str() : "(none)") << endl;
  )
  
  if (i) {
    i->activate();
    active->master->state=MHS_UP_N_HOLD;
    return true;
  }
  
  switch(key) {
    case TK_DOWN:
      DBM(
        cerr << "try to go down\n";
        cerr << "want to go down\n";
      )
      ptr = active->node;
      DBM(cerr << "  currently at node " << ptr->getLabel(0) << endl;)
      ptr = active->node->down;
      while(ptr) {
        TMenuButton * window = 0;
        if (ptr->winarray && ptr->nwinarray>0)
          window = ptr->winarray[0];
        if (window && ptr->isEnabled()) {
          i = window;
          break;
        }
        ptr = ptr->next;
      }
      break;
    case TK_UP:
    case TK_LEFT:
      DBM(
        cerr << "\ntry to go up\n";
        cerr << "the master of the active node is " << active->master->getTitle() << endl;
      )
      if (active->master->btnmaster) {
        DBM(cerr << "  it's master is " << active->master->btnmaster->getTitle();)
        i = active->master->btnmaster;
        active->master->active = 0;
        active->invalidateWindow();
      } else {
        DBM(cerr << "  it has no master itself\n";)
      }
      break;
  }
  
  if (i) {
    i->grabKeyboard();
    i->activate();
    i->master->state=MHS_UP_N_HOLD;
  }
  return true;
}


static TMenuKeyFilter *keyfilter = NULL;

void 
TMenuButton::grabKeyboard()
{
  if (keyfilter==NULL) {
//    if (toad::debug_menubutton) {
//      cout << "ADDING KEYFILTER" << endl;
//    }
    keyfilter = new TMenuKeyFilter();
    TFocusManager::insertEventFilter(keyfilter, NULL, KF_GLOBAL);
  }
}

void 
TMenuButton::dropKeyboard()
{
  if (keyfilter!=NULL) {
//    if (toad::debug_menubutton) {
//      cout << "REMOVING KEYFILTER" << endl;
//    }
    TFocusManager::removeEventFilter(keyfilter);
    delete keyfilter;
    keyfilter=NULL;
  }
}

static const char * statename(EMenuHelperState n) {
  static const char* a[] = {
    "MHS_WAIT",
    "MHS_DOWN",
    "MHS_UP_N_HOLD",
    "MHS_DOWN_N_HOLD",
    "MHS_RESERVED",
    "MHS_DOWN_N_OUTSIDE",
    "MHS_DOWN_N_INSIDE_AGAIN"
  };
  return a[n];
}

void 
TMenuButton::mouseLDown(const TMouseEvent &)
{
  stopat = this;
  DBM(cerr << "+ mouseLDown " << this << ": state " << statename(master->state) << endl;)
  switch(master->state) {
    case MHS_WAIT:
      activate();
      master->state=MHS_DOWN;
      break;
    case MHS_UP_N_HOLD:
      if (master->active!=this) {
        activate();
        master->state=MHS_DOWN;
      } 
      else {
        master->state=MHS_DOWN_N_HOLD;
      }
      break;
    default:
      cerr << __PRETTY_FUNCTION__ << ": unexpected state " << statename(master->state) << endl;
  }
  DBM(cerr << "- mouseLDown " << this << ": state " << statename(master->state) << endl;)
}

void 
TMenuButton::mouseLUp(const TMouseEvent &)
{
  stopat = 0;
  DBM(cerr << "+ mouseLUp " << this << ": state " << statename(master->state) << endl;)
  switch(master->state) {
    case MHS_DOWN:
DBM2(
cerr << "*** MHS_DOWN" << endl;
if (master->btnmaster)
  cerr << "  master->btnmaster is set" << endl;
else
  cerr << "  master->btnmaster isn't set" << endl;
if (node->down)
  cerr << "  node->down is set\n";
else
  cerr << "  node->down isn't set\n";
if (node->isEnabled())
  cerr << "  node is enabled\n";
else
  cerr << "  node isn't enabled\n";
)
      // first level menu button pressed & release in popup menubar
      if ( node->isEnabled() && 
           ( ( !master->btnmaster && 
               !node->down) || 
             ( !node->actions.empty() && 
               (*node->actions.begin())->type==TAction::RADIOBUTTON)
           )
         )
      {
        dropKeyboard();
        trigger();
        master->state=MHS_WAIT;
      } else
  
      if (
        master->btnmaster // this should be always true: master->master->state=2
        && !(node->down && node->isEnabled()) // must be same cond as in OpenPopup
         ) 
      {
        dropKeyboard();
        trigger();
        master->state=MHS_WAIT;
      } else {
        master->state=MHS_UP_N_HOLD;
      }
      break;
    case MHS_DOWN_N_HOLD:
    case MHS_DOWN_N_OUTSIDE:
      deactivate();
      collapse();
      dropKeyboard();
      master->state=MHS_WAIT;
      if (master->close_on_close)
        master->destroyWindow(); 
      break;
    case MHS_DOWN_N_INSIDE_AGAIN:
      dropKeyboard();
      trigger();
      break;
//    case MHS_UP_N_HOLD:
//      break;
    default:
      cout << __PRETTY_FUNCTION__ << ": unexpected state " << statename(master->state) << endl;
  }
  DBM(cerr << "- mouseLUp " << this << ": state " << statename(master->state) << endl;)
}

/**
 * Delegate to mouseLDown for popup menus, which are controlled with
 * the right mouse button.
 */
void 
TMenuButton::mouseRDown(const TMouseEvent &m)
{
  mouseLDown(m);
}

/**
 * Delegate to mouseLUp for popup menus, which are controlled with
 * the right mouse button.
 */
void 
TMenuButton::mouseRUp(const TMouseEvent &m)
{
  mouseLUp(m);
}

void 
TMenuButton::mouseLeave(const TMouseEvent &m)
{
  inside = 0;
  DBM(cerr << "+ TMenuButton::mouseLeave " << getTitle() << ": state " << statename(master->state) << endl;)
  switch(master->state) {
    case MHS_WAIT:
    case MHS_DOWN_N_OUTSIDE:
    case MHS_UP_N_HOLD:
    case MHS_DOWN_N_HOLD:
      break;
    case MHS_DOWN:
    case MHS_DOWN_N_INSIDE_AGAIN:
      master->state=MHS_DOWN_N_OUTSIDE;
      invalidateWindow();
      break;
    default:
      cout << __PRETTY_FUNCTION__ << ": unexpected state " << statename(master->state) << endl;
  }
  DBM(cerr << "- TMenuButton::mouseLeave " << getTitle() << ": state " << statename(master->state) << endl;)
}

void TMenuButton::mouseEnter(const TMouseEvent &m)
{
  inside = this;
  DBM(cerr << "+ TMenuButton::mouseEnter " << getTitle() << ": state " << statename(master->state) << endl;)
  switch(master->state) {
    case MHS_WAIT:
    case MHS_UP_N_HOLD:
    case MHS_DOWN_N_OUTSIDE:
    case MHS_DOWN_N_HOLD:
      if (m.modifier()&(MK_LBUTTON|MK_RBUTTON) && node->isEnabled()) {
        DBM(cout << "  mouseEnter with mouse pressed" << endl;)
        stopat = this;
        if (master->active) {
          #warning "here is a dirty hack to avoid an unwanted close in deactivate"
          bool hack = master->flagPopup;
          master->flagPopup = false;
          master->active->deactivate();
          master->flagPopup = hack;
        }
        activate();
        master->state = MHS_DOWN_N_INSIDE_AGAIN;
        stopat = 0;
      }
      break;
    default:
      cout << "TMenuButton::mouseEnter " << getTitle() << ": unexpected state " << statename(master->state) << endl;
  }
  DBM(cerr << "- TMenuButton::mouseEnter " << getTitle() << ": state " << statename(master->state) << endl;)
}

void 
TMenuButton::closePopup()
{
  DBM(cerr << "+ closePopup " << this << "\n";)
  if (popup) {
    delete popup;
    popup = NULL;
  }
  DBM(cerr << "- closePopup " << this << "\n";)
}

void 
TMenuButton::openPopup()
{
  if (node->down && node->isEnabled() && popup==NULL &&
       (node->actions.empty() || 
         (*node->actions.begin())->type!=TAction::RADIOBUTTON
       )
     )
  {
    DBM(cerr << "+ openPopup " << this << "\n";)
    popup = new TPopup(this, "popup");
    popup->btnmaster = this;
    popup->root.down = node->down;

    if (master->vertical)
      popup->placeWindow(PLACE_PULLDOWN, this, getWidth(), -getHeight());
    else
      popup->placeWindow(PLACE_PULLDOWN, this);

    popup->createWindow();
    DBM(cerr << "- openPopup " << this << "\n";)
  }
}

void 
TMenuButton::activate()
{
  DBM(cerr << "+ activate " << this << "\n";)
//  GrabPopupMouse(TMMM_PREVIOUS, TCursor::MOVE);

  // the following situation happens because of another bug in code
  // currently under development: should be removed later!!!
  if (!isRealized()) {
    cerr << "got 'activate()' for TMenuButton '" << getTitle() 
         << "' but it's not realized anymore" << endl;
    // printStackTrace();
    return;
  }

  grabPopupMouse();
  grabKeyboard();
  if (toad::debug_menubutton) {
    cout << "grabbed " << this << endl;
    DBM_FEEL(cout << "grab " << getTitle() << endl;)
  }

  if (master->active && master->active!=this) {
    master->active->closePopup();
    master->active->invalidateWindow();
  }
  master->active = this;
  if (keyfilter)
    keyfilter->active = this;
  invalidateWindow();

  openPopup();
  DBM(cerr << "- activate " << this << "\n";)
}

void
TMenuButton::deactivate()
{
  DBM(cerr << "+ deactivate " << this << " with master " << master << endl;)
  if (master->active) {
    master->active->closePopup();
    master->active->invalidateWindow();
  }
  master->active = NULL;
  if (keyfilter)
    keyfilter->active = NULL;
  
  ungrabMouse();
  if (toad::debug_menubutton) {
    cout << "ungrabbed " << this << endl;
    DBM_FEEL(cout << "ungrab " << getTitle() << endl;)
  }
  closePopup();
  master->state = MHS_WAIT;

  #warning "just destroying a window because it's master is a popup is to rude"
  if (master->flagPopup) {
DBM2( cerr << "*** MASTER IS A POPUP ==> DESTROY WINDOW" << endl; )
    master->destroyWindow();
  }
  
  DBM(cerr << "- deactivate " << this << endl;)
}

namespace toad {
  class TCommandCollapseMenu:
    public TCommand
  {
      TMenuButton *mb;
    public:
      TCommandCollapseMenu(TMenuButton *m):mb(m) {}
      void execute() {
        DBM(cerr << "DEACTIVATE MENUBUTTON" << endl;)
        mb->deactivate();
      }
  };

  class TCmdTriggerNode:
    public TCommand
  {
      TMenuHelper::TNode *node;
      unsigned idx;
    public:
      TCmdTriggerNode(TMenuHelper::TNode *n, unsigned i) {
        node = n;
        idx = i;
      }
      void execute() {
        DBM(cerr << "TRIGGER NODE" << endl;)
        node->trigger(idx);
      }
  };
}

void
TMenuButton::trigger()
{
  deactivate();

  /* We must collapse before triggering the node
   * o Triggering the node creates a new TAction, ie. when a new
   *   window with a TTextArea is opened.
   * o The new action will cause TMenuLayout to rebuild its tree.
   * o All menubuttons are closed when rebuiling the tree.
   * o The delete message created by collapse would now contain an
   *   invalid reference to a menubutton which was removed in the
   *   previous step.
   */
  collapse();
  sendMessage(new TCmdTriggerNode(node, idx));
}

void TMenuButton::collapse()
{
  if (master->btnmaster && this!=stopat) {
    DBM(cerr << "collapse: " << this << " -> " << master->btnmaster << endl;)
    master->btnmaster->collapse();
  } else {
if (this==stopat) { cerr << "HIT STOPAT" << endl; }
    DBM(cerr << "collapse: " << this << " -> " << "deactivate()" << endl;)
    sendMessage(new TCommandCollapseMenu(this));
  }
}

unsigned
TMenuRadioButton::drawIcon(TPen *pen, int x, int y)
{
  if (pen && choice->getSelection()==idx) {
    pen->fillCircle(x+1,getHeight()/2-3,6,6);
  }
  return 6;
}
