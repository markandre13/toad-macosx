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
  // find 1st derivative over v to find extrema
  TPoint Pu = bez2point(p, u);

  TPoint d = bez2direction(p, u);

  TCoord R = atan2(d.y, d.x)+M_PI/2.0;
  
//  TPoint pressure[4] = { {0,0}, {0.5,0}, {0.5,1}, {1,1} };

  TCoord cosa = cos(r-R);
  TCoord cos2a = cosa*cosa;
  TCoord sina = sin(r-R);
  TCoord sin2a = sina*sina;
  
  TCoord h = s.height;
  TCoord w = s.width;
  TCoord h2 = h*h;
  TCoord w2 = w*w;

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
#endif
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
    drawMinMax(pen, p, p0, u, i, s, pen_r);
    // draw nib
    pen.setColor(0.5,1,0.5);
    TPoint pr0;
    TPoint D = bez2point(p, i);
//    D.x-=160;
//    D.y-=100;
    for(TCoord j = 0.0; j<=360.0; j+=20.0) {
      TCoord r11, r12, r21, r22;
      TCoord r = j/360.0 * 2.0 * M_PI;
      TPoint pr(sin(r)*s.width, cos(r)*s.height);
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
  drawMinMax2(pen, p, p0, u, s, pen_r);

  pen.setColor(0,0,0);
  pen.drawBezier(p, 4);
  pen.setColor(TColor::FIGURE_SELECTION);
  pen.drawLines(p, 4);
  pen.setFillColor(1,1,1);
  for(auto i=0; i<4; ++i) {
    pen.fillRectangle(p[i].x-2, p[i].y-2, 5, 5);
    pen.drawRectangle(p[i].x-2, p[i].y-2, 5, 5);
  }

  pen.setColor(1,0,0);

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


} // unnamed namespace

void
test_path_offset()
{
  TPoint c[] = { {0,0}, {0.5,0}, {0.5,1}, {1,1}};
  for (TCoord x=0; x<=1.0; x+=0.1) {
    curveYatX(c, x);
  }
  return;

  TPressure wnd(NULL, "test path offset");
  toad::mainLoop();
}
