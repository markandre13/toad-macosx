/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

// ------------------------------------------------------------------
// Segment_2 Class - A line segment in the plane
// ------------------------------------------------------------------

#ifndef SEGMENT_2_H
#define SEGMENT_2_H

#include <algorithm>
#include "types.hh"

namespace cbop {

using toad::TPoint;

class Segment_2 {
public:
	/** Default constructor */
	Segment_2 () {}

	/** Constructor from two points **/
	Segment_2 (const TPoint& source, const TPoint& target) : s (source), t (target) {}

	/** Set the source point */
	void setSource (const TPoint& source) { s = source; }
	/** Set the target point */
	void setTarget (const TPoint& target) { t = target; }

	/** Get the source point */
	const TPoint& source () const { return s; }
	/** Get the target point */
	const TPoint& target () const { return t; }

	/** Return the point of the segment with lexicographically smallest coordinate */
	const TPoint& min () const { return (s.x < t.x) || (s.x == t.x && s.y < t.y) ? s : t; }
	/** Return the point of the segment with lexicographically largest coordinate */
	const TPoint& max () const { return (s.x > t.x) || (s.x == t.x && s.y > t.y) ? s : t; }
	bool degenerate () const { return s == t; }
	bool is_vertical () const { return s.x == t.x; }
	/** Change the segment orientation */
	Segment_2& changeOrientation () { std::swap (s, t); return *this; }
private:
	/** Segment endpoints */
	TPoint s, t;
};

inline std::ostream& operator<< (std::ostream& o, const Segment_2& p) {
	return o << p.source () << "-" << p.target ();
}

} // end of namespace cbop
#endif
