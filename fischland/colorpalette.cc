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

#include "colorpalette.hh"

#include <toad/fatcheckbutton.hh>
#include <toad/pushbutton.hh>
#include <fstream>

using namespace toad;

TColorPalette::TColorPalette(TWindow *parent, const string &title,
                             TFigureAttributeModel *preferences):
  TWindow(parent, title)
{
//  bNoBackground = true;
  this->preferences = preferences;
  flagStaticFrame = true;
  setBackground(TColor::DIALOG);
  setSize(320, 153);
  
  fw = fh = 12;
  
  TCoord x, y, w, h;

  TPushButton *btn;
  TFatCheckButton *fcb;
  TTextField *tf;
  
  palettelistrenderer = new TPaletteListAdapter(&palettes);
  y = 4;
  w = getWidth();
  TComboBox *cb = new TComboBox(this, "palettes");
  h = cb->getHeight();
  w-= 4 + h + h + 4;
  cb->setToolTip("select color palette");
  cb->setAdapter(palettelistrenderer);
  cb->setShape(4,y,w,h);
  connect(cb->sigSelection, this, &TThis::paletteSelected, cb);
  
  btn = new TPushButton(this, "add palette");
  btn->setToolTip("create a new palette");
  btn->setShape(4+w+1,y+1,h-2,h-2);
  btn->setLabel("+");

  btn = new TPushButton(this, "delete palette");
  btn->setToolTip("delete current palette");
  btn->setShape(4+w+h+1,y+1,h-2,h-2);
  btn->setLabel("-");

  rectColors.set(4.5,y+h+4.5, getWidth()-4-4, getHeight() - 4 - h - 4 - 4 - h - 4);

  TScrollBar *sb = new TScrollBar(this, "updown", &position);
  rectColors.size.width -= sb->getWidth()+2;
  sb->setShape(floor(rectColors.origin.x+rectColors.size.width), floor(rectColors.origin.y),
               sb->getWidth(), rectColors.size.height);
  position.setValue(0);
  connect(position.sigChanged, this, &TWindow::invalidateWindow, true);
  
  tf = new TTextField(this, "colorname", &colorname);
  h = tf->getHeight();
  y = getHeight() - 4 - h;
  w = getWidth() - 4 - h - h - 4;
  tf->setShape(4+h,y,w-h,tf->getHeight());

  fcb = new TFatCheckButton(this, "linecolor", &doLineColor);
  fcb->setToolTip("set as line color");
  fcb->setLabel("L");
  fcb->setShape(4+w,y,h,h);
  TCLOSURE1(
    doLineColor.sigChanged,
    _this, this,
    if (_this->doLineColor)
      _this->assignLineColor();
  )
  
  fcb = new TFatCheckButton(this, "fillcolor", &doFillColor);
  fcb->setToolTip("set as fill color");
  fcb->setLabel("F");
  fcb->setShape(4+w+h,y,h,h);
  TCLOSURE1(
    doFillColor.sigChanged,
    _this, this,
    if (_this->doFillColor)
      _this->assignFillColor();
  )

  rectCurrent.set(4,y,h,h);

  TDropSiteColor *ds = new TDropSiteColor(this, rectColors);
  connect_value(ds->sigDrop, this, &TColorPalette::dropColor, ds);
  
  loadPalette("palette.txt");
  loadPalette("CGA.txt");

  selectPalette(0);
}

void
TColorPalette::selectPalette(int n)
{
  invalidateWindow();
  ppalette = 0;
  pcolor = 0;

  if (palettes.size()<=n)
    return;
  ppalette = palettes[n];
  if (!ppalette->rgb.empty()) {
    pcolor = &ppalette->rgb[0];
    colorname = pcolor->name;
  }
  int fields_per_row = rectColors.size.width / fw;
#if 1
  position.setExtent(rectColors.size.height / fh);
  position.setMinimum(0);
  position.setMaximum(ppalette->rgb.size()/fields_per_row);
  position.setValue(0);
#else
  position.setRangeProperties(0,                  // value
                              rectColors.h / fh,  // extent
                              0,                  // min
                              ppalette->rgb.size()/fields_per_row);
#endif
}

