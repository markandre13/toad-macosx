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

  if (update_scrollbars) {
    // cout << "paint: update_scrollbars" << endl;
    updateScrollbars();
    update_scrollbars = false;
  }

  if (!editmodel || editmodel->modelpath.empty() ) {
    TPen pen(window);
    pen.setColor(TColor::DIALOG);
    pen.fillRectangle(0,0,window->getWidth(), window->getHeight());
    return;
  }

//  TPen scr(window);
//  scr.identity();  
  TRectangle r;
r = *window; //  scr.getClipBox(&r);
//cerr << "got screen clip box " << r << endl;
//  TBitmap bmp(r.w, r.h);
  TPen *pen = 0;
#ifdef HAVE_LIBCAIRO  
  if (useCairo) {
    pen = new TCairo(&bmp);
  } else
#endif
//  pen = new TPen(&bmp);
//#error "creating this pen causes the shift of all origins..."

#if 0
{
TPen *pen = new TPen(window);
pen->translate(1, 1);
pen->setColor(1,0,0);
pen->drawRectangle(0,0,10,10);
delete pen;
return;
}
#endif

  pen = new TPen(window);

  paintDecoration(*pen); // method in TFigureEditor

  pen->identity();

  pen->setColor(window->getBackground());
  pen->fillRectangle(visible);

#if 0
  // this translate by the position of the window itself
  // dunno why, but under Cocoa this is wrong and it also should be
  // under X11
  pen->translate(window->getOriginX()+visible.x-r.x,
                 window->getOriginY()+visible.y-r.y);
#else
  // this fails when we start to scroll
  pen->translate(window->getOriginX()+visible.x,
                 window->getOriginY()+visible.y);
#endif

//pen->setColor(1,0.5,0);
//pen->fillRectangle(100,100,200,100);

  if (mat)
    pen->multiply(mat);

//pen->setColor(1,0,0);
//TCoord f=96.0;
//pen->fillRectangle(100*f,100*f,200*f,100*f);

//TRectangle r0;
//pen->getClipBox(&r0);
//cerr << "got bitmap clipbox " << r0 << " at line " << __LINE__ << endl;
  paintGrid(*pen);
//pen->getClipBox(&r0);
//cerr << "got bitmap clipbox " << r0 << " at line " << __LINE__ << endl;

  [[NSGraphicsContext currentContext] setShouldAntialias: true];

  unsigned total = 0, painted = 0, skipped = 0;

  for(vector<TFigureModel*>::iterator p = editmodel->modelpath.begin();
      p != editmodel->modelpath.end();
      ++p)
  {  
#if 0
    for(TFigureModel::iterator q = (*p)->begin();
        q != (*p)->end();
        ++q)
    {
      ++total;
      TRectangle fr;
      getFigureShape(*q, &fr, pen->getMatrix());
      if (!r.intersects(fr)) {
        ++skipped;
        continue;
      }
      ++painted;
      TFigure::EPaintType pt = TFigure::NORMAL;
      unsigned pushs = 0;
      if (gadget==*q) {  
        pt = TFigure::EDIT;
      }  

      if ((*q)->mat) {
        pen->push();   
        pushs++;      
        pen->multiply( (*q)->mat );
      }
     
      if (gadget!=*q && selection.find(*q)!=selection.end()) {
        pt = TFigure::SELECT;
      }
      
//cerr << "got bitmap clipbox " << r0 << " at line " << __LINE__ << endl;
      (*q)->paint(*pen, pt);

      while(pushs) {
        pen->pop();  
        pushs--;    
      }

    }
#else
    print(*pen, *p, true);
#endif
  }
//pen->getClipBox(&r0);
//cerr << "got bitmap clipbox " << r0 << " at line " << __LINE__ << endl;
//cerr << "painted " << painted << " out of " << total << " in " << r << endl;
  pen->setLineWidth(0);
  paintSelection(*pen);

//  scr.drawBitmap(r.x,r.y, &bmp);   
//  paintDecoration(scr);
  delete pen;
}
