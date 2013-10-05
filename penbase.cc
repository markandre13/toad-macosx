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

#include <toad/penbase.hh>

using namespace toad;

void
TPenBase::drawRectanglePC(TCoord x, TCoord y, TCoord w, TCoord h)
{
  vdrawRectangle(x,y,w-1,h-1);
}

void
TPenBase::fillRectanglePC(TCoord x, TCoord y, TCoord w, TCoord h)
{
  vfillRectangle(x,y,w-1,h-1);
}

void
TPenBase::drawCirclePC(TCoord x, TCoord y, TCoord w, TCoord h)
{
  vdrawCircle(x,y,w-1,h-1);
}

void
TPenBase::fillCirclePC(TCoord x, TCoord y, TCoord w, TCoord h)
{
  vfillCircle(x,y,w-1,h-1);
}

void
TPenBase::drawArcPC(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r0, TCoord r1)
{
  vdrawArc(x,y,w-1,h-1, r0, r1);
}

void
TPenBase::fillArcPC(TCoord x, TCoord y, TCoord w, TCoord h, TCoord r0, TCoord r1)
{
  vfillArc(x,y,w-1,h-1, r0, r1);
}

void
TPenBase::setColor(TColor::EColor c)
{
  const TRGB* rgb = TColor::lookup(c);
  setColor(rgb->r/255.0, rgb->g/255.0, rgb->b/255.0);
}

void
TPenBase::setLineColor(TColor::EColor c)
{
  const TRGB* rgb = TColor::lookup(c);
  setLineColor(rgb->r/255.0, rgb->g/255.0, rgb->b/255.0);
}

void
TPenBase::setFillColor(TColor::EColor c)
{
  const TRGB* rgb = TColor::lookup(c);
  setFillColor(rgb->r/255.0, rgb->g/255.0, rgb->b/255.0);
}

// 3D rectangle
//----------------------------------------------------------------------------
/**
 * This is a special function for widgets.
 */
void
TPenBase::vdraw3DRectangle(TCoord x, TCoord y, TCoord w, TCoord h, bool inset)
{
  TRGBA stroke2 = stroke, fill2 = fill;

  ++w;
  ++h;

  TPoint p[3];
  if (inset)
    setLineColor(1,1,1);
  else
    setLineColor(0,0,0);
  p[0].set(x+1  ,y+h-1);
  p[1].set(x+w-1,y+h-1);
  p[2].set(x+w-1,y);    
  drawLines(p,3);

  if (inset)  
    setLineColor(TColor::BTNLIGHT);
  else
    setLineColor(TColor::BTNSHADOW);
  p[0].set(x+2  ,y+h-2);
  p[1].set(x+w-2,y+h-2);
  p[2].set(x+w-2,y+1);
  drawLines(p,3);

  if (inset)
    setLineColor(TColor::BTNSHADOW);
  else
    setLineColor(TColor::BTNLIGHT);
  p[0].set(x    ,y+h-1);
  p[1].set(x    ,y);
  p[2].set(x+w-1,y);
  drawLines(p,3);

  if (inset)
    setLineColor(0,0,0);
  else
    setLineColor(1,1,1);
  p[0].set(x+1  ,y+h-2);
  p[1].set(x+1  ,y+1);  
  p[2].set(x+w-2,y+1);  
  drawLines(p,3);

  setLineColor(stroke2.r, stroke2.g, stroke2.b);
  setFillColor(fill2.r, fill2.g, fill2.b);
}

void
TPenBase::draw3DRectanglePC(TCoord x, TCoord y, TCoord w, TCoord h, bool inset)
{
  vdraw3DRectangle(x, y, w-1, h-1, inset);
}

namespace {

struct TWord
{
  const char* pos;
  unsigned bytes; 
  unsigned len;   
  unsigned linefeeds;
};
  
void
count_words_and_lines(const char *text, unsigned* word_count, unsigned* min_lines)
{
  *word_count = 0;
  *min_lines = 1; 
  const char* ptr = text;
  bool word_flag = false;
  while(*ptr) {
    if(!word_flag && *ptr!=' ' && *ptr!='\n') {
      word_flag=true;
      (*word_count)++;
    } else 
    if (word_flag && (*ptr==' ' || *ptr=='\n'))
      word_flag=false;   
    if (*ptr=='\n')      
      (*min_lines)++;    
    ptr++;
  }
}  
   
TWord*
make_wordlist(TFont *font, const char *text, unsigned word_count)
{
  TWord* word = new TWord[word_count];

  unsigned j,i = 0;
  const char* ptr = text;
  bool word_flag = false;
  unsigned lf=0;
  while(*ptr) { 
    if(!word_flag && *ptr!=' ' && *ptr!='\n') {
      word[i].pos = ptr;
      j = 0;
      word_flag=true;
    }
    ptr++;
    j++;  
    if (word_flag && (*ptr==' ' || *ptr=='\n' || *ptr==0)) {
      word[i].bytes     = j;
      word[i].len       = font->getTextWidth(word[i].pos,j);
      word[i].linefeeds = lf;
      word_flag=false;
//      printf("word %2u, bytes=%i\n",i,j);
      i++;
      lf=0;
    }
    if(*ptr=='\n')
      lf++;
  }
//  printf("word_count=%i\n",word_count);
  return word;
}
 
} // namespace

