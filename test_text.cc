/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 2015-2017 by Mark-André Hopf <mhopf@mark13.org>
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

// preparing to get TTextArea to handle a subset of HTML

/**
 * TPreparedDocument		all prepared lines
 *   TPreparedLine		all text fragment in a single line
 *     TTextFragment		a text fragment with a single style to be rendered
 *       TTextAttribute		font, color, etc.
 *   TMarker			markers for cursors, selections, ...
 * TTag
 * TTextEditor2
 *
 * utility functions to move the text index 'cx'
 * void taginc(const string &text, size_t *cx, TTag *tag=0)
 * void tagdec(const string &text, size_t *cx)
 * void entityinc(const string &text, size_t *cx)
 * void entitydec(const string &text, size_t *cx)
 * void xmlinc(const string &text, size_t *cx)
 * void xmldec(const string &text, size_t *offset)
 *
 * returns the string in TTextFragment with enties begin replaced:
 * void fragment2cstr(const TTextFragment *fragment, const char *text, const char **cstr, size_t *length)
 *
 * initialize TPreparedDocument from XML text
 * void prepareHTMLText(const string &text, const vector<size_t> &xpos, TPreparedDocument *document)
 *
 * void renderPrepared(TPen &pen, const char *text, const TPreparedDocument *document, const vector<size_t> &xpos)
 *
 * void updatePrepared(const string &text, TPreparedDocument *document, size_t offset, size_t len)
 * size_t lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x)
 * size_t positionToOffset(const string &text, TPreparedDocument &document, vector<size_t> &xpos, TPoint &pos)
 * void updateMarker(const string &text, TPreparedDocument *document, vector<size_t> &xpos)
 */

#include <toad/wordprocessor.hh>

#include <toad/window.hh>
#include <toad/utf8.hh>

#include <cassert>

using namespace toad;
using namespace toad::wordprocessor;

class TTextEditor2:
  public TWindow
{
    string text; // the text to be displayed & edited (will be replaced by TTextModel)

    vector<size_t> xpos;        // positions relative to text
    TPreparedDocument document; // data for screen representation of the text
    
    bool updown;                // 'true' when moving the cursor up and down
    TCoord updown_x;            // the x position while moving up and down

  public:
    TTextEditor2(TWindow *parent, const string &title);
    void paint() override;
    void keyDown(const TKeyEvent &ke) override;
    void mouseEvent(const TMouseEvent&) override;
};

TTextEditor2::TTextEditor2(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  setSize(800,400);
  text = "Fröhliche.<b>Weihnachten</b>.&times;&times;&times;.100<sup>3</sup> &amp; &lt;tag /&gt;. <br/>"
         "\"Merry Xmas you <i a=\"'7'\" b='\"8\"'>fittle</i> shit.\"<br/>"
         "Is not what we want to hear from Santa.";
text="This was a bold move.";
text="This w<i>as a </i><b><i>bo</i>ld</b> move.";
text="This is a <i><b>bold</b></i> move.";
  xpos.assign(3, 0);
  prepareHTMLText(text, xpos, &document);
  updateMarker(text, &document, xpos);
}

void
TTextEditor2::paint()
{
  TPen pen(this);
  
  pen.drawString(0,0,text);
  
  // raw text
  TCoord x = pen.getTextWidth(text.substr(0, xpos[CURSOR]));
  TCoord h = pen.getHeight();
  pen.drawLine(x,0,x,h);

  // parsed text
  pen.translate(0,72);
  renderPrepared(pen, text.data(), &document, xpos);
}

