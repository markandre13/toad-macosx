/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2006 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/font.hh>

using namespace toad;

PFont toad::default_font;
PFont toad::bold_font;

/*
  NSFont fontWithName: returns pointers to a pool of already allocated fonts;
  reference counting via retain&release must be done here
*/

/*
  NSFontManager *fm = [NSFontManager sharedFontManager];
  NSArray *fl = [fm availableFontFamilies];
  for(NSUInteger i=0; i<[fl count]; ++i) {
    cout << "'" << [[fl objectAtIndex: i] UTF8String] << "'" << endl;
  }
*/

// TFont toad::default_font("arial,helvetica,sans-serif:size=12");

TFont::TFont()
{
  nsfont = 0;
  setFont("arial,helvetica,sans-serif:size=12");
//cerr << "TFont::TFont() -> nsfont=" << nsfont << endl;
}

TFont::TFont(const TFont &f) {
  nsfont = f.nsfont;
  fcname = f.fcname;
  if (nsfont)
    [nsfont retain];
//cerr << "TFont::TFont(const TFont&) -> nsfont=" << nsfont << endl;
}

TFont::TFont(const string &fontname) {
  nsfont = 0;
  setFont(fontname);
//cerr << "TFont::TFont(const string&) -> nsfont=" << nsfont << endl;
}

TFont::~TFont()
{
//cerr << "TFont::~TFont(const TFont&)" << nsfont << endl;
  if (nsfont)
    [nsfont release];
}

void
TFont::setFont(const string &fn)
{
//cout << "TFont::setFont(\""<<fn<<"\")"<<endl;
  fcname = fn;
  if (nsfont)
    [nsfont release];
    
  // hack: we just take the first font in the list
  string family;
  string::const_iterator p0=fn.begin(), p=p0, p1=fn.end();
  for(; p!=fn.end(); ++p) {
    if (*p == ',') {
      if (family.empty())
        family = string(p0, p);
      p0 = p + 1;
    } else
    if (*p == ':') {
      break;
    }
  }
  if (family.empty())
    family = string(p0, p);

  int size = 0; // FIXME: TCoord
  unsigned traits = 0;
  int weight = 5; // normal
  
  size_t n0=fn.find_first_of(":"), n1, n2;
  while(n0!=string::npos) {
    n1 = fn.find_first_of(":=", n0+1);
    string o0, o1;
    if (n1==string::npos) {
      o0=fn.substr(n0+1);
      n0=n1;
    } else
    if (fn[n1]==':') {
      o0=fn.substr(n0+1, n1-n0-1);
      n0=n1;
    } else
    if (fn[n1]=='=') {
      o0 = fn.substr(n0+1, n1-n0-1);
      n2 = fn.find_first_of(":", n1);
      o1 = fn.substr(n1+1, n2-n1-1);
      n0=n2;
    }
    
    if (o0=="size") {
      size = atoi(o1.c_str());
    } else
    if (o0=="italic") {
      traits |= NSItalicFontMask;
    } else
    if (o0=="bold") {
      traits |= NSBoldFontMask;
    }
  }

  if (family.empty())
    family = "Helvetica";
  if (size==0)
    size = 12;
//cout << "TFont::setFont(" << fn << ") -> " << family << ", " << size << ", " << traits << endl;
  NSFontManager *fm = [NSFontManager sharedFontManager];
  nsfont = [fm fontWithFamily: [NSString stringWithUTF8String: family.c_str()]
                       traits: traits
                       weight: weight
                         size:size];
  if (!nsfont) {
    cerr << "failed to find font '" << family << "', using 'Helvetica instead" << endl;
    nsfont = [NSFont fontWithName: [NSString stringWithUTF8String: "Helvetica"] size:size];
  }
  [nsfont retain];
}

const char*
TFont::getFont() const
{
  return fcname.c_str();
}

void
TFont::setFamily(const string &family)
{
}

const char*
TFont::getFamily() const
{
  return "Helvetica";
}

void
TFont::setSize(double size)
{
  cout << "Font::setSize" << size << endl;
}

double
TFont::getSize() const
{
  return nsfont ? [nsfont pointSize] : 0;
}

void
TFont::setWeight(int weight)
{
}

int
TFont::getWeight() const
{
  return 0;
}

void
TFont::setSlant(int slant)
{
}

int
TFont::getSlant() const
{
  return 0;
}

TCoord
TFont::getHeight() { 
  return ([nsfont ascender] + [nsfont leading] - [nsfont descender]);
}

TCoord
TFont::getAscent() {
  return ([nsfont ascender] + [nsfont leading]);
}

TCoord
TFont::getDescent() {
  return (-[nsfont descender]);
}

TCoord
TFont::getTextWidth(const char *text)
{
  if (!nsfont)
    return 0;
  // widthOfString is deprecated sind Mac OS X v10.4 and not available to 64bit applications
  // return [nsfont widthOfString: [NSString stringWithUTF8String: text]];
  NSDictionary *attributes = [NSDictionary dictionaryWithObject: nsfont forKey: NSFontAttributeName];
  NSSize size = [[NSString stringWithUTF8String: text] sizeWithAttributes: attributes];
  //  [attributes release]; DONT!!!
  return size.width;
}

TCoord
TFont::getTextWidth(const char *text, size_t n) {
  TCoord w;
  char *t = 0;
  if (strlen(text)>n) {
    t = strdup(text);
    t[n] = 0;
    w = getTextWidth(t);
    free(t);
  } else {
    w = getTextWidth(text);
  }
  return w;
}
