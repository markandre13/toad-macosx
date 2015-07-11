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
size_t lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x);

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
    void mouseEvent(const TMouseEvent&) override;
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
      xpos[CURSOR]=lineToCursor(document.lineAfter(document.marker[CURSOR].line), text, document, xpos, updown_x);
      move = true;
      break;
    case TK_UP:
      if (!updown) {
        updown = true;
        updown_x = document.marker[CURSOR].pos.x;
      }
      xpos[CURSOR]=lineToCursor(document.lineBefore(document.marker[CURSOR].line), text, document, xpos, updown_x);
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

size_t
lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x)
{
//cout << "------------------------- lineToCursor ---------------------" << endl;
  size_t offset = 0;
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
          offset = fragment->offset + i0;
//          cout << "        found character " << i0 << " -> '" << str << "'" << endl;
          goto adjust;
        }
        i0 = i1;
        x0 = x1;
      }
//      cout << "not here" << endl;
    }
  }
//  cout << "no fragment...?" << endl;
  offset = line->fragments.back()->offset +
           line->fragments.back()->length;

adjust:
  if (offset>0) {
    size_t o0 = offset;
    utf8dec(text, &o0);
    if (text[o0]=='>') {
      tagdec(text, &o0);
      TTag tag;
      size_t o1 = o0;
      taginc(text, &o1, &tag);
      if (tag.name!="br")
        offset=o0;
    }
  }
  
  return offset;
}

