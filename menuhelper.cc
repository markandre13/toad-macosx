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

#include <toad/menuhelper.hh>
#include <toad/menubutton.hh>
#include <toad/io/urlstream.hh>
#include <toad/action.hh>
#include <fstream>

#include <stack>
#include <algorithm>

using namespace std;
using namespace toad;

#define DBM_FEEL(A)
#define DBM(A)

// debugging code added to implement popup menus
#define DBM2(X)


#define TOAD_WARN(MSG) \
  cout << "toad warning (file \"" << __FILE__ << "\", line " << __LINE__ << ')' << endl \
       << MSG << endl;

/**
 * \class toad::TMenuHelper
 *
 * TMenuHelper is a base class for TMenuBar and similar classes.
 
 
 * TMenuEntry
 * TMenuSeparator
 
 
 *
 * \todo
 *   \li
 *     nodes aren't removed in TMenuHelper's destructor
 *   \li
 *     popup menu's are outside the screen when opened at the left or
 *     right screen side
 *   \li
 *    down on menubutton, move to next button, up, popup closes, but the
 *    next click to open the menu fails
 *    toad::TMenuButton::mouseLDown(int, int, unsigned int): unexpected state 5
 *   \li
 *     popup menus
 *   \li
 *     add filter for keyboard event to act on menu shortcuts
 *   \li
 *     activate menu when Alt is pressed
 *   \li
 *     close menu on [ESC]
 *   \li
 *     checkbuttons
 *   \li
 *     add actions not mentioned in the layout to the layout to
 *     ease editing
 *   \li
 *     support 'icon' again
 */
 
/*
 * \note
 *
 * OUT OF DATE:  
 *
 *   \li
 *     You have to create a menubar first.
 *   \li
 *     You have to call TMenuBar::doLayout to read the layout definition.
 *   \li
 *     doLayout will create a tree of TNodes, with 'root' as it's root
 *     node.
 *   \li
 *     When a new TAction is created it calls TMenuBar::find to locate
 *     a menubar and then add's itself to the menubar with TMenuBar::addAction.
 *   \li
 *     TMenuBar::addAction locates a node in the node tree or creates a
 *     new node and calls the nodes addAction method.
 *   \li
 *     The nodes addAction method add's the new action to its list of
 *     actions and connects the actions sigChanged method with it's
 *     actionChanged method.
 *   \li
 *     When the menubar is created, TMenuHelper::resize will create
 *     TMenuButtons by calling TMenuHelper::TNode::createWindowAt.
 *   \li
 *     createWindowAt will create a TMenuButton, TMenuRadioButton, etc.
 *     child in the TMenuHelper window.
 *   \li
 *     The TMenuButton contains the whole state machine for the menubar
 *     behaviour. Including mouse and keyboard handling. The state itself 
 *     is stored in the menubuttons parents 'state' attribute. This way
 *     the menubuttons coordinate their behaviour.
 *   \li
 *     ...
 *
 *   The data structure looks like this:
 *
 *   \pre
           window tree            node tree
           TMenuHelper          TMenuHelper.root.down
               |
         n x TMenuButton            
                                    --node--> node->trigger()
                                   /
 TMenuHelper --------> TMenuButton -popup--> TMenuHelper
             <-master-             <-btnmaster-
 
     \endpre
 */

TMenuHelper::TMenuHelper(TWindow *p, const string& t):
  super(p, t), root(this)
{
  setBackground(TColor::MENU);
  setSize(320, 16);
  active = NULL;
  vertical = true;
  close_on_close = false;
  btnmaster = NULL;
  state = MHS_WAIT;
  menu_width_icon = menu_width_text = menu_width_short = menu_width_sub = 0;
}

TMenuHelper::~TMenuHelper()
{
  // all windows below `root.down' will be destroyed, so we have to
  // clear all nodes below this one
  TNode *node = root.down;
  while(node!=NULL) {
    node->noWindow();
    node = node->next;
  }
  
  // if tree owner then delete nodes!
  // (missing)
  // cerr << "parent: " << root.parent << endl;
}

