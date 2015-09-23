/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

/*
 * This file implements
 *
 *   F. Martínez, C. Ogayar, J.R. Jiménez, A.J. Rueda.
 *   "A simple algorithm for Boolean operations on polygons"
 *   Advances in Engineering Software
 *   Volume 64, October 2013, Pages 11-19
 *
 * and builds upon the example implementation in bop12.zip at
 *
 *   http://www4.ujaen.es/~fmartin/bool_op.html
 *
 * F. Martínez noted that
 *
 *   The programs have some robust problems with "complex polygons".
 *
 * Changes to the upstream code
 *
 * o swapped Polygon for TVectorPath and other tweaks
 * o nextPos(): avoid segfault when newPos is -1
 * o todo: some glitches during union where cageo144.zip threw away points during union
 * o todo: support for curves
 */    

#include <toad/pen.hh>

#include <toad/geometry.hh>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "booleanop.hh"

#define DEBUG_PDF(CMD)

/*
 * \todo
 *   \li adding a curve as a single sweep event? how about add 'em as four lines/events
 *       but calculate the intersection differently?
 *   \li separate preparation and connectEdges calls
 *   \li with the previous separation, try to re-use the raw result for another operation
 *   \li use this for the pencil tool
 *   \li test cases for operations with curves (visual + check against expected results)
 *       when a test case fails, render the algorithms work into a pdf
 *   \li fix the orientation issue i've found
 *   \li revisit the fill tool code
 */

using namespace std;
using namespace toad;

DEBUG_PDF(
  bool global_debug = false;
  ostringstream txt;
)

unsigned sweepcntr=0;

SweepEvent::SweepEvent (bool b, const TPoint& p, SweepEvent* other, PolygonType pt, EdgeType et):
  left(b), point(p), curve(false), otherEvent(other), pol(pt), type(et), inResult(false) //, prevInResult(0)
{
  computed = false;
  id = sweepcntr++;
}

/** Return the point of the segment with lexicographically smallest coordinate */
const TPoint& minlex(const TPoint &s, const TPoint &t) {
  return (s.x < t.x) || (s.x == t.x && s.y < t.y) ? s : t;
}
/** Return the point of the segment with lexicographically largest coordinate */
const TPoint& maxlex(const TPoint &s, const TPoint &t) {
  return (s.x > t.x) || (s.x == t.x && s.y > t.y) ? s : t;
}

std::string SweepEvent::toString() const
{
	std::ostringstream oss;
	oss << "id:" << id
	    << " computed:" << (computed?"true":"false")
	    << " point:(" << point.x << ',' << point.y << ')'
	    << " (" << (left ? "left" : "right") << ')';
//	TPoint min = minlex(point, otherEvent->point);
//	TPoint max = maxlex(point, otherEvent->point);
//	oss << " S:[(" << min.x << ',' << min.y << ") - (" << max.x << ',' << max.y << ")]";
	oss << " pol:" << (pol == SUBJECT ? "SUBJECT" : "CLIPPING");
	std::string et[4] =  { "NORMAL", "NON_CONTRIBUTING", "SAME_TRANSITION", "DIFFERENT_TRANSITION" };
	oss << " type:" << et[this->type]
	    << " inOut:" << (inOut ? "true" : "false")
	    << " otherInOut:" << (otherInOut ? "true" : "false")
	    << " inResult:" << (inResult ? "yes":"no");
	return oss.str();
}

// le1 and le2 are the left events of line segments (le1->point, le1->otherEvent->point) and (le2->point, le2->otherEvent->point)
bool SegmentComp::operator() (SweepEvent* le1, SweepEvent* le2)
{
	if (le1 == le2)
		return false;
	if (signedArea (le1->point, le1->otherEvent->point, le2->point) != 0 || 
		signedArea (le1->point, le1->otherEvent->point, le2->otherEvent->point) != 0) {
		// Segments are not collinear
		// If they share their left endpoint use the right endpoint to sort
		if (le1->point == le2->point)
			return le1->below (le2->otherEvent->point);
		// Different left endpoint: use the left endpoint to sort
		if (le1->point.x == le2->point.x)
			return le1->point.y < le2->point.y;
		SweepEventComp comp;
		if (comp (le1, le2))  // has the line segment associated to e1 been inserted into S after the line segment associated to e2 ?
			return le2->above (le1->point);
		// The line segment associated to e2 has been inserted into S after the line segment associated to e1
		return le1->below (le2->point);
	}
	// Segments are collinear
	if (le1->pol != le2->pol)
		return le1->pol < le2->pol;
	// Just a consistent criterion is used
	if (le1->point == le2->point)
		return le1 < le2;
	SweepEventComp comp;
	return comp (le1, le2);
}

