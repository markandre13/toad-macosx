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

/*
  some call paths:
  
  TFigureEditor::addFigure(f)
  TFigureEditor::model->add(f)
  TFigureModel::add(f)
  TFigureModel::type=ADD, TFigureModel::figures=f, TFigureModel::sigChanged
  TFigureModel::modelChanged(model)
  TWindow::updateScrollBars()
*/

#include <toad/figureeditor.hh>
#include <toad/figure.hh>
#include <toad/figuretool.hh>
#include <toad/colordialog.hh>
#include <toad/scrollbar.hh>
#include <toad/checkbox.hh>
#include <toad/action.hh>
#include <toad/undomanager.hh>
#include <toad/window.hh>

#include <toad/stacktrace.hh>

#include <cmath>
#include <algorithm>

// missing in mingw
#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi */
#endif

using namespace toad;

#define DBM(CMD)
// #define VERBOSE 1

/**
 * \ingroup figure
 * \class toad::TFigureEditor
 * TFigureEditor is a graphical editor for TFigureModels.
 * 
 * TFigureEditor can be used as a window of its own or just as an object 
 * to delegate events from other windows to. The later was needed for
 * some of TOAD's dialog editors.
 *
 * Selection Mode
 *
 * The following keys control the selection mode and are choosen to match
 * the behaviour of selecting objects in TTable:
 *
 * \li click           : select and move object and its handles
 * \li SHIFT+click     : select area
 * \li CTRL+click      : select/deselect single object
 * \li SHIFT+CTRL+click: select additional area
 *
 * \todo
 *   \li
 *      undo, redo & model-view for selection2Top, selection2Bottom, 
 *      selectionUp, selectionDown and rotate
 *   \li
 *      group followed by undo causes a segfault or inifinite recursion
 *      or something like that
 *   \li
 *      scrollbars aren't setup properly during scaling
 *   \li
 *      resize groups
 *   \li
 *      adjust getShape to check transformations
 *   \li
 *      adjust finding figures & handles for transformations
 *   \li
 *      scrollbars aren't update after and during object creation
 *   \li
 *      polgons contain bogus points (can be seen during/after rotation)
 *   \li
 *      Make sure that we can have multiple views of one model
 *   \li
 *      Add a selection model
 *   \li
 *      Consider making two classes of this one.
 *   \li
 *      actVScroll & actHScroll called updateScrollbars which lead to
 *      an endless recursion; these calls are currently removed
 *   \li
 *      allow the usage of multiple models to provide layers
 *   \li
 *      color changes aren't part of undo/redo
 */

/*
 * The 'gadget' attribute is used to
 * o identify the figure currently rotated
 * o ...
 *
 */

TFigureAttributes::TFigureAttributes()
{
  linecolor.set(0,0,0);
  fillcolor.set(1,1,1);
  alpha.setRangeProperties(255,0,0,255);
//  connect(alpha.sigChanged, foobar, this, ALPHA);
  CLOSURE1(alpha.sigChanged, TFigureAttributes *dst, this,
    dst->reason.alpha = true;
    dst->sigChanged();
  );
  outline = true;
  filled = false;
  fontname = "arial,helvetica,sans-serif:size=12";

  drawgrid = true;
//  connect(drawgrid.sigChanged, foobar, this, GRID);
  CLOSURE1(drawgrid.sigChanged, TFigureAttributes *dst, this,
    dst->reason.grid = true;
    dst->sigChanged();
  );
  gridsize = 4;
//  connect(gridsize.sigChanged, foobar, this, GRID);
  CLOSURE1(gridsize.sigChanged, TFigureAttributes *dst, this,
    dst->reason.grid = true;
    dst->sigChanged();
  );
  
  linewidth = 0;
  linestyle = TPen::SOLID;
  arrowmode = TFLine::NONE;
  arrowtype = TFLine::EMPTY;
  
  current = 0;
  tool = 0;
}

TFigureAttributes::~TFigureAttributes()
{
}

void
TFigureAttributes::setOperation(unsigned op)
{
  if (current) current->setOperation(op);
}

void TFigureAttributes::setCreate(TFigure *figure)
{
  setTool(new TFCreateTool(figure));
}

void
TFigureAttributes::setTool(TFigureTool *aTool)
{
  if (tool==aTool)
    return;
  tool = aTool;
  reason.tool = true;
  sigChanged();
}

void
TFigureAttributes::group()
{
  if (current) current->group();
}

void
TFigureAttributes::ungroup()
{
  if (current) current->ungroup();
}

void
TFigureAttributes::selectionDown()
{
  if (current) current->selectionDown();
}

void
TFigureAttributes::selection2Bottom()
{
  if (current) current->selection2Bottom();
}

void
TFigureAttributes::selectionUp()
{
  if (current) current->selectionUp();
}

void
TFigureAttributes::selection2Top()
{
  if (current) current->selection2Top();
}