void
TTextEditor2::keyDown(const TKeyEvent &ke)
{
  TKey key = ke.key;
  string str = ke.string;
  unsigned modifier = ke.modifier;

  // return on deadkeys
  if (key==TK_SHIFT_L || key==TK_SHIFT_R || key==TK_CONTROL_L || key==TK_CONTROL_R)
    return;

  bool move = false;
  size_t oldcursor = xpos[CURSOR];
  size_t sb = xpos[SELECTION_BGN];
  size_t se = xpos[SELECTION_END];
  
  if (modifier & MK_CONTROL) {
    switch(key) {
      case 11:
        text = tagtoggle(text, xpos, "b");
        prepareHTMLText(text, xpos, &document);
        invalidateWindow();
        break;
      case 34:
        text = tagtoggle(text, xpos, "i");
        prepareHTMLText(text, xpos, &document);
        invalidateWindow();
        break;
      default:
        cout << "control '" << key << "'\n";
    }
    return;
  }
  
  if (modifier & MK_SHIFT) {
    if (sb==se) {
      // start a new selection
      sb=se=oldcursor;
    }
  } else {
    // clear an old selection
    xpos[SELECTION_BGN] = xpos[SELECTION_END] = 0;
  }

  switch(key) {
    case TK_RIGHT:
      move = true;
      if (xpos[CURSOR]<text.size())
        xmlinc(text, &xpos[CURSOR]);
      cout << "after right at '" << text[xpos[CURSOR]] << "'" << endl;
      break;
    case TK_LEFT:
      move = true;
      if (xpos[CURSOR]>0)
        xmldec(text, &xpos[CURSOR]);
      break;
  }
  switch(key) {
    case TK_DOWN: {
      if (!updown) {
        updown = true;
        updown_x = document.marker[CURSOR].pos.x;
      }
      TPreparedLine *line = document.lineAfter(document.marker[CURSOR].line);
      if (line) {
        xpos[CURSOR]=lineToCursor(line, text, document, xpos, updown_x);
        move = true;
      }
    } break;
    case TK_UP: {
      if (!updown) {
        updown = true;
        updown_x = document.marker[CURSOR].pos.x;
      }
      TPreparedLine *line = document.lineBefore(document.marker[CURSOR].line);
      if (line) {
        xpos[CURSOR]=lineToCursor(line, text, document, xpos, updown_x);
        move = true;
      }
    } break;
    case TK_BACKSPACE:
      if (xpos[CURSOR]<=0)
        return;
      xmldec(text, &xpos[CURSOR]);
    case TK_DELETE: {
      size_t pos = xpos[CURSOR];
      if (pos>=text.size())
        return;
      textDelete(text, document, xpos);
      invalidateWindow();
      return;
    } break;
    default:
      updown = false;
  }
  
  if (!move) {
    if (xpos[CURSOR]>text.size())
      xpos[CURSOR] = text.size();
//cout << "insert: cursor="<<xpos[CURSOR]<<", text="<<text.size()<<endl;
    text.insert(xpos[CURSOR], str);
    updatePrepared(text, &document, xpos[CURSOR], str.size());
    xmlinc(text, &xpos[CURSOR]);
    updateMarker(text, &document, xpos);
    invalidateWindow();
    return;
  }
  
  if ((modifier & MK_SHIFT) && move) {
    // adjust selection
    if (sb==oldcursor)
      sb=xpos[CURSOR];
    else
      se=xpos[CURSOR];
    if (sb>se) {
      size_t a = sb; sb=se; se=a;
    }
    xpos[SELECTION_BGN] = sb;
    xpos[SELECTION_END] = se;
  }

  invalidateWindow();
  updateMarker(text, &document, xpos);
}

void
TTextEditor2::mouseEvent(const TMouseEvent &me)
{
  TPoint pos(me.pos.x, me.pos.y-72);
  switch(me.type) {
    case TMouseEvent::LDOWN:
      xpos[CURSOR] = xpos[SELECTION_BGN] = xpos[SELECTION_END] = positionToOffset(text, document, xpos, pos);
      break;
    case TMouseEvent::MOVE:
      if (me.modifier() & MK_LBUTTON) {
        size_t sb = xpos[CURSOR];
        size_t se = positionToOffset(text, document, xpos, pos);
        if (sb>se) {
          size_t a = sb; sb=se; se=a;
        }
        xpos[SELECTION_BGN] = sb;
        xpos[SELECTION_END] = se;
      }
      break;
    case TMouseEvent::LUP:
      break;
    default:
      return;
  }
  updateMarker(text, &document, xpos);
  invalidateWindow();
}

int 
test_text()
{
  TTextEditor2 wnd(NULL, "TextEditor II");
  toad::mainLoop();
  return 0;
}
