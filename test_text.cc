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

class TTextEditor2:
  public TWindow
{
    string text; // the text to be displayed & edited (will be replaced by TTextModel)
    TWordProcessor wp;

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
         "Is <u>not</u> what we want to hear from Santa.";
  
//text="This was a bold move.";
//text="This w<i>as a </i><b><i>bo</i>ld</b> move.";
//text="This is a <i><b>bold</b></i> move.";
//text="<b><i>hello</i></b>";
//text="this is a really unbelievable very long line.<br/>and this <b>isn't.</b>";
//text="hello <b>you.</b>";
text="F";

  wp.init(text);
}

void
TTextEditor2::paint()
{
  TPen pen(this);
  
  pen.drawString(0,0,text);
  
  // raw text
//cout << "xpos.size()="<<xpos.size()<<endl;
//cout << "xpos[CURSOR]="<<xpos[CURSOR]<<endl;
  TCoord x = pen.getTextWidth(text.substr(0, wp.xpos[toad::wordprocessor::CURSOR]));
  TCoord h = pen.getHeight();
  pen.drawLine(x,0,x,h);

  // parsed text
  pen.translate(0,72);
  wp.renderPrepared(pen);
}

void
TTextEditor2::keyDown(const TKeyEvent &ke)
{
  if (wp.keyDown(ke))
    invalidateWindow();
}

void
TTextEditor2::mouseEvent(const TMouseEvent &me)
{
  if (wp.mouseEvent(me))
    invalidateWindow();
}

int 
test_text()
{
  TTextEditor2 wnd(NULL, "TextEditor II");
  toad::mainLoop();
  return 0;
}
