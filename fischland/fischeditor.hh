/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2014 by Mark-André Hopf <mhopf@mark13.org>
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

// This file defines the classes: TEditModel, TDocument, TSlide and TLayer.

#ifndef _FISCHLAND_FISCHEDITOR_HH
#define _FISCHLAND_FISCHEDITOR_HH 1

#include <toad/figureeditor.hh>
#include "page.hh"

namespace fischland {

using namespace toad;

class TFischEditor:
  public TFigureEditor
{
    PEditModel editmodel;
    bool modified2;
    
    // speed up painting:
    TFigure *first_figure, *last_figure;
#if __X11__
    cairo_surface_t *before_selection, *after_selection;
#endif
    
    TBoolModel useCairo;
    TBoolModel useDoubleBuffer;
    
  public:
    typedef TFigureEditor super;
  
    TFischEditor(TWindow *parent, const string &title):
      super(parent, title) { 
      modified2 = false;
      first_figure = last_figure = 0;
#ifdef __X11__
      before_selection = after_selection = 0;
#endif
      flagDoubleBuffer = false;
      
      useCairo = false;
      useDoubleBuffer = true;
    }

    TSignal sigFischModified;
    void clearFischModified() {
      if (modified2) {
        modified2 = false;
        sigFischModified();
      }
    }
    bool isFischModified() const {
      return modified2;
    }

    void paint();
    void setEditModel(TEditModel *e) {
      if (editmodel) {
        disconnect(editmodel->sigChanged, this);
        if (editmodel->figuremodel)
          disconnect(editmodel->figuremodel->sigChanged, this);
      }
      editmodel = e;
      setModified(false);

      if (editmodel) {
        CONNECT(editmodel->sigChanged, this, editmodelChanged);
        setModel(editmodel->figuremodel);
        if (editmodel->figuremodel)
          CONNECT(editmodel->figuremodel->sigChanged, this, figuremodelChanged);
      }
      modified2 = false;
    }
    
    void editmodelChanged() {
//cout << __PRETTY_FUNCTION__ << " modified=" << modified << endl;
      if (editmodel) {
//        cout << "set figure model " << editmodel->figuremodel << endl;
        if (getModel())
          disconnect(getModel()->sigChanged, this);
        setModel(editmodel->figuremodel);
        if (getModel())
          CONNECT(getModel()->sigChanged, this, figuremodelChanged);
      } else {
        setModel(0);
      }
      if (!modified2) {
        modified2 = true;
        sigFischModified();
      }
    }
    
    void figuremodelChanged() {
//cout << __PRETTY_FUNCTION__ << " modified=" << modified << endl;
      if (!modified2) {
        modified2 = true;
        sigFischModified();
      }
    }
};

} // namespace fischland

#endif
