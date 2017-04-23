/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2006 by Mark-André Hopf <mhopf@mark13.org>
 * Visit http://www.mark13.org/fischland/.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_LIBCAIRO

#define _TOAD_PRIVATE

#include <toad/toad.hh>
#include "cairo.hh"

using namespace toad;

#ifdef OLD_CAIRO
namespace {
  cairo_matrix_t *cm = 0;
} // namespace
#endif

#if 0
  glitz_surface_t *gls;
  cr = cairo_glitz_surface_create(gls);
#endif
TCairo::TCairo(TWindow *window)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;

  cs = cairo_xlib_surface_create_with_xrender_format(
    x11display,
    window->x11window,
    ScreenOfDisplay(x11display, x11screen),
    XRenderFindVisualFormat(x11display, x11visual),
    window->getWidth(), window->getHeight()
  );

  cr = cairo_create(cs);

  alpha = 255;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

  // clip to the current update region
  TRegion *rgn = window->getUpdateRegion();
  if (!rgn) {
    clipbox.set(0,0,window->getWidth(),window->getHeight());
    return;
  }
  rgn->getBoundary(&clipbox);

  for(long i=0; i<rgn->getNumRects(); i++) {
    TRectangle r;
    rgn->getRect(i, &r);
    cairo_rectangle(cr, r.x, r.y, r.w, r.h);
  }
  cairo_clip(cr);
  //drop path...
  // cairo_set_rgb_color (cr, 1.0, 1.0, 1.0);
  setColor(window->getBackground());
  cairo_fill(cr);
}

TCairo::TCairo(TBitmap *bitmap)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  
  cs = cairo_xlib_surface_create_with_xrender_format(
    x11display,
    bitmap->pixmap,
    ScreenOfDisplay(x11display, x11screen),
    XRenderFindVisualFormat(x11display, x11visual),
    bitmap->getWidth(), bitmap->getHeight()
  );
  
  clipbox.set(0,0,bitmap->width,bitmap->height);

  cr = cairo_create(cs);

  alpha = 255;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  
  cairo_rectangle(cr, 0, 0, bitmap->getWidth(), bitmap->getHeight());
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_fill(cr);
}

TCairo::TCairo(cairo_surface_t *cs)
{
  this->cs = 0;
  cr = cairo_create(cs);

  alpha = 255;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
#if 0  
  cairo_rectangle(cr, 0, 0, bitmap->getWidth(), bitmap->getHeight());
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_fill(cr);
#endif
}

void
TCairo::draw(double x, double y, cairo_surface_t *s)
{
  cairo_set_source_surface(cr, s, x, y);
  cairo_paint(cr);
}


TCairo::TCairo(const string &filename)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  alpha = 255;
  cs = cairo_pdf_surface_create(filename.c_str(), 11.6944 * 72.0, 8.26389 * 72.0); // a4 in inch

  cairo_status_t status = cairo_surface_status(cs);
  if (status) {
    cerr << "cairo: " << cairo_status_to_string(status) << endl;
  }
  
//  cairo_pdf_surface_set_dpi(cs, 600, 600);
  cr = cairo_create(cs);
  
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  // cairo_set_line_width(cr, 48);
}

TCairo::~TCairo()
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (cairo_status(cr)) {
    cerr << "cairo: " << cairo_status_to_string(cairo_status(cr)) << endl;
  }
  cairo_destroy(cr);
  if (cs)
    cairo_surface_destroy(cs);
}

void
TCairo::showPage()
{
  cairo_show_page(cr);
}

void
TCairo::setAlpha(byte a)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (alpha==a)
    return;
  alpha = a;

  cairo_set_source_rgba(cr,(double)foreground.r/255.0, 
                           (double)foreground.g/255.0, 
                           (double)foreground.b/255.0,
                           (double)alpha/255.0);
}

byte
TCairo::getAlpha() const
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  return alpha;
}

void
TCairo::setLineWidth(int w)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_set_line_width(cr,w);
  width = w;
}