void
TFigureAttributes::applyAll()
{
  if (current) current->applyAll();
}

/**
 * This constructer is to be used when TFigureEditor isn't used as a
 * window itself but handles events delegated to it from another window.
 *
 * This feature 
 *
 * \sa setWindow
 */
TFigureEditor::TFigureEditor():
  super(NULL, "(TFigureEditor: no window)")
{
  init(NULL);
  flagExplicitCreate = true; // don't create, see TWindow::createParentless()
  window = NULL;
  row_header_renderer = col_header_renderer = 0;
}

TFigureEditor::TFigureEditor(TWindow *p, const string &t, TFigureModel *m):
  super(p, t)
{
  init(m);
  flagNoBackground = true;
  window = this;
  row_header_renderer = col_header_renderer = 0;
}

TFigureEditor::~TFigureEditor()
{
  setModel(0);
  setAttributes(0);
  if (mat)
    delete mat;
}

/**
 * Handle events for another window.
 *
 * This feature was added for the internal dialogeditor.
 *
 * \param w Then window to handle.
 */
void
TFigureEditor::setWindow(TWindow *w)
{
  invalidateWindow();
  window = w;
  invalidateWindow();
}

void
TFigureEditor::init(TFigureModel *m)
{
  quick = false;
  quickready = false;
  modified = false;
  preferences = 0;
  tool = 0;
  fuzziness = 3;

  handle = -1;
  gadget = NULL;
  operation = OP_SELECT;
  state = STATE_NONE;
  use_scrollbars = true;
  mat = 0;
//  vscroll = NULL;
//  hscroll = NULL;
  window = 0;
  model = 0;
  x1=y1=x2=y2=0;
  if (!m)
    m=new TFigureModel();
  setAttributes(new TFigureAttributes);
  setModel(m);

  TAction *action;

  action = new TAction(this, "edit|cut");
  CONNECT(action->sigClicked, this, selectionCut);
  action = new TAction(this, "edit|copy");
  CONNECT(action->sigClicked, this, selectionCopy);
  action = new TAction(this, "edit|paste");
  CONNECT(action->sigClicked, this, selectionPaste);

  action = new TAction(this, "edit|delete");
  CONNECT(action->sigClicked, this, deleteSelection);

  action = new TAction(this, "object|order|top");
  CONNECT(action->sigClicked, this, selection2Top);
  action = new TAction(this, "object|order|up");
  CONNECT(action->sigClicked, this, selectionUp);
  action = new TAction(this, "object|order|down");
  CONNECT(action->sigClicked, this, selectionDown);
  action = new TAction(this, "object|order|bottom");
  CONNECT(action->sigClicked, this, selection2Bottom);

  action = new TAction(this, "object|group");
  CONNECT(action->sigClicked, this, group);
  action = new TAction(this, "object|ungroup");
  CONNECT(action->sigClicked, this, ungroup);
}

bool
TFigureEditor::restore(TInObjectStream &in)
{
  clearSelection();
  TSerializable *s;

  // ::restorePtr(in, &s);
  s = in.restore();

  TFigureModel *m = dynamic_cast<TFigureModel *>(s);
  if (!m) {
    cerr << "wasn't a TFigureModel" << endl;
    return false;
  }
  setModel(m);
  return true;
}

void
TFigureEditor::store(TOutObjectStream &out) const
{
  if (model)
    ::store(out, model);
}

void 
TFigureEditor::identity() 
{ 
  if (mat) {
    mat->identity();
    updateScrollbars();
    quickready = false;
  }
}

/**
 */
void TFigureEditor::rotate(double d)
{
  if (!mat)
    mat = new TMatrix2D();
  mat->rotate(d);
  updateScrollbars();
  quickready = false;
  invalidateWindow();
}

/**
 */
void TFigureEditor::rotateAt(double x, double y, double radiants)
{
  if (!mat)
    mat = new TMatrix2D();
  mat->rotateAt(x, y, radiants);
  updateScrollbars();
  quickready = false;
  invalidateWindow();
}

/**
 */
void TFigureEditor::translate(TCoord x, TCoord y)
{
  if (!mat)
    mat = new TMatrix2D();
  mat->translate(x, y);
  updateScrollbars();
  quickready = false;
  invalidateWindow();
}

/**
 * Scale the edit pane.
 */
void TFigureEditor::scale(TCoord sx, TCoord sy)
{
  if (!mat)
    mat = new TMatrix2D();
  mat->scale(sx, sy);
/*
cout << "TFigureEditor::scale(" << sx << ", " << sy << ")\n"
     << "  " << mat->a << ", " << mat->b << endl
     << "  " << mat->c << ", " << mat->d << endl
     << "  " << mat->tx << ", " << mat->ty << endl;
*/

// better: create 2 points, transform 'em and calculate the
// distance
  fuzziness = static_cast<int>(2.0 / sx);
  quickready = false;
  updateScrollbars();
  invalidateWindow();
}