/**
 * Draw string 'str' in multiple lines, reduce spaces between words to one 
 * an break lines to fit width. 'str' can contain '\n'.
 */
int
TPenBase::drawTextWidth(TCoord x,TCoord y,const string &str, unsigned width)
{
  return textRatio(x, y, str, width, font, this);
}

int
TPenBase::getHeightOfTextFromWidth(TFont *font, const string &text, int width)
{
  return textRatio(0,0,text,width,font,0);
}

int
TPenBase::textRatio(TCoord x,TCoord y,const string &str, unsigned width, TFont *font, TPenBase *pen)
{
  const char* text=str.c_str();
  
  unsigned i;
#if 0
  if (mat) {
    x+=static_cast<int>(mat->tx);
    y+=static_cast<int>(mat->ty);
  }
#endif
  // 1st step: count words and lines
  unsigned word_count, min_lines;   
  count_words_and_lines(text, &word_count, &min_lines);
  if (!word_count) return 0;
  
  // 2nd step: create a word list
  TWord* word = make_wordlist(font, text, word_count);
  
  // 3rd step: output
  TCoord blank_width = font->getTextWidth(" ",1);
  TCoord line_len = 0;
  unsigned word_of_line = 1;
  
  for(i=0; i<word_count; i++) {
    if ((line_len+word[i].len>width && i!=0) || word[i].linefeeds) {
      if (word[i].linefeeds)
        y+=font->getHeight()*word[i].linefeeds;
      else
        y+=font->getHeight();
      line_len = 0;
      word_of_line = 0;
    }
    if (pen)
      pen->drawString(x+line_len,y, word[i].pos, word[i].bytes);
    line_len+=word[i].len+blank_width;
    word_of_line++;
  }
   
  delete[] word;
  return y+font->getHeight();
}




static void curve(TPolygon&,double,double,double,double,double,double,double,double);
static void fcurve(TDPolygon&,double,double,double,double,double,double,double,double);

#if 0
static bool
points2list(TMatrix2D *mat, const TPoint *p, int n)
{
  if (n<4)
    return false;

  lst.clear();
  int x, y;
  if (!mat) {
    lst.push(p[0].x, p[0].y);
  } else {
    mat->map(p[0].x, p[0].y, &x, &y);
    lst.push(x, y);
  }

  while(n>=4) {
    if (!mat) {
      xcurve(lst,
             p->x, p->y,
             (p+1)->x, (p+1)->y,
             (p+2)->x, (p+2)->y,
             (p+3)->x, (p+3)->y);
      p+=3;
    } else {
      int px[4], py[4];
      for(int j=1; j<4; ++j) {
        ++p;
        mat->map(p->x, p->y, &px[j], &py[j]);
      }
      xcurve(lst,
             x,     y,
             px[1], py[1],
             px[2], py[2],
             px[3], py[3]);
      x=px[3]; y=py[3];
    }
    n-=3;
  }
  return true;
}

static bool
points2list(TMatrix2D *mat, const TDPoint *p, int n)
{
  if (n<4)
    return false;

  lst.clear();
  double x, y;
  if (!mat) {
    lst.push(p[0].x, p[0].y);
  } else {
    mat->map(p[0].x, p[0].y, &x, &y);
    lst.push(x, y);
  }

  while(n>=4) {
    if (!mat) {
      xcurve(lst,
             p->x, p->y,
             (p+1)->x, (p+1)->y,
             (p+2)->x, (p+2)->y,
             (p+3)->x, (p+3)->y);
      p+=3;
    } else {
      double px[4], py[4];
      for(int j=1; j<4; ++j) {
        ++p;
        mat->map(p->x, p->y, &px[j], &py[j]);
      }
      xcurve(lst,
             x,     y,
             px[1], py[1],
             px[2], py[2],
             px[3], py[3]);
      x=px[3]; y=py[3];
    }
    n-=3;
  }
  return true;
}

