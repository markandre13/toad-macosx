/*
 * 
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <toad/window.hh>
#include <toad/dialog.hh>
#include <toad/pen.hh>
#include <toad/textarea.hh>
#include <toad/pushbutton.hh>
#include <toad/table.hh>
#include <set>

using namespace toad;

namespace {

class TFretboard:
  public TWindow
{
    const TCoord left = 40.5;
    const TCoord top = 20.5;
    const TCoord sx = 40.0;
    const TCoord sy = 24.0;
    const TCoord r = 20.0;
    
    std::set<unsigned> marker;

  public:
    TFretboard(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
    void mouseEvent(const TMouseEvent&) override;
    
    void toggle(unsigned x, unsigned y) {
      unsigned i = x+y*256;
      auto p = marker.find(i);
      if (p!=marker.end())
        marker.erase(p);
      else
        marker.insert(i);
    }
    
    bool marked(unsigned x, unsigned y) const {
      return marker.find(x+y*256)!=marker.end();
    }
};

void
TFretboard::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN: {
      TCoord x = me.pos.x - left + sx - (sx-r)/2;
      TCoord y = me.pos.y - top + r/2;
      
      unsigned ix = x/sx;
      unsigned iy = y/sy;
 
      x-=ix*sx;
      y-=iy*sy;
      
      if (x<0 || y<0 || x>r || y>r)
        return;
        
      toggle(ix, iy);
      invalidateWindow();
    } break;
  }
}

//   spanish  english german
// p pulgar   thumb   daumen
// i index    index   zeige
// m mayor    middle  mittel
// a annular  ring    ring
// c chiquito pinkey  kleiner

/*
  C:Xx R3 M2 O I1 O
*/

struct TChord
{
};

// guitar tuning in midi note numbers
unsigned guitarTuning[6] = {
  64, 59, 55, 50, 45, 40
};

string noteName(unsigned n)
{
  // Middle C: C4 (Yamaha numbers it as octave 3: C3), MIDI 60
  // ♯ uses more space than #
  const char *name[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
  };
  unsigned octave = (n/12);
  if (octave==4)
    return name[n%12];
  return format("%s%i", name[n%12], octave-2);
}

void
TFretboard::paint()
{
  TPen pen(this);
  
  TCoord left = 40.5;
  TCoord top = 20.5;
  TCoord sx = 40.0;
  TCoord sy = 24.0;
  TCoord r = 20.0;
    
  pen.setColor(0,0,0);
  for(int i=0; i<6; ++i) {
    TCoord y = top + i*sy;
    pen.drawLine(left,y, left+22*sx, y);
  }
  
  for(int i=0; i<23; ++i) {
    TCoord r = 10.0;
    switch(i) {
      case 2:
      case 4:
      case 6:
      case 8: 
      case 14:
      case 16:
      case 18:
      case 20:
      {
        TCoord x = left+i*sx + (sx-r)/2.0;
        TCoord y = top+sy*2.5-r/2.0;
        pen.fillCircle(x,y,r,r);
      } break;
      case 11: {
        TCoord x = left+i*sx + (sx-r)/2.0;
        TCoord y = top+sy*2.5-r/2.0;
        pen.fillCircle(x,y-r*2,r,r);
        pen.fillCircle(x,y+r*2,r,r);
      } break;
    }
  }
  
  for(int i=0; i<23; ++i) {
    TCoord x = left+i*sx;
    switch(i) {
      case 0:
        pen.setLineWidth(3);
        break;
      case 1:
        pen.setLineWidth(1);
        break;
    }
    pen.drawLine(x, top, x, top+5*sy);
  }

  pen.setFont("helvetica:size=10");
  for(int i=0; i<6; ++i) {
    for(int j=0; j<23; ++j) {
      TCoord x = left+(j-1)*sx + (sx-r)/2.0;
      TCoord y = top + i*sy-r/2;

      bool selected=marked(j, i);

      if (selected)
        pen.setColor(0,0,0);
      else
        pen.setColor(1,1,1);
      pen.fillCircle(x,y,r,r);
      if (!selected)
        pen.setColor(0,0,0);
      pen.drawCircle(x,y,r,r);
      if (selected)
        pen.setColor(1,1,1);
      
      string txt = noteName(guitarTuning[i]+j);
      y += (r-pen.getHeight())/2;
      x += (r-pen.getTextWidth(txt))/2;
      
      pen.drawString(x,y,noteName(guitarTuning[i]+j));
    }
  }
}

class TMyWindow:
  public TDialog
{
  public:
    TMyWindow(TWindow *parent, const string &title):
      TDialog(parent, title)
    {
      TWindow *w = new TFretboard(this, "fretboard");
      w = new TTextArea(this, "name");
      w = new TPushButton(this, "add chord");
      w = new TPushButton(this, "delete chord");
      
      loadLayout("guitar.atv");
    }
};

} // unnamed namespace

void
test_guitar()
{
  TMyWindow wnd(NULL, "Guitar");
  toad::mainLoop();
}