/**
 * Set the menu's TMenuLayout scope to the specified interactor.
 *
 * Use only actions which are part of the specified interactor.
 *
 * This method is used for example by TFigureEditor to create popup
 * menu's for interactors which aren't part of the window tree. Namely 
 * figures in the case of TFigureEditor.
 *
 * Another case is to use a menubar and a popup menu within the same
 * top level window. Actions above the interactor will not be displayed
 * in the menu.
 */
void
TMenuHelper::setScopeInteractor(TInteractor *interactor)
{
  TMenuLayout *layout = dynamic_cast<TMenuLayout*>(getLayout());
  assert(layout);
  layout->scope = TMenuLayout::INTERACTOR;
  layout->interactor = interactor;
}

/**
 * Arrange the windows contents either horizontal (menubar) or 
 * vertical (popup).
 *
 * This one is also responsible to create TMenuButtons.
 */
void 
TMenuHelper::resize()
{
  if (!isRealized()) {
    return;
  }

  menu_width_icon = menu_width_text = menu_width_short = menu_width_sub = 0;
  TInteractor *i;

#if 1
  i = getFirstChild();
  while(i) {
    TMenuButton *mb = dynamic_cast<TMenuButton*>(i);
    if (mb)
      mb->adjustButton();
    i = i->getNextSibling();
  }
#else
  TWindowEvent we;
  we.window = 0;
  we.type   = TWindowEvent::ADJUST;
  i = getFirstChild();
  while(i) {
    i->windowEvent(we);
    i = i->getNextSibling();
  }
#endif

  if (vertical) {
    // vertical
//cerr << "TMenuHelper::resize: vertical" << endl;
    TNode *node = root.down;
    int y=0, rw = 0;
    while(node!=NULL) {
      if (node->isAvailable()) {
        node->createWindowAt(this);
        y+=node->getHeight();
      } else {
        node->deleteWindow();
      }
      node = node->next;
    }
    rw = 3+menu_width_icon+8+menu_width_text+8+menu_width_short+8;
    setSize(rw, y);
    node = root.down;
    y=0;
    while(node!=NULL) {
      if (node->isRealized()) {
        node->setPosition(0, y);
        node->setSize(rw, -1);
        y+=node->getHeight();
      }
      node = node->next;
    }
  } else {
    // horizontal
//cerr << "TMenuHelper::resize: horizontal" << endl;
    TNode *node = root.down;
    int x=0, y=0, rh = 0;
    while(node!=NULL) {
      if (node->isAvailable()) {
        node->createWindowAt(this);
      } else {
        node->deleteWindow();
      }
      if (node->isRealized()) {
        if (x > 0 && x+node->getWidth() > getWidth()) {
          x=0;
          y+=rh;
          rh=0;
        }
        node->setPosition(x,y);
        x+=node->getWidth();
        if (rh < node->getHeight())
          rh = node->getHeight();
      }
      node = node->next;
    }
    setSize(-1, y+rh);
  }
}

void
TMenuHelper::closeRequest()
{
  if (flagPopup) {
    destroyWindow();
  }
}

/**
 * Create and/or return a node in the tree pointed to by root and which
 * corresponds with the path in str.
 */
