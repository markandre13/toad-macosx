/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2006 by Mark-André Hopf <mhopf@mark13.org>
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
#include <toad/figureeditor.hh>

#include <toad/action.hh>
#include <toad/popupmenu.hh>

using namespace toad;

TRectangle
TFPolygon::bounds() const
{
  TRectangle r;
  polygon.getShape(&r);
  return r;
}

void 
TFPolygon::paint(TPenBase &pen, EPaintType)
{
  pen.setAlpha(alpha);
  pen.setLineColor(line_color);
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);
  if (!filled) {
    pen.drawPolygon(polygon);
  } else {
    pen.setFillColor(fill_color);
    pen.fillPolygon(polygon);
  }
  pen.setAlpha(1);
}

TCoord
TFPolygon::distance(const TPoint &pos)
{
  if (filled && polygon.isInside(pos))
    return INSIDE;

  TPolygon::const_iterator p(polygon.begin()), e(polygon.end());
  TCoord x1,y1,x2,y2;
  TCoord min = OUT_OF_RANGE, d;

  assert(p!=e);
  --e;
  assert(p!=e);
  x2=e->x;
  y2=e->y;
  ++e;
  while(p!=e) {
    x1=x2;
    y1=y2;
    x2=p->x;
    y2=p->y;
    d = distance2Line(pos.x, pos.y, x1,y1, x2,y2);
    if (d<min)
      min = d;
    ++p;
  }
  return min;
}

void 
TFPolygon::translate(TCoord dx, TCoord dy)
{
  TPolygon::iterator p(polygon.begin()), e(polygon.end());
  while(p!=e) {
    p->x+=dx;
    p->y+=dy;
    ++p;
  }
}

bool 
TFPolygon::getHandle(unsigned handle, TPoint *p)
{
  if (handle >= polygon.size())
    return false;
  *p = polygon[handle];
  return true;
}

void
TFPolygon::translateHandle(unsigned handle, TCoord x, TCoord y, unsigned m)
{
  polygon[handle]=TPoint(x, y);
}

// polygon creation
//---------------------------------------------------------------------------
unsigned 
TFPolygon::mouseLDown(TFigureEditor *editor, TMouseEvent &m)
{
  switch(editor->state) {
    case TFigureEditor::STATE_START_CREATE:
      polygon.addPoint(m.pos);
      polygon.addPoint(m.pos);
      editor->setAllMouseMoveEvents(true);
      break;
    case TFigureEditor::STATE_CREATE: {
      if (m.dblClick) {
        if (polygon.size()<4)
          return STOP|DELETE;
        polygon.erase(--polygon.end());
        return STOP;
      }
      TPolygon::iterator p(polygon.end()-2);
      if (*p != m.pos) {
        polygon.addPoint(m.pos);
        editor->invalidateFigure(this);
      }
    } break;
    default:
      break;
  }
  return CONTINUE;
}

unsigned 
TFPolygon::mouseMove(TFigureEditor *editor, TMouseEvent &m)
{
  TPolygon::iterator p(--polygon.end());
  editor->invalidateFigure(this);
  *p = m.pos;
  editor->invalidateFigure(this);
  return CONTINUE;
}

unsigned 
TFPolygon::keyDown(TFigureEditor *editor, TKey key, char *str, unsigned)
{
  editor->invalidateFigure(this);
  switch(key) {
    case TK_BACKSPACE:
    case TK_DELETE:
      if (polygon.size()<=1)
        return STOP|DELETE;
      *(polygon.end()-2) = *(polygon.end()-1);
      polygon.erase(polygon.end()-1);
      break;
    case TK_RETURN:
      return STOP;
  }
  editor->invalidateFigure(this);
  return CONTINUE;
}

class TMyPopupMenu:
  public TPopupMenu
{
  public:
    TMyPopupMenu(TWindow *p, const string &t): TPopupMenu(p, t)
    {
//cerr << "create menu " << this << endl;
    }
    ~TMyPopupMenu() {
//cerr << "delete tree " << tree << endl;
      delete tree;
    }
    
    void closeRequest() {
      TPopupMenu::closeRequest();
//cerr << "delete menu " << this << endl;
      delete this;
    }
    
    TInteractor *tree;
};

