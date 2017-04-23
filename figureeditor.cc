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

TFigureAttributeModel::TFigureAttributeModel()
{
  linecolor.set(0,0,0);
  fillcolor.set(1,1,1);
  alpha.setRangeProperties(255,0,0,255);
//  connect(alpha.sigChanged, foobar, this, ALPHA);
  CLOSURE1(alpha.sigChanged, TFigureAttributeModel *dst, this,
    dst->reason.alpha = true;
    dst->sigChanged();
  );
  outline = true;
  filled = false;
  fontname = "arial,helvetica,sans-serif:size=12";

  drawgrid = true;
//  connect(drawgrid.sigChanged, foobar, this, GRID);
  CLOSURE1(drawgrid.sigChanged, TFigureAttributeModel *dst, this,
    dst->reason.grid = true;
    dst->sigChanged();
  );
  gridsize = 4;
//  connect(gridsize.sigChanged, foobar, this, GRID);
  CLOSURE1(gridsize.sigChanged, TFigureAttributeModel *dst, this,
    dst->reason.grid = true;
    dst->sigChanged();
  );
  
  linewidth = 0;
  linestyle = TPen::SOLID;
  arrowmode = TFLine::NONE;
  arrowtype = TFLine::EMPTY;
  
  current = 0;
}

TFigureAttributeModel::~TFigureAttributeModel()
{
}

void
TFigureAttributeModel::group()
{
  if (current) current->group();
}

void
TFigureAttributeModel::ungroup()
{
  if (current) current->ungroup();
}

void
TFigureAttributeModel::selectionDown()
{
  if (current) current->selectionDown();
}

void
TFigureAttributeModel::selection2Bottom()
{
  if (current) current->selection2Bottom();
}

void
TFigureAttributeModel::selectionUp()
{
  if (current) current->selectionUp();
}

void
TFigureAttributeModel::selection2Top()
{
  if (current) current->selection2Top();
}

void
TFigureAttributeModel::selectionAlignVertical()
{
  if (current) current->selectionAlignVertical();
}

void
TFigureAttributeModel::selectionAlignHorizontal()
{
  if (current) current->selectionAlignHorizontal();
}

void
TFigureAttributeModel::applyAll()
{
  // if (current) current->applyAll();
}

/*
 *
 * figure relation handling
 *
 */

std::map<const TFigure*, std::set<const TFigure*>> TFigureEditor::relatedTo;

struct TRelationToBeRestored
{
  TRelationToBeRestored(const TFigure **inFrom, const TFigure *inTo): from(inFrom), to(inTo) {}
  const TFigure **from;
  const TFigure *to;
};
std::vector<TRelationToBeRestored> restoreRelations;

/**
 * \param from the pointer containing the figure being pointed to
 * \param to   the figure containing the figure containg the 'from' pointer
 */
void
TFigureEditor::restoreRelation(const TFigure **from, const TFigure *to)
{
  ::restoreRelations.push_back(TRelationToBeRestored(from, to));
}

