/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2015 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/vector.hh>
#include <toad/figure.hh>
#include <toad/figureeditor.hh>

using namespace toad;

static const TCoord ADDING = 0.001; // to avoid dividing by zero

// two vanishing point perspective
void TFPerspectiveTransform::prepare()
{
  A = orig[0];
  C = orig[2];

  TPoint Au = handle[0];
  TPoint Bu = handle[1];
  TPoint Cu = handle[2];
  Du = handle[3];

  // if points are the same, have to add a ADDING to avoid dividing by zero
  if (Bu.x==Cu.x) Cu.x+=ADDING;
  if (Au.x==Du.x) Du.x+=ADDING;
  if (Au.x==Bu.x) Bu.x+=ADDING;
  if (Du.x==Cu.x) Cu.x+=ADDING;
  TCoord kBC = (Bu.y-Cu.y)/(Bu.x-Cu.x);
  TCoord kAD = (Au.y-Du.y)/(Au.x-Du.x);
  TCoord kAB = (Au.y-Bu.y)/(Au.x-Bu.x);
  TCoord kDC = (Du.y-Cu.y)/(Du.x-Cu.x);

  if (kBC==kAD) kAD+=ADDING;
  E.x = (kBC*Bu.x - kAD*Au.x + Au.y - Bu.y) / (kBC-kAD);
  E.y = kBC*(E.x - Bu.x) + Bu.y;

  if (kAB==kDC) kDC+=ADDING;
  F.x = (kAB*Bu.x - kDC*Cu.x + Cu.y - Bu.y) / (kAB-kDC);
  F.y = kAB*(F.x - Bu.x) + Bu.y;

  if (E.x==F.x) F.x+=ADDING;
  TCoord kEF = (E.y-F.y) / (E.x-F.x);

  if (kEF==kAB) kAB+=ADDING;
  G.x = (kEF*Du.x - kAB*Au.x + Au.y - Du.y) / (kEF-kAB);
  G.y = kEF*(G.x - Du.x) + Du.y;

  if (kEF==kBC) kBC+=ADDING;
  H.x = (kEF*Du.x - kBC*Bu.x + Bu.y - Du.y) / (kEF-kBC);
  H.y = kEF*(H.x - Du.x) + Du.y;
}

TPoint
TFPerspectiveTransform::transform(const TPoint &I) const
{
  TCoord rG = (C.y-I.y)/(C.y-A.y);
  TCoord rH = (I.x-A.x)/(C.x-A.x);

  TPoint J((G.x-Du.x)*rG + Du.x, (G.y-Du.y)*rG + Du.y);
  TPoint K((H.x-Du.x)*rH + Du.x, (H.y-Du.y)*rH + Du.y);

  if (F.x==J.x) J.x+=ADDING;
  if (E.x==K.x) K.x+=ADDING;
  TCoord kJF = (F.y-J.y) / (F.x-J.x); //23
  TCoord kKE = (E.y-K.y) / (E.x-K.x); //12

  TCoord xKE;
  if (kJF==kKE) kKE+=ADDING;

  TPoint Iu;
  Iu.x = (kJF*F.x - kKE*E.x + E.y - F.y) / (kJF-kKE);
  Iu.y = kJF * (Iu.x - J.x) + J.y;

  return Iu;
}

TCoord
TFPerspectiveTransform::_distance(TFigureEditor *fe, TCoord mx, TCoord my)
{
  TCoord d = OUT_OF_RANGE;
  for (auto p: figures) {
    TCoord td = p->_distance(fe, mx, my);
    if (td<d)
      d=td;
  }
  return d;
}

void
TFPerspectiveTransform::translate(TCoord dx, TCoord dy)
{
  TPoint d(dx, dy);
  for(size_t i=0; i<4; ++i)
    handle[i]+=d;
  for(auto p: figures)
    p->translate(dx,dy);
}

void 
TFPerspectiveTransform::init()
{
  assert(!figures.empty());
  TRectangle r;
  TMatrix2D m;
  bool first = true;
  
  TPoint p1, p2;
  
  for(auto f: figures) {
    r = f->bounds();
    m.identity();
    for(int i=0; i<4; ++i) {
      int x, y;
      switch(i) {
        case 0:
          m.map(r.x, r.y, &x, &y);
          break;
        case 1:
          m.map(r.x+r.w, r.y, &x, &y);
          break;
        case 2:
          m.map(r.x+r.w, r.y+r.h, &x, &y);
          break;
        case 3:
          m.map(r.x, r.y+r.h, &x, &y);
          break;
      }
      if (first) {
        p1.x = p2.x = x;
        p1.y = p2.y = y;
        first = false;
      } else {
        if (p1.x>x)
          p1.x=x;
        if (p2.x<x)
          p2.x=x;
        if (p1.y>y) 
          p1.y=y;
        if (p2.y<y)
          p2.y=y;
      }
    }
  }
  
  handle[0] = p1;
  handle[1].set(p2.x, p1.y);
  handle[2] = p2;
  handle[3].set(p1.x, p2.y);
  for(int i=0; i<4; ++i)
    orig[i]=handle[i];
}

void 
TFPerspectiveTransform::paint(TPenBase &pen, EPaintType type)
{
  if (type==EDIT || type==SELECT) {
    pen.setLineColor(TColor::FIGURE_SELECTION);
    pen.setFillColor(TColor::WHITE);
    pen.drawPolygon(handle, 4);

    pen.drawString(E.x, E.y, "E"); // top/bottom
    pen.drawString(F.x, F.y, "F"); // left/right
  }

  for(auto p: figures) {
    TVectorGraphic *vg = p->getPath();
    if (!vg)
      continue;
    for(auto &painter: *vg) {
      for(auto &point: painter->path->points) {
        point = transform(point);
      }
    }
    vg->paint(pen);
  }
}

TRectangle
TFPerspectiveTransform::bounds() const
{
  TPoint p0, p1;
  p0 = p1 = E;
  if (p0.x < F.x) p0.x = F.x;
  if (p0.y < F.y) p0.y = F.y;
  if (p1.x > F.x) p1.x = F.x;
  if (p1.y > F.y) p1.y = F.y;
  for(size_t i=0; i<4; ++i) {
    if (p0.x < handle[i].x) p0.x = handle[i].x;
    if (p0.y < handle[i].y) p0.y = handle[i].y;
    if (p1.x > handle[i].x) p1.x = handle[i].x;
    if (p1.y > handle[i].y) p1.y = handle[i].y;
  }
  return TRectangle(p0.x, p0.y, p1.x-p0.x, p1.y-p0.y);
}

bool
TFPerspectiveTransform::getHandle(unsigned n, TPoint *p)
{
  if (n>=4)
    return false;
  *p = handle[n];
  return true;
}

void
TFPerspectiveTransform::translateHandle(unsigned n, TCoord x, TCoord y, unsigned modifier)
{
  if (n>=4)
    return;
  handle[n].set(x, y);
  prepare();
}

void
TFPerspectiveTransform::store(TOutObjectStream &out) const
{
  figures.store(out);
}

bool
TFPerspectiveTransform::restore(TInObjectStream &in)
{
  if (in.what == ATV_FINISHED) {
    return true;
  }
  if (figures.restore(in))
    return true;
  // FIXME: store handles
  ATV_FAILED(in);
  return false;
}
