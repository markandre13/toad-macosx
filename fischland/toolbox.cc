/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2006 by Mark-André Hopf <mhopf@mark13.org>
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

#include "toolbox.hh"
#include "colorpalette.hh"
#include "fischland.hh"
#include "cairo.hh"

#include <toad/combobox.hh>
#include <toad/pushbutton.hh>
#include <toad/fatradiobutton.hh>
#include <toad/fatcheckbutton.hh>
#include <toad/textfield.hh>
#include <toad/colorselector.hh>
#include <toad/colordialog.hh>
#include <toad/gauge.hh>
#include <toad/figure.hh>
#include <toad/messagebox.hh>

#include "fontdialog.hh"
#include "fpath.hh"

using namespace fischland;

static const TRGB white(255,255,255);
static const TRGB black(0,0,0);

class TArrowTypeAdapter:
  public TSimpleTableAdapter
{

  public:
    size_t getRows() { return 7; }
    void tableEvent(TTableEvent &te) {
      switch(te.type) {
        case TTableEvent::GET_COL_SIZE:
          te.w = 35;
          break;
        case TTableEvent::GET_ROW_SIZE:
          te.h = 18;
          break;
        case TTableEvent::PAINT: {
          renderBackground(te);
          TPoint p1(30,9);
          TPoint p2(5,9);
          te.pen->drawLine(p1, p2);
          TFLine::drawArrow(*te.pen,
                            p1, p2,
                            black, white,
                            8, 16,
                            static_cast<TFLine::EArrowType>(te.row));
          renderCursor(te);
        } break;
      }
    }
};

class TArrowModeAdapter:
  public TSimpleTableAdapter
{

  public:
    size_t getRows() { return 4; }
    void tableEvent(TTableEvent &te) {
      switch(te.type) {
        case TTableEvent::GET_COL_SIZE:
          te.w = 35;
          break;
        case TTableEvent::GET_ROW_SIZE:
          te.h = 18;
          break;
        case TTableEvent::PAINT: {
          renderBackground(te);
          TPoint p1(30,9);
          TPoint p2(5,9);
          te.pen->drawLine(p1, p2);
          if (te.row==1 || te.row==3)
            TFLine::drawArrow(*te.pen, p1, p2, black, white, 5, 10, TFLine::FILLED);
          if (te.row==2 || te.row==3)
            TFLine::drawArrow(*te.pen, p2, p1, black, white, 5, 10, TFLine::FILLED);
          renderCursor(te);
        } break;
      }
    }
};

class TLineStyleAdapter:
  public TSimpleTableAdapter
{
  public:
    size_t getRows() { return 5; }
    void tableEvent(TTableEvent &te) {
      switch(te.type) {
        case TTableEvent::GET_COL_SIZE:
          te.w = 35;
          break;
        case TTableEvent::GET_ROW_SIZE:
          te.h = 18;
          break;
        case TTableEvent::PAINT: {
          renderBackground(te);
          TPoint p1(30,9);
          TPoint p2(5,9);
          te.pen->setLineWidth(2);
          te.pen->setLineStyle(static_cast<TPenBase::ELineStyle>(te.row+1));
          te.pen->drawLine(p1, p2);
          renderCursor(te);
        } break;
      }
    }

};

class TLineWidthAdapter:
  public TSimpleTableAdapter
{
  public:
    size_t getRows() { return 12; }
    void tableEvent(TTableEvent &te) {
      switch(te.type) {
        case TTableEvent::GET_COL_SIZE:
          te.w = 35;
          break;
        case TTableEvent::GET_ROW_SIZE:
          te.h = te.row+6;
          break;
        case TTableEvent::PAINT: {
          renderBackground(te);
          TPoint p1(30,te.h/2);
          TPoint p2(5,te.h/2);
          te.pen->setLineColor(0,0,0);
          te.pen->setLineWidth(te.row);
          te.pen->drawLine(p1, p2);
          te.pen->setLineWidth(0);
          renderCursor(te);
        } break;
      }
    }

};

