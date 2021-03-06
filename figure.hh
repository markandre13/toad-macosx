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

#ifndef _TOAD_FIGURE_HH
#define _TOAD_FIGURE_HH 1

#include <math.h>
#include <toad/penbase.hh>
#include <toad/window.hh>
#include <toad/bitmap.hh>
#include <toad/figuremodel.hh>
#include <toad/io/serializable.hh>
#include <toad/wordprocessor.hh>

namespace toad {

class TFigureEditor;
class TFigureAttributeModel;
class TMatrix2D;
class TVectorGraphic;
class TShapeTool;

class TFigureEditEvent
{
  public:
    enum EType {
      PAINT_NORMAL,
      PAINT_SELECT,
      PAINT_EDIT,
      PAINT_SELECTION,
    
      //! figure was added to model
      ADDED,
      //! the figure is to be removed from the model
      REMOVED,

      RELATION_REPLACED,
      
      //! the figures listed in model->figures are to be removed from the model
      RELATION_REMOVED,
      
      //! the figures listed in model->figures have been modified
      RELATION_MODIFIED,
    
      GET_DISTANCE,
      GET_HANDLE,
      GET_SHAPE,
      BEGIN_TRANSLATE,
      TRANSLATE,
      END_TRANSLATE,
      TRANSLATE_HANDLE,
      START_IN_PLACE,
      STOP_EDIT,
      KEY_EVENT,
      MOUSE_EVENT,
      
    } type;

    union {
      struct RelationReplaced {
        TFigure *oldRelation;
        TFigure *newRelation;
      } relationReplaced;
    } data;

    TFigureEditor *editor; // in:START_IN_PLACE
    TFigureModel *model; // in
    
    TPenBase *pen;    // in
    
    TRectangle shape; // out:GET_SHAPE

    TCoord x, y;         // in:GET_DISTANCE, in:GET_HANDLE, in:TRANSLATE, 
                      // in:TRANSLATE_HANDLE
    TCoord distance;  // out:GET_DISTANCE

    unsigned handle;  // out:GET_HANDLE, in:TRANSLATE_HANDLE, in:PAINT_SELECTION
    
    TKeyEvent *key;
    TMouseEvent *mouse;
};

class TVectorPath;

/**
 * \ingroup figure
 *
 */
class TFigure:
  public TSerializable
{
  public:
    TFigure();
    TFigure(const TFigure &);
    virtual ~TFigure();
    
    virtual TVectorGraphic *getPath() const { return nullptr; }

    virtual bool editEvent(TFigureEditEvent &ee);
    
    enum EPaintType {
      NORMAL,
      SELECT,
      EDIT,
      OUTLINE
    };
    virtual void setAttributes(const TFigureAttributeModel*);
    virtual void getAttributes(TFigureAttributeModel*) const;
    
    //! Called to paint the gadget.
    virtual void paint(TPenBase& pen, EPaintType type = NORMAL) = 0;
    virtual void paintSelection(TPenBase &pen, int handle=-1);

    /**
     * Called to get the gadgets bounding rectangle.
     *
     * Used to
     * \li
     *    invalidate the area the figure occupies to redraw the figure with
     *    TFigureEditor::invalidateFigure
     * \li
     *    calculate the size of the area all gadgets occupy and to set up the
     *    scrolling area accordingly and
     * \li
     *    paint a marker with the default implementation of 'paintSelection'
     */
    virtual TRectangle bounds() const = 0;
    virtual TRectangle editBounds() const;

  public:
  
    // FIXME: the base class of TFigure shouldn't have any attributes

    /**
     * 'true' when TFigureEditor is allowed to delete this object.
     *  NOTE: might be better as a virtual function
     */

    // stage 1: select:
    virtual double _distance(TFigureEditor *fe, TCoord x, TCoord y); // FIXME: DROP
    virtual TCoord distance(const TPoint &pos);
    
    // stage 2: move
    virtual bool transform(const TMatrix2D &transform);

    // TFHandleInterface
    
    // stage 3: manipulate
    static const int NO_HANDLE = -1;
    virtual bool getHandle(unsigned n, TPoint *p);
    virtual bool startTranslateHandle();
    virtual void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier);
    virtual void endTranslateHandle();

    // stage 4: in place editing
    //! Return `true' when in-place editing is desired.
    virtual bool startInPlace();
    virtual unsigned stop(TFigureEditor*);
    virtual unsigned keyDown(TFigureEditor*, TKey, char*, unsigned);

