/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2005 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figure.hh>
#include <toad/figuremodel.hh>
#include <toad/figureeditor.hh>
#include <toad/figure/connectfigure.hh>
#include <cmath>

#include <toad/dialog.hh>
#include <toad/menuhelper.hh>
#include <toad/springlayout.hh>

using namespace toad;

/**
 * \defgroup figure Figures
 *
 * Direct manipulation 2d graphics.
 */

/**
 * \ingroup figure
 * \class toad::TFigure
 *
 * TFigure is the base class for editable 2d graphic objects within the
 * TFigureEditor window.
 *
 * Most of the virtual methods provided by TFigure are to support the
 * TFigureEditor class.
 *
 * TFigure itself supports only translate and scale as built-in
 * transformations.
 *
 * For all other transformations, TFigure can be made a child of either
 * @li TFTransform		affinite transform, including rotate and shear
 * @li TFTransformStrokeAndFX	affinite transform, will also transform line width and fx like patterns
 * @li TFTransformPerspective
 * @li TFTransformBezier
 *
 */

#if 1
double toad::TFigure::OUT_OF_RANGE = HUGE_VAL;
double toad::TFigure::RANGE = 5.0;
double toad::TFigure::INSIDE = -1.0;
#endif

// this class is for backward compability

void
TFigure::initialize()
{
  TObjectStore& serialize(toad::getDefaultStore());
  serialize.registerObject(new TDialogLayout());
  serialize.registerObject(new TFCircle());
  serialize.registerObject(new TFConnection());
  serialize.registerObject(new TFFrame());
  serialize.registerObject(new TFGroup());
  serialize.registerObject(new TFImage());
  serialize.registerObject(new TFRectangle());
  serialize.registerObject(new TFText());
  serialize.registerObject(new TFWindow());
  serialize.registerObject(new TFigureModel());
  serialize.registerObject(new TMatrix2D());
  serialize.registerObject(new TMenuEntry());
  serialize.registerObject(new TMenuLayout());
  serialize.registerObject(new TMenuSeparator());
  serialize.registerObject(new TSerializableRGB());
  serialize.registerObject(new TSpringLayout());
}

void
TFigure::terminate()
{
  toad::getDefaultStore().unregisterAll();
}

TInObjectStream TFigure::serialize;

TFigure::TFigure()
{
}

TFigure::TFigure(const TFigure &f)
{
}

TFigure::~TFigure()
{
}

/**
 * This method is experimental.
 */
bool
TFigure::editEvent(TFigureEditEvent &ee)
{
  switch(ee.type) {
    case TFigureEditEvent::TRANSLATE:
      cerr << "TFigureEditEvent::TRANSLATE not implemented" << endl;
      exit(1);
      // translate(ee.x, ee.y);
      break;
    case TFigureEditEvent::START_IN_PLACE:
      return startInPlace();
  }
  return true;
}

/**
 * This one is a crude hack to speedup selection in drawings with
 * many beziers. I still need to work it out.
 */
TCoord
TFigure::_distance(TFigureEditor *fe, TCoord x, TCoord y)
{
  return distance(TPoint(x, y));
}

TCoord
TFigure::distance(const TPoint &pos)
{
  return bounds().isInside(pos) ? INSIDE : OUT_OF_RANGE;
}

/**
 * Apply a transfomation to the figure.
 *
 * When returning 'false', TSelectionTool will make the figure a child of
 * TFTransform, which will apply the transformation on the output of getPath()
 * instead.
 *
 * Returning true and doing nothing will disable the transformation but also
 * result in a bad user experience.
 */
bool
TFigure::transform(const TMatrix2D &transform)
{
  return false;
}


TAttributedFigure::TAttributedFigure()
{
  outline = true;
  filled = false;
  closed = false;
  line_color.set(0,0,0);
  fill_color.set(0,0,0);
  alpha = 1.0;
  line_style = TPenBase::SOLID;
  line_width = 1;
}

void
TFigure::setAttributes(const TFigureAttributeModel *preferences)
{
}

void
TFigure::getAttributes(TFigureAttributeModel *preferences) const
{
}

void
TAttributedFigure::setAttributes(const TFigureAttributeModel *preferences)
{
  if (preferences->reason.linecolor)
    line_color = preferences->linecolor;
  if (preferences->reason.fillcolor)
    fill_color = preferences->fillcolor;
  if (preferences->reason.fill)
    filled = preferences->filled;
  if (preferences->reason.line)
    outline = preferences->outline;
  if (preferences->reason.linewidth)
    line_width = preferences->linewidth;
  if (preferences->reason.linestyle)
    line_style = preferences->linestyle;
  if (preferences->reason.alpha)
    alpha = preferences->alpha;
}

void
TAttributedFigure::getAttributes(TFigureAttributeModel *preferences) const
{
  preferences->linewidth = line_width;
  preferences->linestyle = line_style;
  preferences->linecolor = line_color;
  preferences->fillcolor = fill_color;
  preferences->filled = filled;
  preferences->alpha  = alpha;
}

/**
 * Called from the gadget editor when the gadget is selected.
 *
 * The default behaviour is to draw small rectangles at the positions
 * delivered by <I>getHandle</I> or at the corners of the rectangle
 * delivered by <I>getShape</I> when <I>getHandle</I> returns `false'
 * for handle 0.
 *
 * When 'handle' is positive, it indicates that the handle of the same
 * number is currently manipulated.
 *
 * \param pen
 *   A pen to draw the figures selection.
 * \param handle
 *   Handle which is currently manipulated or <0, in case of none.
 */