class TFontButton:
  public TPushButton
{
    TFigureAttributes *fa;
  public:
    TFontButton(TWindow *parent, const string &title, TFigureAttributes *fa):
      TPushButton(parent, title)
    {
      this->fa = fa;
      CONNECT(fa->sigChanged, this, fontChanged);
    }
    
    ~TFontButton() {
      disconnect(fa->sigChanged, this);
    }
    
    void fontChanged() {
      if (fa->reason == TFigureAttributes::ALLCHANGED || 
          fa->reason == TFigureAttributes::FONTNAME)
      {
        invalidateWindow();
      }
    }
    
    void paint() {
      TPen pen(this);
      pen.scale(1.0/96.0, 1.0/96.0);
      pen.setFont(fa->getFont());
      const char *text = "F";
      int n =(bDown && bInside)?1:0;
      int x = (getWidth()*96-pen.getTextWidth(text)) >> 1;
      int y = (getHeight()*96-pen.getHeight()) >> 1;
      pen.setColor(TColor::BTNTEXT);
      pen.drawString(x+n, y+n, text);
      if (isFocus()) {
        pen.setLineStyle(TPen::DOT);
        pen.drawRectanglePC(x+n-3,y+n-1,pen.getTextWidth(text)+6,pen.getHeight()+1);
        pen.setLineStyle(TPen::SOLID);
      }
      pen.identity();
      drawShadow(pen, bDown && bInside);
    }
};

TToolBox* TToolBox::toolbox = 0;
PFigureAttributes TToolBox::preferences;

void
setLineWidth(TSingleSelectionModel *model)
{
  int n = model->getRow();
  if (n==0)
    n=48;
  else
    n*=96;
  if (TToolBox::preferences->linewidth != n) {
    TToolBox::preferences->reason = TFigureAttributes::LINEWIDTH;
    TToolBox::preferences->linewidth = n;
    TToolBox::preferences->sigChanged();
  }
}

void
TToolBox::preferencesChanged()
{
  int n;

  // lw_sm, line width selection model
  n = TToolBox::preferences->linewidth;
  if (n==48)
    n=0;
  else
    n/=96;
  if (n != lw_sm->getRow()) {
 //   cout << "line width changed from " << lw_sm->getRow() << " to " << n << endl;
    lw_sm->select(0, n);
  }

  n = TToolBox::preferences->linestyle - 1;
  if (n != ls_sm->getRow()) {
 //   cout << "line style changed from " << ls_sm->getRow() << " to " << n << endl;
    ls_sm->select(0, n);
  } 

  n = TToolBox::preferences->arrowmode;
  if (n != am_sm->getRow()) {
//    cout << "arrow mode changed from " << am_sm->getRow() << " to " << n << endl;
    am_sm->select(0, n);
  } 

  n = TToolBox::preferences->arrowtype;
  if (n != at_sm->getRow()) {
//    cout << "arrow style changed from " << at_sm->getRow() << " to " << n << endl;
    at_sm->select(0, n);
  } 
}

void
setLineStyle(TSingleSelectionModel *model)
{
  TPenBase::ELineStyle n = static_cast<TPenBase::ELineStyle>(model->getRow()+1);
  if (TToolBox::preferences->linestyle != n) {
    TToolBox::preferences->reason = TFigureAttributes::LINESTYLE;
    TToolBox::preferences->linestyle = n;
    TToolBox::preferences->sigChanged();
  }
}

void
setArrowMode(TSingleSelectionModel *model)
{
  TFLine::EArrowMode n = static_cast<TFLine::EArrowMode>(model->getRow());
  if (TToolBox::preferences->arrowmode != n) {
    TToolBox::preferences->reason = TFigureAttributes::ARROWMODE;
    TToolBox::preferences->arrowmode = n;
    TToolBox::preferences->sigChanged();
  }
}

void
setArrowType(TSingleSelectionModel *model)
{
  TFLine::EArrowType n = static_cast<TFLine::EArrowType>(model->getRow());
  if (TToolBox::preferences->arrowtype != n) {
    TToolBox::preferences->reason = TFigureAttributes::ARROWSTYLE;
    TToolBox::preferences->arrowtype = n;;
    TToolBox::preferences->sigChanged();
  }
}

static void
selectFont()
{
  TFontDialog dlg(0, "Select Font");
  dlg.setFont(TToolBox::preferences->getFont());
  dlg.setFontSize(dlg.getFontSize()/96);
  dlg.doModalLoop();
  if (dlg.getResult() == TMessageBox::OK) {
    dlg.setFontSize(dlg.getFontSize()*96);
    TToolBox::preferences->setFont(dlg.getFont());
  }
}

static void
openPalette(TWindow *parent)
{
  TColorPalette *cp = new TColorPalette(parent, "Palette", TToolBox::preferences);
  cp->createWindow();
}



