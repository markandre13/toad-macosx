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

#include <toad/pen.hh>
#include <toad/window.hh>
#include <toad/dialog.hh>
#include <toad/layouteditor.hh>
#include <toad/dialogeditor.hh>
#include <toad/control.hh>
#include <toad/figure.hh>
#include <toad/labelowner.hh>
#include <toad/io/urlstream.hh>
#include <toad/focusmanager.hh>
#include <toad/undomanager.hh>

#include <map>

#include <toad/scrollbar.hh>
#include <toad/pushbutton.hh>
#include <toad/fatradiobutton.hh>
#include <toad/textfield.hh>
#include <toad/figureeditor.hh>
#include <toad/colorselector.hh>

#include <toad/figure/nodetool.hh>

namespace toad {

class TLayoutEditDialog:
  public TLayoutEditor
{
  public:
    TLayoutEditDialog(TWindow*, const string&, TDialogLayout*, TWindow *forWindow);
    ~TLayoutEditDialog();
    
    class TMyMouseFilter:
      public TEventFilter
    {
      public:
        TMyMouseFilter(TLayoutEditDialog *parent) {
          this->parent = parent;
        }
        bool mouseEvent(TMouseEvent &me) {
          parent->gedit.mouseEvent(me);
          return true;
        }
        bool keyEvent(TKeyEvent &ke) {
          parent->gedit.keyEvent(ke);
          return true;
        }
        
        TLayoutEditDialog *parent;
    };
    TMyMouseFilter mymousefilter;
    
    TEventFilter * getFilter();
    
    TDialogLayout * layout;
    TWindow * forWindow;
    TFigureEditor gedit;
    
    void paint();

    string selectionname;
    TTextModel label;
    TIntegerModel width, height;
    TLabelOwner * labelowner;
    
    void enabled();
    void selectionChanged();
    void labelChanged();
    void sizeChanged();
};

class TDialogSelectionTool:
  public TFigureTool
{
    TFigure *figure;
    int handle;
    bool tht; // translate handle transform?
    
    enum {
      STATE_NONE,
      STATE_SELECT_RECT,
      STATE_MOVE,
      STATE_MOVE_HANDLE
    } state;
    
    TPoint down, select, memo;
    bool mouseMoved;
    
  public:
    TDialogSelectionTool();
    static TDialogSelectionTool* getTool();
    void mouseEvent(TFigureEditor *fe, const TMouseEvent &me);
    bool paintSelection(TFigureEditor*, TPenBase &pen);
    void stop(TFigureEditor *fe);
  private:
    bool handleHandles(TFigureEditor *fe, const TMouseEvent &me);
};

} // namespace

using namespace toad;

TDialogSelectionTool::TDialogSelectionTool()
{
  state = STATE_NONE;
}

TDialogSelectionTool*
TDialogSelectionTool::getTool()
{
  static TDialogSelectionTool* tool = 0;
  if (!tool)
    tool = new TDialogSelectionTool();
  return tool;
}

