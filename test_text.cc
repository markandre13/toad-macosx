/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/utf8.hh>

//#define OLD_TOAD 1

using namespace toad;

#ifdef OLD_TOAD
struct TSize {
  TCoord width, height;
};
#endif

enum {
  CURSOR,
  SELECTION_BGN,
  SELECTION_END
};

struct TTextAttribute
{
  TTextAttribute() {
    bold=false;
    italic=false;
    face="times";
    size=12;
  }
  TTextAttribute(const TTextAttribute *a) {
    if (!a) {
      bold=false;
      italic=false;
      face="times";
      size=12;
      return;
    }
    face = a->face;
    size = a->size;
    bold = a->bold;
    italic = a->italic;
    bcolor = a->bcolor;
    fcolor = a->fcolor;
  }
  string face;
  TCoord size;
  bool bold:1;
  bool italic:1;
  TRGB bcolor;
  TRGB fcolor;
  void setFont(TFont &font);
  void setFont(TPen  &pen);
};

void
TTextAttribute::setFont(TFont &font)
{
  ostringstream fontname;
  fontname << face << ":size=" << size;
  if (bold)
    fontname << ":bold";
  if (italic)
    fontname << ":italic";
  font.setFont(fontname.str());
}

void
TTextAttribute::setFont(TPen &pen)
{
  ostringstream fontname;
  fontname << face << ":size=" << size;
  if (bold)
    fontname << ":bold";
  if (italic)
    fontname << ":italic";
  pen.setFont(fontname.str());
}

struct TTextFragment
{
  TTextFragment() {
    text=0;
  }
  TTextFragment(const TTextFragment *t): attr(t?&t->attr:0) {
    text=0;
  }
  TPoint origin;
  TSize size;
  TCoord ascent, descent;

  // the text to be displayed
  const char *text; // FIXME: should be offset
  size_t length;

  TTextAttribute attr;
};

struct TPreparedLine
{
  ~TPreparedLine();
  TPoint origin;
  TSize size;
  TCoord ascent, descent;
  
  size_t text;
  vector<TTextFragment*> fragments;
};

TPreparedLine::~TPreparedLine()
{
  for(auto p: fragments)
    delete p;
}

// marker for cursors, selections, ...
// apple pages selection
// o blue selection background
// o selection disables other background
// o selection is line height (inclusive leading)
// o cursor is text height, not line height
struct TMarker
{
  TMarker() {
    pos.set(0,0);
    idx = 0;
    line = 0;
    fragment = 0;
  }
  TPoint pos;
  size_t idx;
  TPreparedLine *line;
  TTextFragment *fragment;
};

struct TPreparedDocument
{
  ~TPreparedDocument();
  vector<TPreparedLine*> lines;
  vector<TMarker> pos;
};

TPreparedDocument::~TPreparedDocument()
{
  for(auto p: lines)
    delete p;
}

class TTextEditor
{
};

class TTextEditor2:
  public TWindow
{
    string line;

    vector<size_t> xpos;

  public:
    TTextEditor2(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      setSize(800,400);
      line = "Fröhliche.<b>Weihnachten</b>.&times;.100<sup>3</sup> &amp; &lt;tag /&gt;. <br/>"
             "\"Merry Xmas you <i>fittle</i> shit.\"<br/>"
             "Is not what we want to hear from Santa.";
      xpos.resize(3);
      xpos[CURSOR] = 0;
      xpos[SELECTION_BGN] = 0;
      xpos[SELECTION_END] = 0;
    }
    
    void paint() override;
#ifndef OLD_TOAD
    void keyDown(TKey key,char*,unsigned) override;
#else
    void keyDown(const TKeyEvent &ke) override;
#endif
    void mouseLDown(const TMouseEvent&) override;
};

// o an entity is treated like a character
// o a tag is invisible and the cursor will be placed
//   o before the character before a tag
//   o after the tag
//   o tags following one after another will be treated as one tag
// CHANGE THIS: that rule was from thunderbird but it felt weird and felt
// weird and libreoffice has this the other way arround
//
// also:
// libreoffic: <sup> & <sub>
//   raise/lower 33% (of what?)
//   relative font size 58%
inline void
taginc(const string &text, size_t *cx)
{
  int state=1;
  while(state) {
    switch(state) {
      case 1:
        switch(text[*cx]) {
          case '>':
            state=0;
            break;
          case '"':
            state=2;
            break;
        } break;
      case 2:
        switch(text[*cx]) {
          case '"':
            state=1;
            break;
        } break;
    }
    utf8inc(text, cx);
  }
}