class TSelectionTool:
  public TFigureTool
{
    // by default strokes and effects aren't scaled
    TBoolModel scale_strokes_and_fx; // Preferences > General

    bool down;                // is the mouse button down?
    bool rect;                // rectangle selection?
    bool grab;                // grabbed selection for moving
    bool hndl;                // grabbed handle
    unsigned handle;
    TCoord rx0, ry0, rx1, ry1;   // rectangle for rectangle selection
    TCoord x0, y0, x1, y1;       // bounding rectangle
    TCoord ox0, oy0, ox1, oy1;   // bounding rectangle before resizing it
    TCoord last_x, last_y;       // last mouse position in figure coordinates when moving selection
    TCoord last_sx, last_sy;     // last mouse position in screen coordinates when moving selection
    TFigureSet selection;
    vector<TMatrix2D> oldmat;
  public:
    TSelectionTool() {
      scale_strokes_and_fx = false;
      down = false;
      rect = false;
      grab = false;
      hndl = false;
    }
    static TSelectionTool* getTool();
  
    void mouseEvent(TFigureEditor *fe, TMouseEvent &me);
    void paintSelection(TFigureEditor*, TPenBase &pen);
    void stop(TFigureEditor *fe);
    
    void invalidateBounding(TFigureEditor *fe);
    void getBoundingHandle(unsigned i, TRectangle *r);
    void calcSelectionsBoundingRectangle(TFigureEditor *fe);
};

TSelectionTool*
TSelectionTool::getTool()
{
  static TSelectionTool* tool = 0;
  if (!tool)
    tool = new TSelectionTool();
  return tool;
}

void
TSelectionTool::stop(TFigureEditor *fe)
{
  fe->getWindow()->setAllMouseMoveEvents(true);
  fe->getWindow()->setCursor(0);
  fe->state = TFigureEditor::STATE_NONE;
}