static TMenuHelper::TNode* 
insert_node(
  const string &str, 
  TMenuHelper::TNode **root,
  TMenuHelper::TNode *parent,
  unsigned depth=0)
{
  bool added = false;
  size_t strpos;
  string left;
  
//for(unsigned i=0; i<depth; ++i) cerr << "  "; cerr << "str = '" << str << "'\n";

  strpos = str.find('|');
  left = str.substr(0, strpos);
  
  TMenuHelper::TNode *node = NULL;
  if (*root==NULL) {
    *root = node = new TMenuHelper::TNode(left);
DBM_FEEL(cout << "adding node " << left << ":" << parent << endl;)
    node->parent = parent;
    added = true;
  } else {
    node = *root;
    while(true) {
      if (node->title == left) {
        break;
      }
      if (node->title == str) {
        return node;
      }
      if (node->next==NULL) {
        // no node in the tree so add a new one
        node->next = new TMenuHelper::TNode(left);
        node->next->parent = parent;
DBM_FEEL(cout << "adding node " << left << ":" << parent << endl;)
        added = true;
        node = node->next;
        break;
      }
      node = node->next;
    }
  }

// for(unsigned i=0; i<depth; ++i) cerr << "  ";
  
  if (strpos==string::npos) {
//    cerr << "return\n";
#if 0
    if (!added) {
      cerr << "conflict!" << endl;
      return NULL;
    }
#endif
    return node;
  }
// cerr << "  "; cerr << "calling insert_node\n";
  return insert_node(str.substr(strpos+1), &node->down, node, depth+1);
}

static bool flag[1024];

static void 
printTree(TMenuHelper::TNode *p, unsigned d = 0)
{
#if 0
  for(unsigned i=0; i<depth; i++)
    cout << "  ";
  cout << p->title << endl;
  if (p->down)
    print_tree(p->down, depth+1);
  if (p->next)
    print_tree(p->next, depth);
#else
  for(int i=0; i<d; i++) {
    if (i==d-1) {
      if(flag[d-1])
        cout << "\033(0tqqq\033(B";
      else
        cout << "\033(0mqqq\033(B";
    } else {
      if (flag[i])
        cout << "\033(0x\033(B   ";
      else
        cout << "    ";
    }
  }
//  cout << p->Title() << " [" << p << "]" << endl;
  cout << p->getTitle() << " (" << p->getLabel(0) << ") " << " [" << p << "]" << endl;
  
  TMenuHelper::TNode *c = p->down;
  if (c==NULL)
    return;
  do {
    TMenuHelper::TNode *n = c->next;
    flag[d] = (n!=NULL);
    printTree(c, d+1);
    c=n;
  } while(c!=NULL);
#endif
}

/**
 * Perform sanity checks on the tree.
 *
 * The checks are:
 * \li a node with children musn't contain an action
 */
void
TMenuHelper::sanityCheckTree(TMenuHelper::TNode *p)
{
  TMenuHelper::TNode *c = p->down;

  if (c==NULL)
    return;
  
  // a node with children musn't contain an action
  if (!p->actions.empty()) {
    TOAD_WARN("a node with children shouldn't contain an action");
  }

  do {
    sanityCheckTree(c);
    c = c->next;
  } while(c!=NULL);
}

static void
layout2nodes(TMenuEntryList::iterator p, 
             TMenuEntryList::iterator e, 
             TMenuHelper::TNode *top)
{
  TMenuHelper::TNode *node = top->down;
  while(p!=e) {
    TMenuHelper::TNode * newnode = new TMenuHelper::TNode((*p)->title,
                                (*p)->label.c_str(),
                                (*p)->shortcut.c_str(),
                                0, /* bitmap, still missing */
                                (*p)->type);
    newnode->parent = top;
    if (!node) {
      top->down = newnode;
    } else {
      node->next = newnode;
    }
    node = newnode;
    layout2nodes((*p)->entries.begin(), (*p)->entries.end(), node);
    ++p;
  }
}

// TMenuHelper::TNode
//---------------------------------------------------------------------------
TMenuHelper::TNode::TNode(const string &t, 
                   const char *l,
                   const char *s,
                   TBitmap *i,
                   EType nt):
  title(t), type(nt), icon(i)
{
  next = down = parent = NULL;
  winarray = NULL;
  if (l)
    label = l;
  else
    label = title;
  if (s) {
    shortcut = s;
  }
}

TMenuHelper::TNode::TNode()
{
  next = down = parent = NULL;
  winarray = NULL;
  icon = NULL;
  type = NORMAL;
}

TMenuHelper::TNode::~TNode()
{
  TActionSet::iterator p(actions.begin()), e(actions.end());
  while(p!=e) {
    (*p)->sigChanged.remove(this);
    ++p;
  }
  
  if (winarray) {
    for(size_t i=0; i<nwinarray; ++i) {
      delete winarray[i];
    }
    delete[] winarray;
  }
}