void
TCairo::setLineStyle(ELineStyle n)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
return;
  double dash[6];
  double w = width;
//cout << "width is " << width << endl;
  if (width==0.0)
    w = 0.5;
  TMatrix2D *mat = getMatrix();
  if (mat) {  
    double x0, y0, x1, y1;
    mat->map(0.0, 0.0, &x0, &y0);
    mat->map(1.0, 1.0, &x1, &y1);
cout << "scaling factor is " << (x1-x0) << endl;
cout << "inverse scaling factor is " << (1.0/(x1-x0)) << endl;
    double w = w / (x1-x0);
    if (w<0.0)   
      w = -w;
//    w = 1.0 / w;
//    w = round(d);
    if (w<0.5)
      w=0.5;
    cout << "  set line width " << w << " for " << width << endl;
  }

//w*=4.0;

cout << "w="<<w<<endl;
//return;

  for(int i=1;i<6;i++)
    dash[i]=w;
  
  switch(n) {
    case SOLID:
      cairo_set_dash(cr, NULL, 0, 0.0);
      break;
    case DOT:
      dash[0]=w;
      cairo_set_dash(cr, dash, 2, 0.0);
      break;
    default:
      cairo_set_dash(cr, NULL, 0, 0.0);
  }
//  cairo_set_dash (cairo_t *cr, double *dashes, int ndash, double offset);
}

void
TCairo::identity()
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_identity_matrix(cr);
}

void
TCairo::translate(double dx, double dy)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_translate(cr, dx, dy);
}

void
TCairo::rotate(double angle)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_rotate(cr, angle);
}

void
TCairo::scale(double sx, double sy)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_scale(cr, sx, sy);
}

void
TCairo::multiply(const TMatrix2D *m)
{
/*
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_matrix_t m0;
  cairo_get_matrix(cr, &m0);
//printf("m0:\n%lf %lf\n%lf %lf\n%lf %lf\n",m0.xx,m0.xy,m0.yx,m0.yy,m0.x0,m0.y0);
//cout << *m << endl;
  cairo_matrix_t m1;
  cairo_matrix_init(&m1, m->a11, m->a21, m->a12, m->a22, m->tx, m->ty);
//printf("m1:\n%lf %lf\n%lf %lf\n%lf %lf\n",m1.xx,m1.xy,m1.yx,m1.yy,m1.x0,m1.y0);

  cairo_matrix_t m2;
  cairo_matrix_multiply(&m2, &m1, &m0);
//printf("m2:\n%lf %lf\n%lf %lf\n%lf %lf\n",m2.xx,m2.xy,m2.yx,m2.yy,m2.x0,m2.y0);
  
  cairo_set_matrix(cr, &m2);
*/
  cairo_matrix_t m0;
  cairo_matrix_init(&m0, m->a11, m->a21, m->a12, m->a22, m->tx, m->ty);
  cairo_transform(cr, &m0);
}

void
TCairo::_updateCairoMatrix()
{
cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
}

void
TCairo::setMatrix(double a11, double a12, double a21, double a22, double tx, double ty)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_matrix_t m;
  cairo_matrix_init(&m, a11, a21, a12, a22, tx, ty);
  cairo_set_matrix(cr, &m);
}

TMatrix2D*
TCairo::getMatrix() const
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  static TMatrix2D m;
  cairo_matrix_t m0;
  cairo_get_matrix(cr, &m0);
  m.a11 = m0.xx;
  m.a12 = m0.xy;
  m.a21 = m0.yx;
  m.a22 = m0.yy;
  m.tx  = m0.x0;
  m.ty  = m0.y0;
  return &m;
}

void
TCairo::push()
{
  cairo_matrix_t m;
  cairo_get_matrix(cr, &m);
  mstack.push(m);
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
//  cairo_push_group(cr);
}

