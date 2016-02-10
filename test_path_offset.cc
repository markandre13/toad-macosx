/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for X-Windows
 * Copyright (C) 2015 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public 
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/figure.hh>
#include <toad/connect.hh>
#include <toad/vector.hh>
#include <toad/geometry.hh>

#include <memory>

namespace toad {
extern int solveCubic(TCoord a, TCoord b, TCoord c, TCoord d, TCoord *roots, TCoord min, TCoord max);
extern int solveCubic(const TPoint *v, int coord, TCoord val, TCoord *roots, TCoord min, TCoord max);
}

using namespace toad;

namespace {

TPoint p[5] = {
  {  50, 100 },
  {  50,  50 },
  { 150,  50 },
  { 150, 150 },
  { 100, 150 }
};

class TMyWindow:
  public TWindow
{
    int handle;
  public:
    TMyWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
    void mouseEvent(const TMouseEvent &) override;
};

unsigned cntr;

// the Tiller-Hanson offset function doesn't seem to be stable for this kind of offset algorithm
TCoord
distOffset(TSize size, TPoint direction, TCoord pressure, TCoord rotation)
{
  // direction radiants
  TCoord dr = atan2(direction.x, direction.y) - M_PI/2;
  // pen rotation radiants
  TCoord rr = (180-rotation)/360.0 * 2.0 * M_PI;
  // 
  TCoord r = dr + rr;

  // looking for extrema along x-axis (r-axis) of green ellispe rotated by rotation + direction
  //
  // a point on the ellipse is defined by:
  // E(u) = (sin(u)*s.width, cos(u)*s.height) ∧ u∈[0,2π]
  //
  // rotation:
  // R(r) = ⌈ cos(r) -sin(r) ⌉
  //        ⎣ sin(r) cos(r)  ⌋
  //
  // y-coordinate of rotated ellipse:
  //   y(u)  := sin(r) * sin(u) * s.width + cos(r) * cos(u) * s.height;
  // ⇒ y'(u) := sin(r) * cos(u) * s.width - cos(r) * sin(u) * s.height;
  //
  // extrema in u:
  // ⇒ 0 = sin(r) * cos(r) * s.w - cos(r) * sin(u) * s.h
  // ⇔ cos(r) * sin(u) * s.h = sin(r) * cos(u) * s.w
  // ⇔ sin(u) / cos(u) = sin(r) *s.width / cos(r) / s.h
  // ⇔ tan(u) = sin(r) *s.w / cos(r) / s.h
  // ⇔ u = atan(sin(r) *s.w / cos(r) / s.h)
  // ⇔ u = atan(tan(r) * s.w/s.h);
  TCoord u = atan(tan(r) * size.width/size.height);
  
  TCoord y = (sin(r) * sin(u)*size.width + cos(r) * cos(u)*size.height);
  if (y<0)
    y=-y;
  return y*pressure;
}

// approximation of offset curve based on:
// Wayne Tiller, Eric Hanson, "Offsets of Two-Dimensional Profiles",
// IEEE Computer Graphics and Applications, vol.4, no.  9, pp.  36-46,
// September 1984
void
drawOffset(TPen &pen, const TPoint *p, TCoord d0, TCoord d3, unsigned depth=0)
{
  TSize pensize(20,5);
  TCoord rotation = 0.0;
  TCoord pressure = 1.0;

  pen.setColor(1,0.9,0.7);
  
  TPoint lineA[2], lineB[2], lineC[3];
  
  TPoint l, o;
  
  TCoord totaldistance = distance(p[0], p[1]) +  distance(p[1], p[2]) +  distance(p[2], p[3]);

  // offset 
  TCoord d1 = distance(p[0], p[1]) / totaldistance * (d3-d0) + d0;
  TCoord d2 = (totaldistance - distance(p[2], p[3])) / totaldistance * (d3-d0) + d0;

  d0 = distOffset(pensize, bez2direction(p, 0), pressure, rotation);
  d1 = distOffset(pensize, bez2direction(p, distance(p[0], p[1]) / totaldistance), pressure, rotation);
  d2 = distOffset(pensize, bez2direction(p, (totaldistance - distance(p[2], p[3])) / totaldistance), pressure, rotation);
  d3 = distOffset(pensize, bez2direction(p, 1), pressure, rotation);
  
  l = p[1]-p[0];
  o=normalize(TPoint(-l.y, l.x));
  lineA[0] = p[0]+o*d0;
  lineA[1] = p[1]+o*d1;

  l = p[2]-p[1];
  o=normalize(TPoint(-l.y, l.x));
  lineB[0] = p[1]+o*d1;
  lineB[1] = p[2]+o*d2;

  l = p[3]-p[2];
  o=normalize(TPoint(-l.y, l.x));
  lineC[0] = p[2]+o*d2;
  lineC[1] = p[3]+o*d3;
  
//  pen.drawLines(lineA,2);
//  pen.drawLines(lineB,2);
//  pen.drawLines(lineC,2);
  
  TPoint q[4];
  q[0] = lineA[0];
  q[3] = lineC[1];
  
  TCoord ax, ay, bx, by, cross, dx, dy, a;
  
  ax = lineA[1].x - lineA[0].x;
  ay = lineA[1].y - lineA[0].y;
  bx = lineB[1].x - lineB[0].x;
  by = lineB[1].y - lineB[0].y;
  cross = ax*by - ay*bx;
  if (isZero(cross))
    return;
  dx = lineA[0].x - lineB[0].x;
  dy = lineA[0].y - lineB[0].y;
  a = (bx * dy - by * dx) / cross;
  q[1] = TPoint(lineA[0].x + a * ax, lineA[0].y + a * ay);
//  pen.drawCircle(q[1].x-2,q[1].y-2,5,5);

  ax = lineB[1].x - lineB[0].x;
  ay = lineB[1].y - lineB[0].y;
  bx = lineC[1].x - lineC[0].x;
  by = lineC[1].y - lineC[0].y;
  cross = ax*by - ay*bx;
  if (isZero(cross))
    return;
  dx = lineB[0].x - lineC[0].x;
  dy = lineB[0].y - lineC[0].y;
  a = (bx * dy - by * dx) / cross;
  q[2] = TPoint(lineB[0].x + a * ax, lineB[0].y + a * ay);
//  pen.drawCircle(q[2].x-2,q[2].y-2,5,5);

  pen.drawLines(q,4);

  TCoord e = 0.0;
  const TCoord step = 1.0 / 9.0;
  for(TCoord f=step; f<1.0; f+=step) {
  
    TCoord df = d0 + (d3-d0) * f;
    
    df = distOffset(pensize, bez2direction(p, f), pressure, rotation);
  
    if (df<0)
      df=-df;
  
    TCoord e0 = df - distance(bez2point(p, f), bez2point(q, f));
    if (e0<0)
      e0=-e0;
    if (e0>e)
      e=e0;
  }
    
  if (depth<10 && e>0.25) {
    TPoint r[7];
    divideBezier(p, r, 0.5);
    drawOffset(pen, r, d0, d0+(d3-d0)*0.5, depth+1);
    drawOffset(pen, r+3, d0+(d3-d0)*0.5, d3, depth+1);
  } else {
    ++cntr;
    pen.setColor(0,0.5,0);
    pen.drawBezier(q, 4);
  }
}

void
intersectLineEllipse(TPen &pen, const TPoint &l0, const TPoint &l1, const TPoint &center, const TSize &size, TCoord rotation)
{
  TPoint dist = l1 - l0;

  if (dist.x == 0)
    return;

  TCoord e = center.x;
  TCoord f = center.y;

  TCoord m  = dist.y / dist.x;
  TCoord b1 = l0.y - m * l0.x;
  TCoord b2 = b1 + (m*e-f);

  TCoord v = size.height;
  TCoord h = size.width;

  rotation = M_PI - rotation;
  
  TCoord cosa = cos(rotation);
  TCoord cos2a = cosa*cosa;
  TCoord sina = sin(rotation);
  TCoord sin2a = sina*sina;
  TCoord m2 = m*m;
  
  TCoord v2 = v*v;
  TCoord h2 = h*h;

#if 0
  TBitmap bmp(320,200);
  for(TCoord x=0; x<=320; x+=1) {
    for(TCoord y=0; y<=200; y+=1) {
      TCoord x0 = x - e;
      TCoord y0 = y - f;
      // TCoord d = (x0*x0) / (h*h) + (y0*y0) / (v*v);
      TCoord d = pow(x0*cosa+y0*sina,2) / h2 + pow(y0*cosa-x0*sina, 2) / v2;
      if (d>0.9 && d<1.1)
        bmp.setPixel(x,y,0,0.5,1);
      else
        bmp.setPixel(x,y,1,1,1);
    }
  }
  pen.drawBitmap(0,0,bmp);
#endif
  
  TCoord a = v2 * ( cos2a + 2*m*cosa*sina + m2*sin2a ) + h2 * ( m2*cos2a - 2*m*cosa*sina+sin2a);
  TCoord b = 2*v2*b2* ( cosa*sina + m*sin2a ) + 2*h2*b2 * ( m*cos2a - cosa*sina);
  TCoord c = b2*b2 * ( v2*sin2a + h2*cos2a ) - h2*v2;
  
  TCoord x = ( -b + sqrt(b*b-4*a*c) ) / (2*a) + e;
  TCoord y = m*x+b1;

  pen.setColor(1,0,0);
  pen.drawRectangle(x,y,3,3);

  x = ( -b - sqrt(b*b-4*a*c) ) / (2*a) + e;
  y = m*x+b1;
  pen.drawRectangle(x,y,3,3);
}

/**
 * \param pen
 * \param p   cubic bézier
 * \param q  transformer cubic bézier
 * \param u   location on bézier of cut line
 * \param v   location on bézier of ellipse
 * \param s   size of ellipse
 * \param r   rotation of ellipse (radiants)
 */
void
drawMinMax(TPen &pen, const TPoint *p, const TPoint *q, TCoord u, TCoord v, const TSize s, TCoord r)
{
  TPoint Pu = bez2point(p, u);

  TPoint d = bez2direction(p, u);

  TCoord A = atan2(d.y, d.x)+M_PI/2.0;

  TCoord h = s.height;
  TCoord w = s.width;


  TCoord cosa = cos(r-A);
  TCoord cos2a = cosa*cosa;
  TCoord sina = sin(r-A);
  TCoord sin2a = sina*sina;
  
  TCoord h2 = h*h;
  TCoord w2 = w*w;

  TPoint Q = bez2point(q, v);
  TCoord y=-Q.y;
  TCoord
    a = h2 * cos2a + w2 * sin2a,
    b = 2*y*cosa*sina*(h2-w2),
    c = y*y*(h2*sin2a+w2*cos2a)-w2*h2;

  TCoord sq = b*b-4*a*c;
  if (sq<0)
    return;
  sq = sqrt(sq);

  TCoord x0 = ( -b + sq ) / (2*a) + Q.x;
  TCoord x1 = ( -b - sq ) / (2*a) + Q.x;

  TPoint Pv = bez2point(p, v);
  
#if 0
  pen.setColor(1,0,0);
  pen.drawRectangle(x0+160-1,100-1,3,3);
  pen.drawRectangle(x1+160-1,100-1,3,3);
#endif
#if 0
  pen.drawRectangle(Pu.x-1, Pu.y-1, 3,3);
  pen.drawRectangle(Pv.x-1, Pv.y-1, 3,3);
#endif  
#if 1
  d.set(-d.y, d.x);
  d = normalize(d);
  TPoint P0 = Pu+d*x0;
  TPoint P1 = Pu+d*x1;
  pen.drawRectangle(P0.x-1, P0.y-1, 3,3);
  pen.drawRectangle(P1.x-1, P1.y-1, 3,3);
#endif
}

TCoord pressure[4] = { 0.8, 0.1, 1.5, 0.2 };

/**
 * \param pen
 * \param p   cubic bézier
 * \param q  transformer cubic bézier
 * \param u   location on bézier of cut line
 * \param s   size of ellipse
 * \param r   rotation of ellipse (radiants)
 */
void
drawMinMax2(TPen &pen, const TPoint *p, const TPoint *q, TCoord u, const TSize s, TCoord r)
{
cout << __FUNCTION__ << endl;
  // find 1st derivative over v to find extrema
  TPoint Pu = bez2point(p, u);

  TPoint d = bez2direction(p, u);

  TCoord R = atan2(d.y, d.x)+M_PI/2.0;

//  TPoint Q = bez2point(q, v);

TCoord x00, x10, v0, sq0=-1, dx00;
for(TCoord v=-1; v<=2.0; v+=0.001) {
  TCoord u=1-v;
  TCoord u2=u*u;
  TCoord u3=u2*u;
  TCoord v2=v*v;
  TCoord v3=v2*v;

  TPoint Q = q[0]*u3+q[1]*v*u2*3+q[2]*v2*u*3+q[3]*v3;
  TPoint dQ = (q[1]-q[0])*u2 + 2*(q[2]-q[1])*u*v + (q[3]-q[2])*v2;

  TCoord cosa = cos(r-R);
  TCoord cos2a = cosa*cosa;
  TCoord sina = sin(r-R);
  TCoord sin2a = sina*sina;
  
  TCoord h = s.height;
  TCoord w = s.width;
  TCoord h2 = h*h;
  TCoord w2 = w*w;
  
//cout << "dQ="<<dQ<<" =? ("<<dQx<<", "<<dQy<<")"<<endl;

  TCoord y=-Q.y;
  TCoord
    a = h2 * cos2a + w2 * sin2a,
    b = 2*y*cosa*sina*(h2-w2),
    c = y*y*(h2*sin2a+w2*cos2a)-w2*h2;

  TCoord
    B = -2*cosa*sina*(h2-w2),
    C = h2*sin2a+w2*cos2a,
    D = w2*h2;

  TCoord sq = b*b-4*a*c;
/*
  if (sq<0 || sq0<0) {
    v0=v;
    sq0=sq;
    continue;
  }
*/    
//TCoord x0 = ( B*y + sqrt( pow(B*y, 2) - 4 * a * pow(y,2)*C-D ) ) / ( 2*a + Q.x );
//TCoord x1 = ( -B*y - sqrt( pow(B*y, 2) - 4 * a * pow(y,2)*C-D ) ) / ( 2*a + Q.x );
  TCoord x0 = ( B*y + sqrt( pow(B*y, 2) - 4 * a * (pow(y,2)*C-D) ) ) / (2*a + Q.x);
  TCoord x1 = ( B*y - sqrt( pow(B*y, 2) - 4 * a * (pow(y,2)*C-D) ) ) / (2*a + Q.x);

  TCoord dx0, dx1, X0;
{
  TCoord
    A = 2*a,
    B = 2*cosa*sina*(h2-w2);

  X0 =
    ( B*Q.y + sqrt(pow(B*Q.y,2) - 2*A*C*pow(Q.y,2) + 2*A*D) )
  / 
    (A + Q.x);

  dx0 = ( B * dQ.y
          +   Q.y * dQ.y * (-2*A*C+pow(B,2))
            / 
              sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
        ) * (A+Q.x)
      - ( B*Q.y + sqrt(  pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D)  ) )
        * dQ.x;
  dx0 /= pow(A+Q.x,2);

  dx1 =   A*B * dQ.y   * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
        + B*dQ.y * Q.x * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
        - B*Q.y * dQ.x * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
        - 2*A*A*C* Q.y * dQ.y
        + A*B*B*Q.y * dQ.y
        - 2*A*C * Q.y * dQ.y * Q.x
        + B*B * Q.y * dQ.y * Q.x
        - B*B*Q.y*Q.y * dQ.x
        + 2*A*C*Q.y*Q.y * dQ.x
        - 2*A*D * dQ.x
        ;

  dx1 /= pow(A+Q.x,2) * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D));