/**
 * This method is doing nothing yet.
 */
void TFigureEditor::shear(double x, double y)
{
  if (!mat)
    mat = new TMatrix2D();
  mat->shear(x, y);
  quickready = false;
  updateScrollbars();
  invalidateWindow();
}

/**
 * This method is doing nothing yet.
 */
void TFigureEditor::multiply(const TMatrix2D *m)
{
  if (!mat)
    mat = new TMatrix2D(*m);
  else
    mat->multiply(m);
  quickready = false;
  updateScrollbars();
  invalidateWindow();
}

/**
 * \param b 'true' if scrollbars shall be used.
 */
void
TFigureEditor::enableScroll(bool b)
{
  use_scrollbars = b;
}

/**
 * \param b 'true' if a grid shall be drawn into the window.
 */
void
TFigureEditor::enableGrid(bool b)
{
  if (b==preferences->drawgrid)
    return;
  quickready = false;
  preferences->drawgrid = b;
  invalidateWindow(visible);
}

/**
 * Set the size of the grid.
 */
void
TFigureEditor::setGrid(TCoord gridsize) {
  if (gridsize<0)
    gridsize=0;
  preferences->gridsize = gridsize;
  quickready = false;
  invalidateWindow(visible);
}

void
TFigureEditor::resize()
{
  updateScrollbars();
}

int rotx=100;
int roty=100;
double rotd=0.0;
double rotd0;

int select_x;
int select_y;

void
TFigureEditor::paint()
{
  if (!window) {
    cout << __PRETTY_FUNCTION__ << ": no window" << endl;
    return;
  }
  TPen pen(window);
  if (!model) {
    pen.setColor(TColor::DIALOG);
    pen.fillRectangle(0,0,window->getWidth(), window->getHeight());
    return;
  }

  pen.setColor(window->getBackground());
  TCoord x, y;
  window->getOrigin(&x, &y);
  pen.fillRectanglePC(-x,-y,window->getWidth(),window->getHeight());

  if (mat) {
    pen.multiply(mat);
  }

  paintGrid(pen);
  print(pen, model, true);
  paintSelection(pen);
  
  paintDecoration(pen);
}

/**
 * Called from 'paint' to draw the grid.
 */
void
TFigureEditor::paintGrid(TPenBase &pen)
{
  if (!preferences ||
      !preferences->drawgrid ||
      !preferences->gridsize)
  {
    return;
  }

  const TRGB &background_color = window->getBackground();
  pen.setColor(
    background_color.r > 0.5 ? background_color.r-0.5 : background_color.r+0.5,
    background_color.g > 0.5 ? background_color.g-0.5 : background_color.g+0.5,
    background_color.b > 0.5 ? background_color.b-0.5 : background_color.b+0.5
  );
  TCoord x1, x2, y1, y2;
  TCoord g = preferences->gridsize;

  TRegion region(*window->getUpdateRegion());
  region &= visible;

  TRectangle r;
  region.getBoundary(&r);
  x1=r.x;
  y1=r.y;
  x2=r.x+r.w+1;
  y2=r.y+r.h+1;

  const TMatrix2D *mat = pen.getMatrix();
  if (mat) {
/*
cout << "TFigureEditor::paintGrid: matrix" << endl
     << "  " << mat->a << ", " << mat->b << endl
     << "  " << mat->c << ", " << mat->d << endl
     << "  " << mat->tx << ", " << mat->ty << endl;
*/
    TCoord gx0, gx, gy0, gy;
    TMatrix2D m(*mat);   
    m.map(0, 0, &gx0, &gy0);
    m.map(preferences->gridsize, preferences->gridsize, &gx, &gy);
    gx-=gx0;
    gy-=gy0;
//cout << "gx,gy=" << gx << "," << gy << endl;
    m.invert();
    if (gx<=2 || gy<=2) {
      // don't draw grid, it's too small
      return;
    } else {
      TMatrix2D m(*mat);
      m.invert();
      m.map(x1, y1, &x1, &y1);
      m.map(x2, y2, &x2, &y2);
      if (x1>x2) {
        TCoord a = x1; x1 = x2; x2 = a;
      }
      if (y1>y2) {
        TCoord a = y1; y1 = y2; y2 = a;
      }
    }
  }

  // justify to grid
  x1 -= fmod(x1, g);
  y1 -= fmod(y1, g);

  for(auto y=y1; y<=y2; y+=g) {
    for(auto x=x1; x<=x2; x+=g) {
      pen.drawPoint(x, y);
    }
  }
}  

/**
 * Called from 'paint' to draw the selection marks.
 */