inline void
tagdec(const string &text, size_t *cx)
{
  int state=1;
  while(state) {
    utf8dec(text, cx);
    switch(state) {
      case 1:
        switch(text[*cx]) {
          case '<':
            state=0;
            break;
          case '"':
            state=2;
            break;
        } break;
      case 2:
        switch(text[*cx]) {
          case '"':
            state=1;
            break;
        } break;
    }
  }
}

inline void
entityinc(const string &text, size_t *cx)
{
  while(text[*cx]!=';') {
    utf8inc(text, cx);
  }
  utf8inc(text, cx);
}

inline void
entitydec(const string &text, size_t *cx)
{
  while(text[*cx]!='&') {
    utf8dec(text, cx);
  }
}

inline void 
xmlinc(const string &text, size_t *cx)
{
  if (text[*cx]=='&') {
    entityinc(text, cx);
  } else
  if (text[*cx]=='<') {
    taginc(text, cx);
    utf8inc(text, cx);
  } else {
    utf8inc(text, cx);
  }
}

inline void 
xmldec(const string &text, size_t *cx)
{
  utf8dec(text, cx);
  if (text[*cx]==';') {
    entitydec(text, cx);
  }
  if (*cx>0) {
    size_t x = *cx;
    utf8dec(text, &x);
    if (text[x]=='>') {
      *cx = x;
      tagdec(text, cx);
    }
  }
}

void
prepareHTMLText(const string &text, const vector<size_t> &xpos, TPreparedDocument *prepared)
{
  if (text.empty())
    return;
//cout << "------------------------------------------------------" << endl;
  prepared->pos.resize(xpos.size());

  string face="times";
  TFont font;
  font.setFont(face);

  TCoord x=0;
  size_t x0=0, x1=0;
  size_t eol=text.size();
  int c;
  TCoord w;
  
  prepared->lines.push_back(new TPreparedLine());
  TPreparedLine *line = prepared->lines.back();
  line->text = 0;
  
  line->ascent  = 0;
  line->descent = 0;
  
  TTextFragment *fragment = 0;

  while(x0<eol) {
    x1=x0;
    while(x1<eol) {
      c = text[x1];
      if (c=='<' || c=='&')
        break;
      utf8inc(text, &x1);
    }
    if (x1>x0) {
      line->ascent  = max(line->ascent,  font.getAscent());
      line->descent = max(line->descent, font.getDescent());
//cout << "line: " << line << ", " << line->ascent << ", " << line->descent << endl;
      if (!fragment || fragment->text) {
        line->fragments.push_back(new TTextFragment(fragment));
        fragment = line->fragments.back();
        fragment->attr.setFont(font);
      }
      fragment->text = text.data()+x0;
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment->text, fragment->text);
      fragment->length = x1-x0;

      for(auto &pos: xpos) {
        if (x0<=pos && pos<x1) {
          prepared->pos[&pos-xpos.data()].pos.x    = x + font.getTextWidth(text.substr(x0, pos-x0));
          prepared->pos[&pos-xpos.data()].pos.y    = line->origin.y;
          prepared->pos[&pos-xpos.data()].line     = line;
          prepared->pos[&pos-xpos.data()].fragment = fragment;
//printf("%s:%u: x0=%zu, x1=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, x1, pos, prepared->pos[&pos-xpos.data()].x);
        }
      }
      w = font.getTextWidth(text.substr(x0, x1-x0));
      fragment->origin.x = x;
      fragment->size.width=w;
      fragment->size.height=font.getHeight();
      fragment->ascent =  font.getAscent();
      fragment->descent = font.getDescent();
      x+=w;
    }

    x0=x1;

    for(auto &pos: xpos) {
      if (x0==pos) {
        prepared->pos[&pos-xpos.data()].pos.x = x;
        prepared->pos[&pos-xpos.data()].pos.y = line->origin.y;
        prepared->pos[&pos-xpos.data()].line  = line;
        prepared->pos[&pos-xpos.data()].fragment = fragment;
//printf("%s:%u: x0=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, pos, x);
      }
    }

    if (c=='<') {
      taginc(text, &x1);
      string tag = text.substr(x0,x1-x0);
      x0=x1;
      
      if (tag=="<br/>") { // FIXME: the <br/> must be treated like a character
        line->size.width=x;
        line->size.height=line->ascent + line->descent;
        prepared->lines.push_back(new TPreparedLine());
        prepared->lines.back()->origin.y = line->origin.y + line->size.height;
        line = prepared->lines.back();
        line->text = x0;
        line->ascent  = 0;
        line->descent = 0;
        x=0;
      }
      line->fragments.push_back(new TTextFragment(fragment));
      fragment = line->fragments.back();
      if (tag=="<br/>") {
      } else
      if (tag=="<sup>") {
        fragment->attr.size-=2;
        fragment->origin.y+=2;
        fragment->attr.setFont(font);
      } else
      if (tag=="</sup>") {
        fragment->attr.size+=2;
        fragment->origin.y-=2;
        fragment->attr.setFont(font);
      } else
      if (tag=="<i>") {
        fragment->attr.italic=true;
        fragment->attr.size=48;
        fragment->attr.setFont(font);
      } else
      if (tag=="</i>") {
        fragment->attr.size=12;
        fragment->attr.italic=false;
        fragment->attr.setFont(font);
      } else
      if (tag=="<b>") {
        fragment->attr.bold = true;
        fragment->attr.setFont(font);
      } else
      if (tag=="</b>") {
        fragment->attr.bold = false;
        fragment->attr.setFont(font);
      }
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment, fragment->text, fragment->text);
    } else
    if (c=='&') {
      entityinc(text, &x1);

      if (!fragment || fragment->text) {
        line->fragments.push_back(new TTextFragment(fragment));
        fragment = line->fragments.back();
        fragment->origin.x = x;
      }

      string sub=text.substr(x0+1, x1-x0-2);
      if (sub=="times") {
        fragment->text="×";
      } else
      if (sub=="gt") {
        fragment->text=">";
      } else
      if (sub=="lt") {
        fragment->text="<";
      } else
      if (sub=="amp") {
        fragment->text="&";
      }
      if (fragment->text)
        fragment->length=strlen(fragment->text);
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment, fragment->text, fragment->text);
      w = font.getTextWidth(fragment->text, fragment->length);
      fragment->size.width+=w;
      fragment->size.height=font.getHeight();
      fragment->ascent =  font.getAscent();
      fragment->descent = font.getDescent();
      x += w;
      x0=x1;
    }
  }
  line->size.width = x;
  line->size.height=line->ascent + line->descent;
}

