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

// TFont toad::default_font("arial,helvetica,sans-serif:size=12");

TFont::TFont()
{
  setFont("arial,helvetica,sans-serif:size=12");
  nsfont = [NSFont fontWithName: @"Helvetica" size:12.0];
  [nsfont retain];
//cerr << "TFont::TFont() -> nsfont=" << nsfont << endl;
}

TFont::TFont(const TFont &f) {
  nsfont = [NSFont fontWithName: @"Helvetica" size:12.0];
  [nsfont retain];
//cerr << "TFont::TFont(const TFont&) -> nsfont=" << nsfont << endl;
}

TFont::TFont(const string &fontname) {
  setFont(fontname);
  nsfont = [NSFont fontWithName: @"Helvetica" size:12.0];
  [nsfont retain];
//cerr << "TFont::TFont(const string&) -> nsfont=" << nsfont << endl;
}

TFont::~TFont()
{
//cerr << "TFont::~TFont(const TFont&)" << nsfont << endl;
  if (nsfont)
    [nsfont release];
}

void
TFont::setFont(const string &fontname)
{
}

const char*
TFont::getFont() const
{
  return "Helvetica";
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
}

double
TFont::getSize() const
{
  return 12;
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

int
TFont::getHeight() { 
  return [nsfont ascender] - [nsfont descender];
}

int
TFont::getAscent() {
  return [nsfont ascender];
}

int
TFont::getDescent() {
  return -[nsfont descender];
}

int
TFont::getTextWidth(const char *text)
{
  // widthOfString is deprecated sind Mac OS X v10.4 and not available to 64bit applications
  // return [nsfont widthOfString: [NSString stringWithUTF8String: text]];
  NSDictionary *attributes = [NSDictionary dictionaryWithObject: nsfont forKey: NSFontAttributeName];
  NSSize size = [[NSString stringWithUTF8String: text] sizeWithAttributes: attributes];
  //  [attributes release]; DONT!!!
  return size.width;
}

int
TFont::getTextWidth(const char *text, size_t n) {
  int w;
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