void
TFigureEditor::paintSelection(TPenBase &pen)
{
  if (tool) {
    if (tool->paintSelection(this, pen))
      return;
  }

  // draw the selection marks over all figures
  for(auto sp = selection.begin(); sp != selection.end(); ++sp) {
    pen.setLineWidth(1);
    if (*sp!=gadget) {
      (*sp)->paintSelection(pen, -1);
    } else {
      (*sp)->paintSelection(pen, handle);
    }
  }

  if (state==STATE_SELECT_RECT) {
    const TMatrix2D *mat = pen.getMatrix();
    if (mat) {
      pen.push();
      pen.identity();
      pen.setColor(0,0,0);
      pen.setLineStyle(TPen::DOT);
      pen.setLineWidth(1.0);
      TCoord x0, y0, x1, y1;
      mat->map(down_x, down_y, &x0, &y0);
      mat->map(select_x, select_y, &x1, &y1);
      pen.drawRectanglePC(x0, y0, x1-x0, y1-y0);
      pen.pop();
    } else {
      pen.setColor(0,0,0);
      pen.setLineStyle(TPen::DOT);
      pen.setLineWidth(0);
      pen.drawRectanglePC(down_x, down_y, select_x-down_x, select_y-down_y);
    }
  }
}

/**
 * Called from 'paint' to draw the corners and the row and column headers
 */
void
TFigureEditor::paintDecoration(TPen &scr)
{
  if (window==this)
    paintCorner(scr);
  // else ... not implemented yet
  
  if (row_header_renderer) {
    scr.push();
    scr.identity();
    scr.setClipRect(TRectangle(0, visible.y, visible.x, visible.h));
    scr.translate(0, visible.y+window->getOriginY());
    row_header_renderer->render(scr, -window->getOriginY(), visible.h, mat);
    scr.pop();
  }

  if (col_header_renderer) {
    scr.push();
    scr.identity();
    scr.setClipRect(TRectangle(visible.x, 0, visible.w, visible.y));
    scr.translate(visible.x+window->getOriginX(), 0);
    col_header_renderer->render(scr, -window->getOriginX(), visible.w, mat);
    scr.pop();
  }
}

/**
 * Draw all figures.
 *
 * This method is called from 'paint' to draw the whole drawing area.
 * It's provided as an separate method so users can also draw on other
 * devices than the screen.
 *
 * \param pen
 *   The pen to be used, ie. TPen for the screen or TPrinter for the printer.
 * \param model
 *   The figuremodel to be drawn
 * \param withSelection
 *   When set to 'true', the method will call the paint method of all
 *   selected figures with TFigure::SELECT as 2nd parameter. The figure
 *   TFText uses this to draw the text caret when in edit mode.
 * \param justSelection
 *   Only draw figures which are part of the selection
 *
 *   Handles to move, resize and rotate figures are drawn by the paint
 *   method itself.
 */  
void
TFigureEditor::print(TPenBase &pen, TFigureModel *model, bool withSelection, bool justSelection)
{
  if (!model)
    return;
  TRectangle cb, r;
  cb.set(0,0,getWidth(),getHeight());
  pen.getClipBox(&cb);
  for(TFigureModel::iterator p = model->begin();
      p != model->end();
      ++p)
  {
    TRectangle r;
    getFigureShape(*p, &r, pen.getMatrix());
    if (!r.intersects(cb)) {
      continue;
    }

    TFigure::EPaintType pt = TFigure::NORMAL;
    unsigned pushs = 0;
    if (gadget==*p) {
      if (state==STATE_ROTATE) {
//cerr << "paint figure for rotation edited figure at rotd=" << rotd << endl;
        pen.push();
        pushs++;
        pen.translate(rotx, roty);
        pen.rotate(rotd);
        pen.translate(-rotx, -roty);
      } else {
        pt = TFigure::EDIT;
      }
    }

    bool skip = false;
    if (withSelection || justSelection) {
      if (gadget==*p || selection.find(*p)!=selection.end()) {
        if (withSelection)
          pt = TFigure::SELECT;
      } else {
        if (justSelection)
          skip = true;
      }
    }
    if (!skip) {
      (*p)->paint(pen, pt);
    }
    while(pushs) {
      pen.pop();
      pushs--;
    }
  }
}

void 
TFigureEditor::setAttributes(TFigureAttributes *p) {
  if (preferences) {
    disconnect(preferences->sigChanged, this);
    if (preferences->getCurrent() == this)
      preferences->setCurrent(0);
  }
  preferences = p;
  if (preferences) {
    preferences->setCurrent(this);
    setTool(preferences->getTool());
    connect(preferences->sigChanged, this, &TThis::preferencesChanged);
  }
}

void
TFigureEditor::preferencesChanged()
{
  if (!preferences)
    return;
  quickready=false;

  if (preferences->reason.tool) {
    if (state != STATE_NONE)
      stopOperation();
    tool = preferences->getTool();
  }

//  if (preferences->reason.grid) {
    invalidateWindow(visible);
//  }
  
//  if (preferences->reason == TFigureAttributes::ALL) {
//    setTool(preferences->getTool());
//    invalidateWindow(visible);
//  }

  if (tool)
    tool->setAttributes(preferences);
  
  model->setAttributes(selection, preferences);
//  invalidateWindow(visible); 

  preferences->clearReasons();
}