unsigned
TFPolygon::mouseRDown(TFigureEditor *editor, TMouseEvent &m)
{
  if (editor->state != TFigureEditor::STATE_NONE) {
    return NOTHING;
  }

  unsigned i=0;
  bool found=false;
  for(TPolygon::iterator p=polygon.begin();
      p!=polygon.end();
      ++p, ++i)
  {
    if (p->x-editor->fuzziness<=m.pos.x && m.pos.x<=p->x+editor->fuzziness && 
        p->y-editor->fuzziness<=m.pos.y && m.pos.y<=p->y+editor->fuzziness)
    {
      // cerr << "found handle " << i << endl;
      found = true;
      break;
    }
  }
  
  TInteractor *dummy = new TInteractor(0, "dummy interactor");
//cerr << "create tree " << dummy << endl;
  TAction *action;
  if (!found) {
    action = new TAction(dummy, "add point", TAction::ALWAYS);
    TCLOSURE4(
      action->sigClicked,
      figure, this,
      edit, editor,
      _x, m.pos.x,
      _y, m.pos.y,
      edit->invalidateFigure(figure);
      figure->insertPointNear(_x, _y);
      edit->invalidateFigure(figure);
    )
  } else {
    action = new TAction(dummy, "delete point", TAction::ALWAYS);
    TCLOSURE3(
      action->sigClicked,
      figure, this,
      edit, editor,
      _i, i,
      if (figure->polygon.size()<=2) {
        edit->deleteFigure(figure);
      } else {
        edit->invalidateFigure(figure);
        figure->deletePoint(_i);
      }
      edit->invalidateFigure(figure);
    )
  }
  TMyPopupMenu *menu;
  menu = new TMyPopupMenu(editor, "popup");
  menu->tree = dummy;
  menu->setScopeInteractor(dummy);
  menu->open(m.pos, m.modifier());
  return NOTHING;
}

void
TFPolygon::insertPointNear(TCoord x, TCoord y)
{
  _insertPointNear(x, y, true);
}

void
TFPolygon::_insertPointNear(TCoord x, TCoord y, bool filled)
{
  unsigned i=0;
  TCoord min;

  for(unsigned j=0; j < polygon.size(); j++) {
    TCoord d;
    if (j+1<polygon.size()) {
      d = distance2Line(x, y,
                        polygon[j  ].x, polygon[j  ].y,
                        polygon[j+1].x, polygon[j+1].y);
    } else {
      if (!filled)
        break;
      d = distance2Line(x, y,
                        polygon[j  ].x, polygon[j  ].y,
                        polygon[0].x, polygon[0].y);
    }
    if (j==0) {
      min = d;
    } else {
      if (d<min) {
        min = d;
        i = j;
      }
    }
  }

//cout << "i=" << i << ", polygon.size()=" << polygon.size() << endl;
  if (i+1<polygon.size()) {
    polygon.insert(polygon.begin()+i+1, TPoint(x,y));
  } else {
    polygon.insert(polygon.end(), TPoint(x,y));
  }
}

void
TFPolygon::deletePoint(unsigned i)
{
  if (polygon.size()<=2)
    return;

  polygon.erase(polygon.begin()+i);
}

// storage
//---------------------------------------------------------------------------
void
TFPolygon::store(TOutObjectStream &out) const
{
  super::store(out);
  TPolygon::const_iterator p(polygon.begin()), e(polygon.end());
  while(p!=e) {
    out.indent();
    out << p->x << ' ' << p->y;
    ++p;
  }
}

bool
TFPolygon::restore(TInObjectStream &in)
{
  static bool flag;
  static int x;
  int y;

  if (in.what == ATV_START)
    flag = false;

  if (in.attribute.empty() && ::restore(in, &y)) {
    if (flag) {
      polygon.addPoint(x, y);
    } else {
      x = y;
    }
    flag = !flag;
    return true;
  }

  if (super::restore(in))
    return true;
  ATV_FAILED(in)
  return false;
}