  if (!isnan(dx0) && fabs(dx0-dx1)>tolerance ) {
    cerr << "differ: " << (dx0 - dx1) << endl;
    exit(0);
  }
}

//  if (sq>=0) {
//cout << x0 << ":" << dx0 << endl;
  if (v!=0) {
    pen.setColor(0,0,1);
    pen.drawLine(v0*160+80,x00+100, v*160+80,X0+100);
    pen.setColor(0,0.5,1);
    pen.drawLine(v0*160+80,dx00+100, v*160+80,dx0+100);
//    pen.drawRectangle(v*320,dx1+99,3,3);
  }
  v0=v; sq0=sq;
  x00=X0; x10=x1; dx00=dx0;
}
#if 0
  TPoint Q;

{
  TCoord u=1-v;
  TCoord u2=u*u;
  TCoord u3=u2*u;
  TCoord v2=v*v;
  TCoord v3=v2*v;
  Q = q[0]*u3+q[1]*v*u2*3+q[2]*v2*u*3+q[3]*v3;
  // dQ = (q[1]-q[0])*u2 + 2*(q[2]-q[1])*u*v + (q[3]-q[2])*v2;
}
  TCoord y=-Q.y;
  TCoord
    a = h2 * cos2a + w2 * sin2a,
    b = 2*y*cosa*sina*(h2-w2),
    c = y*y*(h2*sin2a+w2*cos2a)-w2*h2;

  TCoord
    B = -2*cosa*sina*(h2-w2),
    C = h2*sin2a+w2*cos2a,
    D = w2*h2;

  TCoord sq = b*b-4*a*c;
  if (sq<0)
    return;
    
