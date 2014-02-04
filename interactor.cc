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

#include <toad/interactor.hh>

using namespace toad;

/**
 * \class toad::TInteractor
 *
 * \brief The base class for TWindow and other objects in the 'window' hierachy.
 *
 * Windows on the screen are organzied as a tree, with the desktop being the
 * root window and each window containing a variable number of other windows.
 *
 * With TOAD, this tree isn't just used for windows, but for other user interface
 * objects too. E.g.:
 *
 * <ul>
 *   <li>
 *     TAction objects can be displayed by the nearest TMenuBar upwards in
 *     the interactor hierachy.
 *   <li>
 *     TUndo objects are added to the nearest TUndoManager upwards in the interactor
 *     hierachy.
 * </ul>
 *
 * Other possible usages:
 * <ul>
 *   <li>lightweight widgets</li>
 *   <li>drop areas for drag'n drop operations</li>
 *   <li>hot spots (invisible area which trigger events on mouse clicks)</li>
 *   <li>figures that use their parent windows for drawing</li>
 *   <li>gesture recognition</li>
 * </ul>
 */

TInteractor::TInteractor(TInteractor *parent, const string &title)
{
  taborder=0;
  flagShell = flagFocusManager = flagFocusTraversal = false;
  flagNoFocus = true;
  before_create = true;
  beforeAddEnabled = false;
  
  if (parent && parent->beforeAddEnabled) {
    parent->beforeAdd(&parent);
  }

  this->parent = parent;
  this->title = title;
  child = next = NULL;
  
  if (parent) {
    TInteractor *ptr = parent->child;
    if (!ptr) {
      parent->child = this;
    } else {
      // new taborder is calculated as `maximum+1' just in case we
      // encounter gaps
      //----------------------------------------------------------
      taborder = max(taborder, ptr->taborder);
      while(ptr->next) {
        ptr = ptr->next;
        taborder = max(taborder, ptr->taborder);
      }
      ptr->next = this;
      taborder++;
    }
  }
}

TInteractor::~TInteractor()
{
  deleteChildren();

  // remove myself from the parent
  if (parent) {

if (!parent->child) {
  cerr << "child '" << getTitle() << "' was already removed from its parent '"
       << parent->getTitle() << "'\n";
  exit(0);
}
  
    if (parent->child == this) {
      parent->child = next;
      next = NULL;
    } else {
      TInteractor *prev = getPrevSibling(this);
      prev->next = next;
      next = NULL;
    }
  }
}

void TInteractor::windowEvent(const TWindowEvent&) {}
void TInteractor::mouseEvent(const TMouseEvent&) {}
void TInteractor::keyEvent(const TKeyEvent&) {}

/**
 * This method was added for TMDIWindow and allows the window to define a
 * new parent before a child window is being added to it. The 
 * `enableBeforeAdd' attribute toggles whether this method is called.
 */
void TInteractor::beforeAdd(TInteractor**)
{
}

/**
 * Called when the window (interactor) or one of its parents received the 
 * keyboard focus.
 * TWindow::isFocus will return `true' when the window really has 
 * the focus.
 *
 * \param focus
 *   obsolete? parent && my focus?
 */
void TInteractor::focus(bool focus){}

/**
 * Called when the keyboard focus has changed.
 *
 * `true' when the windows (interactors) focus domain received the focus and
 * `false' when the focus domain lost the focus.
 */
void TInteractor::domainFocus(bool){}


/**
 * Returns the windows (interactors) first child. This method is faster
 * than <code>LastChild()</code>.
 */
TInteractor* 
TInteractor::getFirstChild() const
{
  return child;
}

TInteractor* 
TInteractor::getLastChild() const
{
  if (!child)
    return NULL;
    
  TInteractor *ptr = child;
  while(ptr->next) {
    ptr = ptr->next;
  }
  return ptr;
}

/**
 * Returns `true' when argument is a direct or indirect child of this window.
 */
bool 
TInteractor::isChildOf(const TInteractor *p) const
{
  const TInteractor *ptr = this->parent;
  while(ptr) {
    if (ptr==p)
      return true;
    ptr = ptr->parent;
  }
  return false;
}

/**
 * Delete all children.
 */  
void
TInteractor::deleteChildren()
{
  TInteractor *ptr;
  ptr = getFirstChild();
  while(ptr) {
    TInteractor *w = ptr;
    ptr = ptr->next;
    delete w;
  }
}

TInteractor* 
TInteractor::getNextSibling(const TInteractor *ptr)
{
  return ptr->next;
}

/**
 * This method is slower than <code>NextSibling</code>.
 */
TInteractor* 
TInteractor::getPrevSibling(const TInteractor *ptr)
{
  TInteractor *prev = ptr->parent->child;
  if (prev==ptr)
    return 0;
  while(prev->next!=ptr)
    prev = prev->next;
  return prev;
}

bool TInteractor::isRealized() const {return true;}
bool TInteractor::setFocus() {return false;}
void TInteractor::setPosition(TCoord x, TCoord y) {}
void TInteractor::setSize(TCoord w, TCoord h) {}
void TInteractor::setShape(TCoord x, TCoord y, TCoord w, TCoord h) {}
//void TInteractor::SetShape(const TRectangle &r){SetShape(r.x,r.y,r.w,r.h);}
//void TInteractor::SetShape(const TRectangle *r){SetShape(r->x,r->y,r->w,r->h);}
void TInteractor::getShape(TRectangle*) const {}
void TInteractor::setSuppressMessages(bool) {}
TCoord TInteractor::getXPos() const {return 0;}
TCoord TInteractor::getYPos() const {return 0;}
TCoord TInteractor::getWidth() const {return 0;}
TCoord TInteractor::getHeight() const {return 0;}
