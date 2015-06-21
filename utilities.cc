/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

#include <algorithm>
#include "utilities.h"
#include "booleanop.hh"

using namespace cbop;

static int findIntersection (double u0, double u1, double v0, double v1, double w[2])
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

int cbop::findIntersection(const SweepEvent* e0, const SweepEvent* e1, TPoint& pi0, TPoint& pi1)
{
  // point, otherEvent->point
	TPoint p0 = e0->point;
	TPoint d0 (e0->otherEvent->point.x - p0.x, e0->otherEvent->point.y - p0.y);
	TPoint p1 = e1->point;
	TPoint d1 (e1->otherEvent->point.x - p1.x, e1->otherEvent->point.y - p1.y);
	double sqrEpsilon = 0.0000001; // it was 0.001 before
	TPoint E (p1.x - p0.x, p1.y - p0.y);
	double kross = d0.x * d1.y - d0.y * d1.x;
	double sqrKross = kross * kross;
	double sqrLen0 = d0.x * d0.x + d0.y * d0.y;
	double sqrLen1 = d1.x * d1.x + d1.y * d1.y;

	if (sqrKross > sqrEpsilon * sqrLen0 * sqrLen1) {
		// lines of the segments are not parallel
		double s = (E.x * d1.y - E.y * d1.x) / kross;
		if ((s < 0) || (s > 1)) {
			return 0;
		}
		double t = (E.x * d0.y - E.y * d0.x) / kross;
		if ((t < 0) || (t > 1)) {
			return 0;
		}
		// intersection of lines is a point an each segment
		pi0 = TPoint (p0.x + s * d0.x, p0.y + s * d0.y);
		if (distance(pi0, e0->point) < 0.00000001)             pi0 = e0->point;
		if (distance(pi0, e0->otherEvent->point) < 0.00000001) pi0 = e0->otherEvent->point;
		if (distance(pi0, e1->point) < 0.00000001)             pi0 = e1->point;
		if (distance(pi0, e1->otherEvent->point) < 0.00000001) pi0 = e1->otherEvent->point;
		return 1;
	}

	// lines of the segments are parallel
	double sqrLenE = E.x * E.x + E.y * E.y;
	kross = E.x * d0.y - E.y * d0.x;
	sqrKross = kross * kross;
	if (sqrKross > sqrEpsilon * sqrLen0 * sqrLenE) {
		// lines of the segment are different
		return 0;
	}

	// Lines of the segments are the same. Need to test for overlap of segments.
	double s0 = (d0.x * E.x + d0.y * E.y) / sqrLen0;  // so = Dot (D0, E) * sqrLen0
	double s1 = s0 + (d0.x * d1.x + d0.y * d1.y) / sqrLen0;  // s1 = s0 + Dot (D0, D1) * sqrLen0
	double smin = std::min (s0, s1);
	double smax = std::max (s0, s1);
	double w[2];
	int imax = ::findIntersection (0.0, 1.0, smin, smax, w);

	if (imax > 0) {
		pi0 = TPoint (p0.x + w[0] * d0.x, p0.y + w[0] * d0.y);
		if (distance(pi0, e0->point) < 0.00000001)             pi0 = e0->point;
		if (distance(pi0, e0->otherEvent->point) < 0.00000001) pi0 = e0->otherEvent->point;
		if (distance(pi0, e1->point) < 0.00000001)             pi0 = e1->point;
		if (distance(pi0, e1->otherEvent->point) < 0.00000001) pi0 = e1->otherEvent->point;
		if (imax > 1)
			pi1 = TPoint (p0.x + w[1] * d0.x, p0.y + w[1] * d0.y);
	}
	return imax;
}