void
renderPrepared(TPen &pen, TPreparedDocument *document, const vector<size_t> &xpos)
{
///cout << "render-------------" << endl;
//cout << "we have " << document->lines.size() << " lines" << endl;
//  pen.setColor(164,205,1);
//  pen.setColor(0.64,0.8,1);
//  pen.setColor(0,1,1);
/*
  pen.fillRectangle(document->pos[SELECTION_BGN].pos.x,
                    document->pos[SELECTION_BGN].pos.y,
                    document->pos[SELECTION_END].pos.x - document->pos[SELECTION_BGN].pos.x,
                    pen.getHeight());
  pen.setColor(0,0,0);
*/
//  cout << "selection " << xpos[SELECTION_BGN] << "-" << xpos[SELECTION_END] << endl;
//  cout << "cursor " <<  document->pos[CURSOR].line << " " << document->pos[CURSOR].line->size.height << endl;
//  for(auto &&line: document->lines) {
  for(vector<TPreparedLine*>::const_iterator p = document->lines.begin();
      p != document->lines.end();
      ++p)
  {
    vector<TPreparedLine*>::const_iterator pe = p;
    ++pe;
    size_t textend = (pe == document->lines.end()) ? string::npos : (*pe)->text;
    TPreparedLine *line = *p;
//    cout << "  line " << line->text << " - " << textend << ", height=" << line->size.height << endl;
    
    TCoord s0=0, s1=line->size.width;
    bool s=false;
    if (line->text < xpos[SELECTION_BGN] && xpos[SELECTION_BGN] < textend) {
      s=true;
      s0 = document->pos[SELECTION_BGN].pos.x;
//      cout << "    begin" << endl;
    }
    if (line->text < xpos[SELECTION_END] && xpos[SELECTION_END] < textend) {
      s=true;
      s1 = document->pos[SELECTION_END].pos.x;
//      cout << "    end" << endl;
    }
    if (!s && xpos[SELECTION_BGN] < line->text && textend < xpos[SELECTION_END]) {
      s=true;
    }
    if (s) {
      pen.setColor(0.64,0.8,1);
      pen.fillRectangle(s0, line->origin.y, s1-s0, line->size.height);
      pen.setColor(0,0,0);
    }

    for(auto fragment: line->fragments) {
      fragment->attr.setFont(pen);
      
      TCoord y = line->origin.y + line->ascent - pen.getAscent();
      
      if (!fragment->text)
        pen.drawString(fragment->origin.x, y, "null");
      else
        pen.drawString(fragment->origin.x, y, fragment->text, fragment->length);
    }
  }

  pen.setColor(0,0,0);
/*  
  cout << "document->pos[CURSOR].line = " << document->pos[CURSOR].line << endl;
  cout << "document->pos[CURSOR].line->ascent      = " << document->pos[CURSOR].line->ascent << endl;
  cout << "document->pos[CURSOR].line->descent     = " << document->pos[CURSOR].line->descent << endl;
  cout << "document->pos[CURSOR].line->size.height = " << document->pos[CURSOR].line->size.height << endl;
*/
  // when there's no selection, draw cursor
  if (xpos[SELECTION_BGN] == xpos[SELECTION_END]) {
    TCoord cy = document->pos[CURSOR].pos.y+
                document->pos[CURSOR].line->ascent-
                document->pos[CURSOR].fragment->ascent;
    pen.drawLine(document->pos[CURSOR].pos.x, cy,
                 document->pos[CURSOR].pos.x, cy + document->pos[CURSOR].fragment->size.height);
  }
}

