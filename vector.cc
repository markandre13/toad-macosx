#include <toad/vector.hh>
#include <toad/geometry.hh>
#include <toad/pen.hh>

using namespace toad;

bool toad::operator==(const TVectorPath &p0, const TVectorPath &p1)
{
  return p0.points == p1.points && p0.type == p1.type;
}

void TVectorPath::join(const TVectorPath &p)
{
  type.insert(type.end(), p.type.begin(), p.type.end());
  points.insert(points.end(), p.points.begin(), p.points.end());
}

void TVectorPath::clear()
{
  type.clear();
  points.clear();
}

ostream& toad::operator<<(ostream &out, const TVectorPath& path)
{
  out <<"TVectorPath {"<<endl;
  const TPoint *pt = path.points.data();
  for(auto p: path.type) {
//cout << "apply type " << p << ", left="<<(points.size()-(pt-points.data()))<<endl;
    switch(p) {
      case TVectorPath::MOVE:
        out << "  move " << pt[0] << endl;
        ++pt;
        break;
      case TVectorPath::LINE:
        out << "  line " << pt[0] << endl;
        ++pt;
        break;
      case TVectorPath::CURVE:
        out << "  curve " << pt[0] << " " << pt[1] << " " << pt[2]<< endl;
        pt+=3;
        break;
      case TVectorPath::CLOSE:
        cout << "  close" << endl;
    }
  }
  out<<"}"<<endl;
  return out;
}

void
TVectorPath::apply(TPenBase &pen) const
{
//cout << "TVectorPath::apply" << endl;
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

TBoundary
TVectorPath::bounds() const
{
  TBoundary b;
  if (points.empty())
    return b;
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

TBoundary
TVectorPath::editBounds() const
{
  TBoundary b;
  if (points.empty())
    return b;
  auto p = points.begin();
  b.x1 = b.x1 = p->x;
  b.y1 = b.y1 = p->y;
  for(++p; p!=points.end(); ++p) {
    b.expand(*p);
  }
  return b;
}

void
TVectorPath::intersectHelper(TIntersectionList &ilist, TVectorPath::EType type0, const TPoint *pt0, const TVectorPath &vp) const
{
  const TPoint *start = 0;
  const TPoint *pt = vp.points.data();
  for(auto &p: vp.type) {
    switch(p) {
      case MOVE:
        start=pt;
        ++pt;
        break;
      case LINE:
        if (pt>vp.points.data()) {
          if (type0==LINE)
            intersectLineLine(ilist, pt0, pt);
          else
            intersectCurveLine(ilist, pt0, pt);
        }
        ++pt;
        break;
      case CURVE:
        if (pt>vp.points.data()) {
          if (type0==LINE)
            intersectLineCurve(ilist, pt0, pt);
          else
            intersectCurveCurve(ilist, pt0, pt);
        }
        pt+=3;
        break;
      case CLOSE:
        if (start && pt>vp.points.data()) {
          TPoint cl[2];
          cl[0]=*(pt0-1);
          cl[1]=*start;
          if (type0==LINE)
            intersectLineLine(ilist, pt0, cl);
          else
            intersectCurveLine(ilist, pt0, cl);
        }
        break;
    }
  }
}

void
TVectorPath::intersect(TIntersectionList &ilist, const TVectorPath &vp) const
{
  const TPoint *start = 0;
  const TPoint *pt = points.data();
  for(auto &p: type) {
    switch(p) {
      case MOVE:
        start=pt;
        ++pt;
        break;
      case LINE:
        if (pt>points.data())
          intersectHelper(ilist, p, pt-1, vp);
        ++pt;
        break;
      case CURVE:
        if (pt>points.data())
          intersectHelper(ilist, p, pt-1, vp);
        pt+=3;
        break;
      case CLOSE:
        if (start && pt>points.data()) {
          TPoint cl[2];
          cl[0]=*(pt-1);
          cl[1]=*start;
          intersectHelper(ilist, LINE, cl, vp);
        }
        break;
    }
  }
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
          subdivideCutter(LINE, cl, r, linesToBezier);
        }
        break;
    }
  }
}

/**
 * Convert all lines in the path to curves
 *
 * @param   tolerance for the path fitting algorithm
 * @radians when the angle between two lines exceeds radians, begin a new curve
 */