TMenuHelper::TRootNode::TRootNode(TMenuHelper *owner) 
{
  this->owner = owner;
}

TMenuHelper::TRootNode::~TRootNode()
{
//  clear();
}


void
TMenuHelper::TRootNode::clear()
{
  if (down)
    deleteTree(down);
  if (next)
    deleteTree(next);
  down = 0;
  next = 0;
}

void
TMenuHelper::TRootNode::deleteTree(TNode *node)
{
  TNode *tmp;
  TNode *ptr = node;
  while(ptr) {
    tmp = ptr;
    ptr = ptr->next;
    tmp->parent = NULL;
    if (tmp->down)
      deleteTree(tmp->down);
    delete tmp;
  }
}

/**
 * @return <code>true</code> when the node is enabled.
 */
bool 
TMenuHelper::TNode::isEnabled() const
{
  if (type==SEPARATOR) {
    return false;
  }

  if (down) {
    TNode *p = down;
    while(p) {
      if (p->isEnabled()) {
        return true;
      }
      p = p->next;
    }
    return !actions.empty();
  }

  if (actions.empty())
    return false;

  TActionSet::iterator p(actions.begin()), e(actions.end());    
  while(p!=e) {
    if ((*p)->isEnabled()) {
      return true;
    }
    ++p;
  }
  return false;
}

/**
 *
 * @return <code>true</code> when the node has at last one action
 *  assigned to it and can be enabled and disabled.
 */
bool 
TMenuHelper::TNode::isAvailable() const
{
  if (type==SEPARATOR)
    return true;

  // nodes with an empty label are always non-available unless they
  // are of type RADIOBUTTON, in which case 'createWindowAt' is performing
  // the required checks
  if (label.empty()) {
    if (actions.empty() ||
       (*actions.begin())->type != TAction::RADIOBUTTON) 
    {
      return false;
    }
  }

DBM2(cerr << "      " << __PRETTY_FUNCTION__ << endl;)
  if (down) {
DBM2(cerr << "        is down\n";)
    TNode *p = down;
    while(p) {
      if (p->type!=SEPARATOR && p->isAvailable()) {
        return true;
DBM2(cerr << "        got available\n";)
      }
      p = p->next;
    }
DBM2(cerr << "        not available\n";)
    return !actions.empty();
  }


  if (actions.empty()) {
DBM2(cerr << "        no actions -> not available\n";)
    return false;
//return true;
  }
DBM2(cerr << "        actions -> available\n";)
  return true;
}

/**
 * Add an action to the node. 
 */
TAction *
TMenuHelper::TNode::addAction(TAction *a)
{
  assert(a!=NULL);
/*
  if (down) {
    TOAD_WARN("a node with children shouldn't contain an action");
  }
*/
  if (!actions.empty() && (*actions.begin())->type!=TAction::BUTTON) {
    cout<< "in file " << __FILE__ << ", line " << __LINE__
        << ": a non button menu entry can't hold multiple actions"
        << endl;
    // i've no plan how to handle it and why it might be neccessary
  }

  TActionSet::iterator p = actions.find(a);
  if (p==actions.end()) {
    actions.insert(a);
    connect(a->sigChanged, this, &TMenuHelper::TNode::actionChanged);
  } else {
    cerr << "warning: try to add action more than once to a node" << endl;
    cerr << "  action: " << a->getTitle() << endl;
    cerr << "  node  : " << (*p)->getTitle() << endl;
  }
  return a;
}

/**
 * Remove an action from the node.
 *
 * \note
 *   \li
 *      This method removes the action, but not the node, as the node
 *      may be part of the layout configuration.
 */
void 
TMenuHelper::TNode::removeAction(TAction *a)
{
DBM(cout << "remove action " << a << endl;)
  TActionSet::iterator p = actions.find(a);
  if (p!=actions.end()) {
    TAction *a = *p;
    actions.erase(p);
    a->sigChanged.remove(this);
    actionChanged();
  }
}

