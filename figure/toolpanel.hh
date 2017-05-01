/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2017 by Mark-André Hopf <mhopf@mark13.org>
 * Visit http://www.mark13.org/fischland/.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TOAD_FIGURE_TOOLPANEL_HH
#define _TOAD_FIGURE_TOOLPANEL_HH 1

#include <toad/figureeditor.hh>
#include <toad/boolmodel.hh>
#include <toad/undo.hh>
#include <toad/undomanager.hh>
#include <map>

namespace toad {

using namespace std;

/**
 * A generic tool panel which can provide access to all TAction objects.
 *
 * The apperance and available options are intended controlled via a TLayout subclass
 * in the future, which will allow users to create their own custom tool panels.
 */
class TToolPanel:
  public TWindow
{
    public:
      TToolPanel(TWindow *parent, const string &title);
    protected:
      void actionsChanged();
      void addAction(const string &title, TAction*);
      void addChoice(const string &title, TChoiceModel*, size_t index);
};

} // namespace toad

#endif