void
TSelectionTool::mouseEvent(TFigureEditor *fe, TMouseEvent &me)
{
  TFigure *figure;
  TCoord x, y;
  TRectangle r;
  
  switch(me.type) {
    case TMouseEvent::LDOWN:
      if (fe->state == TFigureEditor::STATE_NONE) {
        fe->state = TFigureEditor::STATE_CREATE;
        fe->getWindow()->setAllMouseMoveEvents(true);
      }
      if (!fe->selection.empty()) {
        // origin is already applied by scroll pane?
        TCoord x = me.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
        TCoord y = me.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;
//        cout << "down at " << x << ", " << y << endl;
        for(unsigned i=0; i<8; ++i) {
          getBoundingHandle(i, &r);
//          cout << "  check " << r.x << ", " << r.y << endl;
          if (r.isInside(x, y)) {
            handle = i;
            ox0 = x0;
            oy0 = y0;
            ox1 = x1;
            oy1 = y1;
            hndl = true;
            oldmat.clear();
            for(TFigureSet::const_iterator p = fe->selection.begin();
                p != fe->selection.end();
                ++p)
            {
              if ( (*p)->mat || (*p)->cmat ) {
                if (scale_strokes_and_fx)
                  oldmat.push_back(*(*p)->mat);
                else
                  oldmat.push_back(*(*p)->cmat);
              } else {
                oldmat.push_back(TMatrix2D());
              }
            }
//            cout << "its inside handle " << i << endl;
            return;
          }
        }
      }
      fe->mouse2sheet(me.x, me.y, &x, &y);
      figure = fe->findFigureAt(x, y);
      if (fe->selection.find(figure) != fe->selection.end()) {
        fe->sheet2grid(x, y, &last_x, &last_y);
        last_sx = me.x;
        last_sy = me.y;
        grab = true;
        break;
      }
      
      if (!me.modifier() & MK_SHIFT) {
        fe->clearSelection();
      }
      if (!selection.empty()) {
        fe->invalidateWindow();
      }
      selection.clear();
      if (figure) {
        selection.insert(figure);
        fe->invalidateWindow();
      }
      down = true;
      rx0 = rx1 = me.x;
      ry0 = ry1 = me.y;
      break;
    case TMouseEvent::MOVE:
      if (!hndl) {
        if (!fe->selection.empty()) {
          // origin is already applied by scroll pane?
          int x = me.x /*+ fe->getWindow()->getOriginX()*/ - fe->getVisible().x;
          int y = me.y /*+ fe->getWindow()->getOriginY()*/ - fe->getVisible().y;
          TCursor::EType cursor = TCursor::DEFAULT;
          for(unsigned i=0; i<8; ++i) {
            getBoundingHandle(i, &r);
            if (r.isInside(x, y)) {
              static const TCursor::EType wind[8] = {
               TCursor::NW_RESIZE,
               TCursor::N_RESIZE,
               TCursor::NE_RESIZE,
               TCursor::E_RESIZE,
               TCursor::SE_RESIZE,
               TCursor::S_RESIZE,
               TCursor::SW_RESIZE,
               TCursor::W_RESIZE
              };
              cursor = wind[i];
              break;
            }
          }
          fe->getWindow()->setCursor(cursor);
        }
      } else {
        // mouse is holding a handle, scale the selection
        invalidateBounding(fe);
        TCoord x, y;
        x = me.x - fe->getVisible().x;
        y = me.y - fe->getVisible().y;
        switch(handle) {
          case 0:
            x0 = x;
            y0 = y;
            break;
          case 1:
            y0 = y;
            break;
          case 2:
            x1 = x;
            y0 = y;
            break;
          case 3:
            x1 = x;
            break;
          case 4:
            x1 = x;
            y1 = y;
            break;
          case 5:
            y1 = y;
            break;
          case 6:
            x0 = x;
            y1 = y;
            break;
          case 7:
            x0 = x;
            break;
        }

        TCoord sx = (x1-x0)/(ox1 - ox0);
        TCoord sy = (y1-y0)/(oy1 - oy0);
        TCoord X0, OX0, Y0, OY0;
        if (fe->getMatrix()) {
          TMatrix2D m(*fe->getMatrix());
          m.invert();
          m.map(x0, y0, &X0, &Y0);
          m.map(ox0, oy0, &OX0, &OY0);
        } else {
          X0 = x0; Y0 = y0;
          OX0 = ox0; OY0 = oy0;
        }
        TMatrix2D m;
        m.translate(X0, Y0);
        m.scale(sx, sy);
        m.translate(-OX0, -OY0);

        unsigned i=0;
        for(TFigureSet::iterator p = fe->selection.begin();
            p != fe->selection.end();
            ++p, ++i)
        {
          if (scale_strokes_and_fx) {
            if ((*p)->mat) {
              *(*p)->mat = m * oldmat[i];
            } else {
              (*p)->mat = new TMatrix2D(m);
            }
          } else {
            if ((*p)->cmat) {
              *(*p)->cmat = m * oldmat[i];
            } else {
              (*p)->cmat = new TMatrix2D(m);
            }
          }
        }

        invalidateBounding(fe);
        break;
      }
      if (grab) {
        TCoord x, y;
        fe->mouse2sheet(me.x, me.y, &x, &y);
        fe->sheet2grid(x, y, &x, &y);
        TCoord dx = x-last_x;
        TCoord dy = y-last_y;
        last_x=x;
        last_y=y;
        
        invalidateBounding(fe);

        fe->getModel()->translate(fe->selection, dx, dy);
        #if 0
        dx = me.x - last_sx;
        dy = me.y - last_sy;
        last_sx = me.x;
        last_sy = me.y;
        x0 += dx;
        y0 += dy;
        x1 += dx;
        y1 += dy;
        #else
        calcSelectionsBoundingRectangle(fe);
        #endif
        invalidateBounding(fe);
        break;
      }
      if (down) {
        if (!rect) {
          if (me.x < rx0-2 || me.x > rx0+2 || me.y < ry0-2 || me.y > ry0+2) {
            rect = true;
            selection.clear();
          } else {
            break;
          }
        }
        fe->getWindow()->invalidateWindow(
          rx0 + fe->getWindow()->getOriginX(),
          ry0 + fe->getWindow()->getOriginY(),
          rx1-rx0+3,ry1-ry0+2);
        rx1 = me.x;
        ry1 = me.y;
        fe->getWindow()->invalidateWindow(
          rx0 + fe->getWindow()->getOriginX(),
          ry0 + fe->getWindow()->getOriginY(),
          rx1-rx0+3,ry1-ry0+2);
        selection.clear();
        TFigureModel::const_iterator p, e;
        p = fe->getModel()->begin();
        e = fe->getModel()->end();
        TPoint p0, p1;
        fe->mouse2sheet(rx0, ry0, &p0.x, &p0.y);
        fe->mouse2sheet(rx1, ry1, &p1.x, &p1.y);
        TRectangle r0(p1, p0), r1;
        while(p!=e) {
#if 1
          fe->getFigureShape(*p, &r1, NULL);
#else
          (*p)->getShape(&r1);
          
          #warning "why isn't the following already done by getShape?"
          if ( (*p)->mat ) {
            TPoint p0, p1;
            (*p)->mat->map(r1.x       , r1.y       , &p0.x, &p0.y);
            (*p)->mat->map(r1.x+r1.w-1, r1.y+r1.h-1, &p1.x, &p1.y);
            r1.set(p0, p1);
          }
#endif
          if (r0.isInside( r1.x, r1.y ) &&
              r0.isInside( r1.x+r1.w, r1.y+r1.h ) )
          {
            selection.insert(*p);
          }
          ++p;
        }
      }
      break;
    case TMouseEvent::LUP:
      if (hndl) {
        hndl = false;
        // oldmat should be used for undo
        oldmat.clear();
        break;
      }
      if (grab) {
        grab = false;
        break;
      }
      down = rect = false;
      
      // add new selection to figure editors selection
      fe->selection.insert(selection.begin(), selection.end());
      selection.clear();

      // calculate the selections bounding rectangle
      calcSelectionsBoundingRectangle(fe);
      fe->invalidateWindow();
      break;
    default:
      ;
  }
}

