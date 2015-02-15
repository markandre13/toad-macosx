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

#include <toad/command.hh>
#include <toad/window.hh>
#include <iostream>
#include <vector>

using namespace std; 
using namespace toad;

TCommand::TCommand() {}
TCommand::~TCommand() {}

static vector<TCommand*> cmds;

void
toad::sendMessage(TCommand *cmd)
{
  cmds.push_back(cmd);
}

void
toad::executeMessages()
{
  // executing commands may create other commands, hence we loop until we
  // have no more commands
  while(!cmds.empty()) {
    vector<TCommand*> oldcmds(cmds);
    cmds.clear();
    for(auto p = oldcmds.begin();
        p != oldcmds.end();
        ++p)
    {
      (*p)->execute();
      delete *p;
    }
  }
}

void
toad::sendMessageDeleteWindow(TWindow *w)
{
  class TCommandDeleteWindow:
    public TCommand
  {
      TWindow *_window;
    public:
      TCommandDeleteWindow(TWindow *w) { _window = w; }
      void execute() { delete _window; }
  };
  
  sendMessage(new TCommandDeleteWindow(w));
}