void
TCairo::pop()
{
  if (mstack.empty())
    return;
  cairo_set_matrix(cr, &mstack.top());
  mstack.pop();
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
//  cairo_pop_group(cr);
}
void
TCairo::popAll()
{
  while(!mstack.empty())
    mstack.pop();
  cairo_identity_matrix(cr);
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
}

void
TCairo::setColor(byte r, byte g, byte b)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_set_source_rgba(cr,(double)r/255.0, 
                           (double)g/255.0, 
                           (double)b/255.0,
                           (double)alpha/255.0);
  foreground = background = TRGB(r,g,b);
}

void
TCairo::setStrokeColor(const TColor &c)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_set_source_rgba(cr,(double)c.r/255.0, 
                           (double)c.g/255.0, 
                           (double)c.b/255.0,
                           (double)alpha/255.0);
  foreground = c;
}

void
TCairo::setFillColor(const TColor &c)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  background = c;
}

void
TCairo::vdrawLine(int x1, int y1, int x2, int y2)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, x2, y2);
  cairo_stroke(cr);
}

void
TCairo::drawLines(const TPoint *points, int n)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (n<2)
    return;
  cairo_move_to(cr, points->x, points->y);
  ++points;
  --n;
  while(n>0) {
    cairo_line_to(cr, points->x, points->y);
    ++points;
    --n;
  }
  cairo_stroke(cr);
}

void
TCairo::drawLines(const TPolygon &p)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (p.size()<2)
    return;
  TPolygon::const_iterator q = p.begin();
  cairo_move_to(cr, q->x, q->y);
  ++q;
  while(q!=p.end()) {
    cairo_line_to(cr, q->x, q->y);
    ++q;
  }
  cairo_stroke(cr);
}

void
TCairo::vdrawRectangle(int x,int y,int w,int h)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_rectangle(cr, x, y, w, h);
  cairo_stroke(cr);
}

void
TCairo::vfillRectangle(int x,int y,int w,int h)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  cairo_rectangle(cr, x, y, w, h);
  _fill();
}

void
TCairo::vdrawCircle(int x,int y,int w,int h)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  static const double k = 0.5522847498;
  double rx = (double)w / 2.0;
  double ry = (double)h / 2.0;
  double cx = (double)x + rx;
  double cy = (double)y + ry;
  double kx = k * rx;
  double ky = k * ry;
  
  cairo_new_path(cr);
  cairo_move_to(cr, x, cy);
  cairo_curve_to(cr, x, cy+ky,
                     cx-kx, y+h,
                     cx,y+h);
  cairo_curve_to(cr, cx+kx, y+h,
                     x+w, cy+ky,
                     x+w, cy);
  cairo_curve_to(cr, x+w, cy-ky,
                     cx+kx, y,
                     cx, y);
  cairo_curve_to(cr, cx-kx, y,
                     x, cy-ky,
                     x, cy);
  cairo_close_path(cr);
  cairo_stroke(cr);            
}

void
TCairo::vfillCircle(int x,int y,int w,int h)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  static const double k = 0.5522847498;
  double rx = (double)w / 2.0;
  double ry = (double)h / 2.0;
  double cx = (double)x + rx;
  double cy = (double)y + ry;
  double kx = k * rx;
  double ky = k * ry;
  
  cairo_new_path(cr);
  cairo_move_to(cr, x, cy);
  cairo_curve_to(cr, x, cy+ky,
                     cx-kx, y+h,
                     cx,y+h);
  cairo_curve_to(cr, cx+kx, y+h,
                     x+w, cy+ky,
                     x+w, cy);
  cairo_curve_to(cr, x+w, cy-ky,
                     cx+kx, y,
                     cx, y);
  cairo_curve_to(cr, cx-kx, y,
                     x, cy-ky,
                     x, cy);
  cairo_close_path(cr);
  _fill();
}

void
TCairo::_fill()
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (foreground == background) {
    cairo_fill(cr);
  } else {
    cairo_set_source_rgba(cr, (double)background.r/255.0,
                              (double)background.g/255.0,
                              (double)background.b/255.0,
                              (double)alpha/255.0);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, (double)foreground.r/255.0,
                              (double)foreground.g/255.0,
                              (double)foreground.b/255.0,
                              (double)alpha/255.0);
    cairo_stroke(cr);
  }
}


