/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 2014 by Mark-André Hopf <mhopf@mark13.org>
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

#include "colorpicker.hh"
#include "colorpalette.hh"
#include "toolbox.hh"
#include <toad/pen.hh>

using namespace fischland;

static TRectangle fillRect(0.5, 0.5, 27, 27);
static TRectangle lineRect(13.5, 14.5, 27, 27);
static TRectangle flipRect(31.5, 0.5, 9, 9);
static TRectangle initRect(1.5, 30, 11, 11);
static TRectangle typeColorRect(0.5,46.5,13,13);
static TRectangle typeGradientRect(13.5,46.5,14,13);
static TRectangle typeNoneRect(27.5,46.5,13,13);

TColorPicker::TColorPicker(TWindow *p, const string &t, TFigureAttributes *attr)
  :TWindow(p, t)
{
  this->attr = attr;
  if (attr) {
    connect(attr->sigChanged, this, &TColorPicker::preferencesChanged);
    line = COLOR;
    preferencesChanged();
  } else {
    init();
  }

  setBackground(TColor::DIALOG);
  active=FILL;
}

TColorPicker::~TColorPicker()
{
  if (attr)
    disconnect(attr->sigChanged, this);
}

void
TColorPicker::init()
{
  linecolor.set(0,0,0);
  fillcolor.set(1,1,1);
  fill=line=COLOR;
}

void
TColorPicker::preferencesChanged()
{
  fill = attr->filled ? COLOR : NONE;
  line = attr->outline ? COLOR : NONE;
  linecolor = attr->linecolor;
  fillcolor = attr->fillcolor;
  invalidateWindow();
}

void
TColorPicker::paintFill(TPen &pen)
{
  switch(fill) {
    case COLOR:
      pen.setColor(fillcolor);
      pen.fillRectangle(fillRect);
      break;
    case GRADIENT:
      break;
    case NONE:
      pen.setColor(1,1,1);
      pen.fillRectangle(fillRect);
      pen.setColor(1,0,0);
      pen.setLineWidth(1.5);
      pen.drawLine(fillRect.x, fillRect.y+fillRect.h, fillRect.x+fillRect.w, fillRect.y);
      pen.setLineWidth(1);
      break;
  }
  pen.setColor(0,0,0);
  pen.drawRectangle(fillRect);
}

void
TColorPicker::paintLine(TPen &pen)
{
  pen.setColor(1,1,1);
  pen.fillRectangle(lineRect);
  if (line==COLOR) {
    pen.setColor(linecolor);
    pen.fillRectangle(lineRect.x+1.5, lineRect.y+1.5, lineRect.w-3, lineRect.h-3);
  }
  pen.setColor(1,1,1);
  pen.fillRectangle(lineRect.x+1.5+5, lineRect.y+1.5+5, lineRect.w-3-10, lineRect.h-3-10);
  pen.setColor(0,0,0);
  pen.drawRectangle(lineRect.x+1.5+5+1.5, lineRect.y+1.5+5+1.5, lineRect.w-3-10-3, lineRect.h-3-10-3);
  if (line==NONE) {
    pen.setColor(1,0,0);
    pen.setLineWidth(1.5);
    pen.drawLine(lineRect.x, lineRect.y+lineRect.h, lineRect.x+lineRect.w, lineRect.y);
    pen.setLineWidth(1);
    pen.setColor(0,0,0);
  }
  pen.drawRectangle(lineRect);
}

void
TColorPicker::mouseLDown(const TMouseEvent &me)
{
  if (!click(me))
    return;
  invalidateWindow();
  attr->linecolor = linecolor;
  attr->fillcolor = fillcolor;
  attr->outline = line == COLOR;
  attr->filled = fill == COLOR;
  attr->reason.linecolor =
  attr->reason.fillcolor =
  attr->reason.line =
  attr->reason.fill = true;
  attr->sigChanged();
}