//TCoord x0 = ( B*y + sqrt( pow(B*y, 2) - 4 * a * pow(y,2)*C-D ) ) / ( 2*a + Q.x );
//  TCoord x1 = ( -B*y - sqrt( pow(B*y, 2) - 4 * a * pow(y,2)*C-D ) ) / ( 2*a + Q.x );
  TCoord x0 = ( B*y + sqrt( pow(B*y, 2) - 4 * a * (pow(y,2)*C-D) ) ) / (2*a) + Q.x;
  TCoord x1 = ( B*y - sqrt( pow(B*y, 2) - 4 * a * (pow(y,2)*C-D) ) ) / (2*a) + Q.x;

//  sq = sqrt(sq);

//  TCoord x0 = ( -b + sq ) / (2*a) + Q.x;
//  TCoord x1 = ( -b - sq ) / (2*a) + Q.x;

  TPoint Pv = bez2point(p, v);

/*
          -b ± sqrt(b²-4*a*c)
  X(v)  = ────────────────────
               2a + x(v)
               
          -2*y(v)*cosa*sina*(h2-w2) ± sqrt( (2*y(v)*cosa*sina*(h2-w2))² - 4 * a * y(v)²*(h2*sin2a+w2*cos2a)-w2*h2 )
        = ─────────────────────────────────────────────────────────────────────────────────────────────────────────
          2a + x(v)

          B*y(v) ± sqrt( (B*y(v))² - 2 * A * (y(v)²*C-D) )
        = ─────────────────────────────────────────────────── 
          A + x(v)
          
          A := 2*a
          B := -2*cosa*sina*(h2-w2)
          C := h2*sin2a+w2*cos2a
          D := w2*h2
          x(v) := q[0]*u3+q[1]*v*u2*3+q[2]*v2*u*3+q[3]*v3
                = h   *u3+i   *v*u2*3+j   *v2*u*3+k   *v3
                = h*(1-v)³ + i*3*v*(1-v)² + j*3*v²*(1-v) + k*v³
          y(v)  = l*(1-v)³ + m*3*v*(1-v)² + n*3*v²*(1-v) + o*v³

  ⇒ (assisted by https://www.symbolab.com/solver/first-derivative-calculator/)
  
          ⎛          y(v) * y'(v) * 2 * (-2*A*C+B²)    ⎞   ⎛            ⎞   ⎛                                              ⎞
          ⎜B*y'(v) + ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎟ * ⎜A+x(v)-x'(v)⎟ * ⎜ B*y(v) + sqrt( -2*A*(C*y(v)²-D) + B²*y(v)² ) ⎟
          ⎝          sqrt(-2*A * (C*y(v)²-D) + B²*y(v)²⎠   ⎝            ⎠   ⎝                                              ⎠
  X'(v) = ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
          (A+x(v))²
  
  ⇒
      
          ⎛          y(v) * y'(v) * (-2*A*C+B²)        ⎞   ⎛            ⎞   ⎛                                              ⎞
      0 = ⎜B*y'(v) + ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎟ * ⎜A+x(v)-x'(v)⎟ * ⎜ B*y(v) + sqrt( -2*A*(C*y(v)²-D) + B²*y(v)² ) ⎟
          ⎝          sqrt(-2*A * (C*y(v)²-D) + B²*y(v)²⎠   ⎝            ⎠   ⎝                                              ⎠
          
*/
  
#if 1
  pen.setColor(0.5,0,0);
  pen.drawRectangle(x0+160-1,100-1,3,3);
  pen.drawRectangle(x1+160-1,100-1,3,3);
#endif
#if 0
  pen.setColor(1,0,0);
  pen.drawRectangle(Pu.x-1, Pu.y-1, 3,3);
  pen.drawRectangle(Pv.x-1, Pv.y-1, 3,3);
#endif  
#if 1
  pen.setColor(1,0,0);
  d.set(-d.y, d.x);
  d = normalize(d);
  TPoint P0 = Pu+d*x0;
  TPoint P1 = Pu+d*x1;
cout << P0 << ", " << P1 << endl;
  pen.drawRectangle(P0.x-1, P0.y-1, 3,3);
  pen.drawRectangle(P1.x-1, P1.y-1, 3,3);
#endif
#endif
}

/**
 * \param pen
 * \param p  cubic bézier
 * \param q  transformed cubic bézier
 * \param u  location on bézier of cut line
 * \param s  size of ellipse
 * \param r  rotation of ellipse (radiants)
 */
