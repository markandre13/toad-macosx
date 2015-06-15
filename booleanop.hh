/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

#ifndef BOOLEANOP_H
#define BOOLEANOP_H

#include <vector>
#include <list>
#include <string>
#include <set>
#include <queue>
#include <functional>
#include <iostream>
#include <toad/vector.hh>

#include "polygon.h"

namespace cbop {

enum BooleanOpType { INTERSECTION, UNION, DIFFERENCE, XOR };
enum EdgeType { NORMAL, NON_CONTRIBUTING, SAME_TRANSITION, DIFFERENT_TRANSITION };
enum PolygonType { SUBJECT, CLIPPING };

struct SweepEvent; // forward declaration
struct SegmentComp : public std::binary_function<SweepEvent*, SweepEvent*, bool> { // for sorting edges in the sweep line (sl)
	bool operator() (SweepEvent* le1, SweepEvent* le2);
};

struct SweepEvent {
	SweepEvent () {}
	SweepEvent (bool left, const TPoint& point, SweepEvent* otherEvent, PolygonType pt, EdgeType et = NORMAL);

	// data being set when the sweep event is created
        //------------------------------------------------
	bool left;              // is point the left endpoint of the edge (point, otherEvent->point)?
	TPoint point;           // point associated with the event
	SweepEvent* otherEvent; // event associated to the other endpoint of the edge
	PolygonType pol;        // Polygon to which the associated segment belongs to
	EdgeType type;

	//The following fields are only used in "left" events

        // data being set by computeFields()
        //------------------------------------------------
	/**
         * indicates if e determines an inside-outside transition into the polygon,
         * to which e belongs, for a vertical ray that starts below the polygon and
         * intersects e.
         */
        bool inOut;
        /**
         * has the same meaning as the previous flag, but referred to the
         * closest edge to e downwards in sl (S) that belongs to the other polygon
         */
        bool otherInOut;
        /**
         * indicates whether e belongs to the result polygon
         */
        bool inResult;
        /**
         * previous segment in sl belonging to the result of the boolean operation
         *
         * (pointer to the closest edge to e downwards in sl (S) that belongs to the
         * result polygon.  This field is used in the second stage of the
         * algorithm to compute child contours)
         */
        SweepEvent* prevInResult;
	
	/**  Does segment (point, otherEvent->p) represent an inside-outside transition in the polygon for a vertical ray from (p.x, -infinite)? */
	std::set<SweepEvent*, SegmentComp>::iterator posSL; // Position of the event (line segment) in sl
	unsigned int pos;
	bool resultInOut;
	
	// member functions
	/** Is the line segment (point, otherEvent->point) below point p */
	bool below (const TPoint& p) const { return (left) ? signedArea (point, otherEvent->point, p) > 0 : 
                                                          signedArea (otherEvent->point, point, p) > 0; }
	/** Is the line segment (point, otherEvent->point) above point p */
	bool above (const TPoint& p) const { return !below (p); }
	/** Is the line segment (point, otherEvent->point) a vertical line segment */
	bool vertical () const { return point.x == otherEvent->point.x; }
	/** Return the line segment associated to the SweepEvent */
	Segment_2 segment () const { return Segment_2(point, otherEvent->point); }
	std::string toString () const;
};

struct SweepEventComp : public std::binary_function<SweepEvent, SweepEvent, bool> { // for sorting sweep events
// Compare two sweep events
// Return true means that e1 is placed at the event queue after e2, i.e,, e1 is processed by the algorithm after e2
bool operator() (const SweepEvent* e1, const SweepEvent* e2)
{
	if (e1->point.x > e2->point.x) // Different x-coordinate
		return true;
	if (e2->point.x > e1->point.x) // Different x-coordinate
		return false;
	if (e1->point.y != e2->point.y) // Different points, but same x-coordinate. The event with lower y-coordinate is processed first
		return e1->point.y > e2->point.y;
	if (e1->left != e2->left) // Same point, but one is a left endpoint and the other a right endpoint. The right endpoint is processed first
		return e1->left;
	// Same point, both events are left endpoints or both are right endpoints.
	if (signedArea (e1->point, e1->otherEvent->point, e2->otherEvent->point) != 0) // not collinear
		return e1->above (e2->otherEvent->point); // the event associate to the bottom segment is processed first
	return e1->pol > e2->pol;
}
};

class BooleanOpImp
{
public:
	BooleanOpImp(BooleanOpType op);
	void run(const toad::TVectorPath& subj, const toad::TVectorPath& clip, toad::TVectorPath& result);

private:
	BooleanOpType operation;
	std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, SweepEventComp> eq; // event queue (sorted events to be processed)
	std::set<SweepEvent*, SegmentComp> sl; // segments intersecting the sweep line
	std::deque<SweepEvent> eventHolder;    // It holds the events generated during the computation of the boolean operation
	SweepEventComp sec;                    // to compare events

	bool trivialOperation(const Polygon& subject, const Polygon& clipping, const Bbox_2& subjectBB, const Bbox_2& clippingBB);
	void path2events(const toad::TVectorPath& poly, PolygonType type);
	/** @brief Compute the events associated to segment s, and insert them into pq and eq */
	void processSegment (const Segment_2& s, PolygonType pt);
	/** @brief Store the SweepEvent e into the event holder, returning the address of e */
	SweepEvent *storeSweepEvent (const SweepEvent& e) { eventHolder.push_back (e); return &eventHolder.back (); }
	/** @brief Process a posible intersection between the edges associated to the left events le1 and le2 */
	int possibleIntersection (SweepEvent* le1, SweepEvent* le2);
	/** @brief Divide the segment associated to left event le, updating pq and (implicitly) the status line */
	void divideSegment (SweepEvent* le, const TPoint& p);
	/** @brief return if the left event le belongs to the result of the Boolean operation */
	bool inResult(const SweepEvent* le) const;
	/** @brief compute several fields of left event le */
	void computeFields (SweepEvent* le, const std::set<SweepEvent*, SegmentComp>::iterator& prev);
	// connect the solution edges to build the result polygon
	void connectEdges(const std::deque<SweepEvent*> &sortedEvents, toad::TVectorPath& out);
	int nextPos (int pos, const std::vector<SweepEvent*>& resultEvents, const std::vector<bool>& processed);
};

inline void compute(const toad::TVectorPath& subj, const toad::TVectorPath& clip, toad::TVectorPath &result, BooleanOpType op)
{
	BooleanOpImp boi(op);
	boi.run(subj, clip, result);
}

} // end of namespace cbop
#endif