static bool
polygon2list(TMatrix2D *mat, const TPolygon &polygon)
{
  unsigned n = polygon.size();
  if (n<4)
    return false;

  lst.clear();
  int x, y;
  if (!mat) {
    lst.push(polygon[0].x, polygon[0].y);
  } else {
    mat->map(polygon[0].x, polygon[0].y, &x, &y);
    lst.push(x, y);
  }

  TPolygon::const_iterator p(polygon.begin());
  while(n>=4) {
    if (!mat) {
      xcurve(lst,
             p->x, p->y,
             (p+1)->x, (p+1)->y,
             (p+2)->x, (p+2)->y,
             (p+3)->x, (p+3)->y);
      p+=3;
    } else {
      int px[4], py[4];
      for(int j=1; j<4; ++j) {
        ++p;
        mat->map(p->x, p->y, &px[j], &py[j]);
      }
      xcurve(lst,
             x,     y,
             px[1], py[1],
             px[2], py[2],
             px[3], py[3]);
      x=px[3]; y=py[3];
    }
    n-=3;
  }
  return true;
}

static bool
polygon2list(TMatrix2D *mat, const TDPolygon &polygon)
{
  unsigned n = polygon.size();
  if (n<4)
    return false;

  lst.clear();
  double x, y;
  if (!mat) {
    lst.push(polygon[0].x, polygon[0].y);
  } else {
    mat->map(polygon[0].x, polygon[0].y, &x, &y);
    lst.push(x, y);
  }

  TDPolygon::const_iterator p(polygon.begin());
  while(n>=4) {
    if (!mat) {
      xcurve(lst,
             p->x, p->y,
             (p+1)->x, (p+1)->y,
             (p+2)->x, (p+2)->y,
             (p+3)->x, (p+3)->y);
      p+=3;
    } else {
      double px[4], py[4];
      for(int j=1; j<4; ++j) {
        ++p;
        mat->map(p->x, p->y, &px[j], &py[j]);
      }
      xcurve(lst,
             x,     y,
             px[1], py[1],
             px[2], py[2],
             px[3], py[3]);
      x=px[3]; y=py[3];
    }
    n-=3;
  }
  return true;
}
#endif
void 
TPenBase::poly2Bezier(const TPoint* src, size_t n, TPolygon &dst)
{
  dst.erase(dst.begin(), dst.end());
  dst.push_back(TPoint(src[0].x, src[0].y));
  n-=3;
  int i=0;
  while(i<=n) {
    curve(dst,
          src[i].x,   src[i].y,
          src[i+1].x, src[i+1].y,
          src[i+2].x, src[i+2].y,
          src[i+3].x, src[i+3].y);
    i+=3;
  }
}

void 
TPenBase::poly2Bezier(const TPolygon &src, TPolygon &dst)
{
  dst.erase(dst.begin(), dst.end());
  dst.addPoint(src[0]);
  unsigned n = src.size();
  n-=3;
  unsigned i=0;
  while(i<=n) {
    curve(dst,
          src[i].x,   src[i].y,
          src[i+1].x, src[i+1].y,
          src[i+2].x, src[i+2].y,
          src[i+3].x, src[i+3].y);
    i+=3;
  }
}

void 
TPenBase::poly2Bezier(const TDPoint* src, size_t n, TDPolygon &dst)
{
  dst.erase(dst.begin(), dst.end());
  dst.push_back(TDPoint(src[0].x, src[0].y));
  n-=3;
  int i=0;
  while(i<=n) {
    fcurve(dst,
          src[i].x,   src[i].y,
          src[i+1].x, src[i+1].y,
          src[i+2].x, src[i+2].y,
          src[i+3].x, src[i+3].y);
    i+=3;
  }
}

inline double
mid(double a, double b)
{
  return (a + b) / 2.0;
}

#define WEIGHT 4.0