BooleanOpImp::BooleanOpImp(BooleanOpType op)
  : operation (op), eq (), sl (), eventHolder()
{
}

bool
BooleanOpImp::trivialOperation(const toad::TVectorPath& subject,
                               const toad::TVectorPath& clipping,
                               const Bbox_2& subjectBB,
                               const Bbox_2& clippingBB,
                               toad::TVectorPath &out)
{
	// Test 1 for trivial result case
	if (subject.empty() || clipping.empty()) { // At least one of the polygons is empty
		if (operation == DIFFERENCE)
			out = subject;
		if (operation == UNION || operation == XOR)
			out = subject.empty() ? clipping : subject;
		return true;
	}
	// Test 2 for trivial result case
	if (subjectBB.xmin () > clippingBB.xmax () || clippingBB.xmin () > subjectBB.xmax () ||
	    subjectBB.ymin () > clippingBB.ymax () || clippingBB.ymin () > subjectBB.ymax ()) {
		// the bounding boxes do not overlap
		if (operation == DIFFERENCE)
			out = subject;
		if (operation == UNION || operation == XOR) {
			out = subject;
			out.join(clipping);
		}
		return true;
	}

	return false;
}

void BooleanOpImp::path2events(const toad::TVectorPath& poly, PolygonType type)
{
// FIXME: this function must drop neighbouring equal points (degenerated case)
// FIXME: this function must catch empty polygons
        const TPoint *pt = poly.points.data();
        const TPoint *ph, *pp; // head & previous
        for(auto p: poly.type) {
          switch(p) {
            case TVectorPath::MOVE:
              pp=ph=pt;
              ++pt;
              break;
            case TVectorPath::LINE:
              processLine(*pp, *pt, type);
              pp=pt;
              ++pt;
              break;
            case TVectorPath::CURVE:
              processCurve(*pp, pt, type);
              pp=pt+2;
              pt+=3;
              break;
            case TVectorPath::CLOSE:
              processLine(*pp, *ph, type);
              break;
          }
        }
}


/**
 * convert segment/edge into two sweep events
 */
void BooleanOpImp::processLine(const TPoint &p0, const TPoint &p1, PolygonType pt)
{
//cout << "segment " << p0 << " - " << p1 << endl;
	if (p0==p1) // if the two edge endpoints are equal the segment is dicarded
          return;   // This can be done as preprocessing to avoid "polygons" with less than 3 edges
	SweepEvent* e1 = storeSweepEvent(SweepEvent(true, p0, 0, pt));
	SweepEvent* e2 = storeSweepEvent(SweepEvent(true, p1, e1, pt));
	e1->otherEvent = e2;

	if (minlex(p0, p1) == p0) {
		e2->left = false;
	} else {
		e1->left = false;
	}
	eq.push(e1);
	eq.push(e2);
/*
std::cout << "processSegment" << std::endl;
std::cout << "  " << e1->toString() << std::endl;
std::cout << "  " << e2->toString() << std::endl;
*/
}

void BooleanOpImp::processCurve(const TPoint &p0, const TPoint* pn, PolygonType pt)
{
/*	if (s.degenerate ()) // if the two edge endpoints are equal the segment is dicarded
		return;          // This can be done as preprocessing to avoid "polygons" with less than 3 edges */
	SweepEvent* e1 = storeSweepEvent(SweepEvent(true, p0, 0, pt));
	SweepEvent* e2 = storeSweepEvent(SweepEvent(true, *(pn+2), e1, pt));
	e2->point1 = *(pn);
	e2->point2 = *(pn+1);
	e2->curve = true;
	e1->otherEvent = e2;

	if (minlex(p0, *(pn+2)) == p0) {
		e2->left = false;
	} else {
		e1->left = false;
	}
	eq.push(e1);
	eq.push(e2);
}

static TPoint origin;
static TCoord scale;

void
drawArrow(TPen &pen, const TPoint &p2, const TPoint &p1)
{
  TCoord w, h;
  w=h=8.0/::scale;

  TCoord d = atan2(p2.y - p1.y, 
                   p2.x - p1.x);
    
  TCoord height = h;
  TCoord width  = 0.5 * w;
  
  TPoint p0;
  
  TPoint p[4];
  
  p[0] = p1;
  
  p0.x = cos(d) * height + p1.x;
  p0.y = sin(d) * height + p1.y;

  TCoord r = 90.0 / 360.0 * (2.0 * M_PI);
  p[1].x = p0.x + cos(d-r) * width;
  p[1].y = p0.y + sin(d-r) * width;
  p[3].x = p0.x + cos(d+r) * width;
  p[3].y = p0.y + sin(d+r) * width;
  
  pen.drawLine(p[0], p[1]);
  pen.drawLine(p[0], p[3]);
}