void
TSelectionTool::invalidateBounding(TFigureEditor *fe)
{
  fe->getWindow()->invalidateWindow(
    x0-4 + fe->getWindow()->getOriginX() + fe->getVisible().x,
    y0-4 + fe->getWindow()->getOriginY() + fe->getVisible().y,
    x1-x0+10,y1-y0+10);
#if 0
  cout << "invalidate bounding ("
       << x0-4 + fe->getWindow()->getOriginX() + fe->getVisible().x
       << ", "
       << y0-4 + fe->getWindow()->getOriginY() + fe->getVisible().y
       << ", "
       << x1-x0+9
       << ", "
       << y1-y0+9;
#endif
}


void
TSelectionTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (down) {
    // draw the selection marks over all figures
    for(TFigureSet::iterator sp = selection.begin();
        sp != selection.end();
        ++sp)
    {
      if ((*sp)->mat) {
        pen.push();
        pen.multiply( (*sp)->mat );
      }
      pen.setLineWidth(1);   
      (*sp)->paintSelection(pen, -1);
      if ((*sp)->mat)
        pen.pop(); 
    }
    
    // draw selection rectangle
    if (rect) {
      pen.push();
      double tx = 0.0, ty = 0.0;
      if (pen.getMatrix()) {
        tx = pen.getMatrix()->tx - fe->getVisible().x;
        ty = pen.getMatrix()->ty - fe->getVisible().y;
      }
      pen.identity();
      pen.translate(tx, ty);
      pen.setColor(TColor::FIGURE_SELECTION);
      pen.setLineWidth(1);
      pen.setAlpha(0.3);
      if (pen.getAlpha()!=1.0)
        pen.fillRectangle(rx0, ry0, rx1-rx0+1, ry1-ry0);
      else
        pen.drawRectangle(rx0, ry0, rx1-rx0+1, ry1-ry0);
      pen.setAlpha(1.0);
      pen.drawRectangle(rx0, ry0, rx1-rx0+1, ry1-ry0);
      pen.pop();
    }
  } else
  if (!fe->selection.empty()) {
//    cout << "draw bounding rectangle " << x0 << ", " << y0 << " to " << x1 << ", " << y1 << endl;
    const TMatrix2D *m = 0;
    if (pen.getMatrix()) {
      m = pen.getMatrix();
      double tx, ty;
      m->map(0.0, 0.0, &tx, &ty);
      pen.push();
      pen.identity();
      pen.translate(tx, ty);
    }
    pen.setColor(TColor::FIGURE_SELECTION);
    pen.setLineWidth(1);
    pen.setAlpha(1.0);
    pen.drawRectangle(x0, y0, x1-x0, y1-y0);

    pen.setFillColor(1,1,1);
    TRectangle r;
    for(unsigned i=0; i<8; ++i) {
      getBoundingHandle(i, &r);
      pen.fillRectangle(r);
    }
    
    if (pen.getMatrix()) {
      pen.pop();
    }
  }
}

void
TSelectionTool::getBoundingHandle(unsigned i, TRectangle *r)
{
  int w = x1 - x0;
  int h = y1 - y0;
  switch(i) {
    case 0: r->set(x0-3  , y0-3  , 5, 5); break;
    case 1: r->set(x0+w/2, y0-3  , 5, 5); break;
    case 2: r->set(x0+w-1, y0-3  , 5, 5); break;
    case 3: r->set(x0+w-1, y0+h/2, 5, 5); break;
    case 4: r->set(x0+w-1, y0+h-1, 5, 5); break;
    case 5: r->set(x0+w/2, y0+h-1, 5, 5); break;
    case 6: r->set(x0-3,   y0+h-1, 5, 5); break;
    case 7: r->set(x0-3,   y0+h/2, 5, 5); break;
  }
}
      