void
drawMinMax3(TPen &pen, const TPoint *p, const TPoint *q, TCoord u, const TSize s, TCoord r)
{
cout << __FUNCTION__ << endl;
  TPoint Pu = bez2point(p, u);

  TPoint d = bez2direction(p, u);
  TCoord R = atan2(d.y, d.x)+M_PI/2.0;
  
  d.set(-d.y, d.x);
  d = normalize(d);

  TCoord x00, x10, v0, sq0=-1, dx00;
  for(TCoord v=0; v<=0.4; v+=0.01) {
    TCoord u=1-v;
    TCoord u2=u*u;
    TCoord u3=u2*u;
    TCoord v2=v*v;
    TCoord v3=v2*v;

    // Q  := curve(v)
    TPoint Q = q[0]*u3+q[1]*v*u2*3+q[2]*v2*u*3+q[3]*v3;
    // dQ := curve'(v)
    TPoint dQ = (q[1]-q[0])*u2 + 2*(q[2]-q[1])*u*v + (q[3]-q[2])*v2;
    TCoord pressureAtV = pressure[0]*u3+pressure[1]*v*u2*3+pressure[2]*v2*u*3+pressure[3]*v3;

    TCoord cosa = cos(r-R);
    TCoord cos2a = cosa*cosa;
    TCoord sina = sin(r-R);
    TCoord sin2a = sina*sina;
  
    TCoord h = s.height*pressureAtV;
    TCoord w = s.width*pressureAtV;
    TCoord h2 = h*h;
    TCoord w2 = w*w;
  
//cout << "dQ="<<dQ<<" =? ("<<dQx<<", "<<dQy<<")"<<endl;

    TCoord y=-Q.y;
    TCoord
      a = h2 * cos2a + w2 * sin2a,
      b = 2*y*cosa*sina*(h2-w2),
      c = y*y*(h2*sin2a+w2*cos2a)-w2*h2;

    TCoord
      B = -2*cosa*sina*(h2-w2),
      C = h2*sin2a+w2*cos2a,
      D = w2*h2;

    // intersection of circle with line 
    // FIXME: but this somehow wrong!!! results are relative to curve(v), curve(u)).
    // drawMinMax get this one right!
    // also: only pick x0 and x1, when they are around curve(u)
    TCoord x0 = ( B*y + sqrt( pow(B*y, 2) - 4 * a * (pow(y,2)*C-D) ) ) / (2*a + Q.x);
    TCoord x1 = ( B*y - sqrt( pow(B*y, 2) - 4 * a * (pow(y,2)*C-D) ) ) / (2*a + Q.x);
    
    TCoord dx0, dx1, X0;
    {
      TCoord
        A = 2*a,
        B = 2*cosa*sina*(h2-w2);
      
      X0 =
        ( B*Q.y + sqrt(pow(B*Q.y,2) - 2*A*C*pow(Q.y,2) + 2*A*D) )
      / 
        (A + Q.x);
        
      dx0 = ( B * dQ.y
              +   Q.y * dQ.y * (-2*A*C+pow(B,2))
                / 
                  sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
            ) * (A+Q.x)
          - ( B*Q.y + sqrt(  pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D)  ) )
            * dQ.x;
      dx0 /= pow(A+Q.x,2);
      
      dx1 =   A*B * dQ.y   * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
            + B*dQ.y * Q.x * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
            - B*Q.y * dQ.x * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D))
            - 2*A*A*C* Q.y * dQ.y
            + A*B*B*Q.y * dQ.y
            - 2*A*C * Q.y * dQ.y * Q.x
            + B*B * Q.y * dQ.y * Q.x
            - B*B*Q.y*Q.y * dQ.x
            + 2*A*C*Q.y*Q.y * dQ.x
            - 2*A*D * dQ.x
            ;
            
      dx1 /= pow(A+Q.x,2) * sqrt(pow(B*Q.y,2) - 2*A*(C*pow(Q.y,2)-D));
      
      if (!isnan(dx0) && fabs(dx0-dx1)>tolerance ) {
        cerr << "differ: " << (dx0 - dx1) << endl;
//        exit(0);
      }
    }
      
    if (v!=0) {
      pen.setColor(0,0,1);
      pen.drawLine(v0*160+80,x00+100, v*160+80,X0+100);
      pen.setColor(0,0.5,1);
      pen.drawLine(v0*160+80,dx00+100, v*160+80,dx0+100);
    }
#if 0    
    TPoint P0 = Pu+x0*d;
    TPoint P1 = Pu+x1*d;
    pen.setColor(1,0,0);
    pen.drawRectangle(P0.x, P0.y, 0.5,0.5);
    pen.drawRectangle(P1.x, P1.y, 0.5,0.5);
#endif
    v0=v;
    x00=X0; x10=x1; dx00=dx0;
  }
}