void
drawLineWithArrow(TPen &pen, SweepEvent *x)
{
  TPoint p = (x->point - x->otherEvent->point) * 0.5 + x->otherEvent->point;
  pen.drawLine(x->point, x->otherEvent->point);
  if (x->left) {
    drawArrow(pen, x->point, x->otherEvent->point);
    pen.drawString(p.x, p.y, format("%u", x->id));
  } else {
    drawArrow(pen, x->otherEvent->point, x->point);
    pen.drawString(p.x, p.y, format("%u", x->otherEvent->id));
  }
}

void
drawSweepEvents(TPen &pen, std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, SweepEventComp> &eq, SweepEvent* se)
{
  pen.setAlpha(0.3);
  pen.setLineWidth(1.0/scale);
  pen.translate(470.5,10.5);
  pen.scale(::scale,::scale);
  pen.setFont(format("helvetica:size=%f", 8.0/scale));
  pen.translate(-origin.x, -origin.y);
  std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, SweepEventComp> eq2(eq);
  while(!eq2.empty()) {
    SweepEvent* se = eq2.top();
    eq2.pop();
    if (se->pol == SUBJECT)
      pen.setColor(0,0.7,0);
    else
      pen.setColor(0,0,1);
    if (se->left) {
      drawLineWithArrow(pen, se);
    }
  }
  if (se) {
    pen.push();
    pen.setColor(1,0,0);
    pen.setLineStyle(TPen::DOT);
    pen.drawLine(se->point.x, 0, se->point.x, se->point.y);
    pen.pop();
  }
}

DEBUG_PDF(
void
sweep2txt(SweepEvent *se)
{
  if (!se) {
    txt << "none" << endl;
    return;
  }
  if (se->left) {
    txt << se->toString() << endl
        << se->otherEvent->toString() << endl;
  } else {
    txt << se->otherEvent->toString() << endl
        << se->toString() << endl;
  }
}  
)

