/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

// bop12/cpp/booleanop.cpp

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "booleanop.hh"

/*
 * \todo
 *   \li re-enable optimizations
 *   \li separate preparation and connectEdges calls
 *   \li with the previous separation, try to re-use the raw result for another operation
 *   \li use this for the pencil tool
 *   \li test cases for operations with curves (visual + check against expected results)
 *       when a test case fails, render the algorithms work into a pdf
 *   \li fix the orientation issue i've found
 *   \li revisit the fill tool code
 */

using namespace cbop;
using namespace std;
using namespace toad;

SweepEvent::SweepEvent (bool b, const TPoint& p, SweepEvent* other, PolygonType pt, EdgeType et) : 
  left (b), point (p), otherEvent (other), pol (pt), type (et), prevInResult (0), inResult (false), curve(false)
{
}

/** Return the point of the segment with lexicographically smallest coordinate */
const TPoint& minlex(const TPoint &s, const TPoint &t) {
  return (s.x < t.x) || (s.x == t.x && s.y < t.y) ? s : t;
}
/** Return the point of the segment with lexicographically largest coordinate */
const TPoint& maxlex(const TPoint &s, const TPoint &t) {
  return (s.x > t.x) || (s.x == t.x && s.y > t.y) ? s : t;
}


std::string SweepEvent::toString () const
{
	std::ostringstream oss;
	oss << '(' << point.x << ',' << point.y << ')';
	oss << " (" << (left ? "left" : "right") << ')';
	TPoint min = minlex(point, otherEvent->point);
	TPoint max = maxlex(point, otherEvent->point);
	oss << " S:[(" << min.x << ',' << min.y << ") - (" << max.x << ',' << max.y << ")]";
	oss << " (" << (pol == SUBJECT ? "SUBJECT" : "CLIPPING") << ')';
	std::string et[4] =  { "NORMAL", "NON_CONTRIBUTING", "SAME_TRANSITION", "DIFFERENT_TRANSITION" };
	oss << " (" << et[type] << ')';
	oss << " (" << (inOut ? "inOut" : "outIn") << ')';
	oss << " otherInOut: (" << (otherInOut ? "inOut" : "outIn") << ')';
	return oss.str ();
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

void BooleanOpImp::run(const toad::TVectorPath& subj, const toad::TVectorPath& clip, toad::TVectorPath& out)
{
        // for optimizations 1 and 2
        toad::TBoundary sb(subj.editBounds()), cb(clip.editBounds());
        Bbox_2 subjectBB(sb.x1, sb.y1, sb.x2, sb.y2);
        Bbox_2 clippingBB(cb.x1, cb.y1, cb.x2, cb.y2);
        const double MINMAXX = std::min(subjectBB.xmax(), clippingBB.xmax()); // for optimization 2
#if 0
	if (trivialOperation(subject, clipping, subjectBB, clippingBB)) // trivial cases can be quickly resolved without sweeping the plane
		return;
#endif
        // convert 'subject' and 'clipping' into sweep events (eventHolder := all events,  eq := sorted events)
//cout << "SUBJECT................." << endl;
        path2events(subj, SUBJECT);
//cout << "CLIPPING................" << endl;
        path2events(clip, CLIPPING);
//cout << "........................" << endl;

        // compute the events from eq into sortedEvents
        std::deque<SweepEvent*> sortedEvents;
        
        // eq: (Q) priority queue
        // sl: (S) sweep line status
	std::set<SweepEvent*, SegmentComp>::iterator it, prev, next;
	while (! eq.empty ()) {
		SweepEvent* se = eq.top ();
//std::cout << "sweep event " << se->toString() << std::endl;

		// optimization 2
		if ((operation == INTERSECTION && se->point.x > MINMAXX) ||
			(operation == DIFFERENCE && se->point.x > subjectBB.xmax ())) {
			connectEdges(sortedEvents, out);
			return;
		}

		sortedEvents.push_back(se);
		eq.pop ();
		
		if (se->left) { // the line segment must be inserted into sl
//std::cout << "line " << __LINE__ << std::endl;
			next = prev = se->posSL = it = sl.insert(se).first;
			(prev != sl.begin()) ? --prev : prev = sl.end();
			++next;
			computeFields(se, prev);
			// Process a possible intersection between "se" and its next neighbor in sl
			if (next != sl.end()) {
//std::cout << "line " << __LINE__ << std::endl;
				if (possibleIntersection(se, *next) == 2) {
//std::cout << "line " << __LINE__ << std::endl;

					computeFields(se, prev);
					computeFields(*next, it);
				}
			}
			// Process a possible intersection between "se" and its previous neighbor in sl
			if (prev != sl.end ()) {
//std::cout << "line " << __LINE__ << std::endl;
				if (possibleIntersection(*prev, se) == 2) {
//std::cout << "line " << __LINE__ << std::endl;
					std::set<SweepEvent*, SegmentComp>::iterator prevprev = prev;
					(prevprev != sl.begin()) ? --prevprev : prevprev = sl.end();
					computeFields(*prev, prevprev);
					computeFields(se, prev);
				}
			}
		} else { // the line segment must be removed from sl
//std::cout << "line " << __LINE__ << std::endl;
			se = se->otherEvent; // we work with the left event
			next = prev = it = se->posSL; // se->posSL; is equal than sl.find (se); but faster
			(prev != sl.begin()) ? --prev : prev = sl.end();
			++next;
			// delete line segment associated to "se" from sl and check for intersection between the neighbors of "se" in sl
			sl.erase(it);
			if (next != sl.end() && prev != sl.end())
				possibleIntersection(*prev, *next);
		}
	}
	
	connectEdges(sortedEvents, out);
}

bool BooleanOpImp::trivialOperation(const Polygon& subject, const Polygon& clipping, const Bbox_2& subjectBB, const Bbox_2& clippingBB)
{
#if 0
	// Test 1 for trivial result case
	if (subject.ncontours() * clipping.ncontours() == 0) { // At least one of the polygons is empty
		if (operation == DIFFERENCE)
			result = subject;
		if (operation == UNION || operation == XOR)
			result = (subject.ncontours () == 0) ? clipping : subject;
		return true;
	}
	// Test 2 for trivial result case
	if (subjectBB.xmin () > clippingBB.xmax () || clippingBB.xmin () > subjectBB.xmax () ||
	    subjectBB.ymin () > clippingBB.ymax () || clippingBB.ymin () > subjectBB.ymax ()) {
		// the bounding boxes do not overlap
		if (operation == DIFFERENCE)
			result = subject;
		if (operation == UNION || operation == XOR) {
			result = subject;
			result.join(clipping);
		}
		return true;
	}
#endif
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
/*	if (s.degenerate ()) // if the two edge endpoints are equal the segment is dicarded
		return;          // This can be done as preprocessing to avoid "polygons" with less than 3 edges */
	SweepEvent* e1 = storeSweepEvent(SweepEvent(true, p0, 0, pt));
 	e1->id = pt==SUBJECT?"A":"B";
 	e1->id += ".1";
	SweepEvent* e2 = storeSweepEvent(SweepEvent(true, p1, e1, pt));
 	e2->id = pt==SUBJECT?"A":"B";
 	e2->id += ".2";
	e1->otherEvent = e2;

	if (minlex(p0, p1) == p0) {
		e2->left = false;
	} else {
		e1->left = false;
	}
	eq.push(e1);
	eq.push(e2);
}

void BooleanOpImp::processCurve(const TPoint &p0, const TPoint* pn, PolygonType pt)
{
/*	if (s.degenerate ()) // if the two edge endpoints are equal the segment is dicarded
		return;          // This can be done as preprocessing to avoid "polygons" with less than 3 edges */
	SweepEvent* e1 = storeSweepEvent(SweepEvent(true, p0, 0, pt));
 	e1->id = pt==SUBJECT?"C":"D";
 	e1->id += ".1";
	SweepEvent* e2 = storeSweepEvent(SweepEvent(true, *(pn+2), e1, pt));
 	e2->id = pt==SUBJECT?"C":"D";
 	e2->id += ".2";
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


/**
 * Determine if the edge belongs to the result polygon, taking into account
 * its preceding event in sl (S)
 */
void BooleanOpImp::computeFields(SweepEvent* le, const std::set<SweepEvent*, SegmentComp>::iterator& prev)
{
//cout << "computeFields for " << le->toString() << endl;
	// compute inOut and otherInOut fields
	if (prev == sl.end ()) {
//cout << "  no previous: inOut=false, otherInOut=true" << endl;
		le->inOut = false;
		le->otherInOut = true;
	} else if (le->pol == (*prev)->pol) { // previous line segment in sl belongs to the same polygon that "se" belongs to
//cout << "  pol" << endl;
		le->inOut = ! (*prev)->inOut;
		le->otherInOut = (*prev)->otherInOut;
	} else {                          // previous line segment in sl belongs to a different polygon that "se" belongs to
//cout << "  not pol" << endl;
		le->inOut = ! (*prev)->otherInOut;
		le->otherInOut = (*prev)->vertical () ? ! (*prev)->inOut : (*prev)->inOut;
	}
	// compute prevInResult field
	if (prev != sl.end ())
		le->prevInResult = (!inResult (*prev) || (*prev)->vertical ()) ? (*prev)->prevInResult : *prev;
	// check if the line segment belongs to the Boolean operation('s result?)
	le->inResult = inResult(le);
//cout << "  -> inResult = " << le->inResult << endl;
}

bool BooleanOpImp::inResult(const SweepEvent* le) const
{
	switch (le->type) {
		case NORMAL:
			switch (operation) {
				case (INTERSECTION):
					return ! le->otherInOut;
				case (UNION):
					return le->otherInOut;
				case (DIFFERENCE):
					return (le->pol == SUBJECT && le->otherInOut) || (le->pol == CLIPPING && !le->otherInOut);
				case (XOR):
					return true;
			}
		case SAME_TRANSITION:
			return operation == INTERSECTION || operation == UNION;
		case DIFFERENT_TRANSITION:
			return operation == DIFFERENCE;
		case NON_CONTRIBUTING:
			return false;
	}
	return false; // just to avoid the compiler warning
}

int BooleanOpImp::possibleIntersection(SweepEvent* le1, SweepEvent* le2)
{
//std::cout << "possibleIntersection between " << std::endl
//          << "  " << le1->toString() << std::endl
//          << "  " << le2->toString() << std::endl;

//	if (e1->pol == e2->pol) // you can uncomment these two lines if self-intersecting polygons are not allowed
//		return 0;

	TPoint ip1, ip2;  // intersection points
	int nintersections;

	if (!(nintersections = findIntersection(le1, le2, ip1, ip2))) {
//	        std::cout << "  none" << std::endl;
		return 0;  // no intersection
        }

	if ((nintersections == 1) && ((le1->point == le2->point) || (le1->otherEvent->point == le2->otherEvent->point))) {
//	        std::cout << "  none" << std::endl;
		return 0; // the line segments intersect at an endpoint of both line segments
        }

	if (nintersections == 2 && le1->pol == le2->pol) {
		std::cerr << "Sorry, edges of the same polygon overlap\n";
		exit(1); // the line segments overlap, but they belong to the same polygon
	}

	// The line segments associated to le1 and le2 intersect
	if (nintersections == 1) {
	        std::cout << "  one intersection, divide" << std::endl;
		if (le1->point != ip1 && le1->otherEvent->point != ip1) { // if the intersection point is not an endpoint of le1->segment ()
		        cout << "    divide 1st event " << endl;
			divideSegment(le1, ip1);
                }
		if (le2->point != ip1 && le2->otherEvent->point != ip1) { // if the intersection point is not an endpoint of le2->segment ()
		        cout << "    divide 2nd event" << endl;
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

	if ((sortedEvents.size () == 2) || (sortedEvents.size () == 3 && sortedEvents[2])) { 
		// both line segments are equal or share the left endpoint
		le1->type = NON_CONTRIBUTING;
		le2->type = (le1->inOut == le2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
		if (sortedEvents.size () == 3)
			divideSegment (sortedEvents[2]->otherEvent, sortedEvents[1]->point);
		return 2;
	}
	if (sortedEvents.size () == 3) { // the line segments share the right endpoint
		divideSegment (sortedEvents[0], sortedEvents[1]->point);
		return 3;
	}
	if (sortedEvents[0] != sortedEvents[3]->otherEvent) { // no line segment includes totally the other one
		divideSegment (sortedEvents[0], sortedEvents[1]->point);
		divideSegment (sortedEvents[1], sortedEvents[2]->point);
		return 3;
	}
	// one line segment includes the other one
	divideSegment (sortedEvents[0], sortedEvents[1]->point);
	divideSegment (sortedEvents[3]->otherEvent, sortedEvents[2]->point);
	return 3;
}

void BooleanOpImp::divideSegment(SweepEvent* le, const TPoint& p)
{
cout << "divide " << le->id << " at " << p << endl;

//	std::cout << "YES. INTERSECTION" << std::endl;
	// "Right event" of the "left line segment" resulting from dividing le->segment ()
	SweepEvent* r = storeSweepEvent(SweepEvent(false, p, le, le->pol/*, le->type*/));
	r->id=le->id + ".right";
	// "Left event" of the "right line segment" resulting from dividing le->segment ()
	SweepEvent* l = storeSweepEvent(SweepEvent(true, p, le->otherEvent, le->pol/*, le->other->type*/));
	l->id=le->id + ".left";
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
	eq.push(l);
	eq.push(r);
}

void BooleanOpImp::connectEdges(const std::deque<SweepEvent*> &sortedEvents, toad::TVectorPath& out)
{
cout << endl << endl << "CONNECT EDGES ------------------------------------------------" << endl;
	// copy the events in the result polygon to resultEvents array
	std::vector<SweepEvent*> resultEvents;
	resultEvents.reserve (sortedEvents.size ());
	for (std::deque<SweepEvent*>::const_iterator it = sortedEvents.begin (); it != sortedEvents.end (); it++)
		if (((*it)->left && (*it)->inResult) || (!(*it)->left && (*it)->otherEvent->inResult))
			resultEvents.push_back (*it);

	// Due to overlapping edges the resultEvents array can be not wholly sorted
	bool sorted = false;
	while (!sorted) {
		sorted = true;
		for (unsigned int i = 0; i < resultEvents.size (); ++i) {
			if (i + 1 < resultEvents.size () && sec (resultEvents[i], resultEvents[i+1])) {
				std::swap (resultEvents[i], resultEvents[i+1]);
				sorted = false;
			}
		}
	}

	for (unsigned int i = 0; i < resultEvents.size (); ++i) {
		resultEvents[i]->pos = i;
		if (!resultEvents[i]->left)
			std::swap (resultEvents[i]->pos, resultEvents[i]->otherEvent->pos);
	}

	std::vector<bool> processed(resultEvents.size(), false);

	for (unsigned int i = 0; i < resultEvents.size (); i++) {
		if (processed[i])
			continue;
		int pos = i;
		TPoint initial = resultEvents[i]->point;
		out.move(initial);
		while (resultEvents[pos]->otherEvent->point != initial) {
			processed[pos] = true; 
			if (resultEvents[pos]->left) {
				resultEvents[pos]->resultInOut = false;
			} else {
				resultEvents[pos]->otherEvent->resultInOut = true; 
			}
			processed[pos = resultEvents[pos]->pos] = true;
                        if (resultEvents[pos]->curve) {
cout << "  CURVE " << resultEvents[pos]->id << endl;
                          out.curve(resultEvents[pos]->point1,
                                    resultEvents[pos]->point2,
                                    resultEvents[pos]->point);
                        } else {
cout << "  LINE  " << resultEvents[pos]->id << endl;
                          out.line(resultEvents[pos]->point);
                        }
			pos = nextPos(pos, resultEvents, processed);
		}
                out.close();
		processed[pos] = processed[resultEvents[pos]->pos] = true;
		resultEvents[pos]->otherEvent->resultInOut = true; 
	}
}

int BooleanOpImp::nextPos(int pos, const std::vector<SweepEvent*>& resultEvents, const std::vector<bool>& processed)
{
	unsigned int newPos = pos + 1;
	while (newPos < resultEvents.size () && resultEvents[newPos]->point == resultEvents[pos]->point) {
		if (!processed[newPos])
			return newPos;
		else
			++newPos;
	}
	newPos = pos - 1;
	while (processed[newPos])
		--newPos;
	return newPos;
}