void
TTextEditor2::paint()
{
  TPen pen(this);
  
  pen.drawString(0,0,line);
  
  // raw text
  TCoord x = pen.getTextWidth(line.substr(0, xpos[CURSOR]));
  TCoord h = pen.getHeight();
  pen.drawLine(x,0,x,h);

  // parsed text
  pen.translate(0,72);
  TPreparedDocument prepared;
  prepareHTMLText(line, xpos, &prepared);
  renderPrepared(pen, &prepared, xpos);
}

#ifndef OLD_TOAD
void
TTextEditor2::keyDown(TKey key, char *str, unsigned modifier)
{
#else
void
TTextEditor2::keyDown(const TKeyEvent &ke)
{
  TKey key = ke.key();
  string str = ke.str();
  unsigned modifier = ke.modifier();
#endif
  // return on deadkeys
  if (key==TK_SHIFT_L || key==TK_SHIFT_R || key==TK_CONTROL_L || key==TK_CONTROL_R)
    return;

  bool move = false;
  size_t oldcursor = xpos[CURSOR];
  size_t sb = xpos[SELECTION_BGN];
  size_t se = xpos[SELECTION_END];
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
      if (xpos[CURSOR]<line.size())
        xmlinc(line, &xpos[CURSOR]);
      break;
    case TK_LEFT:
      move = true;
      if (xpos[CURSOR]>0)
        xmldec(line, &xpos[CURSOR]);
      break;
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
}

void
foo(TPreparedDocument &document, vector<size_t> xpos, TPoint &pos)
{
  cout << "-------------- foo -------------" << endl;
  for(auto line: document.lines) {
    cout << "line " << line->origin.y << ", " << line->size.height << endl;
    if (line->origin.y <= pos.y && pos.y < line->origin.y + line->size.height) {
      cout << "  found a line" << endl;
      for(auto fragment: line->fragments) {
        cout << "  fragment: " << fragment->origin.x << ", " << fragment->size.width << endl;
        if (fragment->origin.x <= pos.x && pos.x < fragment->origin.x + fragment->size.width) {
          cout << "    found a fragment" << endl;
          TFont font;
          fragment->attr.setFont(font);
          size_t i;
          for(i=1; i<fragment->length; ++i) {
            TCoord w = font.getTextWidth(fragment->text, i);
            cout << "      " << i << ": " << font.getTextWidth(fragment->text, i) << " '" <<fragment->text[i] << "' (" << (pos.x-fragment->origin.x) << ")" << endl;
            if (pos.x < fragment->origin.x + w) {
              break;
            }
          }
          --i;
          // xpos[CURSOR] = fragment->text + i;
          cout << "        found character " << i << " -> '" << fragment->text[i] << "'" << endl;
          break;
        }
      }
      break;
    }
  }
}

void
TTextEditor2::mouseLDown(const TMouseEvent &me)
{
  TPreparedDocument document;
  prepareHTMLText(line, xpos, &document);
#ifndef OLD_TOAD
  TPoint pos(me.pos.x, me.pos.y-72);
  foo(document, xpos, pos);
#else
  TPoint pos(me.x, me.y-72);
  foo(document, xpos, pos);
#endif
}



#ifndef OLD_TOAD
int 
test_text()
{
  TTextEditor2 wnd(NULL, "TextEditor II");
  toad::mainLoop();
  return 0;
}
#else
int
main(int argc, char **argv, char **envv)
{
  toad::initialize(argc, argv, envv); {
  TTextEditor2 wnd(NULL, "TextEditor II");
  toad::mainLoop();
  } toad::terminate();
  return 0;
}
#endif