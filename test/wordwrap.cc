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

    TPoint cursor;
  
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
    
  
    // look for place in sweep events
    void place(const TSize &rectangle);
    void follow(const SweepEvent &event, const TPoint *upperScanLine, const TPoint *lowerScanLine);
    static TCoord distanceAtY(const SweepEvent*, const SweepEvent*, TCoord);
    static TPoint pointForDistance(const SweepEvent *e0, const SweepEvent *e1, TCoord width, TCoord y);

    // sweep event storage
    std::deque<SweepEvent> allEvents;
    struct eventQueueOrder:
      public std::binary_function<SweepEvent, SweepEvent, bool>
    {
      bool operator() (const SweepEvent *e0, const SweepEvent *e1);
    };

    std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, eventQueueOrder> eventQueue;

    void path2events(const TVectorPath& path);
    void processLine(const TPoint &p0, const TPoint &p1);
    void processCurve(const TPoint *p);
    SweepEvent* storeSweepEvent(const SweepEvent &e);
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
TWordWrapper::pointForDistance(const SweepEvent *e0, const SweepEvent *e1, TCoord width, TCoord y)
{
  switch(e0->type) {
    case LINE:
      switch(e1->type) {
        case LINE: {
          const TPoint *lineB = e1->data.line.p;
          TPoint line2[2] = {
            { lineB[0].x - width, lineB[0].y },
            { lineB[1].x - width, lineB[1].y },
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
  cout << "find intersections at line " << cursor.y << endl;
  std::deque<SweepEvent*> sweepLineEvents;

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
    cursor = pointForDistance(left, right, rectangle.width, cursor.y);
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
  cursor = path.bounds().p0;

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
        processLine(*pp, *pt);
        pp=pt;
        ++pt;
        break;
      case TVectorPath::CURVE:
        processCurve(pp);
        pp=pt+2;
        pt+=3;
        break;
      case TVectorPath::CLOSE:
        processLine(*pp, *ph);
        break;
    }
  }
}

bool
TWordWrapper::eventQueueOrder::operator() (const SweepEvent *e0, const SweepEvent *e1)
{
  switch(e0->type) {
    case TWordWrapper::LINE:
      switch(e1->type) {
        case TWordWrapper::LINE:
          // when e0->data.line.p[0] != e1->data.line.p[0], sort by these points
          if (e0->data.line.p[0].y > e1->data.line.p[0].y)
            return true;
          if (e1->data.line.p[0].y > e0->data.line.p[0].y)
            return false;
          if (e0->data.line.p[0].x != e1->data.line.p[0].x)
            e0->data.line.p[0].x > e1->data.line.p[0].x;
          
          // see that e0 comes after e1 clockwise
          return signedArea(e0->data.line.p[0], e0->data.line.p[1], e1->data.line.p[1]) > 0;
          
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
TWordWrapper::processLine(const TPoint &p0, const TPoint &p1)
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
  eventQueue.push(e0);
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
TWordWrapper::processCurve(const TPoint *p)
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

bool TFPath::restore(atv::TInObjectStream&) {}
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
  
  TPen pen(this);
  pen.setColor(0,0,1);

  TWordWrapper wrap;
  wrap.path2events(*path);
  wrap.place(TSize(8,12));

  pen.drawRectangle(wrap.cursor.x, wrap.cursor.y, 8, 12);
}

TEST_F(WordWrap, foo) {
  TTestWrap wnd(NULL, testname());
  wnd.doModalLoop();
/*
  TVectorPath path;
  path.move(160, 40);
//  path.curve(320,10, 50, 100, 310,130);
  path.line(310, 130);
  path.line(130, 190);
  path.line(10,70);
  path.close();

  TWordWrapper wrap;
  wrap.path2events(path);
  
  wrap.place(TSize(8,12));
*/
}

} // namespace