void
TFigureEditor::setLineColor(const TRGB &rgb)
{
  preferences->linecolor = rgb;
  preferencesChanged();
}

void
TFigureEditor::setFillColor(const TRGB &rgb)
{
  preferences->setFillColor(rgb);
}

void
TFigureEditor::unsetFillColor()
{
  preferences->filled = false;
  preferencesChanged();
}

void
TFigureEditor::setFont(const string &fontname)
{
  preferences->fontname = fontname;
  preferencesChanged();
}

void
TFigureEditor::modelChanged()
{
  bool update_scrollbars = false;
  modified = true;
  quickready = false; // force a view update when quick mode is enabled
  if (tool) {
    tool->modelChanged(this);
  }
  switch(model->type) {
    case TFigureModel::MODIFY:
    case TFigureModel::MODIFIED:
    case TFigureModel::ADD:
      update_scrollbars = true;
      for(TFigureSet::iterator p=model->figures.begin();
          p!=model->figures.end();
          ++p)
      {
        invalidateFigure(*p);
      }
      break;
    case TFigureModel::REMOVE:
      update_scrollbars = true;
      for(TFigureSet::iterator p=model->figures.begin();
          p!=model->figures.end();
          ++p)
      {
        invalidateFigure(*p);
        TFigureSet::iterator q = selection.find(*p);
        if (q!=selection.end()) {
          selection.erase(q);
        }
      }
      break;
    case TFigureModel::GROUP:
      #warning "not removing figure from selection"
      selection.clear();
      selection.insert(model->figure);
      invalidateFigure(model->figure);
      break;
    case TFigureModel::_UNDO_GROUP:
      invalidateFigure(model->figure);
      selection.clear();
      selection.insert(model->figures.begin(), model->figures.end());
      break;
    case TFigureModel::UNGROUP:
      #warning "not removing figure (group) from selection"
      invalidateWindow(visible); // OPTIMIZE ME
      break;
  }
  
  if (update_scrollbars) {
    updateScrollbars();
  }
}

/**
 * Add a figure to the editors model.
 */
void
TFigureEditor::addFigure(TFigure *figure)
{
  assert(model!=0);
  model->add(figure);
}

/**
 * Removes a figure from the editors model.
 */
void
TFigureEditor::deleteFigure(TFigure *g)
{
  if (g==gadget)
    gadget=NULL;

  TFigureSet::iterator s;
  s = selection.find(g);
  if (s!=selection.end())
    selection.erase(s); 

  model->erase(g);
}

bool
TFigureEditor::clearSelection()
{
//cout << "TFigureEditor::clearSelection" << endl;
//printStackTrace();
  if (selection.empty())
    return false;
  for(TFigureSet::iterator p = selection.begin();
      p != selection.end();
      ++p)
  {
    invalidateFigure(*p);
  }
  selection.erase(selection.begin(), selection.end());
  return true;
}

/**
 * Delete all selected objects with `removeable' being true.
 */
void
TFigureEditor::deleteSelection()
{
  if (gadget && selection.find(gadget)!=selection.end()) {
    gadget = 0;
  }
  model->erase(selection);
}

void
TFigureEditor::selectAll()
{
  for(TFigureModel::iterator p = model->begin();
      p != model->end();
      ++p)
  {
    selection.insert(*p);
  }
  quickready = false;
  invalidateWindow(visible);
}

void
TFigureEditor::deleteAll()
{
  selectAll();
  deleteSelection();
//  setOrigin(0,0); needed?
  setPanePos(0,0);
  updateScrollbars();
}

void
TFigureEditor::selection2Top()
{
  TFigureModel::iterator p,b,np;
  p = model->end();
  b = model->begin();

  if (p==b)
    return;

  --p; np=p;                      // p, np @ last element

  if (p==b)
    return;

  while(true) {
    if (selection.find(*p)!=selection.end()) {
      if (p!=np) {
        TFigure *akku = *p;
        TFigureModel::iterator mp = p;
        while(mp!=np) {
          TFigureModel::iterator op = mp;
          ++mp;
          *op = *mp;
        }
        *np = akku;
      }
      --np;
    }
    if (p==b)
      break;
    --p;
  }
  quickready = false;
  window->invalidateWindow(visible);
}

void
TFigureEditor::selection2Bottom()
{
  TFigureModel::iterator p, e, np;
  p = np = model->begin();
  e = model->end();
  if (p==e)
    return;

  if (selection.find(*p)!=selection.end())
    ++np;
  ++p;

  while(p!=e) {
    // if *p is in the list
    if (selection.find(*p)!=selection.end()) {
      TFigure *akku = *p;
      TFigureModel::iterator mp = p;
      while(mp!=np) {
        TFigureModel::iterator op = mp;
        --mp;
        *op = *mp;
      }
      *np = akku;
      ++np;
    }
    ++p;
  }
  quickready = false;
  window->invalidateWindow(visible);
}