void BooleanOpImp::run(const toad::TVectorPath& subj, const toad::TVectorPath& clip, toad::TVectorPath& out)
{
DEBUG_PDF(
  TPen *pdf = 0;
  if (global_debug)
    pdf = new TPen("bool.pdf");
  sweepcntr=0;
)

  // for optimizations 1 and 2
  toad::TBoundary sb(subj.editBounds()), cb(clip.editBounds());

DEBUG_PDF(
  toad::TBoundary view(sb);
  view.expand(cb);
  ::origin.x = view.x1;
  ::origin.y = view.y1;
  ::scale    = 480.0 / std::max(view.x2-view.x1, view.y2-view.y1);
)

  Bbox_2 subjectBB(sb.x1, sb.y1, sb.x2, sb.y2);
  Bbox_2 clippingBB(cb.x1, cb.y1, cb.x2, cb.y2);
  const double MINMAXX = std::min(subjectBB.xmax(), clippingBB.xmax()); // for optimization 2

  if (trivialOperation(subj, clip, subjectBB, clippingBB, out)) // trivial cases can be quickly resolved without sweeping the plane
    return;

  // convert 'subject' and 'clipping' into sweep events (eventHolder := all events,  eq := sorted events)
  path2events(subj, SUBJECT);
  path2events(clip, CLIPPING);

DEBUG_PDF(
  if (pdf) {
    pdf->setColor(0,0,0);
    pdf->push();
    pdf->drawString(5,5, "Sweep Events");
    drawSweepEvents(*pdf, eq, 0);
  }
  unsigned cntr = 0;
)

  // 'subj' & 'clip' aren't used anymore, if one of 'em is the same as 'out'
  // we can now clear 'out'
  out.clear();

  // compute the events from eq into sortedEvents
  std::deque<SweepEvent*> sortedEvents;
        
  // eq: (Q) priority queue
  // sl: (S) sweep line status
  std::set<SweepEvent*, SegmentComp>::iterator it, prev, next;
//if (global_debug) cout << "----------------- SWEEP -----------------" << endl;

  while (! eq.empty ()) {
    SweepEvent* se = eq.top ();
    DEBUG_PDF(
      txt.str("");
      txt.clear();
      txt.precision(numeric_limits<double>::max_digits10);
      ++cntr;
    )

    // optimization 2
    if ((operation == INTERSECTION && se->point.x > MINMAXX) ||
        (operation == DIFFERENCE && se->point.x > subjectBB.xmax ()))
    {
      connectEdges(sortedEvents, out);
      return;
    }

    sortedEvents.push_back(se);
    eq.pop();

DEBUG_PDF(
    if (pdf) {
      pdf->pagebreak();
      txt << "sweep step " << cntr << (se->left?", left point of":", right point of")<<endl;
      sweep2txt(se);
      pdf->push();
      drawSweepEvents(*pdf, eq, se);

      pdf->setColor(0,0,0);
      for(const auto &x: sortedEvents) {
        if (x->inResult && sl.find(x)==sl.end())
          drawLineWithArrow(*pdf, x);
      }
  
      pdf->setColor(1,0.5,0);
      for(const auto &x: sl) {
        drawLineWithArrow(*pdf, x);
      }

      pdf->setColor(1,0,0);
      drawLineWithArrow(*pdf, se);
    }
)

    if (se->left) {
      // sweep line has reached a new line, insert it into the sweep line status 'sl'
      next = prev = se->posSL = it = sl.insert(se).first;
assert(*it==se);
      (prev != sl.begin()) ? --prev : prev = sl.end();
      ++next;
      DEBUG_PDF(
        txt<<"compute fields with previous:"<<endl;
        sweep2txt(prev!=sl.end()?*prev:0);
      )
      computeFields(se, prev!=sl.end()?*prev:0);

      DEBUG_PDF(
        txt << "after compute Fields:" << endl;
        sweep2txt(se);
      )

      // intersections with next neighbour
      if (next != sl.end()) {
        DEBUG_PDF(
          txt << "next neighbour is" << endl;
          sweep2txt(*next);
        )
        if (possibleIntersection(se, *next) == 2) {
          DEBUG_PDF(
            txt << "two intersections with next neighbour" << endl;
            txt << "compute fields of:" << endl;
            sweep2txt(se);
            txt << "with previous: " << endl;
            sweep2txt(prev!=sl.end()?*prev:0);
          )
          computeFields(se, prev!=sl.end()?*prev:0);
          DEBUG_PDF(
            txt << "results in" << endl;
            sweep2txt(se);

            txt << "compute fields of: " << endl;
            sweep2txt(next!=sl.end()?*next:0);
            txt << "with previous: " << endl;
            sweep2txt(it!=sl.end()?*it:0);
          )
          computeFields(*next, it!=sl.end()?*it:0);
          DEBUG_PDF(
            txt << "results in" << endl;
            sweep2txt(*next);
          )
        }
      }
      // intersections with previous neighbour
      if (prev != sl.end ()) {
        DEBUG_PDF(
          txt << "previous neighbour is" << endl;
          sweep2txt(*prev);
        )
        if (possibleIntersection(*prev, se) == 2) {
          DEBUG_PDF(txt << "two intersections" << endl;)
          auto prevprev = prev;
          (prevprev != sl.begin()) ? --prevprev : prevprev = sl.end();

          computeFields(*prev, prevprev!=sl.end()?*prevprev:0);
          computeFields(se, prev!=sl.end()?*prev:0);
          DEBUG_PDF(
            txt << "prevprev: " << endl;
            sweep2txt(prevprev!=sl.end()?*prevprev:0);
            txt << "prev: " << endl;
            sweep2txt(*prev);
            txt << "se: " << endl;
            sweep2txt(se);
            txt << "prev: " << endl;
            sweep2txt(prev!=sl.end()?*prev:0);
          )
        }
      }
    } else {
      DEBUG_PDF(txt << "on right side of event" << endl;)
      // sweep line has reached end of a line, remove it from the sweep line status 'sl'
      se = se->otherEvent; // we work with the left event
      next = prev = it = se->posSL; // se->posSL; is equal than sl.find(se); but faster
      (prev != sl.begin()) ? --prev : prev = sl.end();
      ++next;
      DEBUG_PDF(
        txt << "previous neighbour is" << endl;
        if (prev!=sl.end())
          sweep2txt(*prev);
        else
          txt << "none" << endl;
        txt << "next neighbour is" << endl;
        if (next!=sl.end())
          sweep2txt(*next);
        else
          txt << "none" << endl;
      )
      // delete line segment associated to "se" from sl and check for intersection between the neighbors of "se" in sl
      sl.erase(it);
      if (next != sl.end() && prev != sl.end()) {
        DEBUG_PDF(
          txt<<"check intersection between prev"<<endl;
          sweep2txt(*prev);
          txt<<"and next"<<endl;
          sweep2txt(*next);
        )
        possibleIntersection(*prev, *next);
      }
    }

    DEBUG_PDF(
      if (pdf) {
        pdf->pop();
        pdf->setColor(0,0,0);
        pdf->setAlpha(1);
        pdf->setFont("helvetica:size=8");
        pdf->drawTextWidth(5,5,txt.str(),480*2);
      }
    )
  }

  connectEdges(sortedEvents, out);

  DEBUG_PDF(if (pdf) delete pdf;)
}

