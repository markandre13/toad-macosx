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

#include <vector>
#include <map>

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
  const char *text;
  size_t offset;
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
  vector<TMarker> marker;
  void clear();
  TPreparedLine* lineBefore(TPreparedLine*) const;
  TPreparedLine* lineAfter(TPreparedLine*) const;
};

void updateMarker(const string &text, TPreparedDocument *document, vector<size_t> &xpos);
void lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x);

TPreparedDocument::~TPreparedDocument()
{
  clear();
}

void
TPreparedDocument::clear()
{
  for(auto p: lines)
    delete p;
}

TPreparedLine*
TPreparedDocument::lineBefore(TPreparedLine *line) const
{
  TPreparedLine *before = 0;
  for(auto p: lines) {
    if (p==line)
      return before;
    before = p;
  }
  return 0;
}

TPreparedLine*
TPreparedDocument::lineAfter(TPreparedLine *line) const
{
  if (!line)
    return 0;
  TPreparedLine *before = 0;
  for(auto p: lines) {
    if (before==line)
      return p;
    before = p;
  }
  return 0;
}

class TTextEditor2:
  public TWindow
{
    string text; // the text to be displayed & edited (will be replaced by TTextModel)

    vector<size_t> xpos;        // positions relative to text
    TPreparedDocument document; // data for screen representation of the text
    
    bool updown;                // 'true' when moving the cursor up and down
    TCoord updown_x;              // the x position while moving up and down

  public:
    TTextEditor2(TWindow *parent, const string &title);
    
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
struct TTag
{
  TTag() { open=close=false; }
  string name;
  map<string, string> attribute;
  bool open:1;
  bool close:1;
};

ostream& operator<<(ostream &s, const TTag& tag) {
  s<<'<';
  if (!tag.open && tag.close)
    s<<'/';
  s<<tag.name;
  for(auto p: tag.attribute)
    if (p.second.find('"')==string::npos)
      s<<" "<<p.first<<"=\""<<p.second<<"\"";
    else
      s<<" "<<p.first<<"='"<<p.second<<"'";
  if (tag.open && tag.close)
    s<<'/';
  s<<'>';
  return s;
}

/**
 *
 * accept misformed tags but don't add misformed attributes to 'tag'
 */
inline void
taginc(const string &text, size_t *cx, TTag *tag=0)
{
  if (tag)
    tag->open=true;
  string attribute, value;
  int state=1;
  ++(*cx);
  while(state) {
    int c = text[*cx];
    switch(state) {
      case 1: // <...
        switch(c) {
          case '/':
            if (tag) {
              tag->close = true;
              tag->open = false;
            }
            state=2;
            break;
          case '>':
            state=0;
            break;
          case ' ': case '\t': case '\n': case '\r':
            state = 2;
            break;
          default:
            if (tag)
              tag->name.append(1, c);
            state=2;
        } break;
        
      case 2: // <tag...
        switch(c) {
          case '/':
            state=3;
            break;
          case '>':
            state=0;
            break;
          case ' ': case '\t': case '\n': case '\r':
            state=5;
            break;
          default:
            if (tag)
              tag->name.append(1, c);
        } break;
        
      case 3: // <tag/...>
        switch(c) {
          case '>':
            if (tag)
              tag->close=true;
            state=0;
            break;
        } break;

      case 4: // <tag...>
        switch(c) {
          case '>':
            state=0;
            break;
        } break;
        
      case 5: // <tag ...
        switch(c) {
          case '>':
            state = 0;
            break;
          case '/':
            state = 3;
            break;
          case ' ': case '\t': case '\n': case '\r':
            break;
          default:
            if (tag) {
              attribute.clear();
              value.clear();
              attribute.append(1, c);
            }
            state = 6;
        } break;

      case 6: // <tag attr...
        switch(c) {
          case '>':
            state = 0;
            break;
          case '/':
            state = 3;
            break;
          case ' ': case '\t': case '\n': case '\r':
            state=7;
            break;
          case '=':
            state=8;
            break;
          default:
            if (tag)
              attribute.append(1, c);
        } break;
        
      case 7: // <tag attr ...
        switch(c) {
          case '>':
            state = 0;
            break;
          case '/':
            state = 3;
            break;
          case ' ': case '\t': case '\n': case '\r':
            break;
          case '=':
            state = 8;
            break;
          default:
            state = 4;
        } break;
        
      case 8: // tag attr = ?
        switch(c) {
          case '>':
            state = 0;
            break;
          case '/':
            state = 3;
            break;
          case ' ': case '\t': case '\n': case '\r':
            break;
          case '"':
            state = 9;
            break;
          case '\'':
            state = 10;
            break;
          default:
            state = 4;
        } break;
        
      case 9: // <tag attr = "?
        switch(c) {
          case '"':
            state = 11;
            break;
          default:
            if (tag)
              value.append(1, c);
        } break;
        
      case 10: // <tag attr = '?
        switch(c) {
          case '\'':
            state = 11;
            break;
          default:
            if (tag)
              value.append(1, c);
        } break;
        
      case 11: // <tag attr = "value"?
        switch(c) {
          case '>':
            if (tag)
              tag->attribute[attribute]=value;
            state = 0;
            break;
          case '/':
            if (tag)
              tag->attribute[attribute]=value;
            state = 3;
            break;
          case ' ': case '\t': case '\n': case '\r':
            if (tag)
              tag->attribute[attribute]=value;
            state = 5;
            break;
          default:
            state = 4;
        }
        break;
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
    TTag tag;
    taginc(text, cx, &tag);
    if (tag.name!="br")
      utf8inc(text, cx);
  } else {
    utf8inc(text, cx);
  }
}

inline void 
xmldec(const string &text, size_t *offset)
{
//cout << "---------- xmldec at " << *offset << " '" << text[*offset] << "'" << endl;
  if (*offset==0)
    return;
  utf8dec(text, offset);
  if (text[*offset]==';') {
    entitydec(text, offset);
  }
  if (*offset>0) {
//WAIT A SEC!!! WE COULD ALSO USE THE TPREPAREDDOCUMENT STRUCTURE FOR
//THE CURSOR MOVEMENT. XMLDEC & XMLINC CAN BE RESTRICTED TO UPDATE (MOVE)
//TPREPAREDDOCUMENT
    size_t o2 = *offset;
//cout << "  o2="<<o2<< " '" << text[o2] << "'" << endl;
    if (text[o2]!='>')
      utf8dec(text, &o2);
//cout << "  o2="<<o2<< " '" << text[o2] << "'" << endl;
    if (text[o2]=='>') {
      size_t o1=o2;
//cout << "  o1="<<o1<< " '" << text[o1] << "'" << endl;
      tagdec(text, &o1);
//cout << "  o1="<<o1<< " '" << text[o1] << "'" << endl;
      size_t o0 = o1;
      TTag tag;
      taginc(text, &o0, &tag);
//      cout << "BACK: " << tag << ", offset=" << *offset << ", o2=" << o2 << ", o1=" << o1 << endl;
      if (*offset!=o2 && tag.name=="br") {
//        *offset = o2;
      } else {
        // move before tag
        *offset = o1;
      }
    }
  }
}

void
prepareHTMLText(const string &text, const vector<size_t> &xpos, TPreparedDocument *document)
{
  if (text.empty())
    return;
//cout << "------------------------------------------------------" << endl;
  string face="times";
  TFont font;
  font.setFont(face);

  TCoord x=0;
  size_t x0=0, x1=0;
  size_t eol=text.size();
  int c;
  TCoord w;
  
  document->lines.push_back(new TPreparedLine());
  TPreparedLine *line = document->lines.back();
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
      fragment->offset = x0;
      fragment->text = text.data()+x0;
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment->text, fragment->text);
      fragment->length = x1-x0;

      w = font.getTextWidth(text.substr(x0, x1-x0));
      fragment->origin.x = x;
      fragment->size.width=w;
      fragment->size.height=font.getHeight();
      fragment->ascent =  font.getAscent();
      fragment->descent = font.getDescent();
      x+=w;
    }

    x0=x1;

    if (c=='<') {
      TTag tag;
      taginc(text, &x1, &tag);
cout << "TAG: "<<tag<< endl;
      x0=x1;
      
      if (tag.open && tag.name=="br") { // FIXME: the <br/> must be treated like a character
        line->size.width=x;
        line->size.height=line->ascent + line->descent;
        document->lines.push_back(new TPreparedLine());
        document->lines.back()->origin.y = line->origin.y + line->size.height;
        line = document->lines.back();
        line->text = x0;
        line->ascent  = 0;
        line->descent = 0;
        x=0;
      }
      line->fragments.push_back(new TTextFragment(fragment));
      fragment = line->fragments.back();
      if (tag.name=="br") {
      } else
      if (tag.name=="sup") {
        if (tag.open) {
          fragment->attr.size-=2;
          fragment->origin.y+=2;
          fragment->attr.setFont(font);
        }
        if (tag.close) {
          fragment->attr.size+=2;
          fragment->origin.y-=2;
          fragment->attr.setFont(font);
        }
      } else
      if (tag.name=="i") {
        if (tag.open) {
          fragment->attr.italic=true;
          fragment->attr.size=48;
          fragment->attr.setFont(font);
        }
        if (tag.close) {
          fragment->attr.size=12;
          fragment->attr.italic=false;
          fragment->attr.setFont(font);
        }
      } else
      if (tag.name=="b") {
        if (tag.open) {
          fragment->attr.bold = true;
          fragment->attr.setFont(font);
        }
        if (tag.close) {
          fragment->attr.bold = false;
          fragment->attr.setFont(font);
        }
      }
//printf("%s:%u: fragment=%p, text=%p '%s'\n", __FILE__, __LINE__, fragment, fragment, fragment->text, fragment->text);
    } else
    if (c=='&') {
      entityinc(text, &x1);

      if (!fragment || fragment->text) {
        line->fragments.push_back(new TTextFragment(fragment));
        fragment = line->fragments.back();
        fragment->origin.x = x;
        fragment->offset = x0;
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
      s0 = document->marker[SELECTION_BGN].pos.x;
//      cout << "    begin" << endl;
    }
    if (line->text < xpos[SELECTION_END] && xpos[SELECTION_END] < textend) {
      s=true;
      s1 = document->marker[SELECTION_END].pos.x;
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
    TCoord cy = document->marker[CURSOR].pos.y+
                document->marker[CURSOR].line->ascent-
                document->marker[CURSOR].fragment->ascent;
    pen.drawLine(document->marker[CURSOR].pos.x, cy,
                 document->marker[CURSOR].pos.x, cy + document->marker[CURSOR].fragment->size.height);
  }
}

TTextEditor2::TTextEditor2(TWindow *parent, const string &title):
  TWindow(parent, title)
{
  setSize(800,400);
  text = "Fröhliche.<b>Weihnachten</b>.&times;.100<sup>3</sup> &amp; &lt;tag /&gt;. <br/>"
         "\"Merry Xmas you <i a=\"'7'\" b='\"8\"'>fittle</i> shit.\"<br/>"
         "Is not what we want to hear from Santa.";
  xpos.resize(3);
  xpos[CURSOR] = 0;
  xpos[SELECTION_BGN] = 0;
  xpos[SELECTION_END] = 0;
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
  renderPrepared(pen, &document, xpos);
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
    case TK_DOWN:
      if (!updown) {
        updown = true;
        updown_x = document.marker[CURSOR].pos.x;
      }
      lineToCursor(document.lineAfter(document.marker[CURSOR].line), text, document, xpos, updown_x);
      move = true;
      break;
    case TK_UP:
      if (!updown) {
        updown = true;
        updown_x = document.marker[CURSOR].pos.x;
      }
      lineToCursor(document.lineBefore(document.marker[CURSOR].line), text, document, xpos, updown_x);
      move = true;
      break;
    default:
      updown = false;
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
lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x)
{
//cout << "------------------------- lineToCursor ---------------------" << endl;
  if (!line)
    return;
  for(auto fragment: line->fragments) {
//    cout << "  fragment: " << fragment->origin.x << ", " << fragment->size.width << endl;
    if (x < fragment->origin.x + fragment->size.width) {
//      cout << "    found a fragment: " << endl;
      TFont font;
      fragment->attr.setFont(font);
      size_t i0, i1;
      TCoord x0=0.0, x1;
      for(i0=0; i0<fragment->length; ) {
        i1 = i0;
        utf8inc(fragment->text, &i1);
//char str[16];
//memset(str, 0, sizeof(str));
//memcpy(str, fragment->text+i0, i1-i0);
        TCoord x1 = font.getTextWidth(fragment->text, i1);
        TCoord m = (x1-x0)/2 + x0;
            
//        cout << "      " << i0 << ": " << font.getTextWidth(fragment->text, i0) << " '" <<str << "' (" << (x-fragment->origin.x) << "), m=" << m << endl;
        if (x <= fragment->origin.x + m) {
          xpos[CURSOR] = fragment->offset + i0;
//          cout << "        found character " << i0 << " -> '" << str << "'" << endl;
          return;
        }
        i0 = i1;
        x0 = x1;
      }
//      cout << "not here" << endl;
    }
  }
//  cout << "no fragment...?" << endl;
  xpos[CURSOR] = line->fragments.back()->offset +
                 line->fragments.back()->length;
}

// pos to xpos
void
positionToCursor(const string &text, TPreparedDocument &document, vector<size_t> &xpos, TPoint &pos)
{
//  cout << "-------------- foo -------------" << endl;
  for(auto line: document.lines) {
//    cout << "line " << line->origin.y << ", " << line->size.height << endl;
    if (line->origin.y <= pos.y && pos.y < line->origin.y + line->size.height) {
      lineToCursor(line, text, document, xpos, pos.x);
//      cout << "  found a line" << endl;
      break;
    }
  }
}

void
updateMarker(const string &text, TPreparedDocument *document, vector<size_t> &xpos)
{
//  cout << "--------------- updateMarker ----------------" << endl;
  document->marker.resize(xpos.size());
  for(vector<TPreparedLine*>::const_iterator pl = document->lines.begin();
      pl != document->lines.end();
      ++pl)
  {
    vector<TPreparedLine*>::const_iterator pe = pl;
    ++pe;
    size_t textend = (pe == document->lines.end()) ? string::npos : (*pe)->text;
        TPreparedLine *line = *pl;
//    cout << "  line " << line->text << " - " << textend << endl;
    for(auto &pos: xpos) {
      if (line->text<=pos && pos<textend) {
//        cout << "    pos="<<pos<<endl;
//        for(auto fragment: line->fragments) {
//          cout << "    " << fragment->offset << endl;
          vector<TTextFragment*>::const_iterator p;
          for(p = line->fragments.begin()+1;
              p != line->fragments.end();
              ++p)
          {
            if (pos<(*p)->offset) {
              break;
            }
          }
          --p;
//          cout << "    in fragment : " << (*p)->offset << ", " << (*p)->text << endl;
          TFont font;
          (*p)->attr.setFont(font);
          document->marker[&pos-xpos.data()].pos.x    = (*p)->origin.x + font.getTextWidth(text.substr((*p)->offset, pos-(*p)->offset));
          document->marker[&pos-xpos.data()].pos.y    = line->origin.y;
          document->marker[&pos-xpos.data()].line     = line;
          document->marker[&pos-xpos.data()].fragment = *p;
//printf("%s:%u: x0=%zu, x1=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, x1, pos, prepared->pos[&pos-xpos.data()].x);
      }
    }
  }
}

void
TTextEditor2::mouseLDown(const TMouseEvent &me)
{
#ifndef OLD_TOAD
  TPoint pos(me.pos.x, me.pos.y-72);
  positionToCursor(text, document, xpos, pos);
#else
  TPoint pos(me.x, me.y-72);
  positionToCursor(text, document, xpos, pos);
#endif
  updateMarker(text, &document, xpos);
  invalidateWindow();
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
