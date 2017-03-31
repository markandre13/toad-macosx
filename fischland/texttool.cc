/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2007 by Mark-André Hopf <mhopf@mark13.org>
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

#include "texttool.hh"
#include "fischland.hh"

#include <toad/boolmodel.hh>
#include <toad/integermodel.hh>

using namespace fischland;

TTextTool*
TTextTool::getTool()
{
  static TTextTool* tool = 0;
  if (!tool)
    tool = new TTextTool();
  return tool;
}

void
TTextTool::cursor(TFigureEditor *fe, int x, int y)
{
  fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT]);
#if 0
  if (!path) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN]);
    return;
  }
  if (down) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN_DRAG]);
    return;
  }
  if (!path->polygon.empty() &&
       path->polygon.front().x-fe->fuzziness<=x && x<=path->polygon.front().x+fe->fuzziness &&
       path->polygon.front().y-fe->fuzziness<=y && y<=path->polygon.front().y+fe->fuzziness)
  {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN_CLOSE]);
    return;
  }
  if (!path->polygon.empty() &&
       path->polygon.back().x-fe->fuzziness<=x && x<=path->polygon.back().x+fe->fuzziness &&
       path->polygon.back().y-fe->fuzziness<=y && y<=path->polygon.back().y+fe->fuzziness)
  {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN_EDGE]);
    return;
  }
  fe->getWindow()->setCursor(fischland::cursor[CURSOR_PEN]);
#endif
}

void
TTextTool::stop(TFigureEditor *fe)
{
//cout << "stop pen" << endl;
//  fe->getWindow()->ungrabMouse();
  fe->getWindow()->setAllMouseMoveEvents(true);
  fe->getWindow()->setCursor(nullptr);
  fe->state = TFigureEditor::STATE_NONE;
#if 0
  if (path) {
/*
cout << "---------------" << endl;
for(unsigned i=0; i<path->corner.size(); ++i)
  cout << i << ": " << (unsigned)path->corner[i] << endl;
cout << "---------------" << endl;
*/
    if (path->polygon.size()>=4)
      fe->addFigure(path);
    else
      delete path;
    path = 0;
  }
#endif
  fe->invalidateWindow();
}

void
TTextTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
//  cout << "TTextTool::mouseEvent " << me.pos << " " << fe->getWindow() << " '" << fe->getWindow()->getTitle() << "' " << me.name() << endl;

  switch(me.type) {
    case TMouseEvent::ENTER:
      fe->getWindow()->grabMouse();
      break;
    case TMouseEvent::LEAVE:
      TWindow::ungrabMouse();
      return;
  }
  TPoint pos;
  fe->mouse2sheet(me.pos, &pos);
  TFigure *figure = fe->findFigureAt(pos);

// cout << me.pos << " -> " << pos << " -> " << figure << endl;

  if (!figure) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT_AREA]);
  } else
  if (dynamic_cast<TFText*>(figure)) {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT]);
  } else {
    fe->getWindow()->setCursor(fischland::cursor[CURSOR_TEXT_SHAPE]);
  }
}

void
TTextTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
#if 0
  if (ke.type != TKeyEvent::DOWN)
    return;
  if (!path)
    return;
  TPolygon &polygon = path->polygon;
  vector<byte> &corner = path->corner;
  switch(ke.key) {
    case TK_ESCAPE:
      delete path;
      path = 0;
      stop(fe);
      break;
    case TK_DELETE:
    case TK_BACKSPACE:
#if 0
      cout << "delete with " << polygon.size() 
           << " (" << polygon.size()%3 << "), "
           << corner.size() << endl;
#endif
      // fe->invalidateFigure(path);
      fe->invalidateWindow();
      if (polygon.size()==2) {
        if (corner[0] & 2) {
          polygon[1].x = polygon[0].x;
          polygon[1].y = polygon[0].y;
          corner[0] = 0;
        } else {
          polygon.erase(polygon.end()-2, polygon.end());
          corner.erase(corner.end()-1);
        }
      } else
      if (polygon.size()%3 == 1) { // 4 7 10 ...
#if 0
        if (corner[polygon.size()/3] & 1) {
          polygon[polygon.size()-2].x = polygon[polygon.size()-1].x;
          polygon[polygon.size()-2].y = polygon[polygon.size()-1].y;
          corner[polygon.size()/3] = 0;
        } else {
#endif
          polygon.erase(polygon.end()-2, polygon.end());
          corner.erase(corner.end()-1);
//        }
      } else
      if (polygon.size()%3 == 2) { // 5 8 11 ...
        polygon.erase(polygon.end()-1);
        corner.back() &= 1;
      }
#if 0
      cout << "delete with " << polygon.size() 
           << " (" << polygon.size()%3 << "), "
           << corner.size() << endl;
#endif
      break;
  }
#endif
}

bool
TTextTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
#if 0
  if (!path)
    return false;
  path->paint(pen, TFigure::EDIT);

  const TMatrix2D *m0 = pen.getMatrix();
  if (m0) {
    pen.push();
    pen.identity();
  }
  TPolygon &polygon = path->polygon;
  int i = polygon.size();

  if (i<4 || (i%3)!=1)  
    return true;
  --i;
  int x0 = polygon[i].x;
  int y0 = polygon[i].y;
  int x1 = polygon[i].x - (polygon[i-1].x - polygon[i].x);
  int y1 = polygon[i].y - (polygon[i-1].y - polygon[i].y);
  if (m0) {
    m0->map(x0, y0, &x0, &y0);
    m0->map(x1, y1, &x1, &y1);
  }
  pen.drawLine(x0, y0, x1, y1);
  pen.fillCirclePC(x1-2,y1-2,6,6);

  if (m0) {
    pen.pop();
  }
  return true;
#endif
  return false;
}
