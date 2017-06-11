/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>
#include <toad/geometry.hh>

// FIXME: rename <toad/figureeditor.hh into toad/figure/editor.hh
#include <toad/figureeditor.hh>
#include <toad/figure/nodetool.hh>
#include <toad/fischland/fpath.hh>

#include <algorithm>
#include <queue>


namespace {

using namespace toad;


class WordWrap:
  public ::testing::Test
{
  protected:
    static void SetUpTestCase() {
      toad::initialize(0, NULL);
    }
    
    static void TearDownTestCase() {
      toad::terminate();
    }
};


/*
o convert the path into a list of non-overlapping segments
o 
*/

class TWordWrapper
{
  public:
    enum SweepEventType {
      LINE, CURVE
    };
    struct SweepEvent {
      SweepEvent(const TPoint &p0, const TPoint &p1) {
        type = LINE;
        boundary.set(p0, p1);
        data.line.p[0] = p0;
        data.line.p[1] = p1;
      }
      SweepEvent(const TPoint *curve) {
        type = CURVE;
        boundary = curveBounds(curve);
        data.curve.p = curve;
        data.curve.u0 = 0;
        data.curve.u1 = 1;
      }
      SweepEventType type;

      union Data {
        struct {
          TPoint p[2];
        } line;
        struct {
          const TPoint *p;
          TCoord u0, u1;
        } curve;
      } data;
      TRectangle boundary;
      
      SweepEvent *otherEvent;
    };

    // e0 < e1
    struct eventQueueOrder: public std::binary_function<SweepEvent, SweepEvent, bool> {
      bool operator() (const SweepEvent *e0, const SweepEvent *e1);
    };

    TPoint cursor;
    
    std::set<SweepEvent*, eventQueueOrder> eventQueue;
    
    std::deque<SweepEvent> allEvents;
//    std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, eventQueueOrder> eventQueue;
    std::deque<SweepEvent*> sweepLineEvents;

//    TWordWrapper(): eventQueue(eventQueueOrder(), priority_vector) {
//    }
  
    // look for place in sweep events
    void place(const TSize &rectangle);
    bool findPlace(const SweepEvent *e0, const SweepEvent *e1, const TSize &size, TPoint *out);
    
    // the following methods will be obsolete
    void follow(const SweepEvent &event, const TPoint *upperScanLine, const TPoint *lowerScanLine);
    static TCoord distanceAtY(const SweepEvent*, const SweepEvent*, TCoord);
    static TPoint pointForDistance(const SweepEvent *e0, const SweepEvent *e1, const TSize &size, TCoord y);

    // fill allEvents and eventQueue
    void path2events(const TVectorPath& path);
    void addLine(const TPoint &p0, const TPoint &p1);
    void addCurve(const TPoint *p);
    SweepEvent* storeSweepEvent(const SweepEvent &e);

    struct Slicer {
      const std::set<SweepEvent*, eventQueueOrder> &eventQueue;
      std::set<SweepEvent*, eventQueueOrder>::const_iterator ptr;
      struct slice {
        const TWordWrapper::SweepEvent *left, *right;
      };
      vector<slice> slices;
    