/** 
 * Called, when an action was added to or removed from the node and
 * when one of the assigned actions triggers its <code>sigChanged</code> 
 * signal.
 * <p>
 * The method then invalidates the window and calls <code>actionChanged</code> in
 * its parents node.
 */
void TMenuHelper::TNode::actionChanged()
{
DBM_FEEL(cout << "action changed in node " << title << endl;)
  if (winarray) {
    for(size_t i=0; i<nwinarray; i++)
      winarray[i]->invalidateWindow();
  }
  sigChanged();
  if (parent)
    parent->actionChanged();
}

void TMenuHelper::TRootNode::actionChanged()
{
//  cout << "ROOT NODE RECEIVED ACTION CHANGED" << endl;
  owner->resize();
}

/**
 * Triggers <code>sigClicked</code> in all assigned actions.
 *
 * \note
 *   This method uses DelayedTrigger to trigger the actions signal
 *   so the menubar can close all windows before executing the
 *   actions.
 */
void TMenuHelper::TNode::trigger(unsigned idx)
{
  for(auto &&p: actions) {
    if (p->isEnabled()) {
      p->delayedTrigger(idx);
    }
  }
}

/**
 * Create a window for the node in 'parent'.
 *
 * \param parent The parent window for the nodes window to be created.
 */
void
TMenuHelper::TNode::createWindowAt(TMenuHelper *parent)
{

  if (winarray!=NULL)
    return;

//cerr << "TMenuHelper::TNode::createWindowAt: parent=\""<<parent->getTitle()<<"\", me, node with title=\"" << title << "\"" << endl;

  vertical = parent->vertical;

  // - nodes with action=NULL are menubar entries with submenus
  // - assuming, all actions are of the same type
  
  TAction *action = 0;
  TAction::EType type = TAction::BUTTON;
  nwinarray = 1;
  if (!actions.empty()) {
    action = *actions.begin();
    type = action->type;
    nwinarray = action->getSize();
  }

  winarray = new TMenuButton*[nwinarray];
  w=0;
  h=0;
  size_t j=0;
  for(size_t i=0; i<nwinarray; i++) {
    switch(type) {
      case TAction::BUTTON:
        winarray[j] = new TMenuButton(parent, this);
        break;
      case TAction::RADIOBUTTON: {
        TAbstractChoice *choice = dynamic_cast<TAbstractChoice*>(action);
        assert(choice);
        if (!getLabel(i).empty()) {
          winarray[j] = new TMenuRadioButton(parent, this, choice->getModel(), i);
        } else {
          continue;
        }
        } break;
#if 0
      case TAction::CHECKBUTTON:
        winarray[j++] = new TMenuCheckButton(parent, this);
        break;
#endif
    }
    winarray[j]->adjustButton();
    winarray[j]->createWindow();
    
    if (vertical) {
      w = max(w, winarray[j]->getWidth());
      h += winarray[j]->getHeight();
    } else {
      w += winarray[j]->getWidth();
      h = max(h, winarray[j]->getHeight());
    }
    ++j;
  }
  nwinarray = j;
}

/**
 * Delete windows previously created by createWindowAt in this node and
 * all inferior nodes.
 */
void 
TMenuHelper::TNode::deleteWindow()
{
  TNode *p = down;
  while(p) {
    p->deleteWindow();
    p = p->next;
  }
  if (winarray) {
    for(size_t i=0; i<nwinarray; i++) {
      delete winarray[i];
    }
    delete winarray;
    winarray = NULL;
  }
}

TCoord
TMenuHelper::TNode::getHeight()
{
  return h;
}

TCoord
TMenuHelper::TNode::getWidth()
{
  return w;
}

bool
TMenuHelper::TNode::isRealized()
{
  return winarray!=NULL;
}

