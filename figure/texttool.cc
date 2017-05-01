/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figure/texttool.hh>
#include <toad/figureeditor.hh>

using namespace toad;

void
TTextTool::stop(TFigureEditor *fe)
{
  text = nullptr;
  TWindow::ungrabMouse();
  fe->getWindow()->setCursor(nullptr);
  fe->invalidateWindow();
}

void
TTextTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
//  cout << "TTextTool::mouseEvent " << me.pos << " " << fe->getWindow() << " '" << fe->getWindow()->getTitle() << "' " << me.name() << endl;
  TPoint pos;
  fe->mouse2sheet(me.pos, &pos);

  TFText *textUnderMouse = nullptr;
  TFigure *figureUnderMouse = fe->findFigureAt(pos);
  if (!figureUnderMouse) {
    fe->getWindow()->setCursor(cursor[CURSOR_TEXT_AREA]);
  } else {
    textUnderMouse = dynamic_cast<TFText*>(figureUnderMouse);
    if (textUnderMouse) {
      fe->getWindow()->setCursor(cursor[CURSOR_TEXT]);
    } else {
      fe->getWindow()->setCursor(cursor[CURSOR_TEXT_SHAPE]);
    }
  }
  
  switch(me.type) {
    case TMouseEvent::ENTER:
      fe->getWindow()->grabMouse();
      break;
    case TMouseEvent::LEAVE:
      TWindow::ungrabMouse();
      return;
    case TMouseEvent::LDOWN:
      if (textUnderMouse && textUnderMouse != text) {
        textUnderMouse->startInPlace();
        fe->clearSelection();
      }
      if (!textUnderMouse && text)
        fe->invalidateFigure(text);
      text = textUnderMouse;
      if (text) {
        TMouseEvent me2(me, pos);
        text->mouseEvent(me2);
        fe->invalidateFigure(text);
      } else {
        TPoint posInGrid;
        fe->sheet2grid(pos, &posInGrid);
        text = new TFText(posInGrid.x, posInGrid.y, "", figureUnderMouse);
        if (figureUnderMouse) {
          TFigureEditor::relatedTo[figureUnderMouse].insert(text);
        }
        fe->getAttributes()->setAllReasons();
        text->setAttributes(fe->getAttributes());
        fe->addFigure(text);
        text->startInPlace();
        fe->clearSelection();
        fe->invalidateFigure(text);
      }
      break;
  }
}

void
TTextTool::keyEvent(TFigureEditor *fe, const TKeyEvent &ke)
{
  if (!text)
    return;

  switch(ke.type) {
    case TKeyEvent::DOWN:
      text->keyDown(fe, ke.key, const_cast<char*>(ke.string.c_str()), ke.modifier);
      break;
  }
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

static const char cursorData[4][32][32+1] = {
  {
  // CURSOR_TEXT
  // 0        1         2         3
  // 12345678901234567890123456789012
    "     ##   ##                    ",
    "    #..# #..#                   ",
    "     ##.#.##                    ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "      ##.##                     ",
    "     #.....#                    ",
    "      ##.##                     ",
    "       #.#                      ",
    "     ##. .##                    ",
    "    #..# #..#                   ",
    "     ##   ##                    ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_TEXT_AREA
  // 0        1         2         3
  // 12345678901234567890123456789012
    "     ##   ##                    ",
    " #  #..# #..#  #                ",
    "#.#.#.#.#.#.#.#.#               ",
    " # # # #.# # # #                ",
    "#.#    #.#    #.#               ",
    " #     #.#     #                ",
    "#.#    #.#    #.#               ",
    " #     #.#     #                ",
    "#.#    #.#    #.#               ",
    " #     #.#     #                ",
    "#.#    #.#    #.#               ",
    " #    ##.##    #                ",
    "#.#  #.....#  #.#               ",
    " #     #.##    #                ",
    "#.#    #.#    #.#               ",
    " # # ##.#.#  # #                ",
    "#.#.#...#...#.#.#               ",
    "   # ### ### # #                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_TEXT_SHAPE
  // 0        1         2         3
  // 12345678901234567890123456789012
    "      ##   ##                   ",
    "     #..# #..#                  ",
    "     #.#.#.#.#                  ",
    "    # # #.# # #                 ",
    "   #.#  #.#  #.#                ",
    "  # #   #.#   # #               ",
    " #.#    #.#    #.#              ",
    " ##     #.#     ##              ",
    "#.#     #.#     #.#             ",
    " #      #.#      #              ",
    "#.#     #.#     #.#             ",
    " ##    ##.##    ##              ",
    " #.#  #.....#  #.#              ",
    "  # #  ##.##  # #               ",
    "   #.# ##.## #.#                ",
    "    # #..#..# #                 ",
    "     #..#.#..#                  ",
    "      ## # ##                   ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  },{
  // CURSOR_TEXT_PATH
  // 0        1         2         3
  // 12345678901234567890123456789012
    "     ## # ##                    ",
    "    #..#.#..#                   ",
    "     ##.#.##                    ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#                      ",
    "       #.#      #               ",
    "       #.#    ##.#              ",
    "       #.#  ##.##               ",
    "      ##.###.##                 ",
    "     #......#                   ",
    "   ##.##.###                    ",
    " ##.## #.#                      ",
    "#.## ##.#.##                    ",
    " #  #...#...#                   ",
    "     ### ###                    ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "
  }
};

TCursor* TTextTool::cursor[4];

TTextTool*
TTextTool::getTool()
{
  static TTextTool* tool = nullptr;
  if (!tool) {
    cursor[CURSOR_TEXT]       = new TCursor(cursorData[0], 9, 12);
    cursor[CURSOR_TEXT_AREA]  = new TCursor(cursorData[1], 9, 12);
    cursor[CURSOR_TEXT_SHAPE] = new TCursor(cursorData[2], 9, 12);
    cursor[CURSOR_TEXT_PATH]  = new TCursor(cursorData[3], 9, 12);
    tool = new TTextTool();
  }
  return tool;
}