void
TMyWindow::paint()
{
  TPen pen(this);
  pen.translate(-0.5, -0.5);
  
//  TPoint c = p[0];
  TSize s(40,10);
  
  TCoord rotation = 25;
  
//  TPoint d(50,-50); // direction

#if 0
  // min/max of rotated ellispe

  TPoint d = p[1]-c;

  pen.setColor(1,0,0);
  pen.drawLine(c.x, c.y, c.x+d.x, c.y+d.y);
  pen.setColor(0,0,0);
  TCoord dr = atan2(d.x, d.y);

  pen.drawLine(c.x, c.y, c.x+sin(dr)*20, c.y+cos(dr)*20);
  
  // draw circle
  TPoint pr0, po0;
  
  for(TCoord i = 0.0; i<=360.0; i+=20.0) {
    // rotation matrix
    TCoord r11, r12, r21, r22;
    // point rotated
    TPoint pr;
    // point oriented to calculate min/max
    TPoint po;
    // degree to radians
    TCoord r = i/360.0 * 2.0 * M_PI;
    // point on circle
    pr = po = TPoint(sin(r)*s.width, cos(r)*s.height);
    
    // pen rotation to radians
    TCoord pen_r = (180-rotation)/360.0 * 2.0 * M_PI;

    r11 = r22 = cos(pen_r);
    r21 = sin(pen_r);
    r12 = -r21;

    pr.set(r11 * pr.x + r12 * pr.y,
           r21 * pr.x + r22 * pr.y);

    // pen rotation + direction
    r11 = r22 = cos(-dr); 
    r21 = sin(-dr);
    r12 = -r21;
    
    po.set(r11 * po.x + r12 * po.y, 
           r21 * po.x + r22 * po.y);
  
    if (i>0.0) {
      pen.setColor(0,0.5,0);
      pen.drawLine(pr0+c, pr+c);
      pen.setColor(0,0,0);
      pen.drawLine(po0+c, po+c);
    }
    pr0 = pr;
    po0 = po;
  }

  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(p, 2);
  pen.setFillColor(1,1,1);
  for(auto i=0; i<2; ++i) {
    pen.fillRectangle(p[i].x-2, p[i].y-2, 5, 5);
    pen.drawRectangle(p[i].x-2, p[i].y-2, 5, 5);
  }
  
  // find extrema of black ellipse
  TSize size = s;
  TCoord r = -dr;
  
  TCoord u = atan(tan(r) * size.width/size.height);
//cout << u << endl;
  TCoord y = (sin(r) * sin(u)*size.width + cos(r) * cos(u)*size.height);
  if (y<0)
    y=-y;
  pen.setColor(1,0,0);
  pen.drawLine(0,y+c.y,320,y+c.y);
#endif
#if 0
  // using min/max of ellipse to calculate offset curve (result sucks)
  pen.drawBezier(p, 4);

  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(p, 4);
  pen.setFillColor(1,1,1);
  for(auto i=0; i<4; ++i) {
    pen.fillRectangle(p[i].x-2, p[i].y-2, 5, 5);
    pen.drawRectangle(p[i].x-2, p[i].y-2, 5, 5);
  }

  pen.setColor(0,0.5,0);
  cntr=0;
//  drawOffset(pen, p, 0, -10);

  TSize pensize(20,5);
//  TCoord rotation = 0.0;
  TCoord pressure = 1.0;
  for(TCoord f=0.0; f<=1.0; f+=0.01) {
    TPoint b = bez2point(p, f);
    TPoint d = bez2direction(p, f);
    TCoord o = distOffset(pensize, d, pressure, rotation);
    d.set(-d.y, d.x);
    d = normalize(d)*o;
    TPoint pt = bez2point(p, f) + normalize(d)*o;
    
    pen.drawLine(pt.x, pt.y, b.x, b.y);
    pen.drawRectangle(pt.x, pt.y, 1, 1);
  }

//  cout << cntr << endl;
#endif
#if 0
  // intersection ellipse & line
  c = p[1];

  TPoint d = p[2]-c;

  pen.setColor(0,0,0);
  TCoord dr = atan2(d.x, d.y);
  
  s.width = distance(p[0], p[1]);
  s.height = distance(p[1], p[2]);

  // draw circle
  TPoint pr0, po0;

  intersectLineEllipse(pen, p[3], p[4], p[1], s, dr);
  pen.setColor(0,0,0);
  
  for(TCoord i = 0.0; i<=360.0; i+=5.0) {
    // rotation matrix
    TCoord r11, r12, r21, r22;
    TCoord r = i/360.0 * 2.0 * M_PI;
    TPoint pr = TPoint(sin(r)*s.width, cos(r)*s.height);
    
    // pen rotation to radians
    TCoord pen_r = (180-rotation)/360.0 * 2.0 * M_PI;

    r11 = r22 = cos(-dr);
    r21 = sin(-dr);
    r12 = -r21;

    pr.set(r11 * pr.x + r12 * pr.y,
           r21 * pr.x + r22 * pr.y);

    if (i>0.0) {
      pen.drawLine(pr0+c, pr+c);
    }
    pr0 = pr;
  }
  
  // draw selection
  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(p, 3);
  pen.drawLines(p+3,2);
  pen.setFillColor(1,1,1);
  for(auto i=0; i<5; ++i) {
    pen.fillRectangle(p[i].x-2, p[i].y-2, 5, 5);
    pen.drawRectangle(p[i].x-2, p[i].y-2, 5, 5);
  }
#endif

#if 1
  TCoord u = 0.5;
  TCoord V = 0.6;

  TPoint C = bez2point(p, u);
  TPoint D = bez2point(p, V);

  TPoint d = bez2direction(p, u);

  rotation = 90; // nib rotation in degree

  TCoord A = atan2(d.y, d.x)+M_PI/2.0;

  TCoord pen_r = rotation/360.0 * 2.0 * M_PI;
//  pen_r += a;
  
//  rotation = M_PI-a;

  TCoord v = s.height;
  TCoord h = s.width;
  
  TCoord y = 0;
  
  TCoord cosa = cos(pen_r-A);
  TCoord cos2a = cosa*cosa;
  TCoord sina = sin(pen_r-A);
  TCoord sin2a = sina*sina;
  
  TCoord v2 = v*v;
  TCoord h2 = h*h;

#if 0
  // draw nib using formula used in intersection algorithm
{
  TBitmap bmp(320,200);
  for(TCoord x=0; x<=320; x+=1) {
    for(TCoord y=0; y<=200; y+=1) {
      // TCoord d = (x0*x0) / (h*h) + (y0*y0) / (v*v);
      TCoord x0=x-D.x;
      TCoord y0=y-D.y;
      TCoord d = pow(x0*cosa+y0*sina,2) / h2 + pow(y0*cosa-x0*sina, 2) / v2;
      if (d>0.9 && d<1.1)
        bmp.setPixel(x+D.x,y+D.y,0,0.5,1);
      else
        bmp.setPixel(x+D.x,y+D.y,1,1,1);
    }
  }
  pen.drawBitmap(0,0,bmp);
}
#endif

  // draw nib
#if 0
  pen.setColor(0,0.5,0);
  TPoint pr0;
  for(TCoord i = 0.0; i<=360.0; i+=20.0) {
    TCoord r11, r12, r21, r22;
    TCoord r = i/360.0 * 2.0 * M_PI;
    TPoint pr(sin(r)*s.width, cos(r)*s.height);
    r11 = r22 = cos(pen_r);
    r21 = sin(pen_r);
    r12 = -r21;
    pr.set(r11 * pr.x + r12 * pr.y, r21 * pr.x + r22 * pr.y);
    if (i>0.0) {
      pen.drawLine(pr0+D, pr+D);
    }
    pr0 = pr;
  }
#endif

  // draw normal
{
  pen.setColor(0,0.5,0);
  d.set(-d.y, d.x);
  d = normalize(d)*200.0;
  TPoint pt0 = C + d;
  TPoint pt1 = C - d;
  pen.drawLine(pt0.x, pt0.y, pt1.x, pt1.y);
//  pen.drawRectangle(pt.x, pt.y, 1, 1);
}

  TPoint p0[4];
  TMatrix2D mat;
  mat.rotate(-A);
  for(int i=0;i<4;++i) {
    p0[i]=mat.map(p[i]-C);
  }
  for(TCoord i=0.0; i<=1.0; i+=0.05) {
    pen.setColor(1,0,0);
//    drawMinMax(pen, p, p0, u, i, s, pen_r);
    // draw nib
    pen.setColor(0.5,1,0.5);

    TCoord v = i;
    TCoord u=1-v;
    TCoord u2=u*u;
    TCoord u3=u2*u;
    TCoord v2=v*v;
    TCoord v3=v2*v;
    TCoord pressureAtV = pressure[0]*u3+pressure[1]*v*u2*3+pressure[2]*v2*u*3+pressure[3]*v3;

    TPoint pr0;
    TPoint D = bez2point(p, i);
//    D.x-=160;
//    D.y-=100;
    for(TCoord j = 0.0; j<=360.0; j+=20.0) {
      TCoord r11, r12, r21, r22;
      TCoord r = j/360.0 * 2.0 * M_PI;
      TPoint pr(sin(r)*s.width*pressureAtV, cos(r)*s.height*pressureAtV);
      r11 = r22 = cos(pen_r);
      r21 = sin(pen_r);
      r12 = -r21;
      pr.set(r11 * pr.x + r12 * pr.y, r21 * pr.x + r22 * pr.y);
      if (j>0.0) {
        pen.drawLine(pr0+D, pr+D);
      }
      pr0 = pr;
    }
  }
  drawMinMax3(pen, p, p0, u, s, pen_r);

  pen.setColor(0,0,0);
  pen.drawBezier(p, 4);
  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(p, 4);
  pen.setFillColor(1,1,1);
  for(auto i=0; i<4; ++i) {
    pen.fillRectangle(p[i].x-2, p[i].y-2, 5, 5);
    pen.drawRectangle(p[i].x-2, p[i].y-2, 5, 5);
  }

  pen.setColor(0,0,0);
  pen.translate(160,100);

  pen.drawLine(-320,0, 320,0);
  pen.drawLine(0,-200,0,200);
  
  pen.setColor(0.8,0.8,0.8);
  TPoint Q = bez2point(p0, V);
  pen.drawLine(-320,Q.y, 320,Q.y);

  pen.setColor(0,1,0);
  pen.drawRectangle(Q.x-1, Q.y-1, 3,3);
#if 0
  pen.setColor(1,0.5,0);
  pen.drawLines(p0,4);
  pen.drawBezier(p0,4);

  // draw nib
  for(TCoord i = 0.0; i<=360.0; i+=20.0) {
    TCoord r11, r12, r21, r22;
    TCoord r = i/360.0 * 2.0 * M_PI;
    TPoint pr(sin(r)*s.width, cos(r)*s.height);
    r11 = r22 = cos(pen_r-A);
    r21 = sin(pen_r-A);
    r12 = -r21;
    pr.set(r11 * pr.x + r12 * pr.y, r21 * pr.x + r22 * pr.y);
    pr += Q;
    if (i>0.0) {
      pen.drawLine(pr0, pr);
    }
    pr0 = pr;
  }
#endif
#endif

}

void
TMyWindow::mouseEvent(const TMouseEvent &me)
{
  switch(me.type) {
    case TMouseEvent::LDOWN:
      handle=-1;
      for(int i=0; i<8; ++i) {
        if(p[i].x-4<=me.pos.x && me.pos.x <= p[i].x+4 &&
           p[i].y-4<=me.pos.y && me.pos.y <= p[i].y+4)
        {
          handle = i;
          break;
        }
      }
      break;
    case TMouseEvent::MOVE:
      if (handle==-1)
        break;
#if 0
      if (handle==0) {
        TPoint a = p[2]-p[1];
        TPoint n = TPoint(-a.y, a.x) * (distance(p[1], me.pos)/ distance(p[2], p[1]));
        p[0] = p[1]+n;
      } else {
        TCoord d = distance(p[0], p[1]);
        p[handle] = me.pos;
        TPoint a = p[2]-p[1];
        TPoint n = TPoint(-a.y, a.x) * (d/ distance(p[2], p[1]));
        p[0] = p[1]+n;
      }
#else
      p[handle] = me.pos;
#endif
      invalidateWindow();
      break;
    case TMouseEvent::LUP:
      handle = -1;
      break;
  }
}