void
TSelectionTool::calcSelectionsBoundingRectangle(TFigureEditor *fe)
{
  for(TFigureSet::const_iterator p = fe->selection.begin();
      p != fe->selection.end();
      ++p)
  {
    TRectangle r;
#if 1
    fe->getFigureShape(*p, &r, NULL);
#else
    (*p)->getShape(&r);
    if ( (*p)->mat ) {
      TPoint p0, p1;
      (*p)->mat->map(r.x, r.y,             &p0.x, &p0.y);
      (*p)->mat->map(r.x+r.w-1, r.y+r.h-1, &p1.x, &p1.y);
      r.set(p0, p1);
    }
#endif
    if (p==fe->selection.begin()) {
      x0 = r.x; 
      y0 = r.y;
      x1 = r.x + r.w;
      y1 = r.y + r.h;
    } else {
      if (x0 > r.x) x0 = r.x;
      if (y0 > r.y) y0 = r.y;
      if (x1 < r.x + r.w) x1 = r.x + r.w;
      if (y1 < r.y + r.h) y1 = r.y + r.h;
    }
  }

  // map figure coordinates to screen coordinates
  if (fe->getMatrix()) {
    fe->getMatrix()->map(x0, y0, &x0, &y0);
    fe->getMatrix()->map(x1, y1, &x1, &y1);
    --x0;
    --y0;
    --x1;
    --y1;
/*
      x0 += fe->getVisible().x;
      x1 += fe->getVisible().x;
      y0 += fe->getVisible().y;
      y1 += fe->getVisible().y;
*/
  }
}

class TColorPickTool:
  public TFigureTool
{
  public:
    static TColorPickTool* getTool();
    void mouseEvent(TFigureEditor *fe, TMouseEvent &me);
};

TColorPickTool*
TColorPickTool::getTool()
{
  static TColorPickTool* tool = 0;
  if (!tool)
    tool = new TColorPickTool();
  return tool;
}

void 
TColorPickTool::mouseEvent(TFigureEditor *fe, TMouseEvent &me)
{
  if (me.type!=TMouseEvent::LDOWN)
    return;
    
  TCoord x, y;
  fe->mouse2sheet(me.x, me.y, &x, &y);
  TFigure *f = fe->findFigureAt(x, y);
  cout << "found figure " << f << endl;
  if (f) {
    TToolBox::preferences->reason = TFigureAttributes::ALLCHANGED;
    f->getAttributes(TToolBox::preferences);
    TToolBox::preferences->sigChanged();
  }
}