void
TDialogSelectionTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint m, pos;
  fe->mouse2sheet(me.pos, &m);
  m.x = round(m.x);
  m.y = round(m.y);
  fe->sheet2grid(m, &pos);
  switch(me.type) {
    case TMouseEvent::LDOWN: {
      down = pos;

      if (!fe->selection.empty() &&
          !me.dblClick &&
      	  handleHandles(fe, me))
        return;
      
      TFigure *f = fe->findFigureAt(m);
      if (f) {
	TFigureSet::iterator fi = fe->selection.find(f);
        if (me.dblClick) {
        } else
        if (me.modifier() & MK_CONTROL) {
        } else
        if (me.modifier() & MK_SHIFT) {
	  state = STATE_SELECT_RECT;
	  select = m;
        } else {
          memo = { 0, 0 };
          state = STATE_MOVE;
          TUndoManager::beginUndoGrouping();
        }
        if (fi == fe->selection.end()) {
          if (!(me.modifier() & MK_SHIFT))
            fe->clearSelection();
	  fe->selection.insert(f);
	  fe->invalidateFigure(f);
	}
      } else {
	if (!(me.modifier() & MK_SHIFT)) {
	  fe->clearSelection();
	}
	state = STATE_SELECT_RECT;
	select = m;
      }
    } break;
    case TMouseEvent::MOVE: {
      switch(state) {
        case STATE_NONE:
          break;
        case STATE_SELECT_RECT:
          if (select != pos) {
	    select = m;
	    fe->invalidateWindow();
          }
          break;
        case STATE_MOVE: {
          TPoint delta = pos-down;
          down = pos;
          memo += delta;
          fe->getModel()->translate(&fe->selection, delta);
/*        
          TCoord dx = x-down_x; down_x=x;
          TCoord dy = y-down_y; down_y=y;
          memo_x+=dx;
          memo_y+=dy;
          fe->getModel()->translate(fe->selection, dx, dy);
*/
         } break;
      	case STATE_MOVE_HANDLE:
      	  handleHandles(fe, me);
      	  break;
      }
    } break;
    case TMouseEvent::LUP: {
      switch(state) {
      	case STATE_NONE:
      	  break;
      	case STATE_SELECT_RECT: {
      	  state = STATE_NONE;
      	  fe->invalidateWindow();
      	  TRectangle r1(down, pos);
      	  TRectangle r2;
      	  for(auto p=fe->getModel()->begin(); p!=fe->getModel()->end(); ++p) {
      	    r2 = (*p)->bounds();
      	    if (r1.isInside(r2)) {
      	      fe->selection.insert(*p);
      	    }
      	  }
      	} break;
      	case STATE_MOVE:
          TUndoManager::endUndoGrouping();
          state = STATE_NONE;
      	  break;
      	case STATE_MOVE_HANDLE:
      	  handleHandles(fe, me);
      	  break;
      }
    }
  }
}

