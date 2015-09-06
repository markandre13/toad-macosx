/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

// Utility functions

#ifndef UTILITIES_H
#define UTILITIES_H

#include <algorithm>
#include "types.hh"

namespace cbop {

using toad::TPoint;

struct SweepEvent; // forward declaration

// int findIntersection (const Segment_2& seg0, const Segment_2& seg1, TPoint& ip0, TPoint& ip1);
// int findIntersection(const SweepEvent* e1, const SweepEvent* e2, TPoint& ip0, TPoint& ip1);
// point, otherEvent->point

/** Signed area of the triangle (p0, p1, p2) */
inline float signedArea (const TPoint& p0, const TPoint& p1, const TPoint& p2)
{ 
	return (p0.x- p2.x)*(p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
}

/** Signed area of the triangle ( (0,0), p1, p2) */
inline float signedArea (const TPoint& p1, const TPoint& p2)
{ 
	return -p2.x*(p1.y - p2.y) - -p2.y*(p1.x - p2.x);
}

/** Sign of triangle (p1, p2, o) */
inline int sign (const TPoint& p1, const TPoint& p2, const TPoint& o)
{
	float det = (p1.x - o.x) * (p2.y - o.y) - (p2.x - o.x) * (p1.y - o.y);
	return (det < 0 ? -1 : (det > 0 ? +1 : 0));
}

#if 0
inline bool pointInTriangle (const Segment_2& s, const TPoint& o, const TPoint& p)
{
	int x = sign (s.source (), s.target (), p);
	return ((x == sign (s.target (), o, p)) && (x == sign (o, s.source (), p)));
}
#endif

} // end of namespace cbop
#endif