/**
 * Derive the le's inOut and otherInOut fields from le and the previous polygon in sl (S)
 *
 * \in le left sweep event
 */
void BooleanOpImp::computeFields(SweepEvent* le, SweepEvent* prev)
{
  le->computed = true;
//cout << "computeFields for " << le->toString() << endl;

  // compute inOut and otherInOut fields
  if (!prev) {
    // the previous line is not in the sweep line status
    DEBUG_PDF(txt<<"compute: no previous: inOut=false, otherInOut=true"<<endl;)
    le->inOut = false;     			// entering the polygon
    le->otherInOut = true; 			// there was no other polygon, but if there was, we would have left it
  } else if (le->pol == prev->pol) {
    // previous line segment in sl belongs to the same polygon that "se" belongs to
    DEBUG_PDF(
      txt<<"compute: previous is same polygon: inOut=!prev->InOut="
         <<((!prev->inOut)?"true":"false")
         <<", otherInOut=prev->otherInOut="<<(prev->otherInOut?"true":"false")
         <<endl;)
    le->inOut = !prev->inOut;		// crossing a line, so the state changes
    le->otherInOut = prev->otherInOut;	// no contact with the other polygon, nothing has changed
  } else {
    // previous line segment in sl belongs to a different polygon that "se" belongs to
    DEBUG_PDF(
      txt<<"compute: previous is different polygon: inOut=!prev->otherInOut="
         <<((!prev->otherInOut)?"true":"false");
      if(prev->vertical()) {
        txt<<", prev=vertical -> otherInOut=!prev->InOut="<<((!prev->inOut)?"true":"false")<<endl;
      } else {
        txt<<", prev=not vertical -> otherInOut=prev->inOut="<<((prev->inOut)?"true":"false")<<endl;
      }
    )
    // prev->otherInOut has tracked where we where in the polygon belonging to 'le'
    // now that we've crossed it's line, the state changes
    le->inOut = !prev->otherInOut;
    if (!prev->vertical()) {
      // since we haven't touched a line of prev->inOut, we are still one the same side
      // of it
      le->otherInOut = prev->inOut;
    } else {
      // vertical, this is a special case and i have no fcuking idea why we do this
      // the previous line was vertical...  hm. see 7.1 in the paper to grok it.
      le->otherInOut = !prev->inOut;
    }
  }

  // check if the line segment belongs to the Boolean operation('s result?)
  le->inResult = inResult(le);
  DEBUG_PDF(txt<<"compute: inResult(current) = "<<(le->inResult?"true":"false")<<endl;)
}

bool
BooleanOpImp::inResult(const SweepEvent* le) const
{
  switch (le->type) {
    case NORMAL:
      switch(operation) {
        case INTERSECTION:
	  return !le->otherInOut; // 
	case UNION:
	  DEBUG_PDF(txt<<"inResult(): UNION: return otherInOut (="<<(le->otherInOut?"true)":"false)")<<endl;)
	  return le->otherInOut; 
	case DIFFERENCE:
          return (le->pol == SUBJECT && le->otherInOut) || (le->pol == CLIPPING && !le->otherInOut);
	case XOR:
          return true;
      }
    case SAME_TRANSITION:
      DEBUG_PDF(if (operation==UNION)
        txt<<"inResult(): SAME_TRANSITION & UNION: return true"<<endl;)
      return operation == INTERSECTION || operation == UNION;
    case DIFFERENT_TRANSITION:
      DEBUG_PDF(if (operation==UNION)
         txt<<"inResult(): DIFFERENT_TRANSITION & UNION: return false"<<endl;)
      return operation == DIFFERENCE;
    case NON_CONTRIBUTING:
      DEBUG_PDF(txt<<"inResult(): NON_CONTRIBUTING: return false"<<endl;)
      return false;
  }
  return false; // just to avoid the compiler warning
}

static int findIntersection(double u0, double u1, double v0, double v1, double w[2])
{
  if ((u1 < v0) || (u0 > v1))
    return 0;
  if (u1 > v0) {
    if (u0 < v1) {
      w[0] = (u0 < v0) ? v0 : u0;
      w[1] = (u1 > v1) ? v1 : u1;
      return 2;
    } else {
      // u0 == v1
      w[0] = u0;
      return 1;
    }
  } else {
    // u1 == v0
    w[0] = u1;
    return 1;
  }
}