void
TVectorPath::simplify(double tolerance, double radians)
{
//cerr << "simplify("<<tolerance<<", "<<radians<<")"<<endl;
  vector<TPoint> oldpoints;
  vector<EType> oldtype;
  oldpoints.swap(points);
  oldtype.swap(type);

  const TPoint *lineStart = 0;
  const TPoint *pt = oldpoints.data();
  const TPoint *lastPoint = pt + oldpoints.size()-1;

  for(auto p: oldtype) {
    if (p!=LINE && lineStart) {
//      cout << "end of line fitcurve" << endl;
      size_t n = points.size();
      points.pop_back();
      fitPath(lineStart, pt-lineStart-1, tolerance, &points);
      n = points.size() - n;
      for(size_t i=1; i<n; i+=3)
        type.push_back(TVectorPath::CURVE);
      lineStart = nullptr;
    }
again:
    switch(p) {
      case MOVE:
//cout << "move " << *pt << endl;
        type.push_back(TVectorPath::MOVE);
        points.push_back(*pt);
        lineStart = pt;
        ++pt;
        break;
      case LINE:
//cout << "line " << *pt << endl;
        if (lineStart+1<pt && pt!=lastPoint) {
          const TPoint *p0 = pt-2;
          const TPoint *p1 = pt-1;
//cout << "look for edge at " << *p0 << *p1 << *pt << endl;
          double d = atan2(p0->y - p1->y, p0->x - p1->x) - atan2(p1->y - pt->y, p1->x - pt->x);
          if (d>radians || d<-radians) {
//cout << "edge at " << *p1 << endl;
            if (p1-lineStart < 2) {
//cout << "line for " << (p1-lineStart) << " points" << endl;
              type.push_back(TVectorPath::LINE);
              points.push_back(*p1);
            } else {
//cout << "fitPath for " << (p1-lineStart) << " points" << endl;
              size_t oldSize = points.size();
              points.pop_back();
              fitPath(lineStart, p1-lineStart+1, tolerance, &points);
              size_t delta = points.size() - oldSize;
              for(size_t i=1; i<delta; i+=3)
                type.push_back(TVectorPath::CURVE);
            }
            lineStart = p1;
//cout << "new line start at " << *p1 << endl;
            goto again;
          }
        }
        ++pt;
        break;
      case CURVE: // copy
//        cerr << "TVectorPath::simplify(double tolerance): CURVE not implemented yet" << endl;
        pt+=3;
        break;
      case CLOSE:
//cout<<"close"<<endl;
        lineStart = nullptr;
        type.push_back(TVectorPath::CLOSE);
        break;
    }
  }

  if (lineStart) {

//cout << "final: lineStart=" << *lineStart <<", pt="<<*pt<<", lastPoint="<<*lastPoint<<endl;

//        if (lineStart+1<pt && pt!=lastPoint) {
      {
        pt=lastPoint;
          const TPoint *p0 = pt-2;
          const TPoint *p1 = pt-1;
//cout << "look for edge at " << *p0 << *p1 << *pt << endl;
          double d = atan2(p0->y - p1->y, p0->x - p1->x) - atan2(p1->y - pt->y, p1->x - pt->x);
          if (d>radians || d<-radians) {
//cout << "edge at " << *p1 << endl;
            if (p1-lineStart < 2) {
//cout << "line for " << (p1-lineStart) << " points" << endl;
              type.push_back(TVectorPath::LINE);
              points.push_back(*p1);
              type.push_back(TVectorPath::LINE);
              points.push_back(*pt);
            } else {
//cout << "fitPathX for " << (p1-lineStart) << " points" << endl;
              size_t oldSize = points.size();
              points.pop_back();
              fitPath(lineStart, p1-lineStart+1, tolerance, &points);
              size_t delta = points.size() - oldSize;
              for(size_t i=1; i<delta; i+=3)
                type.push_back(TVectorPath::CURVE);
              
              points.push_back(*lastPoint);
              type.push_back(TVectorPath::LINE);
            }
//            lineStart = p1;
//cout << "new line start at " << *p1 << endl;
//            goto again;
          } else {
//cout << "fitPathY for " << (pt-lineStart+1) << " points" << endl;
            size_t oldSize = points.size();
            points.pop_back();
            fitPath(lineStart, pt-lineStart+1, tolerance, &points);
            size_t delta = points.size() - oldSize;
            for(size_t i=1; i<delta; i+=3)
              type.push_back(TVectorPath::CURVE);
          }
        }

/*
    size_t oldSize = points.size();
    cout << "final fitPath for " << (pt-lineStart) << " points" << endl;
    if (pt-lineStart < 2) {
      type.push_back(TVectorPath::LINE);
      points.push_back(*pt);
    } else {
      points.pop_back();
      fitPath(lineStart, pt-lineStart, tolerance, &points);
      size_t delta = points.size() - oldSize;
      for(size_t i=1; i<delta; i+=3)
        type.push_back(TVectorPath::CURVE);
    }
*/
  }
}

TVectorStrokeAndFillOp::TVectorStrokeAndFillOp(const TRGB &s,const TRGB &f):
  stroke(s), fill(f)
{
}

void
TVectorStrokeAndFillOp::paint(TPenBase &pen, const TVectorPath *path)
{
  pen.setColor(stroke);
  path->apply(pen);
  pen.stroke();
  pen.setColor(fill);
  path->apply(pen);
  pen.fill();
}

void
TVectorPainter::paint(TPenBase &pen)
{
  operation->paint(pen, path);
}

void
TVectorGraphic::paint(TPenBase &pen)
{
  for(auto p: *this)
    p->paint(pen);
}
