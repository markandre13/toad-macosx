/***************************************************************************
 *   Developer: Francisco Martínez del Río (2012)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.0                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

// ------------------------------------------------------------------
// Bbox_2 Class : A bounding box
// ------------------------------------------------------------------

#ifndef BBOX_2_H
#define BBOX_2_H

#include "types.hh"

namespace toad {

class Bbox_2 {
public:
        Bbox_2 (const toad::TPoint &p): _xmin(p.x), _ymin(p.y), _xmax(p.x), _ymax(p.y) {}
	Bbox_2 (double x_min = 0, double y_min = 0, double x_max = 0, double y_max = 0) : 
	                                  _xmin (x_min), _ymin (y_min), _xmax (x_max), _ymax (y_max) {}
	double xmin () const { return _xmin; }
	double xmax () const { return _xmax; }
	double ymin () const { return _ymin; }
	double ymax () const { return _ymax; }
	Bbox_2 operator+ (const Bbox_2& b) const { return Bbox_2 (std::min (_xmin, b.xmin ()),
															  std::min (_ymin, b.ymin ()),
															  std::max (_xmax, b.xmax ()),
															  std::max (_ymax, b.ymax ())); }
private:
	double _xmin, _ymin, _xmax, _ymax;
};

} // end of namespace cbop
#endif