// distance of line (p0, p1) to point q
static TCoord
distance(const TPoint &p0, const TPoint &p1, const TPoint &q)
{
  TPoint b(p1-p0);
  TPoint a(q-p0);
  
  TCoord lb = squaredLength(b);
  TCoord t = dot(a, b) / lb;
  if (t<0.0 || t>1.0)
    return 1.0/0.0;
  return fabs(b.y * a.x - b.x * a.y) / sqrt(lb);
}

void
intersectLineLine2(vector<TPoint> &ilist, const TPoint *l0, TPoint *l1)
{
  // IMPROVE ME: we might want to pick a middle point if available?

  bool f00, f01, f10, f11;
  f00=f01=f10=f11=false;

  // overlapping endpoints
  if (distance(l0[0], l1[0]) < tolerance) {
    ilist.push_back(l0[0]);
    f00=f10=true;
  }
  if (distance(l0[0], l1[1]) < tolerance) {
    ilist.push_back(l0[0]);
    f00=f11=true;
  }
  if (distance(l0[1], l1[0]) < tolerance) {
    ilist.push_back(l0[1]);
    f01=f10=true;
  }
  if (distance(l0[1], l1[1]) < tolerance) {
    ilist.push_back(l0[1]);
    f01=f11=true;
  }
  
  // end point overlaps with line
  if (!f10 && distance(l0[0], l0[1], l1[0]) < tolerance)
    ilist.push_back(l1[0]);
  if (!f00 && distance(l1[0], l1[1], l0[0]) < tolerance)
    ilist.push_back(l0[0]);
  if (!f11 && distance(l0[0], l0[1], l1[1]) < tolerance)
    ilist.push_back(l1[1]);
  if (!f01 && distance(l1[0], l1[1], l0[1]) < tolerance)
    ilist.push_back(l0[1]);

  // crossing
  if (!ilist.empty())
    return;

  TCoord ax = l0[1].x - l0[0].x;
  TCoord ay = l0[1].y - l0[0].y;
  TCoord bx = l1[1].x - l1[0].x;
  TCoord by = l1[1].y - l1[0].y;

  TCoord cross = ax*by - ay*bx;
  if (isZero(cross))
    return;
  
  TCoord 
    dx = l0[0].x - l1[0].x,
    dy = l0[0].y - l1[0].y,
    a = (bx * dy - by * dx) / cross,
    b = (ax * dy - ay * dx) / cross;
  if (a<0.0 || a>1.0 || b<0.0 || b>1.0)
    return;
  ilist.push_back(TPoint(l0[0].x + a * ax, l0[0].y + a * ay));
}

static int 
findIntersection(const SweepEvent* e0, const SweepEvent* e1, TPoint& pi0, TPoint& pi1)
{
  TPoint pt[4];
  pt[0] = e0->point;
  pt[1] = e0->otherEvent->point;
  pt[2] = e1->point;
  pt[3] = e1->otherEvent->point;
  
  vector<TPoint> il;
  intersectLineLine2(il, pt, pt+2);
  
  if (il.size()>0) pi0 = il[0];
  if (il.size()>1) pi0 = il[1];
  
  return il.size();
}

