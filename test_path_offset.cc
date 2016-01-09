/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public 
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>
#include <toad/vector.hh>
#include <toad/geometry.hh>

using namespace toad;

namespace {

TPoint p[4] = {
  {  50, 100 },
  {  50,  50 },
  { 150,  50 },
  { 150, 150 }
};

class TMyWindow:
  public TWindow
{
    int handle;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
    void mouseEvent(const TMouseEvent &) override;
};

void
offset(TPoint *q, const TPoint *p, TCoord d)
{
  for(auto i=0; i<4; ++i) {
    q[i]=p[i];
  }
  
  TPoint l, o;
  
  l = p[1]-p[0];
  o=normalize(TPoint(-l.y, l.x))*d;
  q[0] += o;

  l = p[2]-p[1];
  o=normalize(TPoint(-l.y, l.x))*d;
  q[1] += o;
  q[2] += o;
}

// approximation of offset curve based on:
// Wayne Tiller, Eric Hanson, "Offsets of Two-Dimensional Profiles",
// IEEE Computer Graphics and Applications, vol.4, no.  9, pp.  36-46,
// September 1984
void
drawOffset(TPen &pen, const TPoint *p, TCoord d)
{
  pen.setColor(1,0.9,0.7);
  
  TPoint lineA[2], lineB[2], lineC[3];
  
  TPoint l, o;
  
  l = p[1]-p[0];
  o=normalize(TPoint(-l.y, l.x))*d;
  lineA[0] = p[0]+o;
  lineA[1] = p[1]+o;

  l = p[2]-p[1];
  o=normalize(TPoint(-l.y, l.x))*d;
  lineB[0] = p[1]+o;
  lineB[1] = p[2]+o;

  l = p[3]-p[2];
  o=normalize(TPoint(-l.y, l.x))*d;
  lineC[0] = p[2]+o;
  lineC[1] = p[3]+o;
  
//  pen.drawLines(lineA,2);
//  pen.drawLines(lineB,2);
//  pen.drawLines(lineC,2);
  
  TPoint q[4];
  q[0] = lineA[0];
  q[3] = lineC[1];
  
  TCoord ax, ay, bx, by, cross, dx, dy, a;
  
  ax = lineA[1].x - lineA[0].x;
  ay = lineA[1].y - lineA[0].y;
  bx = lineB[1].x - lineB[0].x;
  by = lineB[1].y - lineB[0].y;
  cross = ax*by - ay*bx;
  if (isZero(cross))
    return;
  dx = lineA[0].x - lineB[0].x;
  dy = lineA[0].y - lineB[0].y;
  a = (bx * dy - by * dx) / cross;
  q[1] = TPoint(lineA[0].x + a * ax, lineA[0].y + a * ay);
//  pen.drawCircle(q[1].x-2,q[1].y-2,5,5);

  ax = lineB[1].x - lineB[0].x;
  ay = lineB[1].y - lineB[0].y;
  bx = lineC[1].x - lineC[0].x;
  by = lineC[1].y - lineC[0].y;
  cross = ax*by - ay*bx;
  if (isZero(cross))
    return;
  dx = lineB[0].x - lineC[0].x;
  dy = lineB[0].y - lineC[0].y;
  a = (bx * dy - by * dx) / cross;
  q[2] = TPoint(lineB[0].x + a * ax, lineB[0].y + a * ay);
//  pen.drawCircle(q[2].x-2,q[2].y-2,5,5);

  pen.drawLines(q,4);

  TCoord d0 = (d<0) ? -d : d;
  
  TCoord e = 0.0;
  const TCoord step = 1.0 / 9.0;
  for(TCoord f=step; f<1.0; f+=step) {
    TCoord e0 = d0 - distance(bez2point(p, f), bez2point(q, f));
    if (e0<0)
      e0=-e0;
    if (e0>e)
      e=e0;
  }
    
  if (e>0.25) {
    TPoint r[7];
    divideBezier(p, r, 0.5);
    drawOffset(pen, r, d);
    drawOffset(pen, r+3, d);
  } else {
    pen.setColor(0,0.5,0);
    pen.drawBezier(q, 4);
  }
}

void
TMyWindow::paint()
{
  TPen pen(this);
  pen.translate(-0.5, -0.5);
  pen.drawBezier(p, 4);

  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(p, 4);
  pen.setFillColor(1,1,1);
  for(auto i=0; i<4; ++i) {
    pen.fillRectangle(p[i].x-2, p[i].y-2, 5, 5);
    pen.drawRectangle(p[i].x-2, p[i].y-2, 5, 5);
  }

  drawOffset(pen, p, -10);
  drawOffset(pen, p, 10);
}

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<8; ++i) {
        if(p[i].x-4<=me.pos.x && me.pos.x <= p[i].x+4 &&
           p[i].y-4<=me.pos.y && me.pos.y <= p[i].y+4)
        {
          handle = i;
          break;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
      p[handle] = me.pos;
      invalidateWindow();
      break;
    case TMouseEvent::LUP:
      handle = -1;
      break;
  }
}


} // unnamed namespace

void
test_path_offset()
{
  TMyWindow wnd(NULL, "test path offset");
  toad::mainLoop();
}
