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

#define DBM_FEEL(A)

#include <toad/menubar.hh>
#include <toad/focusmanager.hh>
//#include <toad/form.hh>
//#include <toad/mdiwindow.hh>
#include <toad/pushbutton.hh>
#include <toad/textfield.hh>
#include <toad/action.hh>
#include <toad/utf8.hh>

#include <iostream>
#include <fstream>

using namespace toad;

/**
 * \class toad::TMenuBar
 *
 * A horizontal menubar.
 *
 * This class basically
 * \li is a horizontal TMenuHelper
 * \li sets up a TMenuLayout
 * \li installs a keyboard filter.
 *
 * The keyboard filter looks for a shortcut defined in menu layout and
 * triggers an action if it finds a match.
 *
 * Keyboard shortcuts can be defined as one or more key names connected with
 * an '+' sign, eg.  'alt+f4'.
 *
 * Known key names are case insensitive and can be one of the common keyboard
 * characters or special keys like 'alt', 'ctrl' or 'f1' to 'f20'.
 */

class TMenuBar::TMyKeyFilter:  
  public TEventFilter
{
    public:
      TMyKeyFilter(TMenuBar *menubar) {
        this->menubar = menubar;
      }
    protected:
      TMenuBar *menubar;
      bool keyEvent(TKeyEvent &ke);
};

TMenuBar::TMenuBar(TWindow *p, const string& t):
  super(p, t)
{
  vertical = false; // i'm a horizontal menuhelper
  setLayout(new TMenuLayout()); // i require a layout

  TFocusManager::insertEventFilter(new TMyKeyFilter(this), this, KF_TOP_DOMAIN);  
}

TMenuBar::~TMenuBar()
{
}

static bool
menubar_keyboard_filter_recursion(TMenuBar::TNode *node, const string &str, TKey key, unsigned modifier, unsigned indent=0);

bool
TMenuBar::TMyKeyFilter::keyEvent(TKeyEvent &ke)
{
  // separate ALT, ALTGR and CONTROL from modifier
  unsigned m = ke.modifier();
  unsigned original_modifier = m;

  unsigned modifier = 0;
  if (m & MK_CONTROL)
    modifier|=MK_CONTROL;

  // interpret both ALT and ALTGR as menu modifier
  // FIXME: should be configurable or not here at all!
  if ((m & MK_ALT) || (m & MK_ALTGR))
    modifier|=MK_ALT;

  m &= ~(MK_CONTROL|MK_ALT|MK_ALTGR);
  ke.setModifier(m);

  bool result = menubar_keyboard_filter_recursion(&menubar->root, ke.getString(), ke.getKey(), modifier);

  ke.setModifier(original_modifier);
  return result;
}

/**
 * Find a node in the menubar whose shortcut matches the pressed key.
 */
static bool
menubar_keyboard_filter_recursion(TMenuBar::TNode *node, const string &str, TKey key, unsigned modifier, unsigned indent)
{
  TMenuBar::TNode *p = node;
  while(p) {
    string s = p->getShortcut();    
    int state = 0;
    size_t pos = 0;
    string pattern;
    bool match = true;
    unsigned m = 0;
    while(state!=3) {
      string c;
      if (pos<s.size()) {
        c = s[pos++];
      } else {
        state = 3;
      }
      switch(state) {
        case 0:
          pattern+=c;
          state = 1;
          break;
        case 1:
          if (c=="+") {
            state = 2;
          } else {
            pattern+=c;
          }
          break;
      }
      
      if (state>=2) {
        if (strcasecmp(pattern.c_str(), "strg")==0 ||
            strcasecmp(pattern.c_str(), "ctrl")==0 )
        {
          m |= MK_CONTROL;
        } else
        if (strcasecmp(pattern.c_str(), "alt")==0)
        {
          m |= MK_ALT;
        } else
        if (strcasecmp(pattern.c_str(), "f1")==0) {
          if (key!=TK_F1)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f2")==0) {
          if (key!=TK_F2)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f3")==0) {
          if (key!=TK_F3)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f4")==0) {
          if (key!=TK_F4)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f5")==0) {
          if (key!=TK_F5)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f6")==0) {
          if (key!=TK_F6)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f7")==0) {
          if (key!=TK_F7)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f8")==0) {
          if (key!=TK_F8)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f9")==0) {
          if (key!=TK_F9)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f10")==0) {
          if (key!=TK_F10)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f11")==0) {
          if (key!=TK_F11)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f12")==0) {
          if (key!=TK_F12)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f13")==0) {
          if (key!=TK_F13)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f14")==0) {
          if (key!=TK_F14)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f15")==0) {
          if (key!=TK_F15)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f16")==0) {
          if (key!=TK_F16)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f17")==0) {
          if (key!=TK_F17)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f18")==0) {
          if (key!=TK_F18)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f19")==0) {
          if (key!=TK_F19)
            match = false;
        } else
        if (strcasecmp(pattern.c_str(), "f20")==0) {
          if (key!=TK_F20)
            match = false;
        } else {
          if (strcasecmp(pattern.c_str(), str.c_str())!=0) {
            match = false;
          }
        }
        if (state==2)
          state=0;
        pattern.clear();
      }
    }
    if (!s.empty() && match && m==modifier) {
//      cout << "found match " << p->getLabel(0) << endl;
      p->trigger(0);
      return true;
    }
    
    if (p->down) {
      if (menubar_keyboard_filter_recursion(p->down, str, key, modifier, indent+1))
        return true;
    }
    p = p->next;
  }
  return false;
}