    // TFShapeInterface

    // editor related stuff for manipulation & creation
    //--------------------------------------------------
    virtual void startCreate(const TPoint &start);
    virtual void dragCreate(const TPoint &end);
    virtual void endCreate();
    
    virtual unsigned mouseLDown(TFigureEditor*, TMouseEvent &);
    virtual unsigned mouseMove(TFigureEditor*, TMouseEvent &);
    virtual unsigned mouseLUp(TFigureEditor*, TMouseEvent &);
    virtual unsigned mouseRDown(TFigureEditor*, TMouseEvent &);

    // editor related stuff for all gadgets
    //--------------------------------------
    static inline TCoord distance2Line(TCoord x, TCoord y, TCoord x1, TCoord y1, TCoord x2, TCoord y2) {
      return toad::distance(TPoint(x, y), TPoint(x1, y1), TPoint(x2, y2));
    }

#if 0
    static const double OUT_OF_RANGE = HUGE_VAL;
    static const double RANGE = 5.0;
    static const double INSIDE = -1.0;
#else
    static TCoord OUT_OF_RANGE;
    static TCoord RANGE;
    static TCoord INSIDE;
#endif
    // storage stuff for all gadgets
    //-------------------------------------- 
    static void initialize();
    static void terminate();
    static TInObjectStream serialize;

//    SERIALIZABLE_INTERFACE(toad::, TFigure);
    void store(TOutObjectStream &out) const override;
    bool restore(TInObjectStream &in) override;
};

class TAttributedFigure:
  public TFigure
{
  public:
    typedef TFigure super;
  protected:
    TAttributedFigure();
  public:
    TCoord alpha;
    bool outline:1;       // true when outline
    bool filled:1;        // true when filled
    bool closed:1;        // true when closed
    TPenBase::ELineStyle line_style;
    unsigned line_width;

  protected:
    void store(TOutObjectStream &out) const override;
    bool restore(TInObjectStream &in) override;

  public:
    TSerializableRGB line_color;
    TSerializableRGB fill_color;
    void setAttributes(const TFigureAttributeModel*) override;
    void getAttributes(TFigureAttributeModel*) const override;

    void setStrokeColor(const TRGB &color) {
      line_color = color;
    }
    void setFillColor(const TRGB &color) {
      fill_color = color;
      filled = true;
    }
    void unsetFillColor() {
      filled = false;
    }
    bool isFilled() const { return filled && closed; }
//    virtual void setFont(const string&);
};

/**
 * \ingroup figure
 *
 */
class TFTransformBase:
  public TFigure
{
  public:
    TFigure *figure;
    ~TFTransformBase() {
      if (figure)
        delete figure;
    }
};

/*
class TFPerspectiveTransform:
  public TFTransformBase
{
};

class TFBezierTransform:
  public TFTransformBase
{
};
*/

/**
 * \ingroup figure
 *
 */
class TFAffiniteTransformBase:
  public TFTransformBase
{
  public:
    TMatrix2D matrix;
};

/**
 * \ingroup figure
 *
 * affinite transformation which does also scale stroke & fx like textures
 */
class TFTransformStrokeAndFX:
  public TFAffiniteTransformBase
{
  protected:
    void paint(TPenBase &pen, EPaintType type=NORMAL) override;
};

/**
 * \ingroup figure
 *
 * affinite transformation which does not scale stroke & fx
 */
class TFTransform:
  public TFAffiniteTransformBase
{
  public:
    TVectorGraphic *getPath() const override;
  protected:
    void paint(TPenBase &pen, EPaintType type=NORMAL) override;
    
    TCoord distance(const TPoint &pos) override;
    TRectangle bounds() const override;

    bool getHandle(unsigned n, TPoint *p) override;
    bool startTranslateHandle() override;
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override;
    void endTranslateHandle() override;

    TCloneable* clone() const override { return new TFTransform(*this); }
    const char * getClassName() const override { return "toad::TFTransform"; }
    void store(TOutObjectStream&) const override;
    bool restore(TInObjectStream&) override;
};

/**
 * \ingroup figure
 */
