/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2014 by Mark-André Hopf <mhopf@mark13.org>
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

#ifndef _TOAD_FIGURE_CREATETOOL
#define _TOAD_FIGURE_CREATETOOL 1

#include <toad/figuretool.hh>

namespace toad {

/**
 * Tool to create figures.
 *
 * Mose of the code to actually create figures is located in the figures
 * themselves. This class just creates an interface between the figure
 * editor and the figure.
 */
class TFCreateTool:
  public TFigureTool
{
    TFigure *tmpl;
    TFigure *figure;
  public:
    /**
     * \param tmpl template for the figure to be created. This object will be
     *        deleted along with the TFCreateTool.
     */
    TFCreateTool(TFigure *tmpl) {
      this->tmpl = tmpl;
      figure = 0;
    }
    ~TFCreateTool() {
      delete tmpl;
    }
  protected:    
    void stop(TFigureEditor*);
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
    void keyEvent(TFigureEditor *fe, const TKeyEvent &ke);
    void setAttributes(TFigureAttributeModel *p);
    bool paintSelection(TFigureEditor *fe, TPenBase &pen);
};

} // namespace toad

#endif