#if 0
static void xcurve(
  TXPoints &poly,
  double x0, double y0, 
  double x1, double y1,
  double x2, double y2,
  double x3, double y3)
{
  double vx0 = x1-x0;
  double vx1 = x2-x1;
  double vx2 = x3-x2;
  double vy0 = y1-y0;
  double vy1 = y2-y1;
  double vy2 = y3-y2;
  double vx3 = x2-x0;
  double vx4 = x3-x0;
  double vy3 = y2-y0;
  double vy4 = y3-y0;

  double w0 = vx0 * vy1 - vy0 * vx1;
  double w1 = vx1 * vy2 - vy1 * vx2;
  double w2 = vx3 * vy4 - vy3 * vx4;
  double w3 = vx0 * vy4 - vy0 * vx4;

  if (fabs(w0)+fabs(w1)+fabs(w2)+fabs(w3)<WEIGHT) {
    poly.push(x0, y0);
    poly.push(x1, y1);
    poly.push(x2, y2);
    poly.push(x3, y3);
  } else {
    double xx  = mid(x1, x2);
    double yy  = mid(y1, y2);
    double x11 = mid(x0, x1);
    double y11 = mid(y0, y1);
    double x22 = mid(x2, x3);
    double y22 = mid(y2, y3);
    double x12 = mid(x11, xx);
    double y12 = mid(y11, yy);
    double x21 = mid(xx, x22);
    double y21 = mid(yy, y22);
    double cx  = mid(x12, x21);
    double cy  = mid(y12, y21);
    xcurve(poly, x0, y0, x11, y11, x12, y12, cx, cy);
    xcurve(poly, cx, cy, x21, y21, x22, y22, x3, y3);
  }
}
#endif

static void curve(
  TPolygon &poly,
  double x0, double y0, 
  double x1, double y1,
  double x2, double y2,
  double x3, double y3)
{
  double vx0 = x1-x0;
  double vx1 = x2-x1;
  double vx2 = x3-x2;
  double vy0 = y1-y0;
  double vy1 = y2-y1;
  double vy2 = y3-y2;
  double vx3 = x2-x0;
  double vx4 = x3-x0;
  double vy3 = y2-y0;
  double vy4 = y3-y0;

  double w0 = vx0 * vy1 - vy0 * vx1;
  double w1 = vx1 * vy2 - vy1 * vx2;
  double w2 = vx3 * vy4 - vy3 * vx4;
  double w3 = vx0 * vy4 - vy0 * vx4;

  if (fabs(w0)+fabs(w1)+fabs(w2)+fabs(w3)<WEIGHT) {
    poly.push_back(TPoint(lround(x0), lround(y0)));
    poly.push_back(TPoint(lround(x1), lround(y1)));
    poly.push_back(TPoint(lround(x2), lround(y2)));
    poly.push_back(TPoint(lround(x3), lround(y3)));
  } else {
    double xx  = mid(x1, x2);
    double yy  = mid(y1, y2);
    double x11 = mid(x0, x1);
    double y11 = mid(y0, y1);
    double x22 = mid(x2, x3);
    double y22 = mid(y2, y3);
    double x12 = mid(x11, xx);
    double y12 = mid(y11, yy);
    double x21 = mid(xx, x22);
    double y21 = mid(yy, y22);
    double cx  = mid(x12, x21);
    double cy  = mid(y12, y21);
    curve(poly, x0, y0, x11, y11, x12, y12, cx, cy);
    curve(poly, cx, cy, x21, y21, x22, y22, x3, y3);
  }
}

static void fcurve(
  TDPolygon &poly,
  double x0, double y0, 
  double x1, double y1,
  double x2, double y2,
  double x3, double y3)
{
  double vx0 = x1-x0;
  double vx1 = x2-x1;
  double vx2 = x3-x2;
  double vy0 = y1-y0;
  double vy1 = y2-y1;
  double vy2 = y3-y2;
  double vx3 = x2-x0;
  double vx4 = x3-x0;
  double vy3 = y2-y0;
  double vy4 = y3-y0;

  double w0 = vx0 * vy1 - vy0 * vx1;
  double w1 = vx1 * vy2 - vy1 * vx2;
  double w2 = vx3 * vy4 - vy3 * vx4;
  double w3 = vx0 * vy4 - vy0 * vx4;

  if (fabs(w0)+fabs(w1)+fabs(w2)+fabs(w3)<WEIGHT) {
    poly.push_back(TDPoint(x0, y0));
    poly.push_back(TDPoint(x1, y1));
    poly.push_back(TDPoint(x2, y2));
    poly.push_back(TDPoint(x3, y3));
  } else {
    double xx  = mid(x1, x2);
    double yy  = mid(y1, y2);
    double x11 = mid(x0, x1);
    double y11 = mid(y0, y1);
    double x22 = mid(x2, x3);
    double y22 = mid(y2, y3);
    double x12 = mid(x11, xx);
    double y12 = mid(y11, yy);
    double x21 = mid(xx, x22);
    double y21 = mid(yy, y22);
    double cx  = mid(x12, x21);
    double cy  = mid(y12, y21);
    fcurve(poly, x0, y0, x11, y11, x12, y12, cx, cy);
    fcurve(poly, cx, cy, x21, y21, x22, y22, x3, y3);
  }
}

/**
 * For classes derived from TPenBase which can draw on multiple pages.
 */
void
TPenBase::showPage()
{
}