      Slicer(std::set<SweepEvent*, eventQueueOrder> &theEventQueue):eventQueue(theEventQueue), ptr(theEventQueue.begin()) {}
      bool step();
    };
};

inline ostream& operator<<(ostream &out, const TWordWrapper::SweepEvent &event) {
  switch(event.type) {
    case TWordWrapper::LINE:
      out << "line " << event.data.line.p[0] << " - " << event.data.line.p[1];
      break;
    case TWordWrapper::CURVE:
      out << "curve";
      break;
    default:
      out << event.type;
  }
  return out;
}

// 1st point of e1 is left of e0
bool leftOf(const TWordWrapper::SweepEvent *e0, const TWordWrapper::SweepEvent *e1)
{
  return signedArea(e0->data.line.p[0], e0->data.line.p[1], e1->data.line.p[0]) > 0;
}

bool
TWordWrapper::Slicer::step()
{
  if (ptr==eventQueue.end())
    return false;

  cout << "find a home for " << **ptr << endl;

  // join
  auto slice = slices.begin();
  if (slices.size()>=2) {
    while(true) {
      auto prevSlice = slice;
      ++slice;
      if (slice==slices.end())
        break;
      
      if (prevSlice->right->data.line.p[1] == slice->left->data.line.p[1] &&

          (*ptr)->data.line.p[0].y >= prevSlice->right->data.line.p[1].y ) // FIXME: corner cases

      {
        cout << "JOIN " << *prevSlice->right << " AND " << *slice->left << endl;
        
        prevSlice->right = slice->right;
        slices.erase(slice);
        
        return true;
      }
    }
  }
  
  // continue
  slice = slices.begin();
  while(slice!=slices.end()) {
    if ((*ptr)->data.line.p[0] == slice->left->data.line.p[1]) {
      cout << "found left side" << endl;
      slice->left = *(ptr++);
      return true;
    } else
    if ((*ptr)->data.line.p[0] == slice->right->data.line.p[1]) {
      cout << "found right side" << endl;
      slice->right = *(ptr++);
      return true;
    }
    ++slice;
  }

  // insert
  slice = slices.begin();
  while(slice!=slices.end()) {
    if (leftOf(slice->left, *ptr)) {
      cout << "insert slice" << endl;
      cout << "   " << (*ptr)->data.line.p[0] << " is left of " << *slice->left << endl;
      return true;
    }
    if (leftOf(slice->right, *ptr)) {
      cout << "SPLIT" << endl;
      struct slice s;
      s.left = slice->left;
      s.right = *(ptr++);
      slice->left = *(ptr++);
      slices.insert(slice, s);
      return true;
    }
    ++slice;
  }
  
  cout << "start a new slice" << endl;
  // start a new slice
  struct slice s;
  s.left = *(ptr++);
  s.right = *(ptr++);
  slices.push_back(s);

  return true;
}


#if 0
void
TTest::paint()
{
  TPen pen(this);
  
  TVectorPath path;

/*  
  path.move(10,10);
  path.curve(80,10, 310, 150, 310,190);
  path.line(10,150);
  path.curve(10,10, 150,10, 310, 150);
  path.close();
*/  
/*
  path.move(160,75);
  path.curve(180,10, 250,55, 260,100); // 20 90 10 45
  path.curve(270,145, 160, 190, 160,190);
  path.curve(160,190, 10,180, 10,100);
  path.curve(10,10, 50,10, 160,75);
*/

  path.move(160, 40);
  path.curve(320,10, 50, 100, 310,130);
  path.line(130, 190);
  path.line(10,70);
  path.close();
  
/*
  BooleanOpImp thing(UNION);
  thing.path2events(path, SUBJECT);
  cout << "sweep events: " << thing.eq.size() << endl;
*/  
  path.apply(pen);
  pen.stroke();
  
#if 0
  pen.drawBezier(p, 4);
  pen.drawBezier(p+4, 4);
  for(int i=0; i<8; ++i)
    pen.drawRectangle(p[i].x-2.5, p[i].y-2.5, 5,5);
  
  pen.setColor(1,0,0);
  for(TCoord u=0.0; u<=1.0; u+=0.1) {
    TPoint a = p[0]*(1-u)*(1-u)*(1-u) + p[1]*3*(1-u)*(1-u)*u + p[2]*3*(1-u)*u*u + p[3]*u*u*u;
    pen.drawRectangle(a.x-0.5,a.y-0.5,1,1);
  }
  pen.setColor(0,0,1);
  for(TCoord u=0.0; u<=1.0; u+=0.01) {
    TPoint a, b;
    // a = p[0]*(1 - 3*u + 3*u*u - u*u*u) + p[1]*(3*u - 6*u*u + 3*u*u*u) + p[2]*(3*u*u - 3*u*u*u) + p[3]*u*u*u;
    // a = p[0] - p[0]*3*u + p[0]*3*u*u - p[0]*u*u*u + p[1]*3*u - p[1]*6*u*u + p[1]*3*u*u*u + p[2]*3*u*u -  p[2]*3*u*u*u + p[3]*u*u*u;
    a = 
      p[0] 
      + u * (-p[0]*3 + p[1]*3 )
      + u*u * (p[0]*3 - p[1]*6 + p[2]*3 )
      + u*u*u * (-p[0] + p[1]*3 - p[2]*3 + p[3]);

    b = 
        (-p[0]*3 + p[1]*3 )
      + 2*u * (p[0]*3 - p[1]*6 + p[2]*3 )
      + 3*u*u * (-p[0] + p[1]*3 - p[2]*3 + p[3]);

    pen.drawRectangle(50+(b.x)/20.0-0.5,a.y-0.5,1,1);
  }
  pen.drawLine(50,0,50,200);

  TCoord roots[3];
  int n = solveQuadratic(
    3 * (-p[0].x + p[1].x*3 - p[2].x*3 + p[3].x),
    2 * (p[0].x*3 - p[1].x*6 + p[2].x*3),
    (-p[0].x*3 + p[1].x*3),
    roots, 0, 1);
  
  pen.setColor(0,1,0);
  for(int i=0; i<n; ++i) {
    TPoint pt = bez2point(p, roots[i]);
    TCoord u = roots[i];
    pen.drawRectangle(pt.x-0.5, pt.y-0.5, 1, 1);
  }
  
/*
  (1-u)^2 = 1 - 2u + u^2
  (1-u)^3 = 1 - 3u + 3u^2 - u^3
  
    a0*(1-u)^3 + a1*(1-u)^2*u + a2*(1-u)*u^2 + a3*u^3
  ⇔ a0*(1 - 3u + 3u^2 - u^3) + a1*(u - 3u + u^3) + a2*(u^2-u^3) + a3*u^3
  ⇔ a0 - a0*3u + a0*3u^2 - a0*u^3 + a1*u - a1*3u + a1*u^3 + a2*u^2-a2*u^3 + a3*u^3
*/
#endif
}
#endif

inline bool
isEventInsideSweepLine(const TWordWrapper::SweepEvent *event, TCoord y, TCoord h)
{
  TCoord ey0 = event->boundary.origin.y;
  TCoord ey1 = ey0 + event->boundary.size.height;
  return
    ( (y   <= ey0 && ey0 <= y + h) ||
      (y+h <= ey1 && ey1 <= y + h) ||
      (ey0 <= y   && y+h <= ey1  ) );
}

bool
TWordWrapper::findPlace(const SweepEvent *e0, const SweepEvent *e1, const TSize &size, TPoint *out)
{
  switch(e0->type) {
    case LINE:
      switch(e1->type) {
        case LINE: {
          TPoint a = e0->data.line.p[0],
                 e = e0->data.line.p[1] - a,
                 b = e1->data.line.p[0],
                 f = e1->data.line.p[1] - b;
          if (e.x > 0 && f.x > 0) {
            TPoint d = TPoint(size.width, -size.height);
            TCoord
              E = e.y / e.x,
              v = ( a.y + E * ( b.x - a.x - d.x ) + d.y - b.y ) / ( f.y - E * f.x );
            *out = b + v * f;
            out->x -= size.width;
            return true;
          }
          if (e.x < 0 && f.x < 0) {
            TPoint d = TPoint(size.width, size.height);
            TCoord
              E = e.y / e.x,
              v = ( a.y + E * ( b.x - a.x - d.x ) + d.y - b.y ) / ( f.y - E * f.x );
            *out = b + v * f;
            out->x -= size.width;
            out->y -= size.height;
            return true;
          }
          const TPoint *lineB = e1->data.line.p;
          TPoint line2[2] = {
            { lineB[0].x - size.width, lineB[0].y },
            { lineB[1].x - size.width, lineB[1].y },
          };
          TIntersectionList intersections;
          intersectLineLine(intersections, e0->data.line.p, line2);
          assert(intersections.size()==1);
          *out = intersections[0].seg0.pt;
          return true;
        } break;
        case CURVE:
          break;
      }
      break;
    case CURVE:
      break;
  }
  return false;
}


//    L = p0 + u * ( p1 - p0 )
// ⇔  L = p0 + u * d
// ⇔  x = p0.x + u * d.x ∧ y = p0.y + u * d.y
// ⇔  x = p0.x + u * d.x ∧ u = (y - p0.y ) / d.y
// ⇔  x = p0.x + (y - p0.y ) / d.y * d.x

/**
 * Return the distance along the x-axis of e0 and e1 at y.
 *
 */
TCoord
TWordWrapper::distanceAtY(const SweepEvent *e0, const SweepEvent *e1, TCoord y)
{
  switch(e0->type) {
    case LINE:
      switch(e1->type) {
        case LINE: {
          TPoint d0 = e0->data.line.p[1] - e0->data.line.p[0];
          if (d0.y==0) {
            cerr << __FILE__ << ":" << __LINE__ << endl;
            exit(1);
          }
          TCoord u0 = (y - e0->data.line.p[0].y) / d0.y;
          if (u0<0.0 || u0>1.0) {
            cerr << __FILE__ << ":" << __LINE__ << endl;
            exit(1);
          }
          TCoord x0 = e0->data.line.p[0].x + u0 * d0.x;

          TPoint d1 = e1->data.line.p[1] - e1->data.line.p[0];
          if (d1.y==0) {
            cerr << __FILE__ << ":" << __LINE__ << endl;
            exit(1);
          }
          TCoord u1 = (y - e1->data.line.p[0].y) / d1.y;
          if (u1<0.0 || u1>1.0) {
            cerr << __FILE__ << ":" << __LINE__ << endl;
            exit(1);
          }
          TCoord x1 = e1->data.line.p[0].x + u1 * d1.x;
          return x1 - x0;
        } break;
        case CURVE:
          break;
      }
      break;
    case CURVE:
      break;
  }
  return -1.0;
}

TPoint
TWordWrapper::pointForDistance(const SweepEvent *e0, const SweepEvent *e1, const TSize &size, TCoord y)
{
  switch(e0->type) {
    case LINE:
      switch(e1->type) {
        case LINE: {
          TPoint a = e0->data.line.p[0],
                 e = e0->data.line.p[1] - a,
                 b = e1->data.line.p[0],
                 f = e1->data.line.p[1] - b;
          if (e.x > 0 && f.x > 0) {
            TPoint d = TPoint(size.width, -size.height);
            TCoord
              E = e.y / e.x,
              v = ( a.y + E * ( b.x - a.x - d.x ) + d.y - b.y ) / ( f.y - E * f.x );
            TPoint p = b + v * f;
            p.x -= size.width;
            return p;
          }
          if (e.x < 0 && f.x < 0) {
            TPoint d = TPoint(size.width, size.height);
            TCoord
              E = e.y / e.x,
              v = ( a.y + E * ( b.x - a.x - d.x ) + d.y - b.y ) / ( f.y - E * f.x );
            TPoint p = b + v * f;
            p.x -= size.width;
            p.y -= size.height;
            return p;
          }
          const TPoint *lineB = e1->data.line.p;
          TPoint line2[2] = {
            { lineB[0].x - size.width, lineB[0].y },
            { lineB[1].x - size.width, lineB[1].y },
          };
          TIntersectionList intersections;
          intersectLineLine(intersections, e0->data.line.p, line2);
          assert(intersections.size()==1);
          return intersections[0].seg0.pt;
        } break;
        case CURVE:
          break;
      }
      break;
    case CURVE:
      break;
  }
  return TPoint(0,0);
}

void
TWordWrapper::place(const TSize &rectangle)
{
#if 0
  cout << "find intersections at line " << cursor.y << endl;

  // fill sweep lineLineEvents with all events within sweep line
  while(!eventQueue.empty()) {
    SweepEvent *event = eventQueue.top();
    if (!isEventInsideSweepLine(event, cursor.y, 0 /*rectangle.height*/))
      break;
    eventQueue.pop();
    sweepLineEvents.push_back(event);
    cout << event << ": " << *event << endl;
  }

  // look for enough width for rectangle.width in upper sweep line
  assert(sweepLineEvents.size()%1 == 0);
  auto ptr = sweepLineEvents.begin();
  while(ptr!=sweepLineEvents.end()) {
    SweepEvent *left  = *(ptr++);
    SweepEvent *right = *(ptr++);
    TCoord d = distanceAtY(left, right, cursor.y);
cout << "distanceAtY = " << d << endl;
    if (d < rectangle.width) {
      continue; // does not fit
    }
    return;
  }
  
  // not enough room in upper sweep line, check how deep we can go down the
  // sweep line to find enough room
  cout << "no match at upper sweep line" << endl;

  ptr = sweepLineEvents.begin();
  while(ptr!=sweepLineEvents.end()) {
    SweepEvent *left  = *(ptr++);
    SweepEvent *right = *(ptr++);
    cursor = pointForDistance(left, right, rectangle, cursor.y);
    return;
  }

#if 0

  TPoint upperScanLine[2] = {
    { numeric_limits<TCoord>::min(), cursor.y },
    { numeric_limits<TCoord>::max(), cursor.y }
  };
  TPoint lowerScanLine[2] = {
    { numeric_limits<TCoord>::min(), cursor.y+rectangle.height },
    { numeric_limits<TCoord>::max(), cursor.y+rectangle.height }
  };

  for(auto &&event: events) {
    cout << event.boundary << endl;
    if(event.boundary.origin.y <= cursor.y && cursor.y <= event.boundary.origin.y + event.boundary.size.height) {
cout << "  hit" << endl;
      follow(event, upperScanLine, lowerScanLine);
    }
  }
#endif
#endif
};

void
TWordWrapper::follow(const SweepEvent &event, const TPoint *upperScanLine, const TPoint *lowerScanLine)
{
#if 0
  TIntersectionList upperScanLineIntersections, lowerScanLineIntersections;
  switch(event.type) {
    case LINE:
      intersectLineLine(upperScanLineIntersections, upperScanLine, event.p);
      cout << "    " << upperScanLineIntersections.size() << endl;
      if (upperScanLineIntersections.size()!=1) {
        cerr << __FILE__ << ":" << __LINE__ << ": not implemented yet" << endl;
        exit(1);
      }
      intersectLineLine(lowerScanLineIntersections, lowerScanLine, event.p);
      cout << "    " << lowerScanLineIntersections.size() << endl;
      if (lowerScanLineIntersections.size()!=1) {
        cerr << __FILE__ << ":" << __LINE__ << ": not implemented yet" << endl;
        exit(1);
      }
      break;
    case CURVE:
      cerr << __FILE__ << ":" << __LINE__ << ": not implemented yet" << endl;
      exit(1);
      break;
  }
#endif
}

void
TWordWrapper::path2events(const TVectorPath& path)
{
  cursor = path.bounds().p0; // FIXME: this should not be here

// FIXME: this function must drop neighbouring equal points (degenerated case)
// FIXME: this function must catch empty polygons
  const TPoint *pt = path.points.data();
  const TPoint *ph, *pp; // head & previous
  for(auto p: path.type) {
    switch(p) {
      case TVectorPath::MOVE:
        pp=ph=pt;
        ++pt;
        break;
      case TVectorPath::LINE:
        addLine(*pp, *pt);
        pp=pt;
        ++pt;
        break;
      case TVectorPath::CURVE:
        addCurve(pp);
        pp=pt+2;
        pt+=3;
        break;
      case TVectorPath::CLOSE:
        addLine(*pp, *ph);
        break;
    }
  }
}

// e0 < e1
bool
TWordWrapper::eventQueueOrder::operator() (const SweepEvent *e0, const SweepEvent *e1)
{
  switch(e0->type) {
    case TWordWrapper::LINE:
      switch(e1->type) {
        case TWordWrapper::LINE:
          // when e0->data.line.p[0] != e1->data.line.p[0], sort by these points
          if (e0->data.line.p[0].y < e1->data.line.p[0].y)
            return true;
          if (e1->data.line.p[0].y < e0->data.line.p[0].y)
            return false;
          if (e0->data.line.p[0].x != e1->data.line.p[0].x)
            return e0->data.line.p[0].x < e1->data.line.p[0].x;
          
          // see that e0 comes after e1 clockwise
          return signedArea(e0->data.line.p[0], e0->data.line.p[1], e1->data.line.p[1]) < 0;
          
          break;
        case TWordWrapper::CURVE:
          break;
      }
      break;
    case TWordWrapper::CURVE:
      break;
  }
/*
  if (e0->point.x > e0->point.x) // Different x-coordinate
    return true;
  if (e1->point.x > e0->point.x) // Different x-coordinate
    return false;
  if (e0->point.y != e1->point.y) // Different points, but same x-coordinate. The event with smaller y-coordinate is processed first
    return e1->point.y > e2->point.y;
  if (e0->left != e1->left) // Same point, but one is a left endpoint and the other a right endpoint. The right endpoint is processed first
    return e1->left;
  // Same point, both events are left endpoints or both are right endpoints.
  if (signedArea (e0->point, e0->otherEvent->point, e1->otherEvent->point) != 0) // not collinear
    return e0->above (e1->otherEvent->point); // the event associate to the bottom segment is processed first
  return e0->pol > e1->pol;
*/
  return false;
}

TWordWrapper::SweepEvent*
TWordWrapper::storeSweepEvent(const SweepEvent &e)
{
  allEvents.push_back(e);
  return &allEvents.back();
}

/**
 * convert segment/edge into two sweep events
 */
void
TWordWrapper::addLine(const TPoint &p0, const TPoint &p1)
{
//  events.push_back(SweepEvent(p0, p1));

  if (p0==p1) // if the two edge endpoints are equal the segment is dicarded
    return;   // This can be done as preprocessing to avoid "polygons" with less than 3 edges

  bool swap = false;
  if (p0.y>p1.y) {
    swap = true;
  } else
  if (p0==p1 && p0.x>p1.x)
    swap = true;

  SweepEvent *e0 = storeSweepEvent(swap ? SweepEvent(p1, p0) : SweepEvent(p0, p1));
//  SweepEvent *e1 = storeSweepEvent(SweepEvent(p1, p0));
//  e0->otherEvent = e1;
//  e1->otherEvent = e0;
/*
  if (minlex(p0, p1) == p0) {
    e1->left = false;
  } else {
    e0->left = false;
  }
*/
  if (eventQueue.find(e0)!=eventQueue.end()) {
    cerr << "WARNING: duplicate event in eventQueue" << endl;
    cerr << "         " << *e0 << endl;
    cerr << "         " << **eventQueue.find(e0) << endl;
  }
  eventQueue.insert(e0);
//  eventQueue.push(e1);

/*
//cout << "segment " << p0 << " - " << p1 << endl;
  if (p0==p1) // if the two edge endpoints are equal the segment is dicarded
    return;   // This can be done as preprocessing to avoid "polygons" with less than 3 edges
  SweepEvent* e1 = storeSweepEvent(SweepEvent(true, p0, 0));
  SweepEvent* e2 = storeSweepEvent(SweepEvent(true, p1, e1));
  e1->otherEvent = e2;

  if (minlex(p0, p1) == p0) {
    e2->left = false;
  } else {
    e1->left = false;
  }
  eq.push(e1);
  eq.push(e2);
*/
}

void
TWordWrapper::addCurve(const TPoint *p)
{
//  events.push_back(SweepEvent(p));
/*
  SweepEvent* e0 = storeSweepEvent(SweepEvent(false, p[0], 0, type));
  SweepEvent* e1 = storeSweepEvent(SweepEvent(false, p[3], e0, type));
  e0->otherEvent = e1;

  e0->curve = e1->curve = true;
  e0->cpoint = p[1];
  e1->cpoint = p[2];
  
  if (minlex(p[0], p[3]) == p[0]) {
    e0->left = true;
  } else {
    e1->left = true;
  }
  eq.push(e0);
  eq.push(e1);
*/
}

class TTestWrap:
  public TFigureEditor
{
    unique_ptr<GChoice<TFigureTool*>> choice;
    TVectorPath *path;
  public:
    TTestWrap(TWindow *parent, const string &title);
    void paint() override;
};

class TFPath:
  public TAttributedFigure
{
  public:
    TVectorPath path;
    
    TVectorGraphic* getPath() const override {
      auto *vg = new TVectorGraphic;
      vg->push_back(new TVectorPainter(this, new TVectorPath(path)));
      return vg;
    }
    void paint(TPenBase &pen, EPaintType type=NORMAL) override {
      pen.setColor(0,0,0);
      path.apply(pen);
      pen.stroke();
    }
    TRectangle bounds() const override {
      return path.bounds();
    }
    TCoord distance(const TPoint &point) override {
      return path.distance(point);
    }
    bool transform(const TMatrix2D &transform) override {
      path.transform(transform);
      return true;
    }
    bool getHandle(unsigned handle, TPoint *p) override {
      if (handle>=path.points.size())
        return false;
      *p = path.points[handle];
      return true;
    }
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier) override {
      path.points[handle] = TPoint(x, y);
    }

