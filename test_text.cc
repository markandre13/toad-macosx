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

using namespace toad;

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
  void setFont(TPen &pen);
};

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
  TCoord x,y,width,height; // we might not nees all these

  // the text to be displayed
  const char *text;
  size_t size;

  TTextAttribute attr;
};

struct TPreparedLine
{
  vector<TTextFragment> fragments;
};

struct TPreparedDocument
{
  vector<TPreparedLine> lines;
  vector<TPoint> pos;
};

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
             "\"Merry Xmas you <i>little</i> shit.\"<br/>"
             "Is not what we want to hear from Santa.";
      xpos.resize(3);
      xpos[CURSOR] = 2;
      xpos[SELECTION_BGN] = 2;
      xpos[SELECTION_END] = 8;
    }
    
    void paint() override;
    void keyDown(TKey key,char*,unsigned) override;
};

// o an entity is treated like a character
// o a tag is invisible and the cursor will be placed
//   o before the character before a tag
//   o after the tag
//   o tags following one after another will be treated as one tag

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
    utf8dec(text, cx);
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
  } else {
    utf8inc(text, cx);
    if (text[*cx]=='<') {
      taginc(text, cx);
    }
  }
}

inline void 
xmldec(const string &text, size_t *cx)
{
  utf8dec(text, cx);
  switch(text[*cx]) {
    case '>': tagdec(text, cx); break;
    case ';': entitydec(text, cx); break;
  }
}

void
TTextEditor2::keyDown(TKey key,char*,unsigned)
{
  switch(key) {
    case TK_RIGHT:
      if (xpos[CURSOR]<line.size())
        xmlinc(line, &xpos[CURSOR]);
      break;
    case TK_LEFT:
      if (xpos[CURSOR]>0)
        xmldec(line, &xpos[CURSOR]);
      break;
    case TK_UP:
      xpos[SELECTION_END] = xpos[CURSOR];
      break;
  }
  invalidateWindow();
}

void
prepareHTMLText(TPen &pen, const string &text, const vector<size_t> &xpos, TPreparedDocument *prepared)
{
  if (text.empty())
    return;

  prepared->pos.resize(xpos.size());

  string face="times";
  pen.setFont(face);

  TCoord x=0;
  size_t x0=0, x1=0;
  size_t eol=text.size();
  int c;
  TCoord w;
  
  prepared->lines.push_back(TPreparedLine());
  TPreparedLine *line = &prepared->lines.back();
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
      if (!fragment || fragment->text) {
        line->fragments.push_back(TTextFragment(fragment));
        fragment = &line->fragments.back();
        fragment->attr.setFont(pen);
      }
      fragment->text = text.data()+x0;
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment, fragment->text, fragment->text);
      fragment->size = x1-x0;

      for(auto &pos: xpos) {
        if (x0<=pos && pos<x1) {
          prepared->pos[&pos-xpos.data()].x = x + pen.getTextWidth(text.substr(x0, pos-x0));
//printf("%s:%u: x0=%zu, x1=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, x1, pos, prepared->xpos[&pos-xpos.data()]);
        }
      }

      w = pen.getTextWidth(text.substr(x0, x1-x0));
      fragment->x = x;
      fragment->width=w;
      x+=w;
    }
//    pen.drawString(x,32, line.substr(x0, x1-x0));

    // prepare this for multiple cursors

    x0=x1;
    if (c=='<') {
      taginc(text, &x1);
      string tag = text.substr(x0,x1-x0);
      x0=x1;
      
      if (tag=="<br/>") {
        prepared->lines.push_back(TPreparedLine());
        line = &prepared->lines.back();
        x=0;
      }
      line->fragments.push_back(TTextFragment(fragment));
      fragment = &line->fragments.back();
      if (tag=="<br/>") {
      } else
      if (tag=="<sup>") {
        fragment->attr.size-=2;
        fragment->attr.setFont(pen);
      } else
      if (tag=="</sup>") {
        fragment->attr.size+=2;
        fragment->attr.setFont(pen);
      } else
      if (tag=="<i>") {
        fragment->attr.italic=true;
        fragment->attr.setFont(pen);
      } else
      if (tag=="</i>") {
        fragment->attr.italic=false;
        fragment->attr.setFont(pen);
      } else
      if (tag=="<b>") {
        fragment->attr.bold = true;
        fragment->attr.setFont(pen);
      } else
      if (tag=="</b>") {
        fragment->attr.bold = false;
        fragment->attr.setFont(pen);
      }
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment, fragment->text, fragment->text);
    } else
    if (c=='&') {
      for(auto &pos: xpos) {
        if (x0==pos) {
          prepared->pos[&pos-xpos.data()].x = x;
//printf("%s:%u: x0=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, pos, x);
        }
      }
      entityinc(text, &x1);

      if (!fragment || fragment->text) {
        line->fragments.push_back(TTextFragment(fragment));
        fragment = &line->fragments.back();
        fragment->x = x;
      }

      string sub=text.substr(x0+1, x1-x0-2);
      if (sub=="times") {
        fragment->text="×";
        fragment->size=strlen(fragment->text);
      } else
      if (sub=="gt") {
        fragment->text=">";
        fragment->size=1;
      } else
      if (sub=="lt") {
        fragment->text="<";
        fragment->size=1;
      } else
      if (sub=="amp") {
        fragment->text="&";
        fragment->size=1;
      }
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment, fragment->text, fragment->text);
      w = pen.getTextWidth(fragment->text, fragment->size);
      fragment->width+=w;
      x += w;
      x0=x1;
    }
  }
  for(auto &pos: xpos) {
    if (x0==pos) {
//printf("%s:%u: x0=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, pos, x);
      prepared->pos[&pos-xpos.data()].x = x;
    }
  }
}

void
renderPrepared(TPen &pen, TPreparedDocument *document)
{
//cout << "we have " << prepared->fragments.size() << " fragments" << endl;
  TCoord y=72;
  for(auto line: document->lines) {
    for(auto fragment: line.fragments) {
      fragment.attr.setFont(pen);
      if (!fragment.text)
        pen.drawString(fragment.x, y, "null");
      else
        pen.drawString(fragment.x, y, fragment.text, fragment.size);
    }
    y+=16;
  }
  pen.setColor(0,0,0);
  pen.drawLine(document->pos[CURSOR].x, 72,
               document->pos[CURSOR].x, 72+pen.getHeight());
  pen.drawLine(document->pos[SELECTION_BGN].x, 72+pen.getHeight(),
               document->pos[SELECTION_END].x, 72+pen.getHeight());
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
  TPreparedDocument prepared;
  prepareHTMLText(pen, line, xpos, &prepared);
  renderPrepared(pen, &prepared);
}

int 
test_text()
{
  TTextEditor2 wnd(NULL, "TextEditor II");
  toad::mainLoop();
  return 0;
}
