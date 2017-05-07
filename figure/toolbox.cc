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

#include <toad/figure/toolbox.hh>
#include <toad/eventfilter.hh>
#include <toad/focusmanager.hh>

using namespace toad;

TToolBox*
TToolBox::getToolBox() {
  static TToolBox *tool = nullptr;
  if (!tool) tool = new TToolBox();
  return tool;
}

TToolBox::TToolBox()
{
  activePointer = 0;
/*
*/  
  // FIXME: this would be nicer with closures
  class TMyEventFilter: public TEventFilter {
      TToolBox *toolbox;
    public:
      TMyEventFilter(TToolBox *aToolBox):toolbox(aToolBox) {}
    protected:
      bool mouseEvent(TMouseEvent &me) override {
        if (me.type==TMouseEvent::TABLET_PROXIMITY) {
          toolbox->selectPointer(me.pointerID());
        }
        return false;
      }
  };
  TFocusManager::insertEventFilter(new TMyEventFilter(this), NULL, KF_GLOBAL);
}

void
TToolBox::selectPointer(TMouseEvent::TPointerID pointerID)
{
  if (activePointer == pointerID)
    return;

  // new pointing device, save old one
  toolForPointer[activePointer] = getValue();

  // switch to the previously known tool for the new pointing device
  auto &&knownTool = toolForPointer.find(pointerID);
  if (knownTool != toolForPointer.end()) {
    setValue(knownTool->second);
  }
  
  activePointer = pointerID;
}