    SERIALIZABLE_INTERFACE(toad::, TFPath);
};

bool TFPath::restore(atv::TInObjectStream&) { return true; }
void TFPath::store(atv::TOutObjectStream&) const {}

TTestWrap::TTestWrap(TWindow *parent, const string &title):
  TFigureEditor(parent, title)
{
  TToolBox *tb = TToolBox::getToolBox();
  tb->add("directselection", TNodeTool::getTool());
  setToolBox(tb);
  
  TFPath *path = new TFPath();
  this->path = &path->path;

  path->path.move(160, 40);
//  path.curve(320,10, 50, 100, 310,130);
  path->path.line(310, 130);
  path->path.line(130, 190);
  path->path.line(10,70);
  path->path.close();

  getModel()->add(path);
}

void
TTestWrap::paint()
{
  TFigureEditor::paint();

  string text="Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
  
  TPen pen(this);
  pen.setColor(0,0,1);

  TWordWrapper wrap;
  wrap.path2events(*path);

  ssize_t wordStart = -1;
  for(size_t i=0; i<text.size(); ++i) {
    if (wordStart==-1) {
      if (!isspace(text[i])) {
        wrap.cursor.x += pen.getTextWidth(" ");
        wordStart = i;
      }
    } else {
      if (isspace(text[i])) {
        string word = text.substr(wordStart, i-wordStart);
        
        cout << word << endl;
        
        TSize size(pen.getTextWidth(word), pen.getHeight());

        wrap.place(size);
        pen.drawRectangle(wrap.cursor.x, wrap.cursor.y,size.width, size.height);
        pen.drawString(wrap.cursor.x, wrap.cursor.y, word);
        
        wrap.cursor.x += size.width;
        
        wordStart = -1;
      }
    }
  }

}