void
TCairo::fillPolygon(const TPoint *p, int n)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (n<2)
    return;

  cairo_move_to(cr, p->x, p->y);  
  ++p;
  --n;
  while(n>0) {
     cairo_line_to(cr, p->x, p->y);
     p++;
     n--;
  }
  cairo_close_path(cr);
  _fill();
}

void
TCairo::drawPolygon(const TPoint *p, int n)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (n<2)
    return;

  cairo_move_to(cr, p->x, p->y);  
  ++p;
  --n;
  while(n>0) {
     cairo_line_to(cr, p->x, p->y);
     p++;
     n--;
  }
  cairo_close_path(cr);
  cairo_stroke(cr);
}

void 
TCairo::fillPolygon(const TPolygon &polygon)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (polygon.size()<2) 
    return;
    
  TPolygon::const_iterator p(polygon.begin());
    
  cairo_move_to(cr, p->x, p->y);
  size_t n = polygon.size();
  ++p;
  --n;
  while(n>0) {
    cairo_line_to(cr, p->x, p->y);
    p++;
    n--;
  }
  cairo_close_path(cr);
  _fill();
}

void
TCairo::drawPolygon(const TPolygon &polygon)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (polygon.size()<2) 
    return;
    
  TPolygon::const_iterator p(polygon.begin());
    
  cairo_move_to(cr, p->x, p->y);
  size_t n = polygon.size();
  ++p;
  --n;
  while(n>0) {
    cairo_line_to(cr, p->x, p->y);
    p++;
    n--;
  }
  cairo_close_path(cr);
  cairo_stroke(cr);
}

void
TCairo::drawPolyBezier(const TPoint *p, int n)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (n<4)
    return;
  if (n<4) {
    cairo_move_to(cr, p[0].x, p[0].y);
    cairo_line_to(cr, p[1].x, p[1].y);
    if (n>2)
      cairo_line_to(cr, p[2].x, p[2].y);
    cairo_stroke(cr);
    return;
  }
    
  cairo_move_to(cr, p->x, p->y);
  ++p;
  --n;
  while(n>=3) {
    cairo_curve_to(cr,
      p->x, p->y,
      (p+1)->x, (p+1)->y,
      (p+2)->x, (p+2)->y);
    p+=3;
    n-=3;
  }
  cairo_stroke(cr);
}

void
TCairo::drawPolyBezier(const TPolygon &polygon)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (polygon.size()<4)
    return;
  if (polygon.size()<4) {
    cairo_move_to(cr, polygon[0].x, polygon[0].y);
    cairo_line_to(cr, polygon[1].x, polygon[1].y);
    if (polygon.size()>2)
      cairo_line_to(cr, polygon[2].x, polygon[2].y);
    cairo_stroke(cr);
    return;
  }
    
  TPolygon::const_iterator p(polygon.begin());
    
  cairo_move_to(cr, p->x, p->y);
  size_t n = polygon.size();
  ++p;
  --n;
  while(n>=3) {
    cairo_curve_to(cr,
      (p)->x, (p)->y,
      (p+1)->x, (p+1)->y,
      (p+2)->x, (p+2)->y);
    p+=3;
    n-=3;
  }
  cairo_stroke(cr);
}

void
TCairo::fillPolyBezier(const TPolygon &polygon)
{
//cout << __FILE__ << ':' << __LINE__ << ": " << __PRETTY_FUNCTION__ << endl;
  if (polygon.size()<4) 
    return;
    
  TPolygon::const_iterator p(polygon.begin());
    
  cairo_move_to(cr, p->x, p->y);
  size_t n = polygon.size();
  ++p;
  --n;
  while(n>=3) {
    cairo_curve_to(cr,
      (p)->x, (p)->y,
      (p+1)->x, (p+1)->y,
      (p+2)->x, (p+2)->y);
    p+=3;
    n-=3;
  }
  cairo_close_path(cr);
  _fill();
}