class TNewtonHorner:
  public TWindow
{
  public:
    TNewtonHorner(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
};

double
f(double *a, size_t n, double x)
{
  double r = a[0];
  for(int i=1; i<=n; ++i) {
    r = r * x + a[i];
  }
  return r;
}

double
df(double *a, size_t n, double x)
{
  double r = a[0] * n;
  for(int i=1; i<n; ++i) {
    r = r * x + a[i] * (n-i);
  }
  return r;
}

void
TNewtonHorner::paint()
{
  TPen pen(this);
  pen.translate(160.5, 100.5);
  pen.drawLine(-160,0,160,0);
  pen.drawLine(0,-100,0,100);

  size_t n = 3;
  double a[4] = {
    5,
    -20,
    10,
    10
  };

  double y0, x0;
  for(double x=-16; x<16; x+=0.1) {
    double y=f(a, n, x);
    if (x>-16) {
      pen.drawLine(x0*10,y0, x*10,y);
    }
    y0=y; x0=x;
  }
  
  double x = 16000; // start
  for(int i=0; i<3; ++i) {
  
    // newton-raphson
    for(int i=0; i<200000; ++i) {
      double x0 = x - f(a, n, x)/df(a, n, x);
      if (x0 == x) // no refinement, done
        break;
      x=x0;
    }
  
    // f / (x-x0) via synthetic division (based on horner schema)
    for(int i=1; i<=n; ++i) {
      a[i] = a[i-1] * x + a[i];
    }
    --n;
  
    pen.setColor(1,0,0);
    pen.drawRectangle(x*10-1,-1,3,3);
  }
  
}

class TPressure:
  public TWindow
{
  public:
    TPressure(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
    }
    
    void paint() override;
};

void
TPressure::paint()
{
  TPen pen(this);
  
  double y0, x0;
  for(double v=0; v<=1.0; v+=0.01) {
    TCoord u=1-v;
    TCoord u2=u*u;
    TCoord u3=u2*u;
    TCoord v2=v*v;
    TCoord v3=v2*v;
//    TCoord q[4]={0,0.33333,0.66666,1}; // linear
    TCoord q[4]={1,0,1,0};
    double p = q[0]*u3+q[1]*v*u2*3+q[2]*v2*u*3+q[3]*v3;
    
    cout << v << ": " << p << endl;
    
    pen.drawRectangle(v*320, 200-p*200,1,1);
  }
}

void
curveYatX(const TPoint *vc, const TCoord x)
{
  TCoord
    d = vc[0].x - x,
    c1 = vc[1].x - x,
    c2 = vc[2].x - x,
    p2 = vc[3].x - x,
    c = 3 * (c1 - d),
    b = 3 * (c2 - c1) - c,
    a = p2 - d - c - b;

  // If both a and b are near zero, we should treat the curve as a line in
  // order to find the right solutions in some edge-cases in
  // Curve.getParameterOf()
  if (isZero(a) && isZero(b))
    a = b = 0;
  
  TCoord roots[4];
  int count = toad::solveCubic(a, b, c, d, roots, 0, 1);

  // NOTE: count could be -1 for infinite solutions, but that should only
  // happen with lines, in which case we should not be here.
  for(int i = 0; i < count; ++i) {
    cout << bez2point(vc, roots[i]) << endl;
  }
}

struct TFreehandPoint//:
//  public TPoint
{
  TFreehandPoint(): x(0), y(0), rotation(0), pressure(1.0) {}
  TFreehandPoint(TCoord _x, TCoord _y, TCoord r, TCoord p):x(_x),y(_y),rotation(r),pressure(p) {}
  TCoord x,y,rotation, pressure;
  const TFreehandPoint& operator=(TFreehandPoint const &a) {
    x=a.x; y=a.y; rotation=a.rotation; pressure=a.pressure;
    return *this;
  }
  inline TFreehandPoint operator-() const {
    return TFreehandPoint(-x, -y, -rotation, -pressure);
  }
};

inline bool operator==(const TFreehandPoint &a, const TFreehandPoint &b) {
  return a.x==b.x && a.y==b.y && a.rotation==b.rotation && a.pressure==b.pressure;
}
inline bool operator!=(const TFreehandPoint &a, const TFreehandPoint &b) {
  return a.x!=b.x || a.y!=b.y || a.rotation!=b.rotation || a.pressure!=b.pressure;
}
inline TFreehandPoint operator-(const TFreehandPoint &a, const TFreehandPoint &b) {
  return TFreehandPoint(a.x-b.x, a.y-b.y, a.rotation-b.rotation, a.pressure-b.pressure);
}
inline TFreehandPoint operator+(const TFreehandPoint &a, const TFreehandPoint &b) {
  return TFreehandPoint(a.x+b.x, a.y+b.y, a.rotation+b.rotation, a.pressure+b.pressure);
}
inline TFreehandPoint operator*(TCoord a, const TFreehandPoint &b) {
  return TFreehandPoint(a*b.x, a*b.y, a*b.rotation, a*b.pressure);
}
inline TFreehandPoint operator*(const TFreehandPoint &b, TCoord a) {
  return TFreehandPoint(a*b.x, a*b.y, a*b.rotation, a*b.pressure);
}
inline TFreehandPoint operator/(const TFreehandPoint &a, TCoord b) {
  return TFreehandPoint(a.x/b, a.y/b, a.rotation/b, a.pressure/b);
}
inline TCoord squaredLength(const TFreehandPoint &a) {
  return a.x*a.x + a.y*a.y + a.rotation*a.rotation + a.pressure*a.pressure;
}
inline TCoord length(const TFreehandPoint &a) {
  return sqrt(squaredLength(a));
}
inline TCoord distance(const TFreehandPoint &a, const TFreehandPoint &b) {
  return length(a-b);
}
inline TFreehandPoint normalize(const TFreehandPoint &v) {
  TCoord len = length(v);
  if (len != 0.0) {
    return v/len;
  }
  return v;
}
/* return the dot product of vectors a and b */
inline TCoord dot(const TFreehandPoint &a, const TFreehandPoint &b) {
  // FIXME? is this correct for 4dimensional vector?
  return a.x*b.x+a.y*b.y+a.rotation*b.rotation+a.pressure*b.pressure;
}



/*
 * Philip J. Schneider. “An Algorithm for Automatically Fitting Digitized
 * Curves”.  In Graphics Gems, Academic Press, 1990, pp.  612–626.
 *
 * This code is in the public domain.
 */

// NOTE: should look for other algorithms too
//       ie. "least-squares, approximations to bézier curves and surfaces"
//       from Graphic Gems II, p.406

/* returns squared length of input vector */	
/*
 *  B0, B1, B2, B3 :
 *	Bezier multipliers
 */
static inline double
B0(double u)
{
  double tmp = 1.0 - u;
  return (tmp * tmp * tmp);
}

static inline double
B1(double u)
{
  double tmp = 1.0 - u;
  return (3 * u * (tmp * tmp));
}

static inline double
B2(double u)
{
  double tmp = 1.0 - u;
  return (3 * u * u * tmp);
}

static inline double
B3(double u)
{
  return (u * u * u);
}

/*
 *  GenerateBezier :
 *  Use least-squares method to find Bezier control points for region.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 *  \param uPrime Parameter values for region
 *  \param tHat1, tHat2 Unit tangents at endpoints
 *
 */
template <class T>
void generateBezier(const T* d, int first, int last, const double* uPrime, T tHat1, T tHat2, T* bezCurve)
{
  int 	i;
  int 	nPts;				/* Number of pts in sub-curve 	*/
  double 	C[2][2];		/* Matrix C			*/
  double 	X[2];			/* Matrix X			*/
  double 	det_C0_C1,		/* Determinants of matrices	*/
    	   	det_C0_X,
	   	det_X_C1;
  double 	alpha_l,		/* Alpha values, left and right	*/
    	   	alpha_r;
  T		tmp;			/* Utility variable		*/
  nPts = last - first + 1;
  // CGPoint      A[nPts][2];		/* Precomputed rhs for eqn      */
  unique_ptr<TFreehandPoint[]> A0(new TFreehandPoint[nPts]);
  unique_ptr<TFreehandPoint[]> A1(new TFreehandPoint[nPts]);
 
  /* Compute the A's	*/
  for (i = 0; i < nPts; i++) {
    A0[i] = tHat1 * B1(uPrime[i]);
    A1[i] = tHat2 * B2(uPrime[i]);
  }

  /* Create the C and X matrices */
  C[0][0] = 0.0;
  C[0][1] = 0.0;
  C[1][0] = 0.0;
  C[1][1] = 0.0;
  X[0]    = 0.0;
  X[1]    = 0.0;

  for (i = 0; i < nPts; i++) {
    C[0][0] += dot(A0[i], A0[i]);
    C[0][1] += dot(A1[i], A1[i]);
    // C[1][0] += dot(A[i][0], A[i][1]);
    C[1][0] = C[0][1];
    C[1][1] += dot(A1[i], A1[i]);
    tmp = d[first + i]
          - d[first]* B0(uPrime[i])
          - d[first]* B1(uPrime[i])
          - d[last] * B2(uPrime[i])
          - d[last] * B3(uPrime[i]);
    X[0] += dot(A0[i], tmp);
    X[1] += dot(A1[i], tmp);
  }

  /* Compute the determinants of C and X	*/
  det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  det_C0_X  = C[0][0] * X[1]    - C[1][0] * X[0];
  det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

  /* Finally, derive alpha values	*/
  alpha_l = (det_C0_C1 == 0) ? 0.0 : det_X_C1 / det_C0_C1;
  alpha_r = (det_C0_C1 == 0) ? 0.0 : det_C0_X / det_C0_C1;

  /* If alpha negative, use the Wu/Barsky heuristic (see text) */
  /* (if alpha is 0, you get coincident control points that lead to
   * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
  double segLength = distance(d[last], d[first]);
  double epsilon = 1.0e-6 * segLength;
  if (alpha_l < epsilon || alpha_r < epsilon) {
    /* fall back on standard (probably inaccurate) formula, and subdivide further if needed. */
    double dist = segLength / 3.0;
    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    bezCurve[1] = bezCurve[0] + tHat1 * dist;
    bezCurve[2] = bezCurve[3] + tHat2 * dist;
    return;
  }

  /*  First and last control points of the Bezier curve are */
  /*  positioned exactly at the first and last data points */
  /*  Control points 1 and 2 are positioned an alpha distance out */
  /*  on the tangent vectors, left and right, respectively */
  bezCurve[0] = d[first];
  bezCurve[3] = d[last];
  bezCurve[1] = bezCurve[0] + tHat1 * alpha_l;
  bezCurve[2] = bezCurve[3] + tHat2 * alpha_r;
}