TToolBox::TToolBox(TWindow *p, const string &t):
  super(p, t)
{
TObjectStore& serialize(toad::getDefaultStore());
serialize.registerObject(new TFPath());

  flagParentlessAssistant = true;

  static TFCreateTool frect(new TFRectangle);
  static TFCreateTool fcirc(new TFCircle);
  static TFCreateTool ftext(new TFText);
  static TFCreateTool fpoly(new TFPolygon);
  static TFCreateTool fline(new TFLine);
  static TFCreateTool fbezierline(new TFBezierline);
  static TFCreateTool fbezier(new TFBezier);

  assert(toolbox==0);
  toolbox = this;
  preferences = new TFigureAttributes;
  preferences->linewidth = 48;

  flagNoMenu = true;
  setLayout(0);
  setSize(2+2+28+28+2+2, 480+50+28);
  
  bmp.load(RESOURCE("fischland-small.png"));
  
  TRadioStateModel *state = new TRadioStateModel();
  TWindow *wnd;
  TButtonBase *rb;
  bool odd = true;
  
  int x, y;
  x = 4;
  y = 4+2+28+2+1;

  TFigureAttributes *me = preferences;
  
  for(unsigned i=0; i<20; i++) {
    wnd = 0;
    switch(i) {
      case 0:
        wnd = rb = new TFatRadioButton(this, "pencil", state);
        wnd->setToolTip("Selection");
        rb->loadBitmap(RESOURCE("tool_select.png"));
        CONNECT(rb->sigClicked, me, setTool, TSelectionTool::getTool());
        break;
      case 1:
        wnd = rb = new TFatRadioButton(this, "select", state);
        wnd->setToolTip("Direct Selection");
        rb->loadBitmap(RESOURCE("tool_select.png"));
        CONNECT(rb->sigClicked, me, setOperation, TFigureEditor::OP_SELECT);
        rb->setDown();
        break;
      case 2:
        wnd = rb = new TFatRadioButton(this, "pen", state);
        wnd->setToolTip("Pen: draw beziers");
        rb->loadBitmap(RESOURCE("tool_pen.png"));
        CONNECT(rb->sigClicked, me, setTool, TPenTool::getTool());
        break;
      case 3:
        wnd = rb = new TFatRadioButton(this, "pencil", state);
        wnd->setToolTip("Pencil: freehand curves");
        rb->loadBitmap(RESOURCE("tool_pencil.png"));
        CONNECT(rb->sigClicked, me, setTool, TPencilTool::getTool());
        break;
/*
      case 2:
*/
      case 5:
         wnd = rb = new TFatRadioButton(this, "line", state);
        wnd->setToolTip("Convert Anchor Point");
        rb->loadBitmap(RESOURCE("tool_penc.png"));
//        CONNECT(rb->sigClicked, me, setTool, &fline);
        break;
      case 6:
        wnd = rb = new TFatRadioButton(this, "text", state);
        wnd->setToolTip("create text");
        rb->loadBitmap(RESOURCE("tool_text.png"));
        CONNECT(rb->sigClicked, me, setTool, &ftext);
        break;
      case 7:
        wnd = rb = new TFatRadioButton(this, "circle", state);
        wnd->setToolTip("create ellipse");
        rb->loadBitmap(RESOURCE("tool_circ.png"));
        CONNECT(rb->sigClicked, me, setTool, &fcirc);
        break;
      case 8:
        wnd = rb = new TFatRadioButton(this, "rectangle", state);
        wnd->setToolTip("create rectangle");
        rb->loadBitmap(RESOURCE("tool_rect.png"));
        CONNECT(rb->sigClicked, me, setTool, &frect);
        break;
        
      case 9:
        y+=5;
        wnd = rb = new TFatRadioButton(this, "rotate", state);
        CONNECT(rb->sigClicked, me, setOperation, TFigureEditor::OP_ROTATE);
        wnd->setToolTip("rotate");
        rb->loadBitmap(RESOURCE("tool_rotate.png"));
        break;
      case 10:
        wnd = rb = new TFatRadioButton(this, "free transform", state);
        wnd->setToolTip("free transform");
        break;
        
      case 13:
        y+=5;
        wnd = rb = new TPushButton(this, "group");
        wnd->setToolTip("group selection");
        rb->loadBitmap(RESOURCE("tool_group.png"));
        CONNECT(rb->sigClicked, me, group);
        break;
      case 14:
        wnd = rb = new TPushButton(this, "ungroup");
        wnd->setToolTip("ungroup selection");
        rb->loadBitmap(RESOURCE("tool_ungroup.png"));
        CONNECT(rb->sigClicked, me, ungroup);
        break;

      case 15:
        y+=5;
        wnd = rb = new TPushButton(this, "down");
        wnd->setToolTip("move selection down");
        rb->loadBitmap(RESOURCE("tool_down.png"));
        CONNECT(rb->sigClicked, me, selectionDown);
        break;
      case 16:
        wnd = rb = new TPushButton(this, "bottom");
        wnd->setToolTip("move selection to bottom");
        rb->loadBitmap(RESOURCE("tool_bottom.png"));
        CONNECT(rb->sigClicked, me, selection2Bottom);
        break;

      case 17:
        wnd = rb = new TPushButton(this, "up");
        wnd->setToolTip("move selection up");
        rb->loadBitmap(RESOURCE("tool_up.png"));
        CONNECT(rb->sigClicked, me, selectionUp);
        break;
      case 18:
        wnd = rb = new TPushButton(this, "top");
        wnd->setToolTip("move selection to top");
        rb->loadBitmap(RESOURCE("tool_top.png"));
        CONNECT(rb->sigClicked, me, selection2Top);
        break;
      case 19:
        wnd = new TColorSelector(this, "colorselector", preferences);
        y+=5;
        {
          // some hack, as TColorSelector doesn't create a palette and
          // the whole palette code is still inside this source file
          TInteractor *in = wnd->getFirstChild();
          do {
            if (in->getTitle()=="palette") {
              TPushButton *btn = dynamic_cast<TPushButton*>(in);
              if (btn)
                connect(btn->sigClicked, &openPalette, (TWindow*)0);
              break;
            }
            in = in->getNextSibling();
          } while(in);
        }
        break;
    }
    if (wnd) {
      if (i!=19)
        wnd->setSize(28,28);
      else
        wnd->setSize(28+28,28+28-16);
      if (odd) {
        x = 4;
      } else {
        x = 4 + 28;
      }
      wnd->setPosition(x, y);
      if (!odd)
        y+=28;
      odd = !odd;
    }
  }
  
  TComboBox *cb;
  y+=28+28-16+5;
  
  TTextField *tf = new TTextField(this, "alpha", &alpha);
  tf->setToolTip("alpha");
  tf->setPosition(4,y);
  tf->setSize(28+28-16, 20);
  
  TGauge *gg = new TGauge(this, "alphag", &alpha);
  gg->setToolTip("alpha");
  gg->setPosition(4+28+28-16,y);
  gg->setSize(16,20);

  TCLOSURE1(alpha.sigChanged,
            _this, this,
            _this->preferences->alpha = _this->alpha / 255.0; )
  
//  connect(cb->sigSelection, setLineWidth, cb);
  y+=20+5;

  cb = new TComboBox(this, "linewidth");
  cb->setToolTip("line width");
  cb->setAdapter(new TLineWidthAdapter);
  cb->setPosition(4,y);
  cb->setSize(28+28, 20);
  lw_sm = new TSingleSelectionModel();
  cb->setSelectionModel(lw_sm);
  connect(lw_sm->sigChanged, setLineWidth, lw_sm);
  y+=20;

  cb = new TComboBox(this, "linestyle");
  cb->setToolTip("line style");
  cb->setAdapter(new TLineStyleAdapter);
  cb->setPosition(4,y);
  cb->setSize(28+28, 20);
  ls_sm = new TSingleSelectionModel();
  cb->setSelectionModel(ls_sm);
  connect(cb->sigSelection, setLineStyle, ls_sm);
  y+=20+5;

  cb = new TComboBox(this, "arrowmode");
  cb->setToolTip("arrow mode");
  cb->setAdapter(new TArrowModeAdapter);
  cb->setPosition(4,y);
  cb->setSize(28+28, 20);
  am_sm = new TSingleSelectionModel();
  cb->setSelectionModel(am_sm);
  connect(cb->sigSelection, setArrowMode, am_sm);
  y+=20;

  cb = new TComboBox(this, "arrowtype");
  cb->setToolTip("arrow type");
  cb->setAdapter(new TArrowTypeAdapter);
  cb->setPosition(4,y);
  cb->setSize(28+28, 20);
  at_sm = new TSingleSelectionModel();
  cb->setSelectionModel(at_sm);
  connect(cb->sigSelection, setArrowType, at_sm);
  y+=20+5;

  rb = new TFontButton(this, "font", me);
  rb->setPosition(4,y);
  rb->setSize(28+28,28+28);
  rb->setToolTip("select font");
  connect(rb->sigClicked, selectFont);
  y+=28+28+5;

  tf = new TTextField(this, "gridsize");
  tf->setToolTip("grid size");
  tf->setPosition(4,y);
  tf->setSize(28+8, 20);
  tf->setModel(&preferences->gridsize);

  TFatCheckButton *pb = new TFatCheckButton(this, "enablegrid");
  pb->setToolTip("enabled/disable grid");
  pb->loadBitmap(RESOURCE("enablegrid.png"));
  pb->setPosition(4+28+8,y);
  pb->setSize(20, 20);
  pb->setModel(&preferences->drawgrid);
  y+=20+5;

  rb = new TPushButton(this, "applyall");
  rb->setPosition(4,y);
  rb->setSize(28,28);
  rb->setToolTip("apply all attributes to selection");
  rb->loadBitmap(RESOURCE("tool_fill.png"));
  CONNECT(rb->sigClicked, me, applyAll);
//  y+=20+5;

  rb = new TFatRadioButton(this, "fetchall", state);
//  rb = new TPushButton(this, "fetchall");
  rb->setPosition(4+28,y);
  rb->setSize(28,28);
  rb->setToolTip("pick attributes from figure");
  rb->loadBitmap(RESOURCE("tool_pick.png"));
  TCLOSURE1(
    rb->sigClicked,
    attr, me,
    attr->setTool(TColorPickTool::getTool());
  )
  
  CONNECT(preferences->sigChanged, this, preferencesChanged);
}

void
TToolBox::paint()
{
  TPen pen(this);

  // image
  pen.drawBitmap(4,4,bmp);
  pen.draw3DRectanglePC(3,3,1+28+28,1+28, false);

  int y, h, w;
  w = 4+28+28;

  // image frame
  y = 2; h = 4+28;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // draw tools
  y += h+1; h = 4+4*28;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // rotate & ?
  y += h+1; h = 4+28;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // group & ungroup
  y += h+1; h = 4+28;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // depth
  y += h+1; h = 4+2*28;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // color palette
  y += h+1; h = 4+28+28-16;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // alpha
  y += h+1; h = 4+20;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // line style
  y += h+1; h = 4+2*20;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // arrow style
  y += h+1; h = 4+2*20;
  pen.draw3DRectanglePC(2,y,w,h, true);

  // font button
  y += 25+25+5+5;

  // grid
  y += h+1; h = 4+20;
  pen.draw3DRectanglePC(2,y,w,h, true);
}