// -----------------------------------
void TCairo::shear(double x, double y)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::setBitmap(TBitmap *b)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

static cairo_font_slant_t
slant(int s)
{
  if (s<50)
    return CAIRO_FONT_SLANT_NORMAL;
  if (s<105)
    return CAIRO_FONT_SLANT_ITALIC;
  return CAIRO_FONT_SLANT_OBLIQUE;
}

static cairo_font_weight_t
weight(int w)
{
  return (w<140) ? CAIRO_FONT_WEIGHT_NORMAL : CAIRO_FONT_WEIGHT_BOLD;
}

void TCairo::setFont(const string &fn)
{
  TFont font(fn);
  cairo_select_font_face(cr,
                         font.getFamily(),
                         slant(font.getSlant()),
                         weight(font.getWeight()));
  cairo_set_font_size(cr, font.getSize());
}

void TCairo::setMode(EMode m)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::setColorMode(TColor::EDitherMode m)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::setClipChildren(bool b)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::setClipRegion(TRegion *r)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::setClipRect(const TRectangle &r)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::clrClipBox()
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::getClipBox(TRectangle *r) const
{
  *r = clipbox;
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
/*
  r->x=-0xFFFFFF;
  r->y=-0xFFFFFF;
  r->w=0xFFFFFFF;
  r->h=0xFFFFFFF;
*/
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::operator&=(const TRectangle&)
{
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::operator|=(const TRectangle&)
{
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::operator&=(const TRegion&)
{
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::operator|=(const TRegion&)
{
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawPoint(int, int)
{
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::vdrawArc(int, int, int, int, double, double)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::vfillArc(int, int, int, int, double, double)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawBezier(int, int, int, int, int, int, int, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawBezier(double, double, double, double, double, double, double, double)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawPolyBezier(const TDPoint*, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawPolyBezier(const TDPolygon&)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::fillPolyBezier(const TPoint*, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::fillPolyBezier(const TDPoint*, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::fillPolyBezier(const TDPolygon&)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::vdraw3DRectangle(int, int, int, int, bool)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

int TCairo::vgetTextWidth(const char *txt, size_t len) const
{
  char t[len+1];
  memcpy(t, txt, len);
  t[len]=0;

  cairo_text_extents_t e;
  cairo_text_extents(cr, t, &e);

  return /*e.x_bearing + */e.width/* + e.x_advance*/;
}

int TCairo::getAscent() const
{
  cairo_font_extents_t e;
  cairo_font_extents(cr, &e);
  return e.ascent;
}

int TCairo::getDescent() const
{
  cairo_font_extents_t e;
  cairo_font_extents(cr, &e);
  return e.descent;
}

int TCairo::getHeight() const
{
  cairo_font_extents_t e;
  cairo_font_extents(cr, &e);
  return e.height;
}

void TCairo::vdrawString(int x, int y, const char *txt, int len, bool f)
{
//  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
  cairo_move_to(cr, x, y + getAscent());
  
  char t[len+1];
  memcpy(t, txt, len);
  t[len]=0;
  cairo_show_text(cr, t);
}

int TCairo::drawTextWidth(int x, int y, const string &t, unsigned int l)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
  return 1;
}

void TCairo::drawBitmap(int, int, const TBitmap*)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawBitmap(int, int, const TBitmap&)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawBitmap(int, int, const TBitmap*, int, int, int, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::drawBitmap(int, int, const TBitmap&, int, int, int, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::moveTo(int, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::moveTo(const TPoint&)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::lineTo(int, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::lineTo(const TPoint&)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::curveTo(int, int, int, int, int, int)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::curveTo(const TPoint&, const TPoint&, const TPoint&)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

void TCairo::curveTo(double, double, double, double, double, double)
{
  cout << "not implemented: " << __PRETTY_FUNCTION__ << endl;
}

#endif
