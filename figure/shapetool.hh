/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_FIGURE_CREATETOOL_HH
#define _TOAD_FIGURE_CREATETOOL_HH 1

#include <toad/figuretool.hh>

namespace toad {

/**
 * Tool to create figures.
 *
 * Mose of the code to actually create figures is located in the figures
 * themselves. This class just creates an interface between the figure
 * editor and the figure.
 */
class TShapeTool:
  public TFigureTool
{
    TFigure *tmpl;	// the figure which will be cloned to create new figures
    TFigure *figure;	// the figure currently being created

  public:
    /**
     * \param tmpl template for the figure to be created. This object will be
     *        deleted along with the TShapeTool.
     */
    TShapeTool(TFigure *tmpl) {
      this->tmpl = tmpl;
      figure = nullptr;
    }
    ~TShapeTool() {
      delete tmpl;
    }
  protected:    
    void stop(TFigureEditor*) override;
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me) override;
    void setAttributes(TFigureAttributeModel *p) override;
    bool paintSelection(TFigureEditor *fe, TPenBase &pen) override;
};

} // namespace toad

#endif
