/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2017 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/utf8.hh>
#include <toad/wordprocessor.hh>

using namespace std;
using toad::utf8inc;
using toad::utf8dec;
using toad::utf8charsize;
using toad::TPoint;

#define DBG(CMD)

namespace toad::wordprocessor {

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

TPreparedLine::~TPreparedLine()
{
  for(auto &&p: fragments)
    delete p;
}

TPreparedDocument::~TPreparedDocument()
{
  clear();
}

void
TPreparedDocument::clear()
{
  for(auto &&p: lines)
    delete p;
  lines.clear();
  marker.clear();
}

TPreparedLine*
TPreparedDocument::lineBefore(TPreparedLine *line) const
{
  TPreparedLine *before = nullptr;
  for(auto &&p: lines) {
    if (p==line)
      return before;
    before = p;
  }
  return nullptr;
}

TPreparedLine*
TPreparedDocument::lineAfter(TPreparedLine *line) const
{
  if (!line)
    return nullptr;
  TPreparedLine *before = nullptr;
  for(auto &&p: lines) {
    if (before==line)
      return p;
    before = p;
  }
  return nullptr;
}

// o an entity is treated like a character
// o a tag is invisible and the cursor will be placed
//   o before the character before a tag
//   o after the tag
//   o tags following one after another will be treated as one tag
// CHANGE THIS: that rule was from thunderbird but it felt weird and felt
// weird and libreoffice has this the other way arround
//
// also:
// libreoffice: <sup> & <sub>
//   raise/lower 33% (of what?)
//   relative font size 58%
string TTag::str() const
{
  string s("<");
  if (!open && close)
    s+='/';
  s+=name;
  for(auto &&p: attribute)
    if (p.second.find('"')==string::npos)
      s+=" "+p.first+"=\""+p.second+"\"";
    else
      s+=" "+p.first+"='"+p.second+"'";
  if (open && close)
    s+='/';
  s+='>';
  return s;
}

/**
 * move 'cx' onto the next character the user sees within 'text'
 *
 *
 * accept misformed tags but don't add misformed attributes to 'tag'
 */
void
taginc(const string &text, size_t *cx, TTag *tag)
{
  if (tag)
    tag->open=true;
  string attribute, value;
  int state=1;
  ++(*cx);
  while(*cx<text.size() && state) {
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

void
tagdec(const string &text, size_t *cx)
{
  int state=1;
  while(*cx>0 && state) {
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

void
entityinc(const string &text, size_t *cx)
{
  while(*cx < text.size() && text[*cx]!=';') {
    utf8inc(text, cx);
  }
  utf8inc(text, cx);
}

void
entitydec(const string &text, size_t *cx)
{
  while(*cx > 0 && text[*cx]!='&') {
    utf8dec(text, cx);
  }
}

void
xmlinc(const string &text, size_t *cx)
{
//  cout << "--------------- '" << text << "', " << *cx << endl;
  TTag tag;
  while(true) {
    switch(text[*cx]) {
      case '<':
        taginc(text, cx, &tag);
        // self-closing tags are treated like characters,
        if (tag.open && tag.close)
          return;
        break;
      case '&':
        entityinc(text, cx);
        return;
      default:
        utf8inc(text, cx);
        return;
    }
  }
}

void 
xmldec(const string &text, size_t *offset)
{
//cerr << "---------- xmldec at " << *offset << " '" << text[*offset] << "'" << endl;
  if (*offset==0)
    return;

  // one character back
  utf8dec(text, offset);
  if (*offset==0)
    return;
  if (text[*offset]==';') {
    entitydec(text, offset);
  } else
  if (text[*offset]=='>') {
    tagdec(text, offset);
  }

  // another character back and if it's also a character (or entity or open-close tag), return
  while(*offset>0) {
    size_t offset0 = *offset;
    utf8dec(text, &offset0);
    if (text[offset0]==';') {
      return;
    } else
    if (text[offset0]!='>') {
      return;
    }

    tagdec(text, &offset0);
    size_t offset1 = offset0;
    TTag tag;
    taginc(text, &offset1, &tag);
    if (tag.open && tag.close) {
      return;
    }

    *offset = offset0;
  }
}

void
fragment2cstr(const TTextFragment *fragment, const char *text, const char **cstr, size_t *length)
{
  if (fragment->offset == TTextFragment::npos) {
    *cstr = "";
    *length = 0;
    return;
  }

  const char *p = text + fragment->offset;
  if (p[0]=='&') {
    if (fragment->length == 4) {
      if (memcmp(p, "&gt;", 4)==0) {
        *cstr=">";
        *length=1;
        return;
      }
      if (memcmp(p, "&lt;", 4)==0) {
        *cstr="<";
        *length=1;
        return;
      }
    }
    if (fragment->length == 5) {
      if (memcmp(p, "&amp;", 5)==0) {
        *cstr="&";
        *length=1;
        return;
      }
    }
    if (fragment->length == 7) {
      if (memcmp(p, "&times;", 7)==0) {
        *cstr="×";
        *length=strlen(*cstr);
        return;
      }
    }
  }

  *cstr = p;
  *length = fragment->length;
}

/**
 * initialize TPreparedDocument from XML text
 *
 * @param text		XML input
 * @param xpos		unused
 * @param document	output
 */
void
prepareHTMLText(const string &text, const vector<size_t> &xpos, TPreparedDocument *document)
{
  document->clear();
//cout << "prepareHTMLText ------------------------------------------------------" << endl;
  string face="times";
  TFont font;
  font.setFont(face);

  TCoord x=0;
  size_t x0=0, x1=0;
  size_t eol=text.size();
  int c;
  TCoord w;
  
//cout << "; new line" << endl;
  document->lines.push_back(new TPreparedLine());
  TPreparedLine *line = document->lines.back();
  line->offset = 0;
  
  line->ascent  = 0;
  line->descent = 0;
  
  TTextFragment *fragment = 0;
 
  if (text.empty()) {
    line->fragments.push_back(new TTextFragment(fragment));
    fragment = line->fragments.back();
    fragment->attr.setFont(font);
    fragment->origin.x = 0;
    fragment->origin.y = 0;
    fragment->size.width = 0;
    fragment->size.height = font.getHeight();
    fragment->offset = 0;
    fragment->length = 0;
    line->ascent = fragment->ascent = font.getAscent();
    line->descent = fragment->descent = font.getDescent();
    line->size.height = line->ascent + line->descent;
    line->size.width = 0;
    return;
  }

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
      if (!fragment || fragment->offset != TTextFragment::npos) {
//if (!fragment)
//  printf("%s:%u: add fragment=%p\n", __FILE__, __LINE__, fragment);
//else
//  printf("%s:%u: add fragment=%p, offset=%zu, length=%zu\n", __FILE__, __LINE__, fragment, fragment->offset, fragment->length);
        line->fragments.push_back(new TTextFragment(fragment));
        fragment = line->fragments.back();
        fragment->attr.setFont(font);
      }
      fragment->offset = x0;
      fragment->length = x1-x0;

      const char *cstr;
      size_t size;
      fragment2cstr(fragment, text.data(), &cstr, &size);
      w = font.getTextWidth(cstr, size);
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
//cout << "TAG: "<<tag<<", x0="<<x0<<endl;
      
      if (tag.open && tag.name=="br") { // FIXME: the <br/> must be treated like a character
        if (fragment->offset == TTextFragment::npos) {
          fragment->offset = x0;
          fragment->length = 0;
          line->ascent = fragment->ascent = font.getAscent();
          line->descent = fragment->descent = font.getDescent();
          fragment->size.height = font.getHeight();
        }
        line->size.width=x;
        line->size.height=line->ascent + line->descent;
//cout << "; new line" << endl;
        document->lines.push_back(new TPreparedLine());
        document->lines.back()->origin.y = line->origin.y + line->size.height;
        line = document->lines.back();
        line->offset  = x1;
        line->ascent  = 0;
        line->descent = 0;
        x=0;
      }

//cout << "; line->fragments.size()=" << line->fragments.size() << endl;
      // when a new line begins with a tag, insert an empty fragment so that
      // text can be inserted before the tag
      if (line->fragments.size()==1 && fragment->offset==TTextFragment::npos) {
        fragment = line->fragments.back();
        fragment = line->fragments.back();
        fragment->offset = x0;
        fragment->length = 0;
//cout << "; x0="<<x0<<", fragment->offset="<<fragment->offset<<", fragment->length="<<fragment->length<<endl;
      }

      x0=x1;


      // FIXME: need an attr outside the line to cope with line wraps, by using a 'next fragment'
      if (line->fragments.empty() || line->fragments.back()->offset != TTextFragment::npos)  {
//if (fragment)
//  printf("%s:%u: add fragment=%p, offset=%zu, length=%zu\n", __FILE__, __LINE__, fragment, fragment->offset, fragment->length);
//else
//  printf("%s:%u: add fragment=%p\n", __FILE__, __LINE__, fragment);
        line->fragments.push_back(new TTextFragment(fragment));
      }

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
    } else
    if (c=='&') {
      entityinc(text, &x1);

      if (!fragment || fragment->offset != TTextFragment::npos) {
//printf("%s:%u: add fragment=%p, offset=%zu, length=%zu\n", __FILE__, __LINE__, fragment, fragment->offset, fragment->length);
        line->fragments.push_back(new TTextFragment(fragment));
        fragment = line->fragments.back();
        fragment->origin.x = x;
        fragment->offset = x0;
      }
      fragment->length=x1-x0;
      const char *cstr;
      size_t size;
      fragment2cstr(fragment, text.data(), &cstr, &size);
      w = font.getTextWidth(cstr, size);
      fragment->size.width+=w;
      fragment->size.height=font.getHeight();
      fragment->ascent =  font.getAscent();
      fragment->descent = font.getDescent();
      x += w;
      x0=x1;
    }
  }
  if (fragment->offset == TTextFragment::npos) {
    fragment->offset = x0;
    fragment->length = 0;
    line->ascent = fragment->ascent = font.getAscent();
    line->descent = fragment->descent = font.getDescent();
    fragment->size.height = font.getHeight();
  }
  line->size.width = x;
  line->size.height=line->ascent + line->descent;
}

void
renderPrepared(TPen &pen, const char *text, const TPreparedDocument *document, const vector<size_t> &xpos)
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
//    if ( (*p)->fragments[0]->offset == string::npos)
//      continue;
  
    size_t textend = ((p+1) == document->lines.end()) ? string::npos : (*(p+1))->offset;
    TPreparedLine *line = *p;
//    cout << "  line " << line->text << " - " << textend << ", height=" << line->size.height << endl;
    
    TCoord s0=0, s1=line->size.width;
    bool s=false;
    if (line->offset < xpos[SELECTION_BGN] && xpos[SELECTION_BGN] < textend) {
      s=true;
      s0 = document->marker[SELECTION_BGN].pos.x;
//      cout << "    begin" << endl;
    }
    if (line->offset < xpos[SELECTION_END] && xpos[SELECTION_END] < textend) {
      s=true;
      s1 = document->marker[SELECTION_END].pos.x;
//      cout << "    end" << endl;
    }
    if (!s && xpos[SELECTION_BGN] < line->offset && textend < xpos[SELECTION_END]) {
      s=true;
    }
    if (s) {
      pen.setColor(0.64,0.8,1);
      pen.fillRectangle(s0, line->origin.y, s1-s0, line->size.height);
      pen.setColor(0,0,0);
    }

    for(auto &&fragment: line->fragments) {
      fragment->attr.setFont(pen);
      
      TCoord y = line->origin.y + line->ascent - pen.getAscent();
      
      const char *cstr;
      size_t size;
      fragment2cstr(fragment, text, &cstr, &size);
      pen.drawString(fragment->origin.x, y, cstr, size);
    }
  }

  pen.setColor(0,0,0);

  // when there's no selection, draw cursor
  if (xpos[SELECTION_BGN] == xpos[SELECTION_END]) {
    TCoord cy = document->marker[CURSOR].pos.y+
                document->marker[CURSOR].line->ascent-
                document->marker[CURSOR].fragment->ascent;
    pen.drawLine(document->marker[CURSOR].pos.x, cy,
                 document->marker[CURSOR].pos.x, cy + document->marker[CURSOR].fragment->size.height);
  }
}

/**
 * update TPreparedDocument after text had been added and removed
 *
 * \param text		the text which had been modified
 * \param document	the prepared document data for text
 * \param offset	location in text where the modification took place
 * \param len		number of bytes added/removed
 * for now this function only handles inserting text to an existing fragment
 * todo
 * \li simple inserts must also create new text fragments when between "near" entities/tags
 * \li insert text with tags and entities
 * \li the two points above suggest reusing code from prepareHTML
 *     (keep in mind that stuff like prepareHTML is intended to be plugable,
 *     so that we could also handle syntax highlightning for programming
 *     languages)
 * \li remove
 * \li replace
 */
void
updatePrepared(const string &text, TPreparedDocument *document, size_t offset, ssize_t len)
{
  bool afterOffset = false;
  TCoord diffW = 0.0;

//  cout << "updatePrepared: " << offset << ", " << len << endl;
  for(vector<TPreparedLine*>::const_iterator p = document->lines.begin();
      p != document->lines.end();
      ++p)
  {
    TPreparedLine *line = *p;
    if (afterOffset) {
//cout << ": afterOffset" << endl;
      line->offset += len;
      for(auto &&fragment: line->fragments) {
        fragment->offset+=len;
      }
      continue;
    }
    size_t textend = ((p+1) == document->lines.end()) ? string::npos : (*(p+1))->offset;
//cout << ": line->offset="<<line->offset<<", offset="<<offset<<", textend="<<textend<<endl;
    if (line->offset <= offset && offset < textend) {
//cout << ": offset is within line" << endl;
      for(auto &&fragment: line->fragments) {
//cout << ":   fragment: " << fragment->offset << ", " << fragment->length
//     << ", \"" << (fragment->offset>=text.size() ? "" : text.substr(fragment->offset, fragment->length)) << "\" "
//     << (fragment->attr.bold?", bold":"")
//     << (fragment->attr.italic?", italics":"") << endl;
        if (afterOffset) {
          fragment->offset += len;
          fragment->origin.x += diffW;
        } else
        if (fragment->offset <= offset && offset <= fragment->offset + fragment->length) {
//cout << ":    adjust fragment" << endl;
          fragment->length += len;

          const char *cstr;
          size_t size;
          fragment2cstr(fragment, text.data(), &cstr, &size);
    
          TFont font;
          fragment->attr.setFont(font);
          
          diffW = font.getTextWidth(cstr, size) - fragment->size.width;
          fragment->size.width += diffW;
          
          afterOffset = true;
        }
      }
    }
  }
}

/**
 * convert xpos to document->marker
 */
void
updateMarker(const string &text, TPreparedDocument *document, vector<size_t> &xpos)
{
//  cout << "--------------- updateMarker ----------------" << endl;
  
  document->marker.resize(xpos.size());
  for(vector<TPreparedLine*>::const_iterator pl = document->lines.begin();
      pl != document->lines.end();
      ++pl)
  {
    size_t textend = ((pl+1) == document->lines.end()) ? string::npos : (*(pl+1))->offset;
    TPreparedLine *line = *pl;
//    cout << "  line " << line->offset << " - " << textend << endl;
    for(auto &&pos: xpos) {
      if (line->offset<=pos && pos<textend) {
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
//          cout << "    in fragment : " << (*p)->offset << endl;
          TFont font;
          (*p)->attr.setFont(font);
          
          const char *cstr;
          size_t size;
          fragment2cstr(*p, text.data(), &cstr, &size);

          document->marker[&pos-xpos.data()].pos.x    = (*p)->origin.x + font.getTextWidth(cstr, pos-(*p)->offset);
          document->marker[&pos-xpos.data()].pos.y    = line->origin.y;
          document->marker[&pos-xpos.data()].line     = line;
          document->marker[&pos-xpos.data()].fragment = *p;
//printf("%s:%u: x0=%zu, x1=%zu, pos=%zu, x=%f\n", __FILE__, __LINE__, x0, x1, pos, prepared->pos[&pos-xpos.data()].x);
      }
    }
  }
}

size_t
lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x)
{
  assert(line);
//cout << "------------------------- lineToCursor ---------------------" << endl;
  size_t offset = 0;
  for(auto &&fragment: line->fragments) {
//    cout << "  fragment: " << fragment->origin.x << ", " << fragment->size.width << endl;
    if (x < fragment->origin.x + fragment->size.width) {
//      cout << "    found a fragment: " << endl;
      TFont font;
      fragment->attr.setFont(font);
      size_t i0, i1;
      TCoord x0=0.0, x1;
      const char *cstr;
      size_t size;
      fragment2cstr(fragment, text.data(), &cstr, &size);
      for(i0=0; i0<size; ) {
        i1 = i0;
        utf8inc(cstr, &i1);
//char str[16];
//memset(str, 0, sizeof(str));
//memcpy(str, fragment->text+i0, i1-i0);
        
        TCoord x1 = font.getTextWidth(cstr, i1);
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
  for(auto &&line: document.lines) {
//    cout << "line " << line->origin.y << ", " << line->size.height << endl;
    if (line->origin.y <= pos.y && pos.y < line->origin.y + line->size.height) {
      return lineToCursor(line, text, document, xpos, pos.x);
//      cout << "  found a line" << endl;
      break;
    }
  }
//  cout << "positionToCursor failed" << endl;
  return 0;
}

/*
  handling style changes

  the algorithm:
    brute force for now
    assumption:
      o input has been 'normalized' (been generated with this software)
        (normalized means tags don't contain themselves (ie. no '<b><b>bold</b></b>')
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

/**
 * helper function for tagtoggle
 *
 * @param text		text which would be modified
 * @param tag		style (ie. 'b' for bold) to be toggled within the selection
 * @param bgn		begin of the selection
 * @param end		end of the selection
 * @return              true when selection is not yet covered by tag
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
      TTag tag0;
      taginc(text, &x1, &tag0);
//      cout << x0 << " tag: " << tag0;
      if (tag0.close) {
        if (tag0.name==tag) {
//          cout << " close";
          inside = false;
        }
      } else {
        if (tag0.name==tag) {
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

/**
 * create a list of where tags open and close
 */
void
tagrange(const string &text, vector<TTagRange> *result)
{
  vector<size_t> stack;

  size_t cx=0;
  while(cx<text.size()) {
    size_t x = cx;
    if (text[cx]=='&') {
      entityinc(text, &cx);
    } else
    if (text[cx]=='<') {
      TTag tag;
      taginc(text, &cx, &tag);
      if (tag.open) {
        stack.push_back(result->size());
        (*result).push_back(TTagRange(x));
//cout << x << ": <" << tag.name << ">" << endl;
      }
      if (tag.close) {
        (*result)[stack.back()].end = x;
        stack.pop_back();
//cout << x << ": </" << tag.name << ">" << endl;
      }
    } else {
//cout << cx << ": " << text[cx] << endl;
      utf8inc(text, &cx);
    }
  }

//for(auto &a: *result) cout << a.bgn << " - " << a.end << endl;

}

// FIXME: write tests for this one
// FIXME: implement isadd==false
void
expandselection(const string &text, size_t *sb, size_t *se, const string &name)
{
  DBG(cout << "----------- expand selection ---------------" << endl;)
  size_t cx=0, inside_since;
  enum {
    HEAD_OUTSIDE,
    HEAD_INSIDE,
    HEAD_OUTSIDE_UNUSED
  } head = HEAD_OUTSIDE;
  enum {
    TAIL_LATER,
    TAIL_JUST_PASSED,
    TAIL_PASSED
  } tail = TAIL_LATER;
  
  while(cx<text.size()) {
    if (cx==*sb) {
      DBG(cout << "BGN SELECTION" << endl;)
      if (head!=HEAD_OUTSIDE) {
        DBG(cout << "  NOT HEAD_OUTSIDE, SET SB TO " << inside_since << endl;)
        *sb = inside_since;
      } else {
        DBG(cout << "  HEAD_OUTSIDE" << endl;)
      }
    }
    if (cx==*se) {
      DBG(cout << "END SELECTION AT " << cx << endl;)
      DBG(cout << "TAIL_JUST_PASSED" << endl;)
      tail = TAIL_JUST_PASSED;
    }
    size_t x = cx;
    if (text[cx]=='<') {
      TTag tag;
      taginc(text, &cx, &tag);
      if (tag.open) {
        DBG(cout << x << ": <" << tag.name << ">" << endl;)
        if (tag.name == name) {
          DBG(cout << "  HEAD_INSIDE" << endl;)
          if (head==HEAD_OUTSIDE) {
            inside_since = x;
          }
          head = HEAD_INSIDE;
        }
      }
      if (tag.close) {
        DBG(cout << x << ": </" << tag.name << ">" << endl;)
        if (tag.name == name) {
          DBG(cout << "  HEAD_OUTSIDE_UNUSED" << endl;)
          head = HEAD_OUTSIDE_UNUSED;
        }
      }
    } else {
      if (head==HEAD_OUTSIDE_UNUSED) {
        DBG(cout << "  HEAD_OUTSIDE" << endl;)
        head=HEAD_OUTSIDE;
      }
      if (head==HEAD_OUTSIDE && tail==TAIL_JUST_PASSED) {
        DBG(cout << "TAIL PASSED AT " << x << endl;)
        *se = x;
        tail = TAIL_PASSED;
      }

      if (text[cx]=='&') {
        entityinc(text, &cx);
      } else {
        DBG(cout << cx << ": " << text[cx] << endl;)
        utf8inc(text, &cx);
      }
    }
  }
  if (tail==TAIL_JUST_PASSED) {
    DBG(cout << "TAIL PASSED AT END" << endl;)
    *se = cx;
  }
  
  DBG(cout << "new selection: " << *sb << " - " << *se << endl;)
}

string      
tagtoggle(const string &text, vector<size_t> &xpos, const string &tag)
{
  size_t x0=0, x1=0; // FIXME: do i still need two variables here?
  size_t eol = text.size();
  int c;

  int inside=0; 	// how deep we are inside the tag to be added
  size_t inside_bgn;
  
  size_t sb=xpos[SELECTION_BGN], se=xpos[SELECTION_END];
  
  // find out whether we are about to add or remove tag
  bool add = isadd(text, tag, sb, se);
  DBG(cout << "isadd: " << add << endl;)

  // the algoritm following later is good at removing the onoff related tags
  // when the selection encloses the largest possible area to be within tag.
  // so the following function expands sb and se
  if (add)
    expandselection(text, &sb, &se, tag);
  
  // to handle the onoff related tags we need to know where start tags and
  // their respective end tags are
  vector<TTagRange> tagrange;
  toad::wordprocessor::tagrange(text, &tagrange);
  
  string out;
  vector<size_t> pos(xpos);
//  pos.assign(xpos.size(), 0);
  
  while(x0<eol) {

    for(size_t i=0; i<xpos.size(); ++i) {
      if (xpos[i]==x0) {
        if (pos[i]!=out.size()) {
          DBG(cout << __LINE__ << ": change xpos[" << i << "] from " << xpos[i] << " to " << out.size() << endl;)
          pos[i]=out.size();
        }
      }
    }

    c = text[x0];
    if (sb==x0) {
      if (add) {
        if (!inside) {
          DBG(cout << __LINE__ << ": +<"<<tag<<">, ++inside, inside=" << (inside+1) << endl;)
          out += "<"+tag+">";
        } else {
          DBG(cout << __LINE__ << ": ++inside, inside=" << (inside+1) << endl;)
        }
        ++inside;
      } else {
        --inside;
        if (!inside) {
          DBG(cout << __LINE__ << ": +</"<<tag<<">, --inside, inside=" << (inside) << endl;)
          out += "</"+tag+">";
        }
      }
    }

    if (se==x0) {
      if (add) {
        --inside;
        if (!inside) {
          DBG(cout << __LINE__ << ": +</"<<tag<<">, --inside, inside=" << inside << endl;)
          out += "</"+tag+">";
        }
/*
for(size_t i=0; i<xpos.size(); ++i) {
  if (xpos[i]==x0) {
    if (pos[i]!=out.size()) {
      cout << __LINE__ << ": change xpos[" << i << "] from " << xpos[i] << " to " << out.size() << endl;
      pos[i]=out.size();
    }
  }
}
*/
      } else {

        if (!inside) {
        
          if (c=='<') {
            TTag tag1;
            size_t tmp=x0;
            taginc(text, &tmp, &tag1);
            if (tag1.close && tag1.name==tag) {
              DBG(cout << __LINE__ << ": tag to be removed ended at end of selection, don't reopen it and skip close" << endl;)
              x0 = tmp;
              continue;
            }
          }

          DBG(cout << __LINE__ << ": +<"<<tag<<">, ++inside, inside=" << (inside+1) << endl;)
          out += "<"+tag+">";
        }
        ++inside;
      }
    }

    // copy entity
    if (c=='&') {
      // copy entity
      size_t x2=x0;
      entityinc(text, &x2);
      DBG(cout << __LINE__ << ": " << x0 << "entity: " << text.substr(x0,x2-x0) << endl;)
      out+=text.substr(x0,x2-x0);
      x0=x2;
      continue;
    }
    
    // copy single character
    if (c!='<') {
      size_t l = utf8charsize(text, x0);
      out += text.substr(x0, l);
      x0+=l;
      continue;
    }

    // handle tag
    bool inside_sel = sb<=x0 && x0<se;
    TTag tag0;
    size_t x1=x0;
    taginc(text, &x1, &tag0);
    DBG(cout << x0 << " :" << tag0 << endl;)
    
    bool onoff = false;
    if (add && inside_sel) {
      if (tag0.open && !tag0.close && tag0.name!=tag) {
        for(auto &a: tagrange) {
          if (a.bgn==x0) {
            if (a.end>se) {
              DBG(cout << "add: tag end "<<a.end<<" is behind selection end "<<se<< endl;)
              onoff = true;
            }
          }
        }
      }
      if (!tag0.open && tag0.close && tag0.name!=tag) {
        for(auto &a: tagrange) {
          if (a.end==x0) {
            if (a.bgn<sb) {
              DBG(cout << "add: tag bgn "<<a.end<<" is before selection bgn "<<sb<< endl;)
              onoff = true;
            }
          }
        }
      }
    }

    if (!add && !inside_sel) {
      if (tag0.open && !tag0.close && tag0.name!=tag) {
        DBG(cout << __LINE__ << ": look for " << tag0 << " at " << x0 << endl;)
        for(auto &a: tagrange) {
          if (a.bgn==x0) {
            DBG(cout << __LINE__ << ":  tag has range " << a.bgn << " to " << a.end << endl;)
            if (a.end>sb) {
              DBG(cout << __LINE__ << ": del: tag end "<<a.end<<" is after selection bgn "<<sb<< endl;) // FIXME: text
              onoff = true;
            }
          }
        }
      }
      if (!tag0.open && tag0.close && tag0.name!=tag) {
        DBG(cout << __LINE__ << ": look for " << tag0 << " at " << x0 << endl;)
        for(auto &a: tagrange) {
          if (a.end==x0) {
            DBG(cout << "  tag has range " << a.bgn << " to " << a.end << endl;)
            if (a.bgn<se) {
              DBG(cout << "del: tag bgn "<<a.end<<" is after selection end "<<se<< endl;) // FIXME: text
              onoff = true;
            }
          }
        }
      }
    }
    
    if (onoff && inside) {
      DBG(cout << __LINE__ << ": add </" << tag << ">" << endl;)
      out+="</"+tag+">";
    }
    
    // outside selection
    if (!tag0.open && tag0.close && tag0.name==tag) {
      --inside;
      DBG(cout << __LINE__ << ": --inside, inside=" << (inside) << endl;)
      if (!inside) {
        DBG(cout << __LINE__ << ": add " << tag0 << endl;)
        out += tag0;
      }
    } else
    if (tag0.open && !tag0.close && tag0.name==tag) {
      if (!inside) {
        DBG(cout << __LINE__ << ": add " << tag0 << endl;)
        out += tag0;
      }
      ++inside;
      DBG(cout << __LINE__ << ": ++inside, inside=" << (inside+1) << endl;)
    } else {
      DBG(cout << __LINE__ << ": copy tag " << tag0 << endl;)
      out += tag0;
    }

    if (onoff && inside) {
      DBG(cout << __LINE__ << ": add <" << tag << ">" << endl;)
      out+="<"+tag+">";
    }

    x0=x1;
  }

  // still inside? don't
  if (inside==1)
    out+="</"+tag+">";

for(size_t i=0; i<xpos.size(); ++i) {
  if (xpos[i]==text.size()) {
    cout << __LINE__ << ": change at " << i << " from " << xpos[i] << " to " << out.size() << endl;
    pos[i]=out.size();
  }
}

  for(size_t i=0; i<xpos.size(); ++i) {
    xpos[i]=pos[i];
  }

  return out;
}

void
textDelete(string &text, TPreparedDocument &document, vector<size_t> &xpos)
{
  // FIXME: selection

  size_t pos = xpos[CURSOR];
  if (pos>=text.size())
    return;

  bool fragmentChanged = false;

  TTag tag;
  while (text[pos]=='<') {
    size_t old_pos = pos;
    taginc(text, &pos, &tag);
    if (tag.open && tag.close) {
      pos = old_pos;
      fragmentChanged = true;
      break;
    }
  }
  if (pos>=text.size())
    return;

  if (text[pos] == '&')
    fragmentChanged = true;

  size_t p=pos;
  xmlinc(text, &p);
  ssize_t len = p-pos;
  text.erase(pos, len);

  while(pos>=2 && pos+1<text.size() &&
        text[pos-2]!='/' &&
        text[pos-1]=='>' && 
        text[pos]=='<' && text[pos+1]=='/')
  {
    size_t bgn = pos-1;
    tagdec(text, &bgn);
    if (text[bgn+1]=='/')
      break;
      
    size_t end = pos;
    taginc(text, &end);
    text.erase(bgn, end-bgn);

    pos = bgn;
    fragmentChanged = true;
  }
  if (fragmentChanged) {
    prepareHTMLText(text, xpos, &document);
  } else {
    updatePrepared(text, &document, pos, -len);
  }
  updateMarker(text, &document, xpos);
}

} // namespace toad::wordprocessor
