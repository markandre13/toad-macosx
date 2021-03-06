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

#include <toad/action.hh>

using namespace toad;

TActionStorage TAction::actions;

TAbstractChoice::~TAbstractChoice()
{
}

/**
 * \class toad::TAction
 *
 * TAction is a kind of invisible button widget and was introduced to
 * decouple the widgets from the menubar.
 *
 * For example, the TTextArea widget creates TAction children for cut, copy
 * and paste and connects their signals to its methods.
 *
 * Other widgets with a visual representation like TMenuBar or TToolBar will
 * use a list of all available actions (provided in TAction::actions) and
 * trigger the actions signal.
 *
 * \todo
 *   \li
 *     Currently the actions look is controlled by the menubar which is
 *     wrong.
 */

TAction::TAction(TInteractor *parent, const string &id, EType type, EActivation activation)
  :TInteractor(parent, id)
{
  has_focus = false;
  has_domain_focus = true;
  enabled = true;
  bitmap = nullptr;

  // FIXME: I need to throw some more brain on this statement
  if (activation==PARENT_FOCUS && parent && parent->getParent()==NULL) activation = DOMAIN_FOCUS;

  this->activation = activation;
  this->type = type;

  actions.push_back(this);
}

TAction::~TAction()
{
  actions.erase(this);
}

void
TAction::focus(bool b)
{
//cout << "action '" << getTitle() << "' " << (b?"got":"lost") << " the focus" << endl;
  bool oldstate = isEnabled();
  has_focus = b;
  if (isEnabled()!=oldstate) {
    sigChanged();
  }
}

void 
TAction::domainFocus(bool b)
{
//cout << "action '" << getTitle() << "' " << (b?"got":"lost") << " the focus domain" << endl;
  bool oldstate = isEnabled();
  has_domain_focus = b;
  if (isEnabled()!=oldstate) {
    sigChanged();
  }
}

void 
TAction::setEnabled(bool b)
{
  bool oldstate = isEnabled();
  enabled = b;
  if (isEnabled()!=oldstate) {
    sigChanged();
  }
}

bool 
TAction::isEnabled() const
{
  if (!enabled)
    return false;
// FIXME?: GChoice<T> may also signal via it's model
//  if (!sigClicked.isConnected())
//    return false;
  switch(activation) {
    case ALWAYS:
      return true;
    case DOMAIN_FOCUS:
      return has_domain_focus;
    case PARENT_FOCUS:
      return has_focus;
  }
  return false;
}

bool
TAction::trigger(unsigned idx)
{
  if (!isEnabled())
    return false;
  return sigClicked.trigger();
}

bool
TAction::delayedTrigger(unsigned idx)
{
  if (!isEnabled())
    return false;
  return sigClicked.delayedTrigger();
}

unsigned
TAction::getSize() const
{
  return 1;
}

/**
 * Returns the ID associated with this action.
 *
 * \param idx A value between 0 and Size().
 */
const string& 
TAction::getID(unsigned idx) const
{
  return title;
}
