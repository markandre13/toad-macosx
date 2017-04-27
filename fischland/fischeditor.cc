/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2014 by Mark-André Hopf <mhopf@mark13.org>
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

#include "fischeditor.hh"

using namespace fischland;

void
TFischEditor::paint()
{
  if (!window) {
    cout << __PRETTY_FUNCTION__ << ": no window" << endl;
    return;
  }

  if (!editmodel || editmodel->modelpath.empty() ) {
    TPen pen(window);
    pen.setColor(TColor::DIALOG);
    pen.fillRectangle(0,0,window->getWidth(), window->getHeight());
    return;
  }

  TPen pen(window);

  paintDecoration(pen); // method in TFigureEditor

//  pen->identity();

  pen.setColor(window->getBackground());
  pen.fillRectangle(visible);

  TPoint origin = window->getOrigin();
  
  pen.push();
  if (mat)
    pen.multiply(mat);
//  pen.translate(visible.x, visible.y);
  paintGrid(pen);
  pen.pop();

  pen.setClipRect(visible);

  pen.translate(origin+visible.origin);

  if (mat)
    pen.multiply(mat);

  [[NSGraphicsContext currentContext] setShouldAntialias: true];

  unsigned total = 0, painted = 0, skipped = 0;

  // paint all active layers
  for(auto &p: editmodel->modelpath)
    print(pen, p, true);

  pen.setLineWidth(0);
  paintSelection(pen);
}
