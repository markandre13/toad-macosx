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

#include <toad/dragndrop.hh>
#include <toad/window.hh>

using namespace toad;

void
toad::startDrag(TDnDObject *obj, unsigned modifier)
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
}

TDnDType::TDnDType(const string &m, unsigned actions_in)
{
  wanted = false;
  actions = actions_in;
  mime=m;
  size_t n1, n2;
  n1 = mime.find("/");
  if (n1==string::npos) {
    major="x-not-mime";
    n1 = 0;
  } else { 
    major = mime.substr(0,n1);
    n1++;
  }
  n2 = mime.find_first_of(" ;", n1);
  minor = mime.substr(n1,n2);
#if VERBOSE
  cout << "  new dnd type: " << major << "/" << minor << "'" << endl;
#endif
}

TDropSite::TDropSite(TWindow *p, TRectangle const &r)
{ 
  parent = p;
  rect = r;
  use_parent = false;
  init();
}

void TDropSite::init()
{
  cerr << __PRETTY_FUNCTION__ << " isn't implemented yet" << endl;
#if 0
  TWindowDropSiteMap::iterator p = dropsitemap.find(parent);
  if (p==dropsitemap.end()) {
    TWindowDropSite *wds = new TWindowDropSite;
    dropsitemap[parent] = wds;
    wds->parent = parent;
    wds->Add(this);
  } else {
    (*p).second->Add(this);
  }
#endif
}  

TDropSite::~TDropSite()
{
}

const TRectangle&
TDropSite::getShape()
{
  if (use_parent)
    rect.set(0,0,parent->getWidth(), parent->getHeight());
  return rect;
}

void
TDropSite::setShape(int x, int y, int w, int h)
{
  use_parent = false;
  rect.set(x, y, w, h);
}

void
TDropSite::setShape(const TRectangle &r)
{
  use_parent = false;
  rect = r;
}

void TDropSite::leave()
{  
  cout << __PRETTY_FUNCTION__ << endl;
}