// (10,10)   (20,10)
//    |         |
//    |         |
// (20,10)   (20,20)
TEST_F(WordWrap, order000) {
  TWordWrapper::SweepEvent e0(TPoint(10, 10), TPoint(10, 20));
  TWordWrapper::SweepEvent e1(TPoint(20, 10), TPoint(20, 20));
  
  TWordWrapper::eventQueueOrder compare;

  ASSERT_EQ(true,  compare(&e0, &e1)); // e0 < e1 -> true
  ASSERT_EQ(false, compare(&e1, &e0)); // e0 > e1 -> false
}

// (30,35)    (50,35)
//        \  /
//         \/
//        (45,40)
TEST_F(WordWrap, order001) {
  TWordWrapper::SweepEvent e0(TPoint(40, 35), TPoint(45,40));
  TWordWrapper::SweepEvent e1(TPoint(50, 35), TPoint(45,40));
  
  TWordWrapper::eventQueueOrder compare;

  ASSERT_EQ(true , compare(&e0, &e1)); // e0 < e1 -> true
  ASSERT_EQ(false, compare(&e1, &e0)); // e0 > e1 -> false
}

#if 0
TEST_F(WordWrap, foo) {
  TTestWrap wnd(NULL, testname());
  wnd.doModalLoop();
}
#endif