void
TFigureEditor::restoreRelations()
{
  std::set<const TFigure*> modified;

  for(auto &p: ::restoreRelations) {
    cout << "TFigurEditor::restoreRelations " << p.from << " -> " << *p.from << ", " << p.to << endl;
    TFigureEditor::relatedTo[*p.from].insert(p.to);
    modified.insert(p.to);
  }
  
  TFigureEditEvent ee;
  ee.model = nullptr;
  ee.type = TFigureEditEvent::RELATION_MODIFIED;
  
  for(auto &p: modified) {
    const_cast<TFigure*>(p)->editEvent(ee);
  }
  ::restoreRelations.clear();
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
  toolbox = nullptr;
  fuzziness = 3;

  handle = -1;
  gadget = NULL;
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
  setAttributes(new TFigureAttributeModel);
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
  
  action = new TAction(this, "object|align|left");
  action = new TAction(this, "object|align|horizontal");
  CONNECT(action->sigClicked, this, selectionAlignHorizontal);
  action = new TAction(this, "object|align|right");
  action = new TAction(this, "object|align|top");
  action = new TAction(this, "object|align|vertical");
  CONNECT(action->sigClicked, this, selectionAlignVertical);
  action = new TAction(this, "object|align|bottom");

  action = new TAction(this, "object|distribute|horizontal");
  action = new TAction(this, "object|distribute|vertical");
  action = new TAction(this, "object|distribute|even");

  action = new TAction(this, "object|lock");
  action = new TAction(this, "object|unlock");

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

  // ::restoreObject(in, &s);
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
void TFigureEditor::translate(const TPoint &vector)
{
  if (!mat)
    mat = new TMatrix2D();
  mat->translate(vector);
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
  pen.fillRectangle(0, 0,window->getWidth(),window->getHeight());

  pen.push();
  if (mat)
    pen.multiply(mat);
  paintGrid(pen);
  pen.pop();

  pen.translate(getOrigin());
  if (mat)
    pen.multiply(mat);

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
return;
  if (!preferences ||
      !preferences->drawgrid ||
      preferences->gridsize <= 0.0)
  {
    return;
  }

  const TRGB &background_color = window->getBackground();
  pen.setColor(
    background_color.r > 0.5 ? background_color.r-0.5 : background_color.r+0.5,
    background_color.g > 0.5 ? background_color.g-0.5 : background_color.g+0.5,
    background_color.b > 0.5 ? background_color.b-0.5 : background_color.b+0.5
  );

  TPoint origin = window->getOrigin();
  
  origin.x += visible.x;
  origin.y += visible.y;

  TRegion region(*window->getUpdateRegion());
  TRectangle r;
//  region.getBoundary(&r);
//cout << "paintGrid: update region's boundary: " << r << endl;
//cout << "paintGrid: visible                 : " << visible << endl;
  region &= TRectangle(visible.x, visible.y, visible.w, visible.h);
  region.getBoundary(&r);
//cout << "paintGrid: intersection            : " << r << endl;
//cout << "paintGrid: origin                  : " << origin << endl;

  TCoord gridsize = preferences->gridsize;
  if (mat) {
    TMatrix2D m(*mat);
    m.invert();
    TPoint p0 = m.map(TPoint(r.x, r.y));
    TPoint p1 = m.map(TPoint(r.x+r.w, r.y+r.h));
    r.x = p0.x;
    r.y = p0.y;
    r.w = p1.x - p0.x;
    r.h = p1.y - p0.y;
    origin = m.map(origin);
  }

  TCoord x0, x1, y0, y1;

  double f;
  // Y
  // origin correction
  f = fmod(origin.y, gridsize);
  if (f<0)
    f+=gridsize;

  // area jump FIXME: replace loop
  while(f-gridsize-r.y>=0)
    f -= gridsize;
  while(f-gridsize-r.y<-gridsize)
    f += gridsize;
  y0 = f;

  // X
  // origin correction
  f = fmod(origin.x, gridsize);
  if (f<0)
    f+=gridsize;

  // area jump // FIXME: replace loop
  while(f-gridsize-r.x>=0)
    f -= gridsize;
  while(f-gridsize-r.x<-gridsize)
    f += gridsize;
  x0 = f;
  
  x1 = x0 + r.w;
  y1 = y0 + r.h;
  
//  cout << "paintGrid between x = " << x0 << " ... " << x1 << ", y = " << y0 << " ... " << y1 << ", boundary=" << r << endl;

  for(auto y=y0; y<=y1; y+=gridsize) {
    for(auto x=x0; x<=x1; x+=gridsize) {
      pen.drawPoint(x, y);
//cout << "drawGrid: draw point at " << x << ", " << y << endl;
    }
  }
}  

/**
 * Called from 'paint' to draw the selection marks.
 */
void
TFigureEditor::paintSelection(TPenBase &pen)
{
  if (!toolbox)
    return;
  TFigureTool *tool = toolbox->getTool();
  if (!tool)
    return;
  tool->paintSelection(this, pen);
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
  
  TPoint origin = window->getOrigin();
  
  if (row_header_renderer) {
    scr.push();
    scr.identity();
    scr.setClipRect(TRectangle(0, visible.y, visible.x, visible.h));
    scr.translate(0, visible.y+origin.y);
    row_header_renderer->render(scr, -origin.y, visible.h, mat);
    scr.pop();
  }

  if (col_header_renderer) {
    scr.push();
    scr.identity();
    scr.setClipRect(TRectangle(visible.x, 0, visible.w, visible.y));
    scr.translate(visible.x+origin.x, 0);
    col_header_renderer->render(scr, -origin.x, visible.w, mat);
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

/*
  if (mat) {
    TMatrix2D m(*mat);
    m.invert();
    TPoint p0 = m.map(TPoint(cb.x, cb.y));
    TPoint p1 = m.map(TPoint(cb.x+cb.w, cb.y+cb.h));
    cb.x = p0.x;
    cb.y = p0.y;
    cb.w = p1.x - p0.x;
    cb.h = p1.y - p0.y;
    origin = m.map(origin);
  }
cout << "TFigureEditor::print(): clipbox="<<cb<<endl;
*/

  for(TFigureModel::iterator p = model->begin();
      p != model->end();
      ++p)
  {
/*
    TRectangle r;
    getFigureShape(*p, &r, pen.getMatrix());
    if (!r.intersects(cb)) {
cout << "  shape " << r << " doesn't intersect with clipbox" << endl;
      continue;
    }
*/
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
TFigureEditor::setAttributes(TFigureAttributeModel *p) {
  if (preferences) {
    disconnect(preferences->sigChanged, this);
    if (preferences->getCurrent() == this)
      preferences->setCurrent(0);
  }
  preferences = p;
  if (preferences) {
    preferences->setCurrent(this);
    connect(preferences->sigChanged, this, &TThis::preferencesChanged);
  }
}

void
TFigureEditor::preferencesChanged()
{
  if (!preferences)
    return;
  quickready=false;

//  if (preferences->reason.grid) {
    invalidateWindow(visible);
//  }
  
//  if (preferences->reason == TFigureAttributeModel::ALL) {
//    setTool(preferences->getTool());
//    invalidateWindow(visible);
//  }

  model->setAttributes(selection, preferences);
//  invalidateWindow(visible); 

  preferences->clearReasons();
}

void
TFigureEditor::setStrokeColor(const TRGB &rgb)
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
TFigureEditor::selectionAlignHorizontal()
{
  vector<TRectangle> bounds;
  TCoord left(numeric_limits<TCoord>::max()),
         right(numeric_limits<TCoord>::min());
  for(auto &&f: selection) {
    bounds.push_back(f->bounds());
    TRectangle &b = bounds.back();
    if (left>b.x)
      left=b.x;
    if (right<b.x+b.w)
      right=b.x+b.w;
  }
  TCoord bx = left+(right-left)/2.0;
  auto p = bounds.begin();
  for(auto &&f: selection) {
    TCoord fx = p->x + p->w/2.0;
    TFigureSet set;
    set.insert(f);
    model->translate(set, bx-fx,0);
    ++p;
  }
  window->invalidateWindow();
}

void
TFigureEditor::selectionAlignVertical()
{
  vector<TRectangle> bounds;
  TCoord top(numeric_limits<TCoord>::max()),
         bottom(numeric_limits<TCoord>::min());
  for(auto &&f: selection) {
    bounds.push_back(f->bounds());
    TRectangle &b = bounds.back();
    if (top>b.y)
      top=b.y;
    if (bottom<b.y+b.h)
      bottom=b.y+b.h;
  }
  TCoord by = top+(bottom-top)/2.0;
  auto p = bounds.begin();
  for(auto &&f: selection) {
    TCoord fy = p->y + p->h/2.0;
    TFigureSet set;
    set.insert(f);
    model->translate(set, 0, by-fy);
    ++p;
  }
  window->invalidateWindow();
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

TFigureTool*
TFigureEditor::getTool() const
{
  TFigureTool *tool = nullptr;
  if (!toolbox)
    return tool;
  tool = toolbox->getTool();
  return tool;
}

/**
 * TFigureTool operates in two modes. Passive and active.
 *
 * In passive mode the model or view won't be modified in a way which may
 * require a rollback in case the operation is aborted. TFigureTool uses this
 * to show different cursors depding on which type of figure is benath the cursor.
 *
 * In active mode, initiated by calling start(), TFigureTool may modify the
 * view, model, etc.. One the operation is finished or aborted, stop() will be
 * called.
 *
 * stop() may be called by TFigureTool itself or by the figureeditor when
 * @li start() is invoked again
 * @li the figure editor's model changes
 * @li the figure editor is closed
 *
 * 
 */

TFigureEditor* TFigureEditor::activeEditor = nullptr;
TFigureTool*   TFigureEditor::activeTool   = nullptr;
TFigureModel*  TFigureEditor::activeModel  = nullptr;

void
TFigureEditor::start()
{
  if (activeEditor) {
    activeEditor->stop();
  }
  activeEditor = this;
  activeTool  = getTool();
  activeModel = getModel();
}

void
TFigureEditor::stop()
{
cout << "TFigureEditor::stop(): current tool is " << activeTool << endl;
  if (activeTool)
    activeTool->stop(this);
  activeEditor = nullptr;
  activeTool   = nullptr;
  activeModel  = nullptr;
}

void
TFigureEditor::setToolBox(TToolBox *toolbox)
{
  if (this->toolbox) {
    disconnect(this->toolbox->sigChanged, this);
  }

  this->toolbox = toolbox;

  if (this->toolbox) {
    // switching to a new tool stops the active mode
    connect(this->toolbox->sigChanged, this, [=] {
cout << "toolbox.sigChanged -> TFigureEditor::stop()" << endl;
      this->stop();
    });
  }
}

void
TFigureEditor::setModel(TFigureModel *m)
{
  if (model==m)
    return;
  if (model) {
    // switching to a new model stops the active mode
    stop();
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
  if (!toolbox)
    return;
  TFigureTool *tool = toolbox->getTool();
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
//cerr << "TFigureEditor::mouseEvent " << me.name() << endl;
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
        if (!toolbox) {
          super::mouseEvent(me);
          return;
        }
      }
      break;
    default:
      ;
  }
  
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

  if (!toolbox)
    return;
  TFigureTool *tool = toolbox->getTool();
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
TFigureEditor::invalidateFigure(const TFigure* figure)
{
  if (!window)
    return;
  TRectangle r;
//figure->getShape(&r);
//cout << figure->getClassName() << ": invalidate shape " <<r.x<<"-"<<(r.x+r.w)<<","<<r.y<<"-"<<(r.y+r.h)<<endl;
  TPoint origin = window->getOrigin();
  
  std::set<const TFigure*>::const_iterator ptr;
  
  ssize_t n = 0;
  auto relationList = relatedTo.find(figure);
  if (relationList != relatedTo.end()) {
    ptr = relationList->second.begin();
    n = relationList->second.size();
  }

  while(true) {
    getFigureEditShape(figure, &r, mat);
    r.x += origin.x + visible.x;
    r.y += origin.y + visible.y;
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
    if (n==0)
      break;
    --n;
    figure = *ptr;
    ++ptr;
  }
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
TFigureEditor::getFigureShape(const TFigure* figure, TRectangle *r, const TMatrix2D *mat)
{
  *r = figure->bounds();
  _getFigureShape(figure, r, mat);
}

void
TFigureEditor::getFigureEditShape(const TFigure* figure, TRectangle *r, const TMatrix2D *mat)
{
  *r = figure->editBounds();
  _getFigureShape(figure, r, mat);
}

void
TFigureEditor::_getFigureShape(const TFigure* figure, TRectangle *r, const TMatrix2D *mat)
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
  window->setOrigin(TPoint(-x, -y));
}