bool
TDialogSelectionTool::handleHandles(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint pos(round(me.pos.x), round(me.pos.y));
  switch(me.type) {
    case TMouseEvent::LDOWN:
      for(auto p=fe->selection.begin();
          p!=fe->selection.end();
          ++p)
      {
        // loop over all handles
        unsigned h = 0;
        TPoint memo;
        while((*p)->getHandle(h, &memo)) {
cout << "  is handle " << h << " at " << memo << " near " << pos << endl;
          if (memo.x-fe->fuzziness<=pos.x && pos.x<=memo.x+fe->fuzziness && 
              memo.y-fe->fuzziness<=pos.y && pos.y<=memo.y+fe->fuzziness)
          {
            cout << "      found handle " << h << " at cursor => STATE_MOVE_HANDLE" << endl;
            handle = h;
            figure = *p;
            state = STATE_MOVE_HANDLE;
            tht = figure->startTranslateHandle();
            mouseMoved = false;
            if (fe->selection.size()>1) {
              fe->clearSelection();
              fe->selection.insert(figure);
              fe->sigSelectionChanged();
            }
            fe->invalidateFigure(figure);
            return true;
          }
          ++h;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (!mouseMoved) {
        if (down!=pos) {
          mouseMoved = true;
          TUndoManager::beginUndoGrouping();
        }
      }
      if (mouseMoved) {
        fe->getModel()->translateHandle(figure, handle, pos, me.modifier());
      }
      break;
    case TMouseEvent::LUP:
      if (mouseMoved) {
        fe->getModel()->translateHandle(figure, handle, pos, me.modifier());
        TUndoManager::endUndoGrouping();
      }
      state = STATE_NONE;
      figure->endTranslateHandle();
      fe->invalidateFigure(figure);
      figure = 0;
      handle = -1;
      break;
  }
  return false;
}

bool
TDialogSelectionTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  // draw the selection marks over all figures
  for_each(fe->selection.begin(), fe->selection.end(), [&] (TFigure *f) {
    pen.setLineWidth(1);
    if (f!=figure) {
      f->paintSelection(pen, -1);
    } else {
      f->paintSelection(pen, handle);
    }
  });

  if (state==STATE_SELECT_RECT) {
    pen.push();
    pen.setColor(0,0,0);
    pen.setLineStyle(TPen::DOT);
    pen.setLineWidth(1);
    pen.drawRectanglePC(down.x+0.5, down.y+0.5, select.x-down.x, select.y-down.y);
    pen.pop();
  }

  return true;
}

void
TDialogSelectionTool::stop(TFigureEditor *fe)
{
/*
  fe->getWindow()->setAllMouseMoveEvents(false);
  fe->getWindow()->setCursor(0);
  fe->state = TFigureEditor::STATE_NONE;
  fe->quick = false;
  if (figure) {
    fe->invalidateFigure(figure);
    figure = 0;
  }
*/
}

/**
 * \class toad::TDialog
 * TDialog can be used as a parent for dialog windows. It has some special
 * methods to control the children.
 */

TDialog::TDialog(TWindow* parent, const string &title):
  super(parent, title)
{
  setBackground(TColor::DIALOG);
  flagShell = flagStaticFrame = true;
  flagFocusManager = true;
  flagDrawFocus = false;
  if (getParent())
    placeWindow(PLACE_PARENT_CENTER, getParent());
  setLayout(new TDialogLayout());
}

TDialog::~TDialog()
{
}

void
TDialog::destroy()
{
}

/**
 * When bDrawFocus is true, this method paints a rounded frame around
 * the active control.
 */
void
TDialog::paint()
{
  // move this into TDialogLayout:
  if (flagDrawFocus) {
    TWindow *wnd = TFocusManager::getFocusWindow();
    if (wnd && wnd->getParent()==this) {
      TPen pen(this);
      TRectangle r;
      wnd->getShape(&r);
      pen.drawRectanglePC(r.origin.x-3,r.origin.y-2,r.size.width+6,r.size.height+4);
      pen.drawRectanglePC(r.origin.x-2,r.origin.y-3,r.size.width+4,r.size.height+6);
    }
  }
}

void 
TDialog::childNotify(TWindow *who, EChildNotify type)
{
  cerr << __FUNCTION__ << " isn't implemented yet" << endl;
#if 0
  if (type==TCHILD_FOCUS && bDrawFocus) {
    TRectangle r;
    who->getShape(&r);
    r.x-=3; r.w+=6;
    r.y-=3; r.h+=6;
    invalidateWindow(r);
  }
#endif
}

void
TDialog::adjust()
{
}

//---------------------------------------------------------------------

namespace toad {

bool
restore(TInObjectStream &p, const char *name, PFigureModel &value)
{
  if (p.what != ATV_GROUP)
    return false;
  if (name) {
    if (p.attribute != name)
      return false;
  } else {
    if (!p.attribute.empty())
      return false;
  }
//  if (p.type.empty() || ... )
//    return false;

  TSerializable *s = p.clone(p.type);
  
  if (!s) {
    return false;
  }

  value = dynamic_cast<TFigureModel*>(s);
  if (!value) {
    p.err << "type mismatch is object store";
    delete s;
    return false;
  }
  p.setInterpreter(value);
  return true;
}

} // namespace toad

typedef map<string, TWindow*> TTitleWindowMap;

/**
 * @sa arrange
 */
static void 
arrangeHelper(TFigureModel::iterator p, 
              TFigureModel::iterator e,
              TTitleWindowMap &wmap)
{
  while(p!=e) {
    TFWindow *gw = dynamic_cast<TFWindow*>(*p);
    if (gw) {
      TTitleWindowMap::iterator pm = wmap.find(gw->title);
      if (pm!=wmap.end()) {
        gw->window = (*pm).second;
        TRectangle r;
        r = gw->bounds();
        gw->window->setShape(r.origin.x,r.origin.y,r.size.width,r.size.height);
        TLabelOwner *lo = dynamic_cast<TLabelOwner*>(gw->window);
        if (lo) {
          if (gw->label.size()!=0)
            lo->setLabel(gw->label);
          gw->label.erase(); // ???
        }
        gw->window->taborder = gw->taborder;
        wmap.erase(pm);
      }
    }
    TFGroup *gg = dynamic_cast<TFGroup*>(*p);
    if (gg) {
      arrangeHelper(gg->gadgets.begin(), gg->gadgets.end(), wmap);
    }
    ++p;
  }
}

TDialogLayout::TDialogLayout()
{
  editor = NULL;
  width = 0;
  height = 0;
  drawfocus = false;
  sizeSet = false;
  gadgets = new TFigureModel();
}

TDialogLayout::~TDialogLayout()
{
  if (editor) {
    delete editor;
  }
}

void
TDialogLayout::arrange()
{
  if (!sizeSet && width && height) {
    window->setSize(width, height);
    sizeSet = true; // hack for Cocoa
  }

  // create a map of all child windows
  //-----------------------------------
  TTitleWindowMap wmap;
  TInteractor *ip = window->getFirstChild();
  while(ip) {
    TWindow *wnd = dynamic_cast<TWindow*>(ip);
    if (wnd) {
      if (wmap.find(wnd->getTitle())!=wmap.end()) {
        cerr << "  child title \"" << wnd->getTitle() << "\" isn't unique" << endl;
      } else {
        wmap[wnd->getTitle()] = wnd;
      }
    }
    ip = TInteractor::getNextSibling(ip);
  }

  // arrange all child windows with the information store 
  // in their related TFWindow's
  //-----------------------------------------------------
  arrangeHelper(gadgets->begin(), gadgets->end(), wmap);

  // add TFWindow's for the remaining windows
  //-----------------------------------------------------
  TRectangle noshape(4,4,33,33);
  TTitleWindowMap::iterator p,e;
  p = wmap.begin();
  e = wmap.end();
  while(p!=e) {
    TFWindow *gw = new TFWindow();
    gw->title  = (*p).second->getTitle();
    gw->window = (*p).second;
    gw->setShape(noshape);
    gw->window->setShape(noshape);
    gadgets->add(gw);
    noshape.origin.x+=36;
    if (noshape.origin.x>window->getWidth()) {
      noshape.origin.x=4;
      noshape.origin.y+=36;
    }
    ++p;
  }
}

void
TDialogLayout::paint()
{
  if (!editor || !editor->isEnabled()) {
    TPen pen(window);
    pen.translate(0.5, 0.5);
    TFigureModel::iterator p = gadgets->begin();
    while(p!=gadgets->end()) {
      (*p)->paint(pen, TFigure::NORMAL);
      ++p;
    }
  } else {
    editor->gedit.paint();
  }
}

void
TDialogLayout::store(TOutObjectStream &out) const
{
  ::store(out, "width", width);
  ::store(out, "height", height);
  ::store(out, "drawfocus", drawfocus);
  ::store(out, gadgets);
}

bool
TDialogLayout::restore(TInObjectStream &in)
{
  TFigureModel *m;
#warning "kludge"
  if (::restoreObject(in, &m)) {
    gadgets = m;
    return true;
  }
  if (
    ::restore(in, "width", &width) ||
    ::restore(in, "height", &height) ||
    ::restore(in, "drawfocus", &drawfocus) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in);
  return false;
}

TLayoutEditor *
TDialogLayout::createEditor(TWindow *inWindow, TWindow *forWindow)
{
//  return new TLayoutEditDialog(inWindow, "TDialogLayout.editor", this, forWindow);
  return nullptr;
}

#if 0
TLayoutEditDialog::TLayoutEditDialog(TWindow *parent,
                                     const string &title,
                                     TDialogLayout *layout,
                                     TWindow *forWindow)
  :TLayoutEditor(parent, title), mymousefilter(this)
{
  setBackground(TColor::DIALOG);
  gedit.setWindow(forWindow);
  gedit.setModel(layout->gadgets);
  connect(gedit.sigSelectionChanged, this, &TLayoutEditDialog::selectionChanged);
  this->forWindow = forWindow;

  // let the layout redirect it's paint event to our gadget editor...
  layout->editor = this;
  // ... and store a reference to the layout so that we can remove ourself
  // from it later
  this->layout = layout;

  labelowner = 0;
  
  int x, y, w, h, hmax;
  TPushButton *pb;
  TRadioStateModel *state = new TRadioStateModel();
  TFatRadioButton *rb = NULL;
  
  x=5; y=5; w=64; h=25; hmax=0;

  static TShapeTool gframe(new TFFrame);
  static TShapeTool grect(new TFRectangle);
  static TShapeTool gcirc(new TFCircle);
  static TShapeTool gtext(new TFText);
  static TShapeTool gline(new TFLine);
    
  for(unsigned i=0; i<=5; ++i) {
    rb = NULL;
    switch(i) {
      case 0:
        rb = new TFatRadioButton(this, "Select", state);
//        CONNECT(rb->sigClicked, &gedit, setOperation, TFigureEditor::OP_SELECT);
        CONNECT(rb->sigClicked, &gedit, setTool, TDialogSelectionTool::getTool());
//        CONNECT(rb->sigClicked, &gedit, setTool, TNodeTool::getTool());
        rb->setDown(true);
	rb->sigClicked(); // FIXME: calling setDown should implicitly do that
        break;
      case 1:
        rb = new TFatRadioButton(this, "Frame", state);
        CONNECT(rb->sigClicked, &gedit, setTool, &gframe);
        break;
      case 2:
        rb = new TFatRadioButton(this, "Line", state);
        CONNECT(rb->sigClicked, &gedit, setTool, &gline);
        break;
      case 3:
        rb = new TFatRadioButton(this, "Rect", state);
        CONNECT(rb->sigClicked, &gedit, setTool, &grect);
        break;
      case 4:
        rb = new TFatRadioButton(this, "Circle", state);
        CONNECT(rb->sigClicked, &gedit, setTool, &gcirc);
        break;
      case 5:
        rb = new TFatRadioButton(this, "Text", state);
        CONNECT(rb->sigClicked, &gedit, setTool, &gtext);
        break;
    }
    if (rb) {
      rb->setShape(x,y,w,h);
      y+=h-1;
    }
  }
  hmax=max(hmax, y);

  x += w+5;
  y = 5;
  for(unsigned i=0; i<=5; i++) {
    switch(i) {
      case 0:
        pb = new TPushButton(this, "Top");
        CONNECT(pb->sigClicked, &gedit, selection2Top);
        break;
      case 1:
        pb = new TPushButton(this, "Up");
        CONNECT(pb->sigClicked, &gedit, selectionUp);
        break;
      case 2:
        pb = new TPushButton(this, "Down");
        CONNECT(pb->sigClicked, &gedit, selectionDown);
        break;
      case 3:
        pb = new TPushButton(this, "Bottom");
        CONNECT(pb->sigClicked, &gedit, selection2Bottom);
        break;
      case 4:
        pb = new TPushButton(this, "Group");   
        CONNECT(pb->sigClicked, &gedit, group);
        break;
      case 5:
        pb = new TPushButton(this, "Ungroup");   
        CONNECT(pb->sigClicked, &gedit, ungroup);
        break;
    }
    if (pb) {
      pb->setShape(x,y,w,h);
      y+=h-1;
    }
  }
  
  hmax=max(hmax, y);
  
  x += w+5;
  y = 5;
  
  TColorSelector *cs = new TColorSelector(this, 
                                          "colorselector", 
                                          gedit.getAttributes());
  cs->dialogeditorhack = true;
  cs->setShape(x,y,64,32);
  
  y=80;
  
  TTextField *tf;
  
  tf = new TTextField(this, "label", &label);
  tf->setShape(x+40, y, 145,18);
  connect(label.sigChanged, this, &TLayoutEditDialog::labelChanged);
  
  y+=30;
  tf = new TTextField(this, "width", &width);
  tf->setShape(x+40, y, 40,18);
  tf = new TTextField(this, "height", &height);
  tf->setShape(x+40+40+10, y, 40,18);
  
  width = layout->width;
  height = layout->height;
  
  connect(width.sigChanged, this, &TLayoutEditDialog::sizeChanged);
  connect(height.sigChanged, this, &TLayoutEditDialog::sizeChanged);

  setSize(5+320+5, hmax+5);
}

void
TLayoutEditDialog::paint()
{
  TPen pen(this);
  
  pen.drawString(128+15+40, 63, selectionname);
  pen.drawString(128+15, 63, "Title:");
  pen.drawString(128+15, 63+20, "Label:");
  pen.drawString(128+15, 63+50, "Size:");
  pen.drawString(128+15+82, 63+50, "x");
}

TLayoutEditDialog::~TLayoutEditDialog()
{
  layout->editor = 0;
}


TEventFilter *
TLayoutEditDialog::getFilter()
{
  return &mymousefilter;
}

void
TLayoutEditDialog::enabled()
{
  gedit.getWindow()->invalidateWindow();
}

void
TLayoutEditDialog::selectionChanged()
{
  selectionname = "(none)";
  labelowner = 0;
  if (gedit.selection.size()==1) {
    TFWindow *g = dynamic_cast<TFWindow*>(*gedit.selection.begin());
    if (g) {
      selectionname = g->title;
      if (g->window) {
        TLabelOwner *lo = dynamic_cast<TLabelOwner*>(g->window);
        if (lo)
          label = lo->getLabel();
        labelowner = lo;
      }
    }
  }
//  cerr << "new selectionname = '" << selectionname << "'\n";
//  cerr << "label             = '" << label.getValue() << "'\n";
  invalidateWindow(true);
}

void
TLayoutEditDialog::labelChanged()
{
//  cerr << "label changed to '" << label << "'\n";
  if (labelowner)
    labelowner->setLabel(label);
}

void
TLayoutEditDialog::sizeChanged()
{
  layout->width = width;
  layout->height = height;
  forWindow->setSize(width, height);
}
#endif