void
TMenuHelper::TNode::setPosition(TCoord x, TCoord y)
{
  for(size_t i=0; i<nwinarray; i++) {
    winarray[i]->setPosition(x, y);
    if (vertical) {
      y+=winarray[i]->getHeight();
    } else {
      x+=winarray[i]->getWidth();
    }
  }
}

void
TMenuHelper::TNode::setSize(TCoord w, TCoord h)
{
  for(size_t i=0; i<nwinarray; i++) {
    winarray[i]->setSize(w, h);
  }
}

/**
 * This one is a very stupid design
 */
void
TMenuHelper::TNode::noWindow()
{
  if (winarray)
    delete[] winarray;
  winarray = NULL;
}

const string&
TMenuHelper::TNode::getLabel(unsigned idx) const
{
  TAction *action = 0;
  if (!actions.empty())
    action = *actions.begin();
  if (action && action->type==TAction::RADIOBUTTON) {
    const string &id = action->getID(idx);
    TNode *ptr = down;
    while(ptr) {
      if (ptr->title == id) {
        return ptr->label;
      }
      ptr = ptr->next;
    }
//cerr << "getLabel for '" << title << "' -> '" << action->getID(idx) << "'\n";
    return id;
  }
  static string alt_label;
  bool alt_active = true;
  if (action && action->getState(&alt_label, &alt_active)) {
    return alt_label;
  }
  return label;
}

const string&
TMenuHelper::TNode::getShortcut() const
{
  return shortcut;
}

const string&
TMenuHelper::TNode::getTitle() const
{
  return title;
}

const TBitmap*
TMenuHelper::TNode::getIcon() const
{
  return icon;
}

void
TMenuEntry::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "name", title);
  ::store(out, "label", label);
  ::store(out, "shortcut", shortcut);
  ::store(out, "icon", icon);
  TMenuEntryList::const_iterator p(entries.begin()), e(entries.end());
  while(p!=e) {
    ::store(out, *p);
    ++p;
  }
}

bool
TMenuEntry::restore(TInObjectStream &in)
{
  type = TMenuHelper::TNode::NORMAL;
  TMenuEntry *entry;
  if (::restoreObject(in, &entry)) {
    entries.push_back(entry);
    return true;
  }
  
  if (
    ::restore(in, "name", &title) ||
    ::restore(in, "label", &label) ||
    ::restore(in, "shortcut", &shortcut) ||
    ::restore(in, "icon", &icon) ||
    super::restore(in))
    return true;
  ATV_FAILED(in);
  return false;
}

void
TMenuSeparator::store(TOutObjectStream &out) const
{
}

bool
TMenuSeparator::restore(TInObjectStream &in)
{
  type = TMenuHelper::TNode::SEPARATOR;
  if (TSerializable::restore(in))
    return true;
  ATV_FAILED(in);
  return false;
}

TMenuLayout::TMenuLayout()
{
//  cerr << "TMenuLayout::TMenuLayout() " << this << endl;
  scope = TOPLEVEL;
  interactor = 0;
  connect(TAction::actions.sigChanged, this, &TMenuLayout::actionsChanged);
}

TMenuLayout::TMenuLayout(const TMenuLayout&)
{
//  cerr << "TMenuLayout::TMenuLayout(const TMenuLayout&) " << this << endl;
  scope = TOPLEVEL;
  connect(TAction::actions.sigChanged, this, &TMenuLayout::actionsChanged);
}

TMenuLayout::~TMenuLayout()
{
//  cerr << "TMenuLayout::~TMenuLayout() " << this << endl;
  TAction::actions.sigChanged.remove(this);
}

/**
 * Rebuild the menuhelpers node tree according to TMenuLayout
 *
 * \li clear the menubars node tree
 * \li rebuild the menubars node tree
 * \li add actions from the action list to the node tree
 */
