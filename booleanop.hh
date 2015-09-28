#ifndef __TOAD_BOOLEANOP_HH
#define __TOAD_BOOLEANOP_HH 1

#include <toad/types.hh>
#include <set>

namespace toad {

extern bool booleanop_debug;
extern bool booleanop_gap_error;

enum EdgeType { NORMAL, NON_CONTRIBUTING, SAME_TRANSITION, DIFFERENT_TRANSITION };
enum PolygonType { SUBJECT, CLIPPING };

struct SweepEvent; // forward declaration

// sorting edges in the sweep line buffer 'sl' (generally by y coordinate)
struct SegmentComp: public std::binary_function<SweepEvent*, SweepEvent*, bool> {
  bool operator() (SweepEvent* le1, SweepEvent* le2);
};

// sorting sweep events in the sweep event buffer 'eq' (generally by x coordinate)
struct SweepEventComp: public std::binary_function<SweepEvent, SweepEvent, bool> {
  // return true on e1 > e2
  bool operator() (const SweepEvent* e1, const SweepEvent* e2);
};

struct SweepEvent {
  unsigned id; // debugging

  SweepEvent(bool left, const TPoint& point, SweepEvent* otherEvent, PolygonType pt, EdgeType et = NORMAL);

  // data being set when the sweep event is created
  //------------------------------------------------
  TPoint point;           // point associated with the event
	
  bool curve;
  TPoint point1;		// point on curve
  TPoint point2;
	
  SweepEvent* otherEvent; // event associated to the other endpoint of the edge
  PolygonType pol;        // Polygon to which the associated segment belongs to
  EdgeType type;
	
  bool left:1;             // is point the left endpoint of the edge (point, otherEvent->point)?
  //The following fields are only used in "left" events

  // data being set by computeFields()
  //------------------------------------------------
  bool inOut:1;      // false: we are entering the polygon 'pol'
  bool otherInOut:1; // false: we have entered the other polygon (!pol)
  bool inResult:1;   // this event will endup in the result
	
  std::set<SweepEvent*, SegmentComp>::iterator posSL; // Position of this sweep event (line segment) in sl
  size_t pos;
	
  // member functions
  /** Is the line segment (point, otherEvent->point) below point p */
  bool below(const TPoint& p) const { 
    return (left) ? signedArea (point, otherEvent->point, p) > 0 : 
                    signedArea (otherEvent->point, point, p) > 0;
  }
  /** Is the line segment (point, otherEvent->point) above point p */
  bool above (const TPoint& p) const { return !below (p); }
  /** Is the line segment (point, otherEvent->point) a vertical line segment */
  bool vertical () const { return point.x == otherEvent->point.x; }
  /** Return the line segment associated to the SweepEvent */
  std::string toString () const;
};

} // end namespace toad

#endif