#if 0
// ▘ (0,0)
//      (30,10)
//         ▞▖
//        ▞ ▝▖
//       ▞   ▝▖
//(10,30)      (50,30)
TEST_F(WordWrap, findPlace001) {
  TWordWrapper wrap;
  wrap.cursor = TPoint(0,0);
  wrap.addLine(TPoint(30, 10), TPoint(10,30));
  wrap.addLine(TPoint(30, 10), TPoint(50,30));
  TPoint pos;
  ASSERT_EQ(true, wrap.findPlace(&wrap.allEvents[0], &wrap.allEvents[1], TSize(20,10), &pos));
  ASSERT_EQ(TPoint(20, 20), pos);
}
#endif

// ▘ (0,0)
//    1 2 3 4 5 6 7 8 9
//           
// 1        ▞▖b  
//         ▞ ▝▖c▞▖
// 2    a ▞   ▝▞ ▝▖
//       ▞        ▝▖d
// 3    ▞   g▞▖h   ▝▖
//     ▞    ▞ ▝▖    ▝▖
// 4  ▞    j▝▖▞i     ▞
//    ▝▖     ▝      ▞
// 5   ▝▖          ▞
//      ▝▖        ▞
// 6    f▝▖      ▞e
//        ▝▖    ▞
// 7       ▝▖  ▞
//          ▝▖▞
// 8         ▝
TEST_F(WordWrap, stripes) {
  TWordWrapper wrap;
  wrap.cursor = TPoint(0,0);
  wrap.addLine(TPoint(10, 42), TPoint(40,10)); // a
  wrap.addLine(TPoint(40, 10), TPoint(55,20)); // b
  wrap.addLine(TPoint(55, 20), TPoint(60,15)); // c
  wrap.addLine(TPoint(60, 15), TPoint(80,38)); // d
  wrap.addLine(TPoint(80, 38), TPoint(50,80)); // e
  wrap.addLine(TPoint(50, 80), TPoint(10,42)); // f

  wrap.addLine(TPoint(40, 35), TPoint(45,30)); // g
  wrap.addLine(TPoint(45, 30), TPoint(50,35)); // h
  wrap.addLine(TPoint(50, 35), TPoint(45,40)); // i
  wrap.addLine(TPoint(45, 40), TPoint(40,35)); // j
  
  ASSERT_EQ(10, wrap.eventQueue.size());
  
  cout << "allEvents.size() = " << wrap.allEvents.size() << endl;
  cout << "eventQueue.size() = " << wrap.eventQueue.size() << endl;
  
//  for(auto &&p: wrap.eventQueue) {
//    cout << *p << endl;
//  }
  
  TWordWrapper::Slicer slicer(wrap.eventQueue);
  
  // new slice
  cout << "-------------- new slice -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;
  ASSERT_EQ(1, slicer.slices.size());
  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].right->data.line.p[0]);
  ASSERT_EQ(TPoint(55, 20), slicer.slices[0].right->data.line.p[1]);

  // new slice
  cout << "-------------- new slice -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;
  ASSERT_EQ(2, slicer.slices.size());
  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].right->data.line.p[0]);
  ASSERT_EQ(TPoint(55, 20), slicer.slices[0].right->data.line.p[1]);

  ASSERT_EQ(TPoint(60, 15), slicer.slices[1].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(55, 20), slicer.slices[1].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(60, 15), slicer.slices[1].right->data.line.p[0]);
  ASSERT_EQ(TPoint(80, 38), slicer.slices[1].right->data.line.p[1]);

  // join slices
  cout << "-------------- join slices -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;