void
TMenuLayout::arrange()
{
//cerr << "TMenuLayout::arrange: this=" << this << ", window=" << window << endl;
  if (!window)
    return;
  if (!window->isRealized())
    return;

  TMenuHelper *menu = dynamic_cast<TMenuHelper*>(window);
  if (!menu) {
    cerr << "error: TMenuLayout isn't assigned to a TMenuHelper object" << endl;
    return;
  }
DBM2(cerr << "TMenuLayout::arrange '" << menu->getTitle() << "'\n";)
  
  menu->root.clear();
  layout2nodes(entries.begin(), entries.end(), &menu->root);
  
DBM2(cerr << "  find top level window of window '" << window->getTitle() << "'\n";)
  TWindow *toplvl = window;
  while((toplvl->flagPopup || !toplvl->flagShell) && toplvl->getParent()) {
    toplvl = toplvl->getParent();
  }
DBM2(cerr << "  okay, it's '" << toplvl->getTitle() << "'\n";)

  TActionStorage::iterator p(TAction::actions.begin()), e(TAction::actions.end());
  while(p!=e) {
    TMenuHelper::TNode *node;
DBM2(cerr << "  check node " << (*p)->getTitle() << endl;)
    switch(scope) {
      case GLOBAL:
DBM2(cerr << "    global menubar, always add\n";)
DBM2(cerr << "      create node\n";)
        node = insert_node((*p)->getTitle(), &menu->root.down, &menu->root);
        node->addAction(*p);
        break;
      case TOPLEVEL: {
DBM2(cerr << "    toplevel menubar, check that action is a child\n";)
          TInteractor *ptr = *p;
          while(ptr) {
            TWindow *w = dynamic_cast<TWindow*>(ptr);
            if (w) {
DBM2(cerr << "      create node\n";)
              node = insert_node((*p)->getTitle(), &menu->root.down, &menu->root);
DBM2(cerr << "      found window '" << w->getTitle() << "'\n";)
              if (w->flagShell || !w->getParent()) {
DBM2(cerr << "       it's a shell window or has no other parent\n";)
                if (toplvl == w) {
DBM2(cerr << "         and it's the top level window\n";)
                  node->addAction(*p);
                } else {
DBM2(cerr << "         but it's not the top level window\n";)
                }
                break;
              }
            }
DBM2(cerr << "      go up to parent\n";)
            ptr = ptr->getParent();
          }
        } break;
      case INTERACTOR:
DBM2(cerr << "      interactor scope menubar, check that action is a child\n";)
        if (interactor==*p || (*p)->isChildOf(interactor)) {
DBM2(cerr << "      create node\n";)
              TMenuHelper::TNode *node = insert_node((*p)->getTitle(), &menu->root.down, &menu->root);
DBM2(cerr << "        okay\n";)
          node->addAction(*p);
        }
        break;
    }
    ++p;
  }
//printTree(&menu->root);
DBM2(cout << "TMenuLayout::arrange done" << endl << endl << endl;)
  menu->resize();
}

void
TMenuLayout::actionsChanged()
{
//  cerr << __PRETTY_FUNCTION__ << endl;
  arrange();
}

void
TMenuLayout::store(TOutObjectStream &out) const
{
  super::store(out);
  
  string scopestr;
  switch(scope) {
    case GLOBAL: scopestr="global"; break;
    case TOPLEVEL: scopestr="toplevel"; break;
  }
  ::store(out, "scope", scopestr);
  
  TMenuEntryList::const_iterator p(entries.begin()), e(entries.end());
  while(p!=e) {
    ::store(out, *p);
    ++p;
  }
}

bool
TMenuLayout::restore(TInObjectStream &in)
{
  TMenuEntry *entry;
  if (::restoreObject(in, &entry)) {
    entries.push_back(entry);
    return true;
  }
  
  string scopestr;
  if (::restore(in, "scope", &scopestr)) {
    if (scopestr=="global")
      scope = GLOBAL;
    else if (scopestr=="toplevel")
      scope = TOPLEVEL;
    else {
      in.err << "scope must be either 'global' or 'toplevel'" << endl;
      ATV_FAILED(in);
      return false;
    }
    return true;
  }

  if (super::restore(in))
    return true;
  ATV_FAILED(in);
  return false;
}
