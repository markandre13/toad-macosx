/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef __TOAD_INTERACTOR_HH
#define __TOAD_INTERACTOR_HH 1

#include <toad/types.hh>
#include <toad/connect.hh>

namespace toad {

class TWindow;
class TMouseEvent;
class TKeyEvent;
class TWindowEvent;

class TInteractor:
  public TSlot
{
  public:
    TInteractor(TInteractor *parent, const string &title);
    virtual ~TInteractor();

    virtual void setTitle(const string &title) {
      this->title = title;
    }
    const string& getTitle() const { return title; }
    TInteractor* getParent() const {return parent;}
    TInteractor* getFirstChild() const;
    TInteractor* getLastChild() const;
    bool isChildOf(const TInteractor*) const;
    void deleteChildren();

    // Get the next/previous sibling of 'ptr' and return NULL when 'ptr'
    // points to the last/first child.
    static TInteractor* getNextSibling(const TInteractor *ptr);
    TInteractor* getNextSibling() const { return getNextSibling(this); }
    static TInteractor* getPrevSibling(const TInteractor *ptr);
    TInteractor* getPrevSibling() const { return getPrevSibling(this); }

    bool flagShell:1;
    bool flagFocusManager:1;
    bool flagFocusTraversal:1;
    bool flagNoFocus:1;
    bool before_create:1;

    virtual bool isRealized() const;
    virtual bool setFocus();
    virtual void setPosition(TCoord x, TCoord y);
    virtual void setSize(TCoord w, TCoord h);
    virtual void setShape(TCoord x, TCoord y, TCoord w, TCoord h);
    inline void setShape(const TRectangle &r){setShape(r.origin.x,r.origin.y,r.size.width,r.size.height);}
    inline void setShape(const TRectangle *r){setShape(*r);}
    virtual void getShape(TRectangle*) const;
    virtual void setSuppressMessages(bool);
    virtual TCoord getXPos() const;
    virtual TCoord getYPos() const;
    virtual TCoord getWidth() const;
    virtual TCoord getHeight() const;

//  protected:

    // to avoid clashes with other methods, these virtual methods are
    // prefixed with 'interactor', to show that they're internal to
    // TInteractor and TWindow
    virtual void _interactor_init(){}
    virtual void _interactor_adjustW2C(){}
    virtual void _interactor_create(){}

    virtual void domainFocus(bool);
    virtual void focus(bool);
    
    unsigned taborder;

  protected:
    bool beforeAddEnabled:1;
    virtual void beforeAdd(TInteractor**);
    string title;

  private:
    TInteractor *parent;
    TInteractor *child;
    TInteractor *next;
};

} // namespace toad

#endif