//  cout << "slice: " << *slicer.slices[1].left << " <---> " << *slicer.slices[1].right << endl;
  
  ASSERT_EQ(1, slicer.slices.size());
  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(60, 15), slicer.slices[0].right->data.line.p[0]);
  ASSERT_EQ(TPoint(80, 38), slicer.slices[0].right->data.line.p[1]);

  // split slices
  cout << "-------------- split slices -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;
  
  ASSERT_EQ(2, slicer.slices.size());

  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(45, 30), slicer.slices[0].right ->data.line.p[0]);
  ASSERT_EQ(TPoint(40, 35), slicer.slices[0].right ->data.line.p[1]);
  
  ASSERT_EQ(TPoint(45, 30), slicer.slices[1].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(50, 35), slicer.slices[1].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(60, 15), slicer.slices[1].right->data.line.p[0]);
  ASSERT_EQ(TPoint(80, 38), slicer.slices[1].right->data.line.p[1]);

  cout << "-------------- continue right -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;

  ASSERT_EQ(2, slicer.slices.size());

  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(40, 35), slicer.slices[0].right ->data.line.p[0]);
  ASSERT_EQ(TPoint(45, 40), slicer.slices[0].right ->data.line.p[1]);
  
  ASSERT_EQ(TPoint(45, 30), slicer.slices[1].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(50, 35), slicer.slices[1].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(60, 15), slicer.slices[1].right->data.line.p[0]);
  ASSERT_EQ(TPoint(80, 38), slicer.slices[1].right->data.line.p[1]);

  cout << "-------------- continue left -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;

  ASSERT_EQ(2, slicer.slices.size());

  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);   // a
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(40, 35), slicer.slices[0].right ->data.line.p[0]);  // j
  ASSERT_EQ(TPoint(45, 40), slicer.slices[0].right ->data.line.p[1]);
  
  ASSERT_EQ(TPoint(50, 35), slicer.slices[1].left ->data.line.p[0]);   // i
  ASSERT_EQ(TPoint(45, 40), slicer.slices[1].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(60, 15), slicer.slices[1].right->data.line.p[0]);   // d
  ASSERT_EQ(TPoint(80, 38), slicer.slices[1].right->data.line.p[1]);

  cout << "-------------- continue right -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;

  ASSERT_EQ(2, slicer.slices.size());

  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(40, 35), slicer.slices[0].right ->data.line.p[0]);
  ASSERT_EQ(TPoint(45, 40), slicer.slices[0].right ->data.line.p[1]);
  
  ASSERT_EQ(TPoint(50, 35), slicer.slices[1].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(45, 40), slicer.slices[1].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(80 ,38), slicer.slices[1].right->data.line.p[0]);
  ASSERT_EQ(TPoint(50, 80), slicer.slices[1].right->data.line.p[1]);

  cout << "-------------- join -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;

  ASSERT_EQ(1, slicer.slices.size());

  ASSERT_EQ(TPoint(40, 10), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(80 ,38), slicer.slices[0].right->data.line.p[0]);
  ASSERT_EQ(TPoint(50, 80), slicer.slices[0].right->data.line.p[1]);

  cout << "-------------- continue left -----------------" << endl;
  ASSERT_EQ(true, slicer.step());
  cout << "--------------------------------------------" << endl;