bool
TColorPicker::click(const TMouseEvent &me)
{
  if (me.dblClick) {
    TColorPalette *cp = new TColorPalette(0, "Palette", TToolBox::preferences);
    cp->createWindow();
    return true;
  }
  switch(active) {
    case LINE:
      if (lineRect.isInside(me.x, me.y))
        return false;
      if (fillRect.isInside(me.x, me.y)) {
        active = FILL;
        return true;
      }
      break;
    case FILL:
      if (fillRect.isInside(me.x, me.y))
        return false;
      if (lineRect.isInside(me.x, me.y)) {
        active = LINE;
        return true;
      }
      break;
  }
  
  if (flipRect.isInside(me.x, me.y)) {
    Type ab = line; line=fill; fill=ab;
cout << "TColorPicker::flipRect:" << endl;
cout << "  linecolor=" << linecolor << (line==COLOR?", yes":", no") << endl;
cout << "  fillcolor=" << fillcolor << (fill==COLOR?", yes":", no") << endl;
    TRGB rc = linecolor; linecolor=fillcolor; fillcolor=rc;
cout << "  linecolor=" << linecolor << (line==COLOR?", yes":", no") << endl;
cout << "  fillcolor=" << fillcolor << (fill==COLOR?", yes":", no") << endl;
    return true;
  }
  
  if (initRect.isInside(me.x, me.y)) {
    init();
    return true;
  }
  
  if (typeColorRect.isInside(me.x, me.y)) {
    switch(active) {
      case FILL: fill = COLOR; break;
      case LINE: line = COLOR; break;
    }
    return true;
  }
  if (typeGradientRect.isInside(me.x, me.y)) {
    active = FILL;
    fill = GRADIENT;
    return true;
  }
  if (typeNoneRect.isInside(me.x, me.y)) {
    switch(active) {
      case FILL: fill = NONE; break;
      case LINE: line = NONE; break;
    }
    return true;
  }
  return false;
}

void
TColorPicker::paint()
{
  TPen pen(this);

  switch(active) {
    case LINE:
      paintFill(pen);
      paintLine(pen);
      break;
    case FILL:
      paintLine(pen);
      paintFill(pen);
      break;
  }

  // reset to default colors
  pen.setColor(0,0,0);
  pen.fillRectangle(initRect.x+2.5, initRect.y+3, initRect.w-3, initRect.h-3);
  pen.setColor(1,1,1);
  pen.fillRectangle(initRect.x+4.5, initRect.y+5, initRect.w-7, initRect.h-7);
  pen.fillRectangle(initRect.x,initRect.y+0.5, initRect.w-4, initRect.h-4);
  pen.setColor(0,0,0);
  pen.drawRectangle(initRect.x,initRect.y+0.5, initRect.w-4, initRect.h-4);

  // arrows
  pen.setColor(0,0,0);
  TPolygon p;
  p.addPoint(flipRect.x  , flipRect.y+2);
  p.addPoint(flipRect.x+2, flipRect.y);
  p.addPoint(flipRect.x+2, flipRect.y+4);
  pen.fillPolygon(p);
  pen.drawPolygon(p);

  p.clear();
  p.addPoint(flipRect.x+flipRect.w-2  , flipRect.y+flipRect.h);
  p.addPoint(flipRect.x+flipRect.w-4  , flipRect.y+flipRect.h-2);
  p.addPoint(flipRect.x+flipRect.w  , flipRect.y+flipRect.h-2);
  pen.fillPolygon(p);
  pen.drawPolygon(p);

  p.clear();
  p.addPoint(flipRect.x+2, flipRect.y+2);
  p.addPoint(flipRect.x+flipRect.w-2, flipRect.y+2);
  p.addPoint(flipRect.x+flipRect.w-2, flipRect.y+2);
  p.addPoint(flipRect.x+flipRect.w-2, flipRect.y+flipRect.h-2);
  pen.drawBezier(p);

  // color,gradient,none
  pen.setColor(0.65, 0.65, 0.65);
  switch(active==FILL ? fill : line) {
    case COLOR: pen.fillRectangle(typeColorRect); break;
    case GRADIENT: pen.fillRectangle(typeGradientRect); break;
    case NONE: pen.fillRectangle(typeNoneRect); break;
  }

  pen.setColor(0,0,0);
  pen.drawRectangle(0.5,46.5, 40, 13);
  pen.drawLine(13.5,46.5, 13.5, 59.5);
  pen.drawLine(27.5,46.5, 27.5,59.5);

  // color
  switch(active) {
    case LINE: pen.setColor(linecolor); break;
    case FILL: pen.setColor(fillcolor); break;
  }
  pen.fillRectangle(0.5+3,46.5+3, 7, 7);
  pen.setColor(0,0,0);
  pen.drawRectangle(0.5+3,46.5+3, 7, 7);
  
  // gradient
  pen.drawRectangle(0.5+3+13,46.5+3, 8, 7);

  // none
  pen.setColor(1,1,1);
  pen.fillRectangle(0.5+3+27,46.5+3, 7, 7);
  pen.setColor(1,0,0);
  pen.setLineWidth(1.5);
  pen.drawLine(0.5+3+27,46.5+3+7, 0.5+3+27+7, 46.5+3);
  pen.setLineWidth(1);
  pen.setColor(0,0,0);
  pen.drawRectangle(0.5+3+27,46.5+3, 7, 7);
}