class TFRectangle:
  public TAttributedFigure
{
    typedef TAttributedFigure super;
  public:
    static TShapeTool* getTool();
  
    TFRectangle() = default;
    TFRectangle(TCoord x, TCoord y, TCoord w, TCoord h) {
      setShape(x, y, w, h);
    };
    void setShape(TCoord x, TCoord y, TCoord w, TCoord h) {
      p1.x = x;
      p1.y = y;
      p2.x = x+w;
      p2.y = y+h;
    }
    void setShape(const TRectangle &r) {
      p1 = r.origin;
      p2 = r.origin + r.size;
    }
    TVectorGraphic* getPath() const override;
    void paint(TPenBase &pen, EPaintType type=NORMAL) override;
    TRectangle bounds() const override;

    TCoord distance(const TPoint &pos) override;
    bool transform(const TMatrix2D &transform) override;
    bool getHandle(unsigned n, TPoint *p) override;
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override;

    SERIALIZABLE_INTERFACE(toad::, TFRectangle);
    
  protected:
    TPoint p1, p2;

    void startCreate(const TPoint &start) override;
    void dragCreate(const TPoint &end) override;
};

class TFigureArrow
{
  public:
    enum EArrowMode {
      NONE, HEAD, TAIL, BOTH
    } arrowmode;
    
    enum EArrowType {
      SIMPLE,
      EMPTY,
      FILLED,
      EMPTY_CONCAVE,
      FILLED_CONCAVE,
      EMPTY_CONVEX,
      FILLED_CONVEX
    } arrowtype;
    
    unsigned arrowheight;
    unsigned arrowwidth;

    TFigureArrow();
    void setAttributes(const TFigureAttributeModel*);
    void getAttributes(TFigureAttributeModel*) const;
    void store(TOutObjectStream &out) const;
    bool restore(TInObjectStream &in);

    static void drawArrow(TPenBase &pen,
                          const TPoint &p0, const TPoint &p1, 
                          const TRGB &line, const TRGB &fill,
                          TCoord w, TCoord h,
                          EArrowType type);
};

/**
 * \ingroup figure
 */
class TFCircle:
  public TFRectangle
{
  public:
    TFCircle(){}
    TFCircle(TCoord x, TCoord y, TCoord w, TCoord h):
      TFRectangle(x,y,w,h) {}
    void paint(TPenBase &, EPaintType) override;
    
    TCoord distance(const TPoint &pos) override;
    TVectorGraphic* getPath() const override;
    
    TCloneable* clone() const override { return new TFCircle(*this); }
    const char * getClassName() const override { return "toad::TFCircle"; } 
};

/**
 * \ingroup figure
 */
class TFText:
  public TAttributedFigure
{
    typedef TAttributedFigure super;
    TMatrix2D matrix;
    TSize size;
    
  public:
    TFText();
    TFText(const TFText&);
    TFText(TCoord x,TCoord y, const string &aText, TFigure *aRelation=nullptr);
    
    void setText(const string &aText) {
      text = aText;
      wp.init(text);
      calcSize();
    }
    void setFont(const string &fontname) {
      this->fontname = fontname;
    }

    void setAttributes(const TFigureAttributeModel*) override;
    void getAttributes(TFigureAttributeModel*) const override;

    void paint(TPenBase &, EPaintType) override;
    bool transform(const TMatrix2D &transform) override;
    TRectangle bounds() const override;
    // TVectorGraphic* getPath() const override; [NSBezierPath appendBezierPathWithGlyphs:count:inFont]

    TCoord distance(const TPoint &pos) override;
    bool getHandle(unsigned n, TPoint *p) override;

    bool startInPlace() override;
    void startCreate(const TPoint &start) override;
    unsigned stop(TFigureEditor*) override;

    unsigned keyDown(TFigureEditor*, TKey, char*, unsigned) override;
    unsigned mouseLDown(TFigureEditor*, TMouseEvent &) override;
    unsigned mouseMove(TFigureEditor*, TMouseEvent &) override;
    unsigned mouseLUp(TFigureEditor*, TMouseEvent &) override;
    void mouseEvent(TMouseEvent &me) { // FIXME
//      me.pos -= p1;
//      wp.mouseEvent(me);
    }

    TCloneable* clone() const override { return new TFText(*this); }

    const char * getClassName() const override { return "toad::TFText"; } 
    void store(TOutObjectStream&) const override;
    bool restore(TInObjectStream&) override;
    
    TFigure *relation;
  protected:
    TWordProcessor wp;
    
    string text;
    string fontname;
    void calcSize();

    bool editEvent(TFigureEditEvent &editEvent) override;
};

