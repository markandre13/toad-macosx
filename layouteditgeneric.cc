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

#include <toad/window.hh>
#include <toad/textarea.hh>
#include <toad/pushbutton.hh>
#include <toad/io/urlstream.hh>

#include <sstream>

namespace toad {

/**
 * \class TLayoutEditGeneric
 *
 * A simple layout editor to edit the layout as text.
 *
 * \todo
 *   \li
 *     TEditLayout: show error messages for 'restore'
 *   \li
 *     Show when the layout was modified
 *   \li
 *     Load/Save with different filenames
 */
class TLayoutEditGeneric:
  public TLayoutEditor
{
    TWindow *window;    // window to be edited
    TLayout *layout;    // layout to be edited
    PTextModel text;

  public:
    TLayoutEditGeneric(TWindow*, const string&, TLayout*, TWindow *forWindow);
  protected:
    void fetch();
    void apply();
};

} // namespace toad


TLayoutEditor::TLayoutEditor(TWindow *parent, const string &title):
  super(parent, title)
{
}

/**
 * When the layout editor has to handle events in the window being edited,
 * it should return a filter here to get these event or NULL otherwise.
 * (The default implementation returns NULL.)
 */
TEventFilter *
TLayoutEditor::getFilter()
{
  return 0;
}




TLayoutEditGeneric::TLayoutEditGeneric(TWindow *parent,
                                       const string &title,
                                       TLayout *layout,
                                       TWindow *forWindow)
  :TLayoutEditor(parent, title)
{
  int x, y;
  
  this->layout = layout;
  this->window = forWindow;
  
  text = new TTextModel();

  setBackground(TColor::DIALOG);
  
  TTextArea *ta;
  TPushButton *pb;
  
  x=5; y=5;
  
  ta = new TTextArea(this, "layout", text);
  PFont font = new TFont(ta->getPreferences()->getFont());
  ta->setShape(x, y, font->getTextWidth("x")*60, 
                     font->getHeight()*25);
  ta->getPreferences()->tabwidth=2;
  
  y+=ta->getHeight()+5;
  
  pb = new TPushButton(this, "Fetch");
  pb->setShape(x,y,80,25);
  connect(pb->sigClicked, this, &TLayoutEditGeneric::fetch);
  
  pb = new TPushButton(this, "Apply");
  pb->setShape(x+80+5,y,80,25);
  connect(pb->sigClicked, this, &TLayoutEditGeneric::apply);

  y+=25+5;
  
  setSize(5+ta->getWidth()+5, y);
  fetch();
}

void
TLayoutEditGeneric::fetch()
{
  ostringstream os;
  TOutObjectStream oo(&os);
  oo.store(layout);
  text->setValue(os.str());
}

void
TLayoutEditGeneric::apply()
{
  istringstream is(text->getValue());
  TInObjectStream io(&is);
  TSerializable *s = io.restore();
  if (s) {
    TLayout *layout2 = dynamic_cast<TLayout*>(s);
    if (layout2) {
      layout = layout2;
      layout->setModified(true);
      window->setLayout(layout);
    } else {
      delete s;
    }
  }
}