void
TFigureEditor::selectionUp()
{
  TFigureModel::iterator p,e,b,prev;
  p = e = prev = model->end();
  b = model->begin();
  if (p==b)
    return;
  while(true) {
    if (selection.find(*p)!=selection.end()) {
      if (prev!=e) {
        TFigure* a = *p;
        *p = *prev;
        *prev = a;
      }
      prev = e;
    } else {
      prev = p;
    }
    if (p==b)
      break;
    --p;
  }
  quickready = false;
  window->invalidateWindow(visible);
}

void
TFigureEditor::selectionDown()
{
  TFigureModel::iterator p,e,prev;
  p = model->begin();
  e = prev = model->end();
  while(p!=e) {
    if (selection.find(*p)!=selection.end()) {
      if (prev!=e) {
        TFigure* a = *p;
        *p = *prev;
        *prev = a;
      }
      prev=e;
    } else {
      prev=p;
    }
    ++p;
  }
  quickready = false;
  window->invalidateWindow(visible);
}

void
TFigureEditor::group()
{
  if (model)
    model->group(selection);
}

void
TFigureEditor::ungroup()
{
  if (model)
    model->ungroup(selection, &selection);
}

void
TFigureEditor::setModel(TFigureModel *m)
{
  if (model==m)
    return;
  if (model) {
    stopOperation();
    clearSelection();
    disconnect(model->sigChanged, this);
    TUndoManager::unregisterModel(this, model);
  }
  model = m;
  modified = false;
  if (model) {
    connect(model->sigChanged, this, &TFigureEditor::modelChanged);
    TUndoManager::registerModel(this, model);
  }
  if (isRealized()) {
    invalidateWindow(visible);
    updateScrollbars();
  }
}

/**
 * Abondon the current mode of operation and select a new mode.
 */
void
TFigureEditor::setOperation(unsigned op)
{
#if VERBOSE
  cout << "Setting Operation " << op << endl;
#endif
  stopOperation();
  clearSelection();
  if (window)
    window->setFocus();
  operation = op;
  tool = 0;
  setTool(0);
}

void
TFigureEditor::setCreate(TFigure *figure)
{
  setTool(new TFCreateTool(figure));
}

void
TFigureEditor::setTool(TFigureTool *aTool)
{
//cout << "TFigureEditor: setTool " << aTool << endl;
  stopOperation();
  clearSelection();
  if (tool!=aTool) {
    tool = aTool;
    toolChanged(aTool);
  }
  if (window)
    window->setFocus();
}

void
TFigureEditor::toolChanged(TFigureTool*)
{
}

void
TFigureEditor::applyAll()
{
  preferences->setAllReasons();
  model->setAttributes(selection, preferences);
}

/**
 * Abort the current operation mode.
 */
void
TFigureEditor::stopOperation()
{
  if (tool) {
    tool->stop(this);
  } else {
    switch(state) {
      case STATE_CREATE:
        clearSelection();
        if (gadget) {
          // selection.insert(gadget);
          model->figures.clear();
          model->figures.insert(gadget);
          model->type = TFigureModel::MODIFIED;
          model->sigChanged();
        }
        window->setAllMouseMoveEvents(true);
        break;
    }
  }
  if (gadget) {
    invalidateFigure(gadget);
    gadget = NULL;
  }
  state = STATE_NONE;
}

namespace {
  TFigureVector clipboard;
}

void
TFigureEditor::selectionCut()
{
  selectionCopy();
  deleteSelection();
}

void
TFigureEditor::selectionCopy()
{
  for(TFigureVector::iterator p = clipboard.begin();
      p != clipboard.end();
      ++p)
  {
    delete *p;
  }
  clipboard.clear();

  for(TFigureModel::iterator p = model->begin();
      p != model->end();  
      ++p)
  {
    if (selection.find(*p)!=selection.end()) {
      clipboard.push_back( static_cast<TFigure*>( (*p)->clone() ) );
    }
  }
}

void
TFigureEditor::selectionPaste()
{
  clearSelection();

  TFigureVector copy;
  for(TFigureVector::iterator p = clipboard.begin();
      p != clipboard.end();
      ++p)
  {
    TFigure *f = static_cast<TFigure*>( (*p)->clone() );
    selection.insert(f);
    copy.push_back(f);
  }
  model->add(copy);
}

void
TFigureEditor::keyEvent(const TKeyEvent &ke)
{
  if (!model)
    return;
  if (tool)
    tool->keyEvent(this, ke);
}

void
TFigureEditor::mouse2sheet(TPoint mouse, TPoint *sheet)
{
  mouse-=visible.origin();
  if (mat) {
    TMatrix2D m(*mat);
    m.invert();
    m.map(mouse, &mouse);
  }
  *sheet = mouse;
}