// pos to xpos
size_t
positionToOffset(const string &text, TPreparedDocument &document, vector<size_t> &xpos, TPoint &pos)
{
//  cout << "-------------- foo -------------" << endl;
  for(auto line: document.lines) {
//    cout << "line " << line->origin.y << ", " << line->size.height << endl;
    if (line->origin.y <= pos.y && pos.y < line->origin.y + line->size.height) {
      return lineToCursor(line, text, document, xpos, pos.x);
//      cout << "  found a line" << endl;
      break;
    }
  }
  cout << "positionToCursor failed" << endl;
  return 0;
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

/*
  the algorithm:
    brute force for now
    assumption:
      o input has been 'normalized' (been generated with this software)
      o calculating the width of a rendered string is expensive because of all
        the glyph and kerning operations.
      o parsing just the xml isn't (as long as it is just a few pages)
    single pass possible?

  test cases:
  add:
    OVERLAP AT TAIL
      hello <b>this is</b> really awesome.
                   ^            ^
      hello <b>this is really</b> awesom.
    OVERLAP AT HEAD
      hello <b>this is</b> really awesome.
        ^        ^
      he<b>llo this is</b> really awesome.
  remove:
    OVERLAP EXACTLY
      hello <b>this is</b> really awesome.
               ^     ^
      hello this is really awsome.
    OVERLAP AT HEAD
      hello <b>this is</b> really awesome.
               ^  ^
      hello this<b> is</b> really awesome.
    OVERLAP AT TAIL
      hello <b>this is</b> really awsome.
                    ^^
      hello <b>this </b>is really awesome.
    INSIDE
      hello <b>this is</b> really awesome.
                 ^  ^
      hello <b>th</b>is i<b>s really awesome.
    CONTAINING A SINGLE ONE
      hello <b>this is</b> really awesome.
         ^                   ^
      hel<b>lo this is rea</b>lly awesome.
    CONTAINING MULTIPLE ONES EXACT OVERLAP
      hello <b>this</b> is <b>really</b> awesome.
               ^                   ^
      hello <b>this is really</b> awesome.
    OUTSIDE
      hello <b>this is really</b> awesome.
  
  then there's also tag with attributes, for font selection
    hello <font family="sans-serif">this is</font> really awesome.
                                      ^  ^
    hello <font family="sans-serif">th</font><font family="sans">is i</font><font family="sans-serif">s</font> really awesome.
    
    here i could compare the tag name for equal and the rest for not equal.
*/

struct test {
  const char *in;
  size_t bgn, end;
  const char *out;
};

/**
 * if there is an area in text not covered by tag between bgn and end, return true
 */
bool
isadd(const string &text, const string &tag, size_t bgn, size_t end)
{
//cout << "-----------------------" << endl;
  // we'll optimize this later
  size_t idx = 0;
  
  size_t x0=0, x1=0;
  size_t eol = text.size();
  int c;

  size_t xpos[3];
  xpos[SELECTION_BGN]=bgn;
  xpos[SELECTION_END]=end;
  bool inside=false;
  
  while(x0<xpos[SELECTION_END]) {
    x1=x0;
    while(x1<eol) {
      c = text[x1];
      if (c=='<' || c=='&')
        break;
      if (xpos[SELECTION_BGN] <= x1 && !inside) {
//cout << "true in text at " << x1 << endl;
        return true;
      }
      utf8inc(text, &x1);
    }
    x0=x1;
    if (c=='<') {
      taginc(text, &x1);
      string tag0 = text.substr(x0,x1-x0);
//      cout << x0 << " tag: " << tag0;
      if (tag0[1]=='/') {
        if (tag0.compare(2, tag.size(), tag)==0) {
//          cout << " close";
          inside = false;
        }
      } else {
        if (tag0.compare(1, tag.size(), tag)==0) {
//          cout << " open";
          inside = true;
//      cout << endl;
      if (xpos[SELECTION_BGN] < (x1-1) && !inside) {
//cout << "true in tag at " << x1 << endl;
        return true;
      }
        }
      }
//      cout << endl;
      x0=x1;
    } else
    if (c=='&') {
      entityinc(text, &x1);
//      cout << x0 << "entity: " << text.substr(x0,x1-x0) << endl;
    }
  }
  return false;
}


#ifndef OLD_TOAD
int 
test_text()
{
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

test test[] = {
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
      "<b>Hello </b><b>this totally</b> really awesome." // IMPROVE
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
  }
};

for(size_t idx=0; idx<(sizeof(test)/sizeof(struct test)); ++idx) {
cout << "----------------------------------- " << test[idx].bgn << ", " << test[idx].end << endl;
  string text = test[idx].in;
  
  size_t x0=0, x1=0;
  size_t eol = text.size();
  int c;

  size_t xpos[3];
  xpos[SELECTION_BGN]=test[idx].bgn;
  xpos[SELECTION_END]=test[idx].end;
  string tag="b";
  int inside=0;
  size_t inside_bgn;
  
  // add says what we want to do
  bool add = isadd(text, tag, xpos[SELECTION_BGN], xpos[SELECTION_END]);
  
  string out;
  
  while(x0<eol) {
    x1=x0;
    // text & entity loop
    while(x1<eol) {
      c = text[x1];
      
      
      if (xpos[SELECTION_BGN]==x1) {
        if (add) {
          if (!inside) {
cout << __LINE__ << ": +<"<<tag<<">, ++inside, inside=" << (inside+1) << endl;
            out += "<"+tag+">";
          } else {
cout << __LINE__ << ": ++inside, inside=" << (inside+1) << endl;
          }
          ++inside;
        } else {
          --inside;
          if (!inside) {
cout << __LINE__ << ": +</"<<tag<<">, --inside, inside=" << (inside) << endl;
            out += "</"+tag+">";
          } else {
cout << __LINE__ << endl;
          }
        }
      }
      if (xpos[SELECTION_END]==x1) {
        if (add) {
          --inside;
          if (!inside) {
cout << __LINE__ << ": +</"<<tag<<">, --inside, inside=" << inside << endl;
            out += "</"+tag+">";
          } else {
cout << __LINE__ << ": --inside, inside=" << inside << endl;
          }
        } else {
          if (!inside) {
cout << __LINE__ << ": +<"<<tag<<">, ++inside, inside=" << (inside+1) << endl;
            out += "<"+tag+">";
          } else {
cout << __LINE__ << endl;
          }
          ++inside;
        }
      }

      if (c=='<')
        break;

      cout << x1 << " :" << (char)c << " inside=" << inside << endl;
      if (c=='&') {
        size_t x2=x1;
        entityinc(text, &x2);
cout << x1 << "entity: " << text.substr(x1,x2-x1) << endl;
        out+=text.substr(x1,x2-x1);
        x1=x2;
      } else {
        size_t l = utf8charsize(text, x1);
        out += text.substr(x1, l);
        x1+=l;
      }
    }
    x0=x1;

    // tag
    if (c=='<') {
//      bool inside_sel = xpos[SELECTION_BGN] <= x0 && x0<xpos[SELECTION_END];
      taginc(text, &x1);
      string tag0 = text.substr(x0,x1-x0);
      cout << x0 << " :" << tag0 << endl;
      
//      if (!inside_sel) {
        // outside selection
        if (tag0[1]=='/' &&
            tag0.compare(2, tag.size(), tag)==0)
        {
          --inside;
cout << __LINE__ << ": --inside, inside=" << (inside) << endl;
          if (!inside) {
            out += tag0;
          }
        } else
        if (tag0[1]!='/' &&
            tag0.compare(1, tag.size(), tag)==0)
        {
cout << __LINE__ << ": ++inside, inside=" << (inside+1) << endl;
          if (!inside) {
            out += tag0;
          }
          ++inside;
        } else {
cout << __LINE__ << endl;
          out += tag0;
        }
/*
      } else
      // inside selection
      if (tag0[1]=='/' &&
          tag0.compare(2, tag.size(), tag)==0)
      {
cout << __LINE__ << endl;
        if (inside) {
          out += tag0;
        }
        --inside;
      } else
      if (tag0[1]!='/' &&
          tag0.compare(1, tag.size(), tag)==0)
      {
cout << __LINE__ << endl;
        if (!inside) {
cout << __LINE__ << endl;
          out += tag0;
        }
        ++inside;
      }
*/
      x0=x1;
    }
  }
  cout << out << endl;
  if (out!=test[idx].out) {
    cout << test[idx].out << endl;
    cout << "failed" << endl;
    exit(1);
  }
  cout << "Ok" << endl;
}

return 0;
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
