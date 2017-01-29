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
#include <toad/pen.hh>
#include <toad/utf8.hh>

#include <cassert>
#include <vector>
#include <map>

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
    case TK_DELETE: {
      // text.erase(0,1);
    } break;
    case TK_BACKSPACE: {
    } break;
    default:
      updown = false;
  }
  
  if (!move) {
    text.insert(xpos[CURSOR], str);
    xmlinc(text, &xpos[CURSOR]);
    updateMarker(text, &document, xpos);
    updatePrepared(text, &document, xpos[CURSOR], str.size());
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
#ifndef OLD_TOAD
  TPoint pos(me.pos.x, me.pos.y-72);
#else
  TPoint pos(me.x, me.y-72);
#endif
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
/*
  vector<TTagRange> ranges;
  tagrange("buh <p>Hello you <i>this <u>is</u> totally really</i> awesome.</p>", &ranges);
return 0;
*/
/*
  // test willclose
  struct test {
    const char *in;
    size_t bgn, end;
  };
  
  static test test[] = {
    { // 0         1         2         3         4         5         6         8
      // 01234567890123456789012345678901234567890123456789012345678901234567890
        "<p>Hello you <i>this <u>is</u> totally really</i> awesome.</p>",		// +1
      // 0 1          1 2     2 3  3  2               2  1         1  0
      //          ^                              <
                  0,                             31,
    }
  };
  
  for(size_t idx=0; idx<(sizeof(test)/sizeof(struct test)); ++idx) {
    cout << "----------------------------------- " << test[idx].bgn << ", " << test[idx].end << endl;
    string text = test[idx].in;
  
    size_t xpos[3];
    size_t sb = test[idx].bgn;
    size_t se = test[idx].end;
    
    for(size_t i=0; i<
  }
  

return 0;
*/
  // test isadd

  if (isadd("<b>abc</b>defg", "b", 0,  4)!=false) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //         ^   <
  if (isadd("a<b>bcd</b>efg", "b", 0,  5)!=true ) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //         ^    <
  if (isadd("<b>abc</b>defg", "b", 0,  6)!=false) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //         ^     <
  if (isadd("<b>abc</b>defg", "b", 0,  9)!=false) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //         ^        <
  if (isadd("<b>abc</b>defg", "b", 0, 10)!=false) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //         ^         <
  if (isadd("<b>abc</b>defg", "b", 0, 11)!=true ) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //         ^          <
  if (!isadd("abcdefg", "b", 4,6)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //              ^ <
  if (!isadd("abcdefg", "b", 5,6)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //               ^<
  if (isadd("ab<b>cde</b>fg", "b", 2,7)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //           ^   <
  if (isadd("ab<b>cde</b>fg", "b", 2,9)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //           ^     <
  if (isadd("ab<b>cde</b>fg", "b", 2,10)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //           ^      <
  if (isadd("ab<b>cde</b>fg", "b", 2,12)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //           ^         <
  if (!isadd("ab<b>cde</b>fg", "b", 2,13)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //            ^          <
  if (!isadd("ab<b>cde</b>fg", "b", 2,14)) { cout << "isadd: " << __LINE__ << endl; return 1; }
  //            ^           <
  cout << "checked isadd... Ok" << endl;

  //
  // test addtag
  //

  struct test {
    const char *in;	// input
    size_t bgn, end;	// selection
    const char *out;	// input after adding <b> to selection
  };

  static test test[] = {
    // touch at head
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^    <
         0,   5,
        "<b>Hello</b> <b>this totally</b> really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^     <
         0,    6,
        "<b>Hello this totally</b> really awesome."
    },
#if 0
    // ignore: end of selection is inside a tag
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^       <
         0,      8,
        "<b>Hello this totally</b> really awesome."
    },
#endif
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^        <
         0,       9,
        "<b>Hello this totally</b> really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^         <
         0,        10,
        "<b>Hello this totally</b> really awesome."
    },
    // touch at tail
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                    ^<
                            19,20,
        "Hello <b>this total</b>l<b>y</b> really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                    ^     <
                            19,   25,
        "Hello <b>this total</b>ly really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                     ^    <
                            20,   25,
        "Hello <b>this totall</b>y really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                      ^    <
                              21,  26,
        "Hello <b>this totally </b>really awesome."
    },
    
    // ...
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //               ^                 <
                       14,               32,
        "Hello <b>this totally really</b> awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally really</b> awesome.",
      //               ^      <
                       14,    21,
        "Hello <b>this </b>totally<b> really</b> awesome."
    },

    // interlace

// FIXME: needs more tests, also: how does it behave when we throw <b> into the mix?

    // enter tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this totally really</i> awesome.",			// +1
      // ^            <
         0,           13,
        "<b>Hello </b><i><b>this</b> totally really</i> awesome."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this <u>is totally</u> really</i> awesome.",		// +2
      // ^                  <
         0,                 19,
        "<b>Hello </b><i><b>this </b><u><b>is</b> totally</u> really</i> awesome."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this <u>is</u> totally really</i> awesome.",		// +1
      // ^                              <
         0,                             31,
        "<b>Hello </b><i><b>this <u>is</u> totally</b> really</i> awesome."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b>Hello <i>this is <u>not totally</u> really</i> awesome.</b>",
      //                  ^         <
                          17,       27,
        "<b>Hello </b><i><b>this </b>is <u>not <b>totally</b></u><b> really</b></i><b> awesome.</b>"
    },

    { // 0         1         2         3         4         5         6         7         8         9
      // 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
        "<b>Hello </b><i><b>this </b>is <u>not <b>totally</b></u><b> really</b></i><b> awesome.</b>",
      // ^                                                                                         <
         0,                                                                                        90,
        "<b>Hello <i>this is <u>not totally</u> really</i> awesome.</b>"
    },
    { // 0         1         2         3         4         5         6         7         8         9
      // 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
        "<b>Hello </b><i><b>this </b>is <u>not <b>totally</b></u><b> really</b></i><b> awesome.</b>",
      //                     ^                       <
                             20,                     44,
        "<b>Hello <i>this is <u>not totally</u> really</i> awesome.</b>"
    },

    // leave tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this is totally really</i> awesome.",
      //                          ^                 <
                                  25,               43,
        "Hello <i>this is totally <b>really</b></i><b> awesome</b>."	// -1
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this <u>is totally</u> really</i> awesome.",		// -2
      //                     ^                             <
                             20,                           50,
        "Hello <i>this <u>is <b>totally</b></u><b> really</b></i><b> awesome</b>."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this is <u>totally</u> really</i> awesome.",		// -1
      //               ^                                   <
                       14,                                 50,
        "Hello <i>this <b>is <u>totally</u> really</b></i><b> awesome</b>."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b>Hello <i>this <u>is not</u> totally really</i> awesome.</b>",
      //                        ^              <
                                23,            38,
        "<b>Hello </b><i><b>this </b><u><b>is </b>not</u> totally<b> really</b></i><b> awesome.</b>"
    },

    // drop tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This was a <b>bold</b> move.",
      //            ^      <
                    11,    18,
        "This was a bold move."
    },
  };

  for(size_t idx=0; idx<(sizeof(test)/sizeof(struct test)); ++idx) {
    cout << "----------------------------------- " << test[idx].bgn << ", " << test[idx].end << endl;
    string text = test[idx].in;
  
    vector<size_t> xpos;
    xpos.assign(3,0);
    xpos[SELECTION_BGN]=test[idx].bgn;
    xpos[SELECTION_END]=test[idx].end;

    string out = tagtoggle(test[idx].in, xpos, "b");

    cout << "in  : " << text << endl;
    cout << "want: " << test[idx].out << endl;
    cout << "got : " << out  << endl;
    if (out!=test[idx].out) {
      cout << "failed" << endl;
      exit(1);
    }
    cout << "Ok" << endl;
  }
  cout << "checked tagtoggle... Ok" << endl;

  string text;
  vector<size_t> xpos;
  TPreparedDocument document;
  //    0         1         2         3         4         5
  //    012345678901234567890123456789012345678901234567890
  text="This w<i>as a </i><b><i>bo</i>ld</b> move.";
  // 0, 6
  // 9, 5, italics
  // 24, 2, bold, italics
  // 30, 2 bold
  // 36, 6
  
  struct ftest {
    size_t offset, length;
    bool bold, italics;
  };
  struct t {
    const char *in;
    vector<ftest> frags;
  };
  
  t tt[] = {
    { .in = "This w<i>as a </i><b><i>bo</i>ld</b> move.",
      .frags = {
        { .offset=0, .length=6 },
        { .offset=9, .length=5, .italics=true },
        { .offset=24, .length=2, .italics=true, .bold=true },
        { .offset=30, .length=2, .bold=true },
        { .offset=36, .length=6 },
      }
    },
    { .in = "This was a <b>bold</b> move." }
  };
  
  xpos.assign(3, 0);
  prepareHTMLText(text, xpos, &document);
  
  for(auto &line: document.lines) {
    cout << "line:" << endl;
    for(auto &fragment: line->fragments) {
      cout << "  fragment: " << fragment->offset << ", " << fragment->length
           << (fragment->attr.bold?", bold":"")
           << (fragment->attr.italic?", italics":"") << endl;
    }
  }

return 0;
  TTextEditor2 wnd(NULL, "TextEditor II");
  toad::mainLoop();
  return 0;
}
