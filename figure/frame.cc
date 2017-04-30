/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2003 by Mark-André Hopf <mhopf@mark13.org>
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

using namespace toad;

void 
TFFrame::paint(TPenBase &pen, EPaintType type)
{
  TPoint p[3];
  pen.setColor(TColor::WHITE);
  TRectangle r(p1, p2);
  p[0].set(r.origin.x               , r.origin.y+r.size.height-1);
  p[1].set(r.origin.x+r.size.width-1, r.origin.y+r.size.height-1);
  p[2].set(r.origin.x+r.size.width-1, r.origin.y);
  pen.drawLines(p,3);

  p[0].set(r.origin.x+1             , r.origin.y+r.size.height-2);
  p[1].set(r.origin.x+1             , r.origin.y+1);
  p[2].set(r.origin.x+r.size.width-2, r.origin.y+1);
  pen.drawLines(p,3);
  pen.setColor(TColor::GRAY);
  pen.drawRectanglePC(r.origin.x,r.origin.y,r.size.width-1,r.size.height-1);

  if (!text.empty()) {
    pen.setFont(fontname);
    int fh = pen.getHeight();
    int tw = pen.getTextWidth(text);
    pen.setColor(TColor::DIALOG);
    pen.fillRectanglePC(r.origin.x+5-1, r.origin.y-fh/2, tw+2, fh);
    pen.setColor(line_color);
    pen.drawString(r.origin.x+5, r.origin.y-fh/2, text);
  }
#if 0
  if (type==EDIT) {
    pen.setColor(line_color);
    TCoord fh = getDefaultFont().getHeight();
    unsigned dx = pen.getTextWidth(text.substr(0, cx))+5;
    TCoord yp = r.y-fh/2;
    pen.drawLine(r.origin.x+dx,yp,r.origin.x+dx,yp+pen.getHeight());
  }
#endif
}

TRectangle
TFFrame::bounds() const
{
  PFont font = new TFont(fontname);
  TCoord a = font->getHeight()/2;
  TRectangle r = TFRectangle::bounds();
  r.origin.y-=a;
  r.size.height+=a;
  return r;
}

TCoord
TFFrame::distance(const TPoint &m)
{
//  cout << __PRETTY_FUNCTION__ << endl;
#if 1
  if (!text.empty()) {
    PFont font = new TFont(fontname);
    TCoord fh = font->getHeight();
    TCoord tw = font->getTextWidth(text);
    TRectangle r(min(p1.x,p2.x)+5-1, min(p1.y,p2.y)-fh/2, tw+2, fh);
    if (r.isInside(m))
      return INSIDE;
  }
#endif
  filled = false;
//cout << "is " << mx << "," << my
//     <<" in " << x << "," << y << "," << w << "," << h << endl;
  return TFRectangle::distance(m);
}

unsigned
TFFrame::stop(TFigureEditor *editor)
{
  return 0;
}

unsigned
TFFrame::keyDown(TFigureEditor *editor, TKey key, char *txt, unsigned m)
{
#if 0
//  cout << __PRETTY_FUNCTION__ << endl;
  if (key==TK_RETURN)
    return STOP;

//  int fh = TOADBase::DefaultFont().Height();
//  int tw = TOADBase::DefaultFont().TextWidth(text);
//  TRect r(x+5-1, y-fh/2, tw+2, fh);
  
  editor->invalidateFigure(this);
  unsigned result = TFText::keyDown(editor, key, txt, m);
//  r.w = TOADBase::DefaultFont().TextWidth(text)+2;
  editor->invalidateFigure(this);
  return result;
  return STOP;
#else
  return 0;
#endif
}

bool
TFFrame::getHandle(unsigned handle, TPoint *p)
{
//  cout << __PRETTY_FUNCTION__ << endl;
  return TFRectangle::getHandle(handle, p);
}

static bool flag;

unsigned 
TFFrame::mouseLDown(TFigureEditor *e, TMouseEvent &m)
{
//  cout << __PRETTY_FUNCTION__ << endl;
#if 0
  switch(e->state) {
    case TFigureEditor::STATE_START_CREATE:
cout << "start create frame " << this << endl;
flag = true;
      TFRectangle::mouseLDown(e, m);
//      TFText::mouseLDown(e, m);
      break;
      
    case TFigureEditor::STATE_CREATE:
    case TFigureEditor::STATE_EDIT:
cout << "create/edit frame " << this << endl;
      if (distance(m.pos)>RANGE) {
        e->invalidateFigure(this);
        cout << "stop" << endl;
        return STOP|REPEAT;
      }
      cout << "still in range" << endl;
      break;
      
    default:
      break;
  }
  return CONTINUE;
#else
  return 0;
#endif
}

unsigned 
TFFrame::mouseMove(TFigureEditor *e, TMouseEvent &m)
{
cout << "mouse move frame " << this << endl;
  if (flag)
    TFRectangle::mouseMove(e, m);
//  return CONTINUE;
  return 0;
}

unsigned 
TFFrame::mouseLUp(TFigureEditor *e, TMouseEvent &m)
{
cout << "mouse up frame " << this << endl;
  TFRectangle::mouseLUp(e, m);
flag = false;
//  return CONTINUE;
  return 0;
}

void
TFFrame::calcSize()
{
//  cout << __PRETTY_FUNCTION__ << endl;
}
