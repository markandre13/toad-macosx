#include <toad/vector.hh>
#include <toad/geometry.hh>
#include <toad/pen.hh>

using namespace toad;

void
TVectorPath::apply(TPen &pen) const
{
cout << "TVectorPath::apply" << endl;
  const TPoint *pt = points.data();
  for(auto p: type) {
//cout << "apply type " << p << ", left="<<(points.size()-(pt-points.data()))<<endl;
/*
switch(p) {
  case MOVE: cout << "  MOVE" << endl; break;
  case LINE: cout << "  LINE" << pt[0] << endl; break;
  case CURVE: cout << "  CURVE" << pt[0] << pt[1] << pt[2]<< endl; break;
  case CLOSE: cout << "  CLOSE" << endl;
}
*/
    switch(p) {
      case MOVE: pen.move(pt); ++pt; break;
      case LINE: pen.line(pt); ++pt; break;
      case CURVE: pen.curve(pt); pt+=3; break;
      case CLOSE: pen.close(); break;
    }
  }
}

Box
TVectorPath::bounds() const
{
  Box b;
  const TPoint *pt = points.data();
  b.x1 = b.x1 = pt->x;
  b.y1 = b.y1 = pt->y;
  for(auto p: type) {
    switch(p) {
      case MOVE:
      case LINE:
        b.expand(*pt);
        ++pt;
        break;
      case CURVE: {
        assert(pt>points.data());
        TRectangle r(curveBounds(pt-1));
        b.expand(TPoint(r.x, r.y));
        b.expand(TPoint(r.x+r.w, r.y+r.h));
        pt+=3;
      } break;
      case CLOSE:
        break;
    }
  }
  return b;
}

void
Box::expand(const TPoint &pt)
{
  if (pt.x < x1)
    x1 = pt.x;
  else
  if (pt.x > x2)
    x2 = pt.x;
  if (pt.y < y1)
    y1 = pt.y;
  else
  if (pt.x > x2)
    y2 = pt.y;
}

Box
TVectorPath::editBounds() const
{
  Box b;
  auto p = points.begin();
  b.x1 = b.x1 = p->x;
  b.y1 = b.y1 = p->y;
  for(++p; p!=points.end(); ++p) {
    b.expand(*p);
  }
  return b;
}

void
TVectorPath::subdivideCutter(EType t, const TPoint *pt, const TRectangle &bounds, bool linesToBezier)
{
//cout << "TVectorPath::subdivideCutter --------------------------------" << endl;
  TIntersectionList ilist;
  
  TPoint bez[4];
  if (linesToBezier && t==LINE) {
    TPoint d=(pt[1]-pt[0])*0.25;
    bez[0]=pt[0];
    bez[1]=pt[0]+d;
    bez[2]=pt[0]-d;
    bez[3]=pt[1];
    t = CURVE;
    pt = bez;
  }

  for(TCoord x=0; x<320; x+=40) {
    TPoint line[] = {{x,0}, {x,200}};
    if (t==TVectorPath::LINE)
      intersectLineLine(ilist, pt, line);
    else
      intersectCurveLine(ilist, pt, line);
  }

  for(TCoord y=0; y<200; y+=40) {
    TPoint line[] = {{0,y}, {320,y}};
    if (t==TVectorPath::LINE)
      intersectLineLine(ilist, pt, line);
    else
      intersectCurveLine(ilist, pt, line);
  }

  sort(ilist.begin(), ilist.end(), [](const auto &p, const auto &q) {
    return (p.seg0.u < q.seg0.u);
  });

#if 0
  TCoord s = 1.0/ilist.size();
  TCoord c = 0;
  for(auto p: ilist) {
    gpen->setColor(c,0,1-c); c+=s;
    gpen->drawRectangle(p.seg0.pt.x-1.5, p.seg0.pt.y-1.5, 4,4);
    gpen->drawRectangle(p.seg1.pt.x-0.5, p.seg1.pt.y-0.5, 2,2);
  }
#endif
  
  auto p = ilist.begin();
  if (p==ilist.end()) {
    type.push_back(t);
    points.push_back(pt[1]);
    if (t==CURVE) {
      points.push_back(pt[2]);
      points.push_back(pt[3]);
    }
    return;
  }
  
  if (t==LINE) {
    TCoord lastU=0.0;
    for(;p!=ilist.end();++p) {
      if (isZero(lastU-p->seg0.u))
        continue;
      type.push_back(LINE);
      points.push_back(p->seg0.pt);
      lastU = p->seg0.u;
    }
    if (isZero(1-lastU))
      return;
    type.push_back(LINE);
    points.push_back(pt[1]);
    return;
  }

  TPoint sub[4];
  TCoord lastU=0.0;
  for(;p!=ilist.end();++p) {
    if (isZero(lastU-p->seg0.u))
      continue;
    divideBezier(pt, sub, lastU, p->seg0.u);
    type.push_back(CURVE);
    points.push_back(sub[1]);
    points.push_back(sub[2]);
    points.push_back(sub[3]);
    lastU = p->seg0.u;
  }
  if (isZero(1-lastU))
    return;
  divideBezier(pt, sub, lastU, 1);
  type.push_back(CURVE);
  points.push_back(sub[1]);
  points.push_back(sub[2]);
  points.push_back(sub[3]);
}

void
TVectorPath::subdivide()
{
  vector<TPoint> oldpoints;
  vector<EType> oldtype;
  oldpoints.swap(points);
  oldtype.swap(type);

  TRectangle r; // = bounds();
  bool linesToBezier = true;

  const TPoint *start = 0;
  
  const TPoint *pt = oldpoints.data();
  for(auto p: oldtype) {
    switch(p) {
      case MOVE:
        type.push_back(MOVE);
        points.push_back(*pt);
        start=pt;
        ++pt;
        break;
      case LINE:
        if (pt>oldpoints.data())
          subdivideCutter(p, pt-1, r, linesToBezier);
        ++pt;
        break;
      case CURVE:
        if (pt>oldpoints.data())
          subdivideCutter(p, pt-1, r, linesToBezier);
        pt+=3;
        break;
      case CLOSE:
        if (start && pt>oldpoints.data()) {
          TPoint cl[2];
          cl[0]=*(pt-1);
          cl[1]=*start;
          subdivideCutter(TVectorPath::LINE, cl, r, linesToBezier);
        }
        break;
    }
  }
}
