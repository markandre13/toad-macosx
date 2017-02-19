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

#ifndef _TOAD_FIGUREEDITOR
#define _TOAD_FIGUREEDITOR 1

#include <toad/figure.hh>
#include <toad/figuremodel.hh>
#include <toad/scrollpane.hh>
#include <toad/undo.hh>

#include <toad/boolmodel.hh>
#include <toad/textmodel.hh>
#include <toad/integermodel.hh>
#include <toad/floatmodel.hh>

#include <toad/figure/createtool.hh>

namespace toad {

class TFigureTool;
class TFigureEditor;
class TScrollBar;

class TFigureEditorHeaderRenderer
{
  public:
    virtual void render(TPenBase &pen, int pos, int size, TMatrix2D *mat) = 0;
    virtual int getSize() = 0;
    virtual void mouseEvent(const TMouseEvent&);
};

class TFigureAttributes:
  public TModel
{
    TFigureEditor *current;
    TFigureTool *tool;
    TFigureAttributes(const TFigureAttributes&) {};
  public:
    TFigureAttributes();
    virtual ~TFigureAttributes();

    /**
     * ...
     */
    void setCurrent(TFigureEditor *current) {
      if (this->current == current)
        return;
      this->current = current;
      reason.current = true;
      sigChanged();
    }
    
    TFigureEditor* getCurrent() const {
      return current;
    }

    // These methods delegate to the current TFigureEditor.
    void setOperation(unsigned);
    // unsigned getOperation() const { return current->getOperation(); }
    void setCreate(TFigure *figure);
    void setTool(TFigureTool*);
    TFigureTool* getTool() const { return tool; }
    
    void group();
    void ungroup();
    void selectionDown();
    void selection2Bottom();
    void selectionUp();
    void selection2Top();
    void applyAll();
    
    // additional information on why sigChanged was triggered
    
    struct {
      bool current:1; // we are going to edit another object
      bool grid:1;

      bool linecolor:1;
      bool fillcolor:1;
      bool alpha:1;

      bool line:1;
      bool fill:1;

      bool linewidth:1;
      bool linestyle:1;
      bool arrowmode:1;
      bool arrowstyle:1;
      
      bool fontname:1;
      
      bool tool:1; // we might not want to place the tool here
    } reason;
    
    void setAllReasons(bool f=true) {
      reason.current =
      reason.grid =
      reason.linecolor =
      reason.fillcolor =
      reason.alpha =
      reason.line =
      reason.fill =
      reason.linewidth =
      reason.linestyle =
      reason.arrowmode =
      reason.arrowstyle =
      reason.fontname =
      reason.tool = f;
    }

    void clearReasons() {
      setAllReasons(false);
    }
    
    void setLineColor(const TRGB &rgb) { 
      linecolor = rgb;
      reason.linecolor = true;
      sigChanged();
    }
    void setFillColor(const TRGB &rgb) {
      fillcolor = rgb; 
      filled = true; 
      reason.fillcolor = true;
      reason.fill = true;
      sigChanged();
    }
    void unsetFillColor() { 
      filled = false; 
      reason.fill = true;
      sigChanged();
    }
    void setFont(const string &font) {
      this->font.setFont(font);
      fontname = font;
      reason.fontname = true;
      sigChanged();
    }
    const string& getFont() const {
      return fontname;
    }

    TBoolModel outline; // FIXME: rename me into line
    TBoolModel filled; // FIXME: rename me into fill
    TRGB linecolor;
    TRGB fillcolor;
    TFloatModel alpha;
    TTextModel fontname;
    TFont font;
    TRGB background_color;
    TBoolModel drawgrid;
    // TBoolModel use_grid;
    TIntegerModel gridsize;
    
    unsigned linewidth; // FIXME: TCoord
    TPenBase::ELineStyle linestyle;
    
    TFLine::EArrowMode arrowmode;
    TFLine::EArrowType arrowtype;
    unsigned arrowwidth;
    unsigned arrowheight;
};
typedef GSmartPointer<TFigureAttributes> PFigureAttributes;

/**
 * \ingroup figure
 */
class TFigureEditor:
  public TScrollPane
{
    typedef TScrollPane super;
    typedef TFigureEditor TThis;
    PFigureModel model;
    TFigureTool *tool;
  public:
    bool quick:1;     // active TFigureTool wants quick drawing method
    bool quickready:1;// TFigureEditor is prepared for quick drawing mode
    
    TFigureEditor();
    void setWindow(TWindow*);
    TWindow* getWindow() const { return window; }
    TFigureTool* getTool() const { return tool; }

    TFigureEditor(TWindow*, const string &title, TFigureModel *model=0);
    ~TFigureEditor();
    
    void setAttributes(TFigureAttributes *p);
    TFigureAttributes* getAttributes() const {
      return preferences;
    }
    void preferencesChanged();
    void modelChanged();

    void enableScroll(bool);
    void enableGrid(bool);
    void setGrid(TCoord gridsize);

    void setRowHeaderRenderer(TFigureEditorHeaderRenderer *r) {
      row_header_renderer = r;
    }
    TFigureEditorHeaderRenderer* getRowHeaderRenderer() const {
      return row_header_renderer;
    }
    void setColHeaderRenderer(TFigureEditorHeaderRenderer *r) {
      col_header_renderer = r;
    }
    TFigureEditorHeaderRenderer* getCowHeaderRenderer() const {
      return col_header_renderer;
    }
    
    unsigned result;            // values are defined in TFigure
    
    void setModel(TFigureModel *m);
    TFigureModel * getModel() const {
      return model;
    }
    
    void setModified(bool modified) {
      this->modified = modified;
    }
    bool isModified() const {
      return modified;
    }

  protected:
    bool modified;
    PFigureAttributes preferences;
  
    TWindow *window;            // current window
    TMatrix2D *mat;             // transformation for the editor
    
    TFigureEditorHeaderRenderer *row_header_renderer;
    TFigureEditorHeaderRenderer *col_header_renderer;

  public:
    static const unsigned OP_SELECT = 0;
//    static const unsigned OP_CREATE = 1;
    static const unsigned OP_ROTATE = 2;
    void setOperation(unsigned);
    unsigned getOperation() const { return operation; }
    void setCreate(TFigure *figure);
    void setTool(TFigureTool*);
    virtual void toolChanged(TFigureTool*);
    
    // not all these methods work now, but the first 4 should do
    void identity();
    void rotate(double);
    void rotateAt(double x, double y, double radiants);
    void translate(TCoord, TCoord);
    void scale(TCoord sx, TCoord sy);
    void shear(double, double);
    void multiply(const TMatrix2D*);

    const TMatrix2D* getMatrix() const { return mat; }

    // methods to modify selected or objects to be created
    void setLineColor(const TRGB&);
    void setFillColor(const TRGB&);
    void unsetFillColor();
    void setFont(const string &fontname);

    void invalidateWindow(bool b=true) { 
      if (window) 
        window->invalidateWindow(b); 
    }
    void invalidateWindow(TCoord x, TCoord y, TCoord w, TCoord h, bool b=true) {
      if (window)
        window->invalidateWindow(x, y, w, h, b);
    }
    void invalidateWindow(const TRectangle &r, bool b=true) {
      if (window)
        window->invalidateWindow(r, b);
    }
    void invalidateWindow(const TRegion &r, bool b=true) {
      if (window)
        window->invalidateWindow(r, b);
    }
    virtual void invalidateFigure(TFigure*);
    void getFigureShape(TFigure*, TRectangle*, const TMatrix2D*);
    void getFigureEditShape(TFigure*, TRectangle*, const TMatrix2D*);
    void _getFigureShape(TFigure*, TRectangle*, const TMatrix2D*);

    void addFigure(TFigure*);   
    void deleteFigure(TFigure*);

    void selectAll();

    //! Unselect all selected objects.
    bool clearSelection();
    
    //! Delete Selected Objects
    void deleteSelection();
    void deleteAll();
    
    void selection2Top();
    void selection2Bottom();
    void selectionUp();
    void selectionDown();
    void applyAll();
    
    void selectionCut();
    void selectionCopy();
    void selectionPaste();

#if 0
    void figure2Top(TFigure*);
    void figure2Bottom(TFigure*);
    void figureUp(TFigure*);
    void figureDown(TFigure*);
#endif

    void group();
    void ungroup();
    
    TFigure* findFigureAt(TPoint pos);

    static const unsigned STATE_NONE = 0;
    
    // states for OP_SELECT
    static const unsigned STATE_MOVE = 1;           // moving objects
    static const unsigned STATE_MOVE_HANDLE = 2;    // move handle
    static const unsigned STATE_SELECT_RECT = 3;    // select rectangular area
    static const unsigned STATE_EDIT = 4;           // edit object
    static const unsigned STATE_ROTATE = 5;         // rotate object
    static const unsigned STATE_MOVE_ROTATE = 6;
    
    // states for OP_CREATE
    static const unsigned STATE_START_CREATE = 20;   // set during `startCreate' and first `mouseLDown'
    static const unsigned STATE_CREATE = 21;
    
    TCoord fuzziness; // fuzziness to catch handles
    unsigned state;
    
    TCoord down_x, down_y;                             // last mouseXDown postion

    // undo stuff:
    TCoord memo_x, memo_y;
    unsigned memo_n;
    TPoint memo_pt;

    // triggered after `selection' was modified
    TSignal sigSelectionChanged;

    TFigureSet selection;

    void paint() override;
    
    void paintGrid(TPenBase &pen);
    void paintSelection(TPenBase &pen);
    void paintDecoration(TPen &pen);
    virtual void print(TPenBase &pen, TFigureModel *model, bool withSelection=false, bool justSelection=false);
    
    void resize() override;
    void mouseEvent(const TMouseEvent&) override;
    void keyEvent(const TKeyEvent&) override;

    virtual void mouse2sheet(TPoint mouse, TPoint *sheet);
    virtual void sheet2grid(TPoint sheet, TPoint *grid);
    virtual void minimalAreaSize(TCoord *x1, TCoord *y1, TCoord *x2, TCoord *y2);
    
    bool restore(TInObjectStream&);
    void store(TOutObjectStream&) const;

    void setCurrent(TFigure *f) { gadget = f; }
    TFigure* getCurrent() const { return gadget; }

  protected:
    void init(TFigureModel *m);
    
    unsigned operation;
    
    TFigure* gadget;        // the current gadget during create & edit

    int handle;             // the current handle or -1 during select
    bool tht; // translate handle transform?
    
    bool use_scrollbars;
    TCoord x1,x2, y1,y2;
    void updateScrollbars();
    void scrolled(TCoord dx, TCoord dy) override;
    
    void adjustPane() override;

    void stopOperation();   // stop the current operation
};

} // namespace toad

#endif