/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 *    \param degree The degree of the bezier curve
 *    \param V      Array of control points
 *    \param t      Parametric value to find point for
 */
template <class T>
T evaluateBezier(int degree, const T *V, double t)
{
  assert(degree<=3);
  int i, j;

  /* Copy array	*/
  T Vtemp[4];     	/* Local copy of control points         */
  for (i = 0; i <= degree; i++) {
    Vtemp[i] = V[i];
  }

  /* Triangle computation	*/
  for (i = 1; i <= degree; i++) {	
    for (j = 0; j <= degree-i; j++) {
      Vtemp[j] = (1.0 - t) * Vtemp[j] + t * Vtemp[j+1];
    }
  }

  return Vtemp[0]; // Point on curve at parameter t
}

/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 *  \param Q Current fitted curve
 *  \param P Digitized point
 *  \param u Parameter value for "P"
 */
template <class T>
double newtonRaphsonRootFind(const T* Q, T P, double u)
{
  double numerator, denominator;
  T Q1[3], Q2[2];    // Q' and Q''
  T Q_u, Q1_u, Q2_u; // u evaluated at Q, Q', & Q''
  int i;
    
  /* Compute Q(u)	*/
  Q_u = evaluateBezier(3, Q, u);
    
  /* Generate control vertices for Q'	*/
  for (i = 0; i <= 2; i++) {
    Q1[i] = (Q[i+1] - Q[i]) * 3.0;
  }
    
  /* Generate control vertices for Q'' */
  for (i = 0; i <= 1; i++) {
    Q2[i] = (Q1[i+1] - Q1[i]) * 2.0;
  }
    
  /* Compute Q'(u) and Q''(u)	*/
  Q1_u = evaluateBezier(2, Q1, u);
  Q2_u = evaluateBezier(1, Q2, u);
    
  /* Compute f(u)/f'(u) */
  numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
  denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		      	  (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
  if (denominator == 0.0f) return u;

  /* u = u - f(u)/f'(u) */
  return u - (numerator/denominator);
}

/*
 *  Reparameterize:
 *   Given set of points and their parameterization, try to find
 *   a better parameterization.
 *   \param d Array of digitized points
 *   \param first, last Indices defining region
 *   \param[inout] u Current parameter values
 *   \param bezCurve Current fitted curve
 */
template <class T>
void reparameterize(const T *d, int first, int last, double *u, const T* bezCurve)
{
  for (int i = first; i <= last; i++) {
    u[i-first] = newtonRaphsonRootFind(bezCurve, d[i], u[i-first]);
  }
}

/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 * Approximate unit tangents at endpoints and "center" of digitized curve
 * \param d Digitized points
 * \param end Index to "left" end of region
 */
template <class T>
inline T computeLeftTangent(const T* d, int end)
{
  return normalize(d[end+1] - d[end]);
}

/**
 * \param d Digitized points
 * \param end Index to "right" end of region
 */
template <class T>
inline T computeRightTangent(const T *d, int end)
{
  return normalize(d[end-1] - d[end]);
}

/**
 * \param d Digitized points
 * \param center Index to point inside region
 */
template <class T>
T computeCenterTangent(const T *d, int center)
{
  T V1, V2, tHatCenter;

  V1 = d[center-1] - d[center];
  V2 = d[center] - d[center+1];
  tHatCenter = (V1 + V2)/2.0;
  return normalize(tHatCenter);
}


/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points 
 *	using relative distances between points.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 */
template <class T>
void chordLengthParameterize(const T *d, int first, int last, double *u)
{
  int i;	

  u[0] = 0.0;
  for (i = first+1; i <= last; i++) {
    u[i-first] = u[i-first-1] +	distance(d[i], d[i-1]);
  }

  for (i = first + 1; i <= last; i++) {
    u[i-first] = u[i-first] / u[last-first];
  }
}

/*
 *  ComputeMaxError :
 *	Find the maximum squared distance of digitized points
 *	to fitted curve.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 *  \param bezCurve Fitted Bezier curve
 *  \param u Parameterization of points
 *  \param splitPoint Point of maximum error
 */
template <class T>
double computeMaxError(const T *d, int first, int last, const T* bezCurve, const double *u, int *splitPoint)
{
  int		i;
  double	maxDist;	/*  Maximum error		*/
  double	dist;		/*  Current error		*/
  T		P;		/*  Point on curve		*/
  T		v;		/*  Vector from point to curve	*/

  *splitPoint = (last - first + 1)/2;
  maxDist = 0.0;
  for (i = first + 1; i < last; i++) {
    P = evaluateBezier(3, bezCurve, u[i-first]);
    v = P - d[i];
    dist = squaredLength(v);
    if (dist >= maxDist) {
      maxDist = dist;
      *splitPoint = i;
    }
  }
  return maxDist;
}

template <class T>
inline void drawBezierCurve(int n, const T *curve, vector<T> *pathOut)
{
  for(int i=0; i<n; ++i)
    pathOut->push_back(curve[i]);
}

/*
 *  FitCubic :
 *  	Fit a Bezier curve to a (sub)set of digitized points
 *  \param d Array of digitized points
 *  \param first, last Indices of first and last pts in region
 *  \param tHat1, tHat2 Unit tangent vectors at endpoints
 *  \param error User-defined error squared
 */
template <class T>
void fitCubic(const T *d, int first, int last, const T& tHat1, const T& tHat2, double error, vector<T> *out)
{
  T	        curve[4];	/*Control points of fitted Bezier curve*/
  double	maxError;	/*  Maximum fitting error	 */
  int		splitPoint;	/*  Point to split point set at	 */
  int		nPts;		/*  Number of points in subset  */
  double	iterationError; /*Error below which you try iterating  */
  int		maxIterations = 4; /*  Max times to try iterating  */
  T		tHatCenter;   	/* Unit tangent vector at splitPoint */
  int		i;		

  iterationError = error * error;
  nPts = last - first + 1;

  /*  Use heuristic if region only has two points in it */
  if (nPts == 2) {
    double dist = distance(d[last], d[first]) / 3.0;
    curve[0] = d[first];
    curve[3] = d[last];
    curve[1] = curve[0] + tHat1 * dist;
    curve[2] = curve[3] + tHat2 * dist;
    drawBezierCurve(3, curve, out);
    return;
  }

  /*  Parameterize points, and attempt to fit curve */
  double u[last-first+1]; // parameter values for point
  chordLengthParameterize(d, first, last, u);
  generateBezier(d, first, last, u, tHat1, tHat2, curve);

  /*  Find max deviation of points to fitted curve */
  maxError = computeMaxError(d, first, last, curve, u, &splitPoint);
  if (maxError < error) {
    drawBezierCurve(3, curve, out);
    return;
  }

  /*  If error not too large, try some reparameterization  */
  /*  and iteration */
  if (maxError < iterationError) {
    for (i = 0; i < maxIterations; i++) {
      reparameterize(d, first, last, u, curve);
      generateBezier(d, first, last, u, tHat1, tHat2, curve);
      maxError = computeMaxError(d, first, last, curve, u, &splitPoint);
      if (maxError < error) {
        drawBezierCurve(3, curve, out);
        return;
      }
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  tHatCenter = computeCenterTangent(d, splitPoint);
  fitCubic(d, first, splitPoint, tHat1, tHatCenter, error, out);
  fitCubic(d, splitPoint, last, -tHatCenter, tHat2, error, out);
}

/*
 *  fitPath:
 *  Fit a Bezier curve to a set of digitized points 
 *  \param d Array of digitized points
 *  \param nPts Number of digitized points
 *  \param error User-defined error squared
 */
template <class T>
void fitPath(const T *d, size_t nPts, TCoord error, vector<T> *out)
{
  T tHat1, tHat2;	/*  Unit tangent vectors at endpoints */

  tHat1 = computeLeftTangent(d, 0);
  tHat2 = computeRightTangent(d, nPts - 1);
  fitCubic(d, 0, nPts - 1, tHat1, tHat2, error, out);
  out->push_back(d[nPts-1]);
}

TFreehandPoint in[] = {
  {  40, 175, 0, 1.0 },
//  {  40, 100, 0, 1.0 },
  { 120,  50, M_PI/2, 5.0 },
//  { 160, 100, 0, 1.0 },
  { 240, 100, M_PI, 2.0 },
  { 280, 175, M_PI/2, 3.0 }
};

class TFitWindow:
  public TWindow
{
    vector<TFreehandPoint> out;
  public:
    TFitWindow(TWindow *parent, const string &title):
      TWindow(parent, title)
    {
      fitPath(in, 4, 0.1, &out);
    }
    
    void paint() override;
};

void
TFitWindow::paint()
{
  TPen pen(this);
  
  // draw input
  for(int i=0; i<3; ++i) {
    pen.drawLine(in[i].x, in[i].y, in[i+1].x, in[i+1].y);
  }

  // draw fitted curve
  pen.setColor(0,0.5,1);
  for(int i=0; i<out.size()-1; i+=3) {
    pen.drawCurve(
      out[i].x, out[i].y,
      out[i+1].x, out[i+1].y,
      out[i+2].x, out[i+2].y,
      out[i+3].x, out[i+3].y);
  }
  
  // draw nibs
  pen.setColor(0.5,1,0.5);
  TSize s(4,8);
  for(int j=0; j<out.size()-1; j+=3) {
    TPoint p[] = {
      { out[j  ].x, out[j  ].y },
      { out[j+1].x, out[j+1].y },
      { out[j+2].x, out[j+2].y },
      { out[j+3].x, out[j+3].y }
    };
    TCoord pen_r = 0.0;
    for(TCoord i=0.0; i<=1.0; i+=0.05) {
      TPoint pr0;
      TPoint D = bez2point(p, i);

      TCoord pressure;
      {
        TCoord t=i;
        TCoord u=1-t;
        TCoord u2=u*u;
        TCoord t2=t*t;
        pressure = out[j].pressure*u2*u+out[j+1].pressure*t*u2*3+out[j+2].pressure*t2*u*3+out[j+3].pressure*t2*t;
      }
      TCoord rotation;
      {
        TCoord t=i;
        TCoord u=1-t;
        TCoord u2=u*u;
        TCoord t2=t*t;
        rotation = out[j].rotation*u2*u+out[j+1].rotation*t*u2*3+out[j+2].rotation*t2*u*3+out[j+3].rotation*t2*t;
      }
      //    D.x-=160;
      //    D.y-=100;
      for(TCoord j = 0.0; j<=360.0; j+=20.0) {
        TCoord r11, r12, r21, r22;
        TCoord r = j/360.0 * 2.0 * M_PI;
        TPoint pr(sin(r)*s.width*pressure, cos(r)*s.height*pressure);
        r11 = r22 = cos(rotation);
        r21 = sin(rotation);
        r12 = -r21;
        pr.set(r11 * pr.x + r12 * pr.y, r21 * pr.x + r22 * pr.y);
        if (j>0.0) {
          pen.drawLine(pr0+D, pr+D);
        }
        pr0 = pr;
      }
    }
  }

  int j = 0;
  TPoint p[] = {
    { out[j  ].x, out[j  ].y },
    { out[j+1].x, out[j+1].y },
    { out[j+2].x, out[j+2].y },
    { out[j+3].x, out[j+3].y }
  };

  TCoord u = 0.8;

  TCoord pressure;
  {
    TCoord t=1-u;
    TCoord u2=u*u;
    TCoord t2=t*t;
    pressure = out[j].pressure*t2*t+out[j+1].pressure*u*t2*3+out[j+2].pressure*u2*t*3+out[j+3].pressure*u2*u;
  }
  TCoord rotation;
  {
    TCoord t=1-u;
    TCoord u2=u*u;
    TCoord t2=t*t;
    rotation = out[j].rotation*t2*t+out[j+1].rotation*u*t2*3+out[j+2].rotation*u2*t*3+out[j+3].rotation*u2*u;
  }

  TCoord v = s.height*pressure;
  TCoord h = s.width*pressure;
  
  TCoord pen_r = rotation;

  TPoint C = bez2point(p, u);			// point through which we want to find min & max
  TPoint d = bez2direction(p, u);
  TCoord A = atan2(d.y, d.x)+M_PI/2.0;		// rotation of curve at C

  TCoord cosa = cos(pen_r-A);
  TCoord cos2a = cosa*cosa;
  TCoord sina = sin(pen_r-A);
  TCoord sin2a = sina*sina;
  
  TCoord v2 = v*v;
  TCoord h2 = h*h;

  // draw normal at u
  pen.setColor(0,0.5,0);
  d.set(-d.y, d.x);
  d = normalize(d)*200.0;
  TPoint pt0 = C + d;
  TPoint pt1 = C - d;
  pen.drawLine(pt0.x, pt0.y, pt1.x, pt1.y);
  
  // rotate curve
  TPoint p0[4];
  TMatrix2D mat;
  mat.rotate(-A);
  for(int i=0;i<4;++i) {
    p0[i]=mat.map(p[i]-C);
  }

  drawMinMax3(pen, p, p0, u, s, pen_r);
}


} // unnamed namespace

void
test_path_offset()
{
#if 0
  TPoint c[] = { {0,0}, {0.5,0}, {0.5,1}, {1,1}};
  for (TCoord x=0; x<=1.0; x+=0.1) {
    curveYatX(c, x);
  }
  return;
#endif
//  TFitWindow wnd(NULL, "test path offset");
  TMyWindow wnd(NULL, "test path offset");
  
  toad::mainLoop();
}