void
TColorPalette::paletteSelected(TComboBox *cb)
{
  unsigned i = cb->getSelectionModel()->getRow();
  selectPalette(i /*cb->getLastSelectionRow()*/);
}

bool
TColorPalette::loadPalette(const string &filename)
{
  ifstream in;
  in.open(filename.c_str());
  if (!in) {
    cerr << "failed to open color palette '" << filename << "'\n";
    return false;
  }
  TNamedPalette *np = new TNamedPalette();
  palettes.push_back(np);
  np->modified = false;
  np->filename = filename;
  np->name = filename;
  
  char buffer[256];
  char name[256];
  int r, g, b;
  in.getline(buffer, sizeof(buffer));
  while(in) {
    in.getline(buffer, sizeof(buffer));
    if (buffer[0]=='\0' || buffer[0]=='#')
      continue;
    sscanf(buffer, "%i %i %i %[ -~]", &r, &g, &b, name); /*"*/
    np->rgb.push_back(TNamedColor(r/255.0, g/255.0, b/255.0, name));
  }
  npos = 0;
  palettelistrenderer->modelChanged();
  return true;
}

TColorPalette::TNamedColor *
TColorPalette::mouse2color(TPoint m)
{
  if (!ppalette)
    return 0;

  int x, y;
  x = rectColors.origin.x;
  y = rectColors.origin.y;
  
  int first = position * (rectColors.size.width / fw);
  if (first>=ppalette->rgb.size())
    return 0;
  
  for(TPalette::iterator p = ppalette->rgb.begin()+first;
      p!=ppalette->rgb.end();
      ++p)
  {
    if (x<=m.x && m.x<=x+fw && y<=m.y && m.y<=y+fh) {
      return &(*p);
    } 
    x+=fw;
    if (x+fw>=rectColors.origin.x+rectColors.size.width) {
      x = rectColors.origin.x;
      y+=fh;
      if (y+fh>=rectColors.origin.y+rectColors.size.height) {
        break;
      }
    }
  }
  return 0;
}

void
TColorPalette::paint()
{
  if (!ppalette)
    return;

  TPen pen(this);
  if (pcolor!=0) {
    pen.setColor(*pcolor);
    pen.fillRectangle(rectCurrent);
  }
  
  TCoord x, y;
  x = rectColors.origin.x;
  y = rectColors.origin.y;
  
  int first = position * (rectColors.size.width / fw);
  if (first>=ppalette->rgb.size())
    return;
  
  for(TPalette::iterator p = ppalette->rgb.begin()+first;
      p!=ppalette->rgb.end();
      ++p)
  {
    pen.setColor(*p);
    if (&(*p)!=pcolor) {
      pen.fillRectanglePC(x+1,y+1,fw-1,fh-1);
    } else {
      pen.fillRectanglePC(x,y,fw,fh);
      pen.setColor(0,0,0);
      pen.drawRectanglePC(x-1, y-1, fw+2, fh+2);
    }
    
    x+=fw;
    if (x+fw>=rectColors.origin.x+rectColors.size.width) {
      x = rectColors.origin.x;
      y+=fh;
      if (y+fh>=rectColors.origin.y+rectColors.size.height) {
        break;
      }
    }
  }
}

void
TColorPalette::mouseLDown(const TMouseEvent &m)
{
  TNamedColor *color = mouse2color(m.pos);
  if (!color)
    return;
  colorname = color->name;
  pcolor = color;
  if (doLineColor)
    assignLineColor();
  if (doFillColor)
    assignFillColor();
  invalidateWindow();
}

void
TColorPalette::mouseMDown(const TMouseEvent &m)
{
  TNamedColor *color = mouse2color(m.pos);
  if (!color)
    return;  
  startDrag(new TDnDColor(*color), m.modifier());
}

void
TColorPalette::dropColor(const PDnDColor &drop)
{
  TNamedColor *color = mouse2color(drop->pos);
  if (!color)
    return;
//  *color = drop->rgb;
  color->set(drop->rgb.r, drop->rgb.g, drop->rgb.b);
  invalidateWindow();
}

void
TColorPalette::assignLineColor()
{
  if (preferences && pcolor)
    preferences->setStrokeColor(*pcolor);
}

void
TColorPalette::assignFillColor()
{
  if (preferences && pcolor)
    preferences->setFillColor(*pcolor);
}