void
TFigure::paintSelection(TPenBase &pen, int handle)
{
//cout << getClassName() << "::paintSelection() called TFigure" << endl;
  pen.setStrokeColor(TColor::FIGURE_SELECTION);
  pen.setFillColor(TColor::WHITE);

  const TMatrix2D *m0 = pen.getMatrix();
  if (m0) {
    pen.push();
    pen.identity();
  }

  unsigned h=0;
  TPoint pt;
  while(true) {
    if ( !getHandle(h, &pt) )
      break;
    int x, y;
    if (m0) {
      m0->map(pt.x, pt.y, &x, &y);
    } else {
      x = pt.x;
      y = pt.y;
    }
    pen.setLineWidth(1);
    if (handle!=h) {
      pen.fillRectanglePC(x-2,y-2,5,5);
      pen.drawRectanglePC(x-2,y-2,5,5);
    } else {
      pen.setFillColor(TColor::FIGURE_SELECTION);
      pen.fillRectanglePC(x-2,y-2,5,5);
      pen.setFillColor(TColor::WHITE);
    }
    h++;
  }
  
  // no handles found, use the figures shape instead
  if (h==0) {
    TRectangle r = bounds();
    int x, y;
    for(int i=0; i<4; ++i) {
      switch(i) {
        case 0: x = r.origin.x;                y = r.origin.y;       break;
        case 1: x = r.origin.x+r.size.width-1; y = r.origin.y;       break;
        case 2: x = r.origin.x+r.size.width-1; y = r.origin.y+r.size.height-1; break;
        case 3: x = r.origin.x;                y = r.origin.y+r.size.height-1; break;
      }
      if (m0)
        m0->map(x, y, &x, &y);
      pen.setLineWidth(1);
      pen.fillRectanglePC(x-2,y-2,5,5);
      pen.drawRectanglePC(x-2,y-2,5,5);
    }
  }

  if (m0)
    pen.pop();
}

TRectangle
TFigure::editBounds() const
{
  TRectangle r = bounds();
  r.inflate(2);
  return r;
}

/**
 * Return <I>true</I> and the position of handle <I>n</I> in <I>p</I> or
 * <I>false</I> when there's no handle <I>n</I>.
 *
 * The first handle is 0.
 */
bool
TFigure::getHandle(unsigned n, TPoint *p)
{
  return false;
}

bool
TFigure::startTranslateHandle()
{
  return true;
}

void
TFigure::endTranslateHandle()
{
}

/**
 * Set handle <I>handle</I> to position (x,y).
 */
void
TFigure::translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier)
{
}

bool
TFigure::startInPlace()
{
  return false;
}

unsigned
TFigure::stop(TFigureEditor*) 
{ 
  return 0;
}

unsigned 
TFigure::keyDown(TFigureEditor*, TKey, char*, unsigned)
{ 
  return 0;
}

void
TFigure::startCreate(const TPoint&)
{
}

void
TFigure::dragCreate(const TPoint&)
{
}

void
TFigure::endCreate()
{
}

unsigned
TFigure::mouseLDown(TFigureEditor*, TMouseEvent &)
{
  return 0;
}

unsigned
TFigure::mouseMove(TFigureEditor*, TMouseEvent &)
{
  return 0;
}

unsigned 
TFigure::mouseLUp(TFigureEditor*, TMouseEvent &)
{
  return 0;
}

unsigned
TFigure::mouseRDown(TFigureEditor*, TMouseEvent &)
{
  return 0;
}

namespace {

struct TStylePair {
  TPenBase::ELineStyle code;
  const char *name;
};

TStylePair sp[] = {
  { TPenBase::SOLID, "solid" },
  { TPenBase::DASH,  "dash" },
  { TPenBase::DOT, "dot" },
  { TPenBase::DASHDOT, "dashdot" },
  { TPenBase::DASHDOTDOT, "dashdotdot" }
};

} // namespace

void
TFigure::store(TOutObjectStream &out) const
{
  TSerializable::store(out);
}

void
TAttributedFigure::store(TOutObjectStream &out) const
{
  super::store(out);
  ::store(out, "linecolor", line_color);
  if (filled) {
    ::store(out, "fillcolor", fill_color);
  }
  if (line_width>0) {
    ::store(out, "linewidth", line_width);
  }
  if (line_style!=TPenBase::SOLID) {
    for(unsigned i=0; i<5; ++i) {
      if (sp[i].code == line_style) {
        ::store(out, "linestyle", sp[i].name);
        break;
      }
    }
  }
  if (alpha<1.0) {
    ::store(out, "alpha", alpha);
  }
}

bool
TFigure::restore(TInObjectStream &in)
{
  if (
    TSerializable::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}

bool
TAttributedFigure::restore(TInObjectStream &in)
{
  bool b;
  if (in.what==ATV_START) {
    filled = false;
    return true;
  }
  if (::restore(in, "filled", &b))
    return true;
  if (::restore(in, "fillcolor", &fill_color)) {
    filled = true;
    return true;
  }
  string style;
  if (::restore(in, "linestyle", &style)) {
    for(unsigned i=0; i<5; ++i) {
      if (style == sp[i].name) {
        line_style = sp[i].code;
        return true;
      }
    }
    return false;
  }

  if (::restore(in, "alpha", &alpha)) {
    return true;
  }
  
  if (
    ::restore(in, "linecolor", &line_color) ||
    ::restore(in, "linewidth", &line_width) ||
    super::restore(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}
