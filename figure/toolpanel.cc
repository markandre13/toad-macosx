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

#include <toad/figure/toolpanel.hh>
#include <toad/figure/toolbutton.hh>

using namespace toad;

TToolPanel::TToolPanel(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  flagParentlessAssistant = true;

  connect(TAction::actions.sigChanged, this, [=] {
    this->actionsChanged();
  });
  setShape(640, 900, 320, 32);
  
  actionsChanged();
}

void
TToolPanel::actionsChanged()
{
//cout << endl;
//cout << "TToolPanel::actionsChanged()" << endl;
  std::set<std::string> found;

  for(TActionStorage::iterator i = TAction::actions.begin();
      i != TAction::actions.end();
      ++i)
  {
    TAction *action = *i;
    switch(action->type) {
      case TAction::BUTTON:
//cout << "  button '" << action->getTitle() << "'" << endl;
        found.insert(action->getTitle());
        addAction(action->getTitle(), action);
        break;
      case TAction::CHECKBUTTON:
//cout << "  checkbox" << endl;
        break;
      case TAction::RADIOBUTTON:
//cout << "  radiobutton '" << action->getTitle() << "'" << endl;
        TAbstractChoice *choice = dynamic_cast<TAbstractChoice*>(action);
        if (!choice) {
//          cout << "    choice is not an abstract choice" << endl;
          break;
        }
        for(size_t i=0; i<choice->getSize(); ++i) {
          string title = choice->getTitle() + '|' + choice->getID(i);
//cout << "    '" << title << "'" << endl;
          found.insert(title);
          addChoice(title, choice->getModel(), i);
        }
        break;
    }
  }
}

void
TToolPanel::addAction(const string &title, TAction*)
{
}

void
TToolPanel::addChoice(const string &title, TChoiceModel *choice, size_t index)
{
//cout << "toolbar: addChoice(" << title << ", ...)" << endl;

  TCoord x = 0;
  for(TInteractor *child = getFirstChild(); child; child=child->getNextSibling()) {
    if (child->getTitle() == title)
      return;
    x += 32;
  }

  auto&& button = new TToolButton(this, title, choice, index);
  if (title=="tool|toolbox|selection")
    button->loadBitmap("resource/tool_select.png");
  else
  if (title=="tool|toolbox|directselection")
    button->loadBitmap("resource/tool_directselect.png");
  if (title=="tool|toolbox|pen")
    button->loadBitmap("resource/tool_pen.png");
  else
  if (title=="tool|toolbox|pencil")
    button->loadBitmap("resource/tool_pencil.png");
  else
  if (title=="tool|toolbox|text")
    button->loadBitmap("resource/tool_text.png");
  else
  if (title=="tool|toolbox|circle")
    button->loadBitmap("resource/tool_circ.png");
  else
  if (title=="tool|toolbox|rectangle")
    button->loadBitmap("resource/tool_rect.png");

  button->setShape(x,0,32,32);
  connect(button->sigClicked, [=] {
    choice->select(index);
  });
  
}