int BooleanOpImp::possibleIntersection(SweepEvent* le1, SweepEvent* le2)
{
  TIntersectionList il;
  TPoint l0[2] = { le1->point, le1->otherEvent->point };
  TPoint l1[2] = { le2->point, le2->otherEvent->point };
  intersectLineLine(il, l0, l1);

  DEBUG_PDF(
    txt<<"intersectLineLine found " << il.size() << " intersection" << endl;
    for(const auto &p: il)
      txt<<"  "<<p.seg0.pt<<" or "<<p.seg1.pt<<endl;
  )

//	if (e1->pol == e2->pol) // you can uncomment these two lines if self-intersecting polygons are not allowed
//		return 0;

  TPoint ip1, ip2;  // intersection points
  int nintersections;

  if (!(nintersections = findIntersection(le1, le2, ip1, ip2))) {
    DEBUG_PDF(txt<<"no intersection => do nothing" << endl;)
    return 0;  // no intersection
  }
  DEBUG_PDF(txt<<"found " << nintersections << " intersections" << endl;)

  if ((nintersections == 1) && ((le1->point == le2->point) || (le1->otherEvent->point == le2->otherEvent->point))) {
    DEBUG_PDF(txt<<"line segments intersect at an endpoint of both line segments => no nothing"<<endl;)
    return 0; // the line segments intersect at an endpoint of both line segments
  }

  if (nintersections == 2 && le1->pol == le2->pol) {
    std::cerr << "Sorry, edges of the same polygon overlap\n";
    std::cerr << "  " << le1->toString() << endl;
    std::cerr << "  " << le2->toString() << endl;
    std::cerr << "  le1=" << le1 << endl;
    std::cerr << "  le2=" << le2 << endl;
return 0;
    exit(1); // the line segments overlap, but they belong to the same polygon
  }

  // The line segments associated to le1 and le2 intersect
  if (nintersections == 1) {
    DEBUG_PDF(txt<<"one intersection => divide" << endl;)
    if (le1->point != ip1 && le1->otherEvent->point != ip1) { // if the intersection point is not an endpoint of le1->segment ()
      divideSegment(le1, ip1);
    }
    if (le2->point != ip1 && le2->otherEvent->point != ip1) { // if the intersection point is not an endpoint of le2->segment ()
      divideSegment(le2, ip1);
    }
    return 1;
  }
	// The line segments associated to le1 and le2 overlap
//std::cout << "  overlap" << std::endl;
  std::vector<SweepEvent*> sortedEvents;

  if (le1->point == le2->point) {
    sortedEvents.push_back(0);
  } else if (sec(le1, le2)) {
    sortedEvents.push_back(le2);
    sortedEvents.push_back(le1);
  } else {
    sortedEvents.push_back(le1);
    sortedEvents.push_back(le2);
  }


	if (le1->otherEvent->point == le2->otherEvent->point) {
		sortedEvents.push_back(0);
	} else if (sec (le1->otherEvent, le2->otherEvent)) {
		sortedEvents.push_back(le2->otherEvent);
		sortedEvents.push_back(le1->otherEvent);
	} else {
		sortedEvents.push_back(le1->otherEvent);
		sortedEvents.push_back(le2->otherEvent);
	}

	if ((sortedEvents.size() == 2) || (sortedEvents.size() == 3 && sortedEvents[2])) { 
	        DEBUG_PDF(txt<<"overlap: line segments overlap, both line segments are equal or share the left endpoint" << endl;)
		// both line segments are equal or share the left endpoint
		// we take one line out of the equation
		le1->type = NON_CONTRIBUTING;
DEBUG_PDF(txt<<"setting event id:" << le1->id << " to NON_CONTRIBUTING"<<endl;)

		le2->type = (le1->inOut == le2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
DEBUG_PDF(
txt<<"setting event id:" << le2->id << " to " 
                         << (le2->type==SAME_TRANSITION?"SAME_TRANSITION because inOut equals":"DIFFERENT_TRANSITION because inOut is non-equal")
                         << endl;
)
		if (sortedEvents.size()==3) {
			divideSegment(sortedEvents[2]->otherEvent, sortedEvents[1]->point);
                }
		return 2;
	}




	if (sortedEvents.size () == 3) { // the line segments share the right endpoint
DEBUG_PDF(txt<<"overlap: line segments share the right endpoint, divide " << sortedEvents[0]->point << " - " << sortedEvents[0]->otherEvent->point << " at " << sortedEvents[1]->point << endl;)
		divideSegment (sortedEvents[0], sortedEvents[1]->point);
		return 3;
	}
	if (sortedEvents[0] != sortedEvents[3]->otherEvent) { // no line segment includes totally the other one
DEBUG_PDF(txt<<"overlap: no line segment includes totally the other one" << endl;)
		divideSegment (sortedEvents[0], sortedEvents[1]->point);
		divideSegment (sortedEvents[1], sortedEvents[2]->point);
		return 3;
	}
	// one line segment includes the other one
DEBUG_PDF(txt<<"overlap: one line segment includes the other one" << endl;)
	divideSegment(sortedEvents[0], sortedEvents[1]->point);
	divideSegment(sortedEvents[3]->otherEvent, sortedEvents[2]->point);
	return 3;
}

/**
 * split
 *    (le -> le->otherEvent)
 * at p into
 *    (le -> r) (l -> le->leOtherEvent)
 */
void BooleanOpImp::divideSegment(SweepEvent* le, const TPoint& p)
{
DEBUG_PDF(txt << "divide: "<<endl; sweep2txt(le);)
	// "Right event" of the "left line segment" resulting from dividing le->segment ()
	SweepEvent* r = storeSweepEvent(SweepEvent(false, p, le, le->pol/*, le->type*/));
	// "Left event" of the "right line segment" resulting from dividing le->segment ()
	SweepEvent* l = storeSweepEvent(SweepEvent(true, p, le->otherEvent, le->pol/*, le->other->type*/));
	if (sec(l, le->otherEvent)) { // avoid a rounding error. The left event would be processed after the right event
		std::cout << "Oops" << std::endl;
		le->otherEvent->left = true;
		l->left = false;
	}
	if (sec(le, r)) { // avoid a rounding error. The left event would be processed after the right event
		std::cout << "Oops2" << std::endl;
	}
	le->otherEvent->otherEvent = l;
	le->otherEvent = r;
DEBUG_PDF(
txt << "into:"<<endl; sweep2txt(le);
txt<<"and"<<endl;     sweep2txt(l);
)
	eq.push(l);
	eq.push(r);
}

void BooleanOpImp::connectEdges(const std::deque<SweepEvent*> &sortedEvents, toad::TVectorPath& out)
{
  // copy the events in the result polygon to resultEvents array
  std::vector<SweepEvent*> resultEvents;
  resultEvents.reserve(sortedEvents.size());
  for(auto it: sortedEvents) {
    if((it->left && it->inResult) || (!it->left && it->otherEvent->inResult))
      resultEvents.push_back(it);
  }

  // Due to overlapping edges the resultEvents array can be not wholly sorted
  bool sorted = false;
  while(!sorted) {
    sorted = true;
    for(size_t i = 0; i < resultEvents.size (); ++i) {
      if(i + 1 < resultEvents.size() && sec (resultEvents[i], resultEvents[i+1])) {
        std::swap(resultEvents[i], resultEvents[i+1]);
        sorted = false;
      }
    }
  }

  for(size_t i = 0; i < resultEvents.size (); ++i) {
    resultEvents[i]->pos = i;
      if(!resultEvents[i]->left)
        std::swap(resultEvents[i]->pos, resultEvents[i]->otherEvent->pos);
  }

  std::vector<bool> processed(resultEvents.size(), false);
  for(size_t i = 0; i < resultEvents.size (); ++i) {
    if (processed[i])
      continue;
    ssize_t pos = i;
    const TPoint &initial = resultEvents[i]->point;
    out.move(initial);
    while(resultEvents[pos]->otherEvent->point != initial) {

      processed[pos] = true; 

      if (resultEvents[pos]->left) {
        resultEvents[pos]->resultInOut = false;
      } else {
        resultEvents[pos]->otherEvent->resultInOut = true; 
      }
      pos = resultEvents[pos]->pos;
      processed[pos] = true;
      if(resultEvents[pos]->curve) {
        out.curve(resultEvents[pos]->point1,
                  resultEvents[pos]->point2,
                  resultEvents[pos]->point);
      } else {
        out.line(resultEvents[pos]->point);
      }
ssize_t oldpos = pos;

      pos = nextPos(pos, resultEvents, processed);
      if(pos<0) {
        pos = oldpos;
        break;
      }
    }
    out.close();
    processed[pos] = processed[resultEvents[pos]->pos] = true;
    resultEvents[pos]->otherEvent->resultInOut = true; 
  }
}

ssize_t
BooleanOpImp::nextPos(ssize_t pos, const std::vector<SweepEvent*>& resultEvents, const std::vector<bool>& processed)
{
  ssize_t newPos = pos + 1;
  while(newPos < resultEvents.size() && resultEvents[newPos]->point == resultEvents[pos]->point) {
    if (!processed[newPos])
      return newPos;
    else
      ++newPos;
  }
  newPos = pos - 1;
  while(newPos>=0 && processed[newPos]) {
    --newPos;
  }
  return newPos;
}

void
toad::boolean(const TVectorPath &subj, const TVectorPath &clip, TVectorPath *result, BooleanOpType op)
{
#if 0
{
  TPoint p0, p1;
  SweepEvent ose0(true, TPoint(135.1312211366641 ,226.82039836077857), 0, SUBJECT);
  SweepEvent  se0(true, TPoint(133.59520376433233,229.00462271647683), &ose0, SUBJECT);
  SweepEvent ose1(true, TPoint(134.54184884140335,227.65854220348569), 0, CLIPPING);
  SweepEvent  se1(true, TPoint(133.97914732243984,228.51482008130631), &ose1, CLIPPING);
  int n = findIntersection(&se0, &se1, p0, p1);
  cout << "1st intersection found " << n << endl;
}
{
  TPoint p0, p1;
  SweepEvent ose0(true, TPoint(135.1312211366641 ,226.82039836077857), 0, SUBJECT);
  SweepEvent  se0(true, TPoint(133.59520376433233,229.00462271647683), &ose0, SUBJECT);
  SweepEvent ose1(true, TPoint(135.1312211366641 ,226.82039836077857), 0, CLIPPING);
  SweepEvent  se1(true, TPoint(134.54184884140335,227.65854220348569), &ose1, CLIPPING);
  int n = findIntersection(&se0, &se1, p0, p1);
  cout << "2nd intersection found " << n << endl;
}
exit(0);
#endif
  BooleanOpImp boi(op);
  boi.run(subj, clip, *result);
}

