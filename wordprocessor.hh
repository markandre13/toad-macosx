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

#include <toad/types.hh>
#include <toad/color.hh>
#include <toad/font.hh>
#include <toad/pen.hh>

#include <vector>
#include <map>

namespace toad {
  namespace wordprocessor {

    using toad::TCoord;
    using toad::TRGB;
    using toad::TFont;
    using toad::TPen;

    enum {
      CURSOR,
      SELECTION_BGN,
      SELECTION_END
    };

    struct TTextAttribute
    {
      string face;
      TCoord size;
      bool bold:1;
      bool italic:1;
      TRGB bcolor;
      TRGB fcolor;
      
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
        }
        face = a->face;
        size = a->size;
        bold = a->bold;
        italic = a->italic;
        bcolor = a->bcolor;
        fcolor = a->fcolor;
      }

      void setFont(TFont &font);
      void setFont(TPen  &pen);
    };

    struct TTextFragment
    {
      static const size_t npos = (size_t)-1;

      TTextFragment() {
        offset = npos;
      }
      TTextFragment(const TTextFragment *t): attr(t?&t->attr:0) {
        offset = npos;
      }

      // information on rendering
      TPoint origin;
      TSize size;
      TCoord ascent, descent;

      // the text to be displayed
      size_t offset;	// absolute offset
      size_t length;	// length of the text fragment

      TTextAttribute attr;
    };
    void fragment2cstr(const TTextFragment *fragment, const char *text, const char **cstr, size_t *length);

    struct TPreparedLine
    {
      ~TPreparedLine();
      TPoint origin;
      TSize size;
      TCoord ascent, descent;
  
      size_t offset;	// == fragments[0]->offset ?
      vector<TTextFragment*> fragments;
    };

    // marker for cursors, selections, ...
    // apple pages selection is rendered as follows:
    // o blue selection background
    // o selection disables other background
    // o selection is line height (inclusive leading)
    // o cursor is text height, not line height
    struct TMarker
    {
      TMarker() {
        pos.set(0,0);
        line = 0;
        fragment = 0;
      }
      TPoint pos;
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
    
    // prepare text for screen
    void prepareHTMLText(const string &text, const vector<size_t> &xpos, TPreparedDocument *document);
    // render text on screen
    void renderPrepared(TPen &pen, const char *text, const TPreparedDocument *document, const vector<size_t> &xpos);
    void updatePrepared(const string &text, TPreparedDocument *document, size_t offset, size_t len);

    void updateMarker(const string &text, TPreparedDocument *document, vector<size_t> &xpos);
    size_t positionToOffset(const string &text, TPreparedDocument &document, vector<size_t> &xpos, TPoint &pos);

    size_t lineToCursor(const TPreparedLine *line, const string &text, TPreparedDocument &document, vector<size_t> &xpos, TCoord x);

    // tagtoggle and its helper functions
    string tagtoggle(const string &text, vector<size_t> &xpos, const string &tag);
    bool isadd(const string &text, const string &tag, size_t bgn, size_t end);
    struct TTagRange {
      TTagRange(size_t b): bgn(b) {}
      size_t bgn, end;
    };
    void tagrange(const string &text, vector<TTagRange> *result);
    void expandselection(const string &text, size_t *sb, size_t *se, const string &name, bool isadd);

    struct TTag
    {
      TTag() { open=close=false; }
      string name;
      map<string, string> attribute;
      bool open:1;
      bool close:1;
      string str() const;
    };

    inline ostream& operator<<(ostream &s, const TTag& tag) { s<<tag.str(); return s; }
    inline string operator+(const string &s, const TTag &tag) { return s+tag.str(); }
    inline string& operator+=(string &s, const TTag &tag) { s+=tag.str(); return s; }

    void taginc(const string &text, size_t *cx, TTag *tag=nullptr);
    void tagdec(const string &text, size_t *cx);
    void entityinc(const string &text, size_t *cx);
    void entitydec(const string &text, size_t *cx);
    void xmlinc(const string &text, size_t *cx);
    void xmldec(const string &text, size_t *offset);


  } // namespace wordprocessor

} // namespace toad