void
TFigureEditor::mouseEvent(const TMouseEvent &me)
{
//cerr << "TFigureEditor::mouseEvent" << endl << endl << endl;
  if (!model)
    return;

  TPoint pos = getOrigin();
  pos += me.pos;
  
  switch(me.type) {
    case TMouseEvent::LDOWN:
    case TMouseEvent::MDOWN:
    case TMouseEvent::RDOWN:
/*
    // this would require some kind of grab emulation...
    case TMouseEvent::LUP:
    case TMouseEvent::MUP:
    case TMouseEvent::RUP:
    ...
*/
      if (row_header_renderer &&
          pos.x < visible.x &&
          pos.y >= visible.y ) 
      {
        TMouseEvent me2(me, TPoint(pos.x, me.pos.y));
        row_header_renderer->mouseEvent(me2);
      } else
      if (col_header_renderer &&
          pos.x >= visible.x &&
          pos.y < visible.y ) 
      {
        TMouseEvent me2(me, TPoint(me.pos.x, pos.y));
        row_header_renderer->mouseEvent(me2);
      } else {
        if (!tool) {
          super::mouseEvent(me);
          return;
        }
      }
      break;
    default:
      ;
  }
  
  if (!tool)
    return;
  if (!window)
    return;
    
  if (me.type==TMouseEvent::LDOWN ||
      me.type==TMouseEvent::MDOWN ||
      me.type==TMouseEvent::RDOWN)
  {
    setFocus();
    if (preferences)
      preferences->setCurrent(this);
  }  

  tool->mouseEvent(this, me);
}

void
TFigureEditorHeaderRenderer::mouseEvent(const TMouseEvent &me)
{
}

/**
 * Return the closest point to (inX, inY) on the grid in (outX, outY)
 */
void
TFigureEditor::sheet2grid(TPoint in, TPoint *out)
{
TPoint gridorigin(0.5, 0.5);
  if (!preferences->drawgrid) {
    *out = in;
    return;
  }
  if (state!=STATE_ROTATE && state!=STATE_MOVE_ROTATE) {
    int g = preferences->gridsize;
    out->x = round(in.x/g)*g+gridorigin.x;
    out->y = round(in.y/g)*g+gridorigin.y;
  } else {
    *out = in;
  }
}

void
TFigureEditor::invalidateFigure(TFigure* figure)
{
  if (!window)
    return;
  TRectangle r;
//figure->getShape(&r);
//cout << figure->getClassName() << ": invalidate shape " <<r.x<<"-"<<(r.x+r.w)<<","<<r.y<<"-"<<(r.y+r.h)<<endl;
  getFigureEditShape(figure, &r, mat);
  r.x+=window->getOriginX() + visible.x;
  r.y+=window->getOriginY() + visible.y;
  if (r.x < visible.x ) {
    TCoord d = visible.x - r.x;
    r.x += d;
    r.w -= d;
  }
  if (r.y < visible.y ) {
    TCoord d = visible.y - r.y;
    r.y += d;
    r.h -= d;
  }
//cout << figure->getClassName() << ": invalidate window " <<r.x<<"-"<<(r.x+r.w)<<","<<r.y<<"-"<<(r.y+r.h)<<endl;
  invalidateWindow(r);
}

/**
 * Get the figures shape (bounding rectangle)
 *
 * TFigure::getShape's returned will not include it's 'mat' and 'cmat'
 * transformations, thus this method is doing that.
 *
 * \note
 *   This function should be better part for TFigure. And we could also
 *   declare mat and cmat as protected.
 *
 * \param figure
 *   The figure, whose shape we seek.
 * \param r
 *   The resulting shape
 * \param mat
 *   A matrix or NULL.
 */
void
TFigureEditor::getFigureShape(TFigure* figure, TRectangle *r, const TMatrix2D *mat)
{
  *r = figure->bounds();
  _getFigureShape(figure, r, mat);
}

void
TFigureEditor::getFigureEditShape(TFigure* figure, TRectangle *r, const TMatrix2D *mat)
{
  *r = figure->editBounds();
  _getFigureShape(figure, r, mat);
}

