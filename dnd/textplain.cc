/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2005 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/dnd/textplain.hh>

using namespace toad;

// text/plain
//---------------------------------------------------------------------------
TDnDTextPlain::TDnDTextPlain(const string &t)
{
  setType("text/plain", ACTION_COPY);
  text = t;
}

bool 
TDnDTextPlain::select(TDnDObject &drop)
{
  return TDnDObject::select(drop, "text", "plain");
}

void TDnDTextPlain::flatten()
{
  flatdata = text;
}

PDnDTextPlain 
TDnDTextPlain::convertData(TDnDObject &drop)
{
  TDnDTextPlain *result;
  result = dynamic_cast<TDnDTextPlain*>(&drop);
  if (result || !drop.type)
    return result;
    
  if (drop.type->mime!="text/plain")
    return result;

  result = new TDnDTextPlain(drop.flatdata);
  result->pos = drop.pos;
  return result;
}

void 
TDropSiteTextPlain::dropRequest(TDnDObject &drop)
{
  if (TDnDTextPlain::select(drop)) {
    drop.action = ACTION_COPY;
    return;
  }
  drop.action = ACTION_NONE;
}

void 
TDropSiteTextPlain::drop(TDnDObject &drop)
{
  value = TDnDTextPlain::convertData(drop);
  if (value) {
    sigDrop();
    value = NULL;
  }
}