/**
 * \ingroup figure
 */
class TFFrame:
  public TFRectangle
{
    typedef TFText super;
    string fontname;
    string text;
  public:
    TFFrame() {}
    TFFrame(TCoord x, TCoord y, TCoord w, TCoord h, const string &text="") {
      this->text = text;
      setShape(x,y,w,h);
    };
    void paint(TPenBase &, EPaintType) override;

    TRectangle bounds() const override;
    TCoord distance(const TPoint &pos) override;
    unsigned stop(TFigureEditor*) override;
    unsigned keyDown(TFigureEditor*, TKey, char*, unsigned) override;
    bool getHandle(unsigned n, TPoint *p) override;
    unsigned mouseLDown(TFigureEditor *e, TMouseEvent &) override;
    unsigned mouseMove(TFigureEditor *e, TMouseEvent &) override;
    unsigned mouseLUp(TFigureEditor *e, TMouseEvent &) override;
    
    TCloneable* clone() const override { return new TFFrame(*this); }
    const char * getClassName() const override { return "toad::TFFrame"; } 

    void calcSize();
};

/**
 * \ingroup figure
 */
class TFWindow:
  public TFRectangle
{
    typedef TFRectangle super;
  public:
    TFWindow();

    void paint(TPenBase&, EPaintType) override;
    TCoord distance(const TPoint &pos) override;
    bool transform(const TMatrix2D &transform) override;
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override;
    
    TCloneable* clone() const override { return new TFWindow(*this); }
    const char * getClassName() const override { return "toad::TFWindow"; }
    void store(TOutObjectStream&) const override;
    bool restore(TInObjectStream&) override;
    
    string title;
    string label;
    unsigned taborder;
    TWindow *window;
};

/**
 * \ingroup figure
 */
class TFGroup:
  public TFRectangle
{
    typedef TFRectangle super;
  public:
    TFGroup();
    TFGroup(const TFGroup &g);
    ~TFGroup();
    void paint(TPenBase&, EPaintType) override;
    TCoord _distance(TFigureEditor *fe, TCoord x, TCoord y) override;
    bool getHandle(unsigned n, TPoint *p) override;
    bool startTranslateHandle() override;
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override;
    void endTranslateHandle() override;
    
    void drop() {
      gadgets.drop();
    }
    
    void calcSize();

    TFigureModel gadgets;

    bool transform(const TMatrix2D &transform) override;
    bool editEvent(TFigureEditEvent &ee) override;

    TCloneable* clone() const override { return new TFGroup(*this); }
    const char * getClassName() const override { return "toad::TFGroup"; }
    void store(TOutObjectStream&) const override;
    bool restore(TInObjectStream&) override;

  protected:
    void modelChanged();
};

class TFPerspectiveTransform:
  public TFigure
{
    TFigureModel figures;
    TPoint orig[4];
    TPoint handle[4];

    TPoint A, C, Du, E, F, G, H;
    void prepare();
    
    TPoint transform(const TPoint&) const;
  public:
    void add(TFigure *f) { figures.add(f); }
    void init();
    void paint(TPenBase&, EPaintType) override;
    TRectangle bounds() const override;

    bool transform(const TMatrix2D &transform) override;
    TCoord _distance(TFigureEditor *fe, TCoord mx, TCoord my) override;
    bool getHandle(unsigned n, TPoint *p) override;
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override;

    TCloneable* clone() const override { return new TFPerspectiveTransform(*this); }
    const char * getClassName() const override { return "toad::TFPerspectiveTransform"; }
    void store(TOutObjectStream&) const override;
    bool restore(TInObjectStream&) override;
};

// class TFTransformRotate
// class TFTransformPerspective
// class TFTransformBezier
// ...

class TFImage:
  public TFigure
{
    typedef TFigure super;
  protected:
    string filename;
    PBitmap bitmap;
    TCoord x, y;
  public:
    TFImage();
    TFImage(const string &filename);
    bool startInPlace() override;

    void paint(TPenBase &, EPaintType) override;
    TRectangle bounds() const override;
    
    bool editEvent(TFigureEditEvent &ee) override;

    TCloneable* clone() const override { return new TFImage(*this); }
    const char * getClassName() const override { return "toad::TFImage"; } 
    void store(TOutObjectStream&) const override;
    bool restore(TInObjectStream&) override;
};

} // namespace toad

#endif