void
TFigureEditor::_getFigureShape(TFigure* figure, TRectangle *r, const TMatrix2D *mat)
{
  if (mat) {
    TMatrix2D m;
    if (mat)
      m=*mat;
      
    TCoord x1, x2, y1, y2;
    TCoord x, y;
    m.map(r->x, r->y, &x, &y);
    x1 = x2 = x;
    y1 = y2 = y;
    for(int i=1; i<4; ++i) {
      switch(i) {
        case 1:
          m.map(r->x+r->w, r->y, &x, &y);
          break;
        case 2:
          m.map(r->x+r->w, r->y+r->h, &x, &y);
          break;
        case 3:
          m.map(r->x, r->y+r->h, &x, &y);
          break;
      }
      if (x1>x)
        x1=x;
      if (x2<x)
        x2=x;
      if (y1>y)
        y1=y;
      if (y2<y)
        y2=y;
    }
    r->set(TPoint(x1,y1), TPoint(x2, y2));
  }
//cout << "invalidating shape " << r.x << "," << r.y << "," << r.w << "," << r.h << endl;

  // add some extra for the figure handles
  // (this would be be delegated to the figure itself, as the figure contains
  // the code to draw the handles!)
/*
  r->x-=3;
  r->y-=3;
  r->w+=6;
  r->h+=6;
*/
}

/**
 * Find the gadget at position at (mx, my).
 *
 * This method doesn't find gadgets which are currently created or edited.
 */
TFigure*
TFigureEditor::findFigureAt(TPoint m)
{
#if VERBOSE
  cerr << "TFigureEditor::findFigureAt " << m << endl;
#endif

TCoord mx = m.x;
TCoord my = m.y;

  double distance = INFINITY;
  TFigureModel::const_iterator p,b,found;
  p = found = model->end();
  b = model->begin();
  TMatrix2D stack;

  TCoord inside = 0.4 * fuzziness * TFigure::RANGE;

  bool stop = false;
  while(p!=b && !stop) {
    --p;
    if (*p!=gadget) {
      TCoord x, y;
      x = mx;
      y = my;
//cerr << "  after rotation ("<<x<<", "<<y<<")\n";
      TCoord d = (*p)->_distance(this, x, y);
      if (d==TFigure::INSIDE) {
        d = inside;
        stop = true;
      }
//cerr << "  distance = " << d << endl;
      stack.identity(); // why is this instruction here?
      if (d<distance) {
        distance = d;
        found = p;
      }
    }
  }

  if (found == model->end())
    return NULL;

//  if (distance > TFigure::RANGE)
  if (distance > 0.5*fuzziness*TFigure::RANGE) {
    return NULL;
  }
  return *found;
}

void
TFigureEditor::adjustPane()
{
  if (!window)
    return;
  if (!use_scrollbars) {
    visible = pane;
    return;
  }
  visible.set(0,0,window->getWidth(), window->getHeight());
  if (row_header_renderer) {
    visible.x = row_header_renderer->getSize();
    visible.w -= visible.x;
  }
  if (col_header_renderer) {
    visible.y = col_header_renderer->getSize();
    visible.h -= visible.y;
  }
}

void
TFigureEditor::minimalAreaSize(TCoord *x1, TCoord *y1, TCoord *x2, TCoord *y2)
{
  *x1 = *y1 = INT_MAX;
  *x2 = *y2 = INT_MIN;
}

void
TFigureEditor::updateScrollbars()
{
  if (!window || !use_scrollbars)
    return;
DBM(cout << __PRETTY_FUNCTION__ << ": entry" << endl;)

  // determine area size
  //-----------------------------------------------------------------
  TCoord x1, y1; // upper, left corner
  TCoord x2, y2; // lower, right corner
  
  minimalAreaSize(&x1, &y1, &x2, &y2);

  if (model) {
    TRectangle r;
    for(TFigureModel::iterator p = model->begin();
        p != model->end();
        ++p)
    {
      TCoord ax1, ay1, ax2, ay2;
      r=(*p)->editBounds();
      ax1=r.x;
      ay1=r.y;
      ax2=r.x+r.w-1;
      ay2=r.y+r.h-1;

      if (ax1<x1)
        x1=ax1;  
      if (ax2>x2)
        x2=ax2;  
      if (ay1<y1)
        y1=ay1;  
      if (ay2>y2)
        y2=ay2;  
    }
  }
  
  if (x1>0) x1=0;
  if (y1>0) y1=0;
  if (x2<0) x2=0;
  if (y2<0) y2=0;

//cout << "area size: (" << x1 << ", " << y1 << ") - (" << x2 << ", " << y2 << ")\n";
//cout << "final" << endl << endl;
  if (!mat) {
    pane.x = x1;
    pane.y = y1;
    pane.w = x2-x1+1;
    pane.h = y2-y1+1;
  } else {
    TCoord dx1, dy1, dx2, dy2;
    mat->map(x1, y1, &dx1, &dy1);
    mat->map(x2+1, y2+1, &dx2, &dy2);
    pane.x = dx1;
    pane.y = dy1;
    pane.w = dx2-dx1;
    pane.h = dy2-dy1;
  }
  doLayout();
DBM(cout << __PRETTY_FUNCTION__ << ": exit" << endl << endl;)
}

void
TFigureEditor::scrolled(TCoord dx, TCoord dy)
{
  quickready = false;
  TCoord x, y;
  getPanePos(&x, &y);
  // window->scrollTo(-x, -y);
  window->setOrigin(-x, -y);
}