//  for(auto &&p: slicer.slices) {
//    cout << "-> " << *p.left << " : " << *p.right << endl;
//  }

  ASSERT_EQ(1, slicer.slices.size());

  ASSERT_EQ(TPoint(10, 42), slicer.slices[0].left ->data.line.p[0]);
  ASSERT_EQ(TPoint(50, 80), slicer.slices[0].left ->data.line.p[1]);
  ASSERT_EQ(TPoint(80 ,38), slicer.slices[0].right->data.line.p[0]);
  ASSERT_EQ(TPoint(50, 80), slicer.slices[0].right->data.line.p[1]);

  cout << "-------------- done -----------------" << endl;
  ASSERT_EQ(false, slicer.step());
  cout << "--------------------------------------------" << endl;
}

#if 0
//      (30,10)
//         ▞▖
//        ▞▗▝▖ (30,20)
//       ▞   ▝▖
//(10,30)      (50,30)
TEST_F(WordWrap, findPlace002) {
  TWordWrapper wrap;
  wrap.cursor = TPoint(30,20);
  wrap.addLine(TPoint(30, 10), TPoint(10,30));
  wrap.addLine(TPoint(30, 10), TPoint(50,30));
  TPoint pos;
  ASSERT_EQ(true, wrap.findPlace(TSize(10,10), &pos);
  ASSERT_EQ(TPoint(30, 20), pos);
}
#endif

#if 0
//
//      (30,10)
//         ▞▖
//        ▞ ▝▖▖(50,20)
//       ▞   ▝▖
//(10,30)      (50,30)
TEST_F(WordWrap, findPlace003) {
  TWordWrapper wrap;
  wrap.cursor = TPoint(50,20);
  wrap.addLine(TPoint(30, 10), TPoint(10,30));
  wrap.addLine(TPoint(30, 10), TPoint(50,30));
  TPoint pos;
//  ASSERT_EQ(false, wrap.findPlace(TSize(10,10), &pos))
  ASSERT_EQ(true, wrap.findPlace(TSize(10,10), &pos))
}
#endif

} // namespace
