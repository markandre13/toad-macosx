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

/**
 * @defgroup callback Signal & Slot
 *
 * TOAD's implementation of callbacks to member functions.
 */

#include <cstddef>
#include <toad/connect.hh>

// debug only:
#include <iostream>
#include <assert.h>

using namespace std;

namespace toad {

TSignalLink::TSignalLink()
{
  next = 0;
  lock = false;
  dirty = false;
}

TSignalLink::~TSignalLink() {}
void* TSignalLink::objref() {return NULL; }
TSignalLink::TMethod TSignalLink::metref() {return NULL; }

TSignal::TSignal()
{
  _list = NULL;
#ifdef TOAD_SECURE
  delayedtrigger=0;
#endif
}

TSignal::~TSignal()
{
#ifdef TOAD_SECURE
  assert(delayedtrigger==0);
#endif
  remove();
}

/**
 * Lock the signal.
 *
 * The callbacks connected with this signal aren't called. Instead a dirty
 * flag will be set.
 *
 * The behaviour when callbacks are added or removed while a lock is active
 * is undefined.
 *
 * \sa unlock
 */
void
TSignal::lock()
{
  if (_list)
    _list->lock = true;
}

/**
 * Unlock the signal and trigger it in case it was triggered while the
 * lock was active.
 *
 * The behaviour when callbacks are added or removed while a lock is active
 * is undefined.
 *
 * \sa lock
 */
void
TSignal::unlock()
{
  if (_list) {
    bool flag = _list->lock && _list->dirty;
    _list->lock = false;
    _list->dirty = false;
    if (flag)
      trigger();
  }
}

/**
 * This method prints information for debugging purposes.
 */
void
TSignal::print()
{
  unsigned count = 0;
  TSignalLink *p = _list;
  while(p) {
    ++count;
    p = p->next;
  }
  cerr << "signal owns " << count << " links" << endl;
}

TSignalLink*
TSignal::add(TSignalLink *node)
{
  if (!node)
    return NULL;

  node->next = NULL;
  if (_list==NULL) {
    _list = node;
  } else {
    TSignalLink *p = _list;
    while(p->next)
      p=p->next;
    p->next = node;
  }
  return node;
}

void TSignal::remove()
{
//  cout << "remove all" << endl;
  TSignalLink *p;
  while(_list) {
    p = _list;
    _list = _list->next;
    delete p;
  }
}

void TSignal::remove(void(*f)(void))
{
//  cout << "remove function" << endl;
  #warning "no code to remove function!"
}

void TSignal::remove(void *object, TSignalLink::TMethod method)
{
//  cout << "remove object/method" << endl;
  if (!object)
    return;
  TSignalLink *p = _list, *last = NULL;
  while(p) {
    if (p->objref()==object &&
        p->metref()==method) {
//      cout << "found method" << endl;
      if (p==_list) {
        _list = p->next;
        delete p;
        p = _list;
        continue;
      } else {
        last->next = p->next;
        delete p;
        p = last->next;
        continue;
      }
    }
    last = p;
    p = p->next;
  }
}

// Remove all connections for an object
//-------------------------------------
void TSignal::remove(void *object)
{
//  cout << "remove all for one object" << endl;
  if (!object)
    return;
  TSignalLink *p = _list, *last = NULL;
  while(p) {
    if (p->objref()==object) {
//      cout << "found" << endl;
      if (p==_list) {
        _list = p->next;
        delete p;
        p = _list;
        continue;
      } else {
        last->next = p->next;
        delete p;
        p = last->next;
        continue;
      }
    }
    last = p;
    p = p->next;
  }
}

void TSignal::remove(TSignalLink *node)
{
  if (!node)
    return;
  TSignalLink *p = _list, *last = NULL;
  while(p) {
    if (p==node) {
//      cout << "found node" << endl;
      if (p==_list) {
        _list = p->next;
        delete p;
        p = _list;
        continue;
      } else {
        last->next = p->next;
        delete p;
        p = last->next;
        continue;
      }
    }
    last = p;
    p = p->next;
  }
}

/**
 * Invoke all actions connected to the signal.
 *
 * Actions are executed before returning when the signal isn't locked.
 *
 * When the signal is locked, it is triggered during 'unlock'.
 * 
 * \return 'false' when the signal is connected to anything
 * \sa delayedTrigger, unlock
 */
bool 
TSignal::trigger()
{
  if (!_list) return false;
  
  if (_list->lock) {
    _list->dirty = true;
    return true;
  }
  
  TSignalLink *p = _list;
  while(p) {
    p->execute();
    p = p->next;
  }
  return true;
}

std::map<TSlot*, std::map<TSignal*, std::set<TSignalLink*> > > TSlot::slotContainer;

TSignalLink*
toad::connect(TSignal &signal, TSlot *slot, std::function<void()> closure)
{
//cout << "connect from signal " << &signal << " slot " << slot << endl;
  TSignalLink *link = signal.add(closure);
  TSlot::slotContainer[slot][&signal].insert(link);
  return link;
}   

void
toad::disconnect(TSignal &signal, TSlot *slot)
{
//cout << "disconnect from signal " << &signal << " slot " << slot << endl;
  auto &&storedSlot = TSlot::slotContainer.find(slot);
  if (storedSlot==TSlot::slotContainer.end())
    return;
  auto &&storedSignal = storedSlot->second.find(&signal);
  if (storedSignal == storedSlot->second.end())
    return;
  for(auto &&link: storedSignal->second)
    storedSignal->first->remove(link);
  storedSlot->second.erase(storedSignal);
  if (storedSlot->second.empty())
    TSlot::slotContainer.erase(storedSlot);
}

TSlot::~TSlot() {
  auto &&slot = slotContainer.find(this);
  if (slot==slotContainer.end())
    return;
  for(auto &&signal: slot->second) {
    for(auto &&link: signal.second) {
      signal.first->remove(link);
    }
  }
  slotContainer.erase(slot);
}



} // namespace toad
