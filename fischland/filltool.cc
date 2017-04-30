/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2007 by Mark-André Hopf <mhopf@mark13.org>
 * Visit http://www.mark13.org/fischland/.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * still unhandled cases:
 * - bezier (4 point segment of path) intersects with itself
 * - more than 2 bezier intersect at the same point
 * - intersections with non-TFPath figures
 */

#ifndef DEBUG

#include <cmath>
#include <cfloat>
#include <algorithm>

#include "filltool.hh"

#ifdef DEBUG
#undef DEBUG
#endif

#else

#include <toad/toad.hh>
#include <toad/messagebox.hh>
#include <toad/figuremodel.hh>
#include <toad/figureeditor.hh>
#include "filltool.hh"
#include <cmath>
#include <cfloat>

#endif

using namespace toad;
using namespace fischland;

//#define DEBUG_PDF 1

#ifdef DEBUG_PDF
#include <sstream>
#endif
static TPenBase *pen = 0;

#define TEST07 1

static void
bez2point(const TFPath *path, TCoord u, TPoint *p)
{
  size_t i = 0;
  while(u>1.0) {
    u-=1.0;
    i+=3;
  }
  p->x = p->y = 0;
  for(unsigned j=0; j<4; ++j) {
    TCoord b = B(j, u);
    p->x += path->polygon[i+j].x * b;
    p->y += path->polygon[i+j].y * b;
  }
}

using namespace fischland;

TFillTool*
TFillTool::getTool()
{
  static TFillTool* tool = 0;
  if (!tool)
    tool = new TFillTool();
  return tool;
}

void
TFillTool::mouseEvent(TFigureEditor *fe, const TMouseEvent &me)
{
  TPoint pos;
  TFPath *path;
  
  switch(me.type) {
    case TMouseEvent::LDOWN:
      fe->mouse2sheet(me.pos, &pos);
      fe->getWindow()->setCursor(TCursor::WAIT);
//      toad::flush();
      path = floodfill(*fe->getModel(), pos.x, pos.y);
      fe->getWindow()->setCursor(TCursor::DEFAULT);
      if (path) {
        path->closed = true;
        fe->getAttributes()->setAllReasons();
        path->setAttributes(fe->getAttributes());
        fe->getAttributes()->clearReasons();
        fe->getModel()->add(path);
      }
      break;
    default:
      ;
  }
}


void
printPage(const TFigureModel &model)
{
  for(auto p = model.begin(); p != model.end(); ++p) {
    (*p)->paint(*pen);
  }
}

void
intersect_model_with_horizontal_line(const TFigureModel &model,
                                     TCoord y,
                                     IntersectionPointList *linelist)
{
  TFPath line;
  line.addPoint(INT_MIN, y);
  line.addPoint(INT_MIN, y);
  line.addPoint(INT_MAX, y);
  line.addPoint(INT_MAX, y);

  for(auto p = model.begin(); p != model.end(); ++p) {
    const TFPath *path = dynamic_cast<const TFPath*>(*p);
    if (!path)
      continue;
    // filling next to already filled areas failes sometimes, so we
    // do not do this until the real issue is fixed
    if (path->isFilled())
      continue;
    bezierIntersection(*linelist, &line, path);
  }
}

void
find_nearest_intersection_on_the_left(const IntersectionPointList &linelist,
                                      TCoord x,
                                      TCoord *nearx,
                                      TCoord *neara,
                                      const TFPath **nearpath)
{
  *nearx = -DBL_MAX;
  *neara = 0.0;
  *nearpath = 0;
  for(auto q = linelist.begin(); q != linelist.end(); ++q) {
    if (q->coord.x < x && q->coord.x > *nearx) {
      *nearx = q->coord.x;
      *neara = q->b;
      *nearpath = q->bp;
    }
  }
}

void
add_directions_overlapping_with(directionstack_t *direction,
                                const IntersectionPointList &list,
                                TCoord a,
                                const TFPath *minp,
                                TCoord minb)
{
  // add other directions from lines which do overlap with this intersection
  cout << "looking for other points intersecting with " << minp << " at " << minb << endl;
  for(auto q = list.begin(); q != list.end(); ++q) {
    // list contains duplicates, ignore 'em here for now...
    bool boo = false;
    for(auto r = direction->begin(); r != direction->end(); ++r) {
      if (q->bp == r->path && q->b == r->position) {
        boo = true;
      }
    }
    if (boo) continue;
/*
    if (q->ap == nearpath && q->a == mina) {
      cout << "got origin path, ignore" << endl;
      continue;
    }
*/
    if (q->b==minb && q->bp==minp) {
      cout << "got first intersecting path, ignore" << endl;
      continue;
    }
    if (!pointsOverlap(q->bp, q->b, minp, minb, 0.2))
      continue;
cout << "got " << q->bp << " at " << q->b << endl;
    double b = angleAtPoint(q->bp, q->b);
    double b0 = b + 2.0 * M_PI;
    double b1 = b + 3.0 * M_PI;
    while(b0>a)
      b0 -= 2.0 * M_PI;
    while(b1>a)
      b1 -= 2.0 * M_PI;
    direction->push_back(direction_t(b0, q->bp, q->b, true));  // forward
    direction->push_back(direction_t(b1, q->bp, q->b, false)); // backward
  }
}

void
debug_pdf_print_page_one(TPenBase *pen,
                         TCoord x, TCoord y,
                         const TFigureModel &model)
{
  if (!pen)
    return;
  pen->drawString(100,100, "Page 1");

  pen->scale(1.0/(96.0), 1.0/(96.0));
  pen->setFont("arial,helvetica,sans-serif:size=1152");
  
  printPage(model);

  pen->setLineWidth(96.0);
  pen->setColor(0,0.5,0);
  pen->drawLine(x-200, y-200, x+200, y+200);
  pen->drawLine(x+200, y-200, x-200, y+200);

  pen->showPage();
}

void
debug_pdf_print_directions(TPenBase *pen,
                           TCoord x, TCoord y,
                           const TFigureModel &model,
                           const directionstack_t &direction)
{
  if (!pen)
    return;
  printPage(model);

  pen->setLineWidth(96.0);
  pen->setColor(0,0.5,0);
  pen->drawLine(x-200, y-200, x+200, y+200);
  pen->drawLine(x+200, y-200, x-200, y+200);

  pen->setColor(0,0.5,0);
  for(auto p = direction.begin(); p != direction.end(); ++p) {
    TPoint pt;
    bez2point(p->path, p->position, &pt);
    pen->drawLine(
      pt.x, pt.y,
      pt.x + cos(p->degree) * 800, pt.y + sin(p->degree) * 800);
  }
  pen->setColor(0,0,0);
  ostringstream txt;
  txt << "found " << direction.size() << " directions";
  pen->drawString(10, 10, txt.str());
  pen->showPage();
}

void
debug_pdf_print_taking_direction(TPenBase *pen,
                                 TCoord x, TCoord y,
                                 const TFigureModel &model,
                                 const directionstack_t &direction,
                                 directionstack_t::const_iterator dp)
{
  if (!pen)
    return;
  const TFPath *nearpath = dp->path;
  TCoord neara = dp->position;

  printPage(model);
  pen->setLineWidth(96.0);
  pen->setColor(0,0.5,0);
  pen->drawLine(x-200, y-200, x+200, y+200);
  pen->drawLine(x+200, y-200, x-200, y+200);
  pen->setColor(0,0.5,0);
  TPoint pt;
  for(auto p = direction.begin(); p != direction.end(); ++p) {
    if (!p->path)
      continue;
    bez2point(p->path, p->position, &pt);
    pen->drawLine(
      pt.x, pt.y,
      pt.x + cos(p->degree) * 800, pt.y + sin(p->degree) * 800);
  }
  pen->setColor(1,0.5,0);
  bez2point(nearpath, neara, &pt);
  pen->drawLine(
    pt.x, pt.y,
    pt.x + cos(dp->degree) * 800, pt.y + sin(dp->degree) * 800);
  pen->setColor(0,0,0);
  pen->drawString(10, 10, "taking direction");
  pen->showPage();
}

void
debug_pdf_mark_intersections(TPenBase *pen,
                             TCoord x, TCoord y,
                             const IntersectionPointList &list,
                             const segmentstack_t &segmentstack,
                             const TFPath *nearpath,
                             TCoord neara,
                             const TFPath *minp,
                             TCoord mina,
                             TCoord minb,
                             bool backtrace)
{
  if (!pen)
    return;
  pen->setLineWidth(96.0);

  // mark start position
  pen->setColor(0,0.5,0);
  pen->drawLine(x-200, y-200, x+200, y+200);
  pen->drawLine(x+200, y-200, x-200, y+200);

  TFPath *path = segmentstack2path(segmentstack);
  if (path) {
    pen->setColor(0,1,0);
    path->paint(*pen, TFigure::NORMAL);
    delete path;
  }

  // mark found intersections
  pen->setColor(1,0,0);
  TPoint pt;
  for(auto q = list.begin(); q != list.end(); ++q) {
    if (nearpath==q->ap) {
      pen->setColor(1,0,0);
      bez2point(q->ap, q->a, &pt);
      pen->drawLine(pt.x-300,pt.y-300,pt.x+300,pt.y+300);
      pen->drawLine(pt.x+300,pt.y-300,pt.x-300,pt.y+300);
    }
    if (nearpath==q->bp) {
      pen->setColor(0,0,1);
      bez2point(q->bp, q->b, &pt);
      pen->drawLine(pt.x-300,pt.y-300,pt.x+300,pt.y+300);
      pen->drawLine(pt.x+300,pt.y-300,pt.x-300,pt.y+300);
    }
  }
  if (minp) {
    pen->setColor(1,0.5,0);
    bez2point(minp, minb, &pt);
    pen->drawLine(pt.x-300,pt.y-300,pt.x+300,pt.y+300);
    pen->drawLine(pt.x+300,pt.y-300,pt.x-300,pt.y+300);
  }

  pen->setColor(0,0,0);
  if (!backtrace) {
    pen->drawString(10, 10, "collected intersections after taking new direction");
  } else {
    pen->drawString(10, 10, "collected intersections after backtrace");
  }
  if (!minp) {
    pen->drawString(10, 10 + 14 * 96, "no_minp_found");
  } else {
    bez2point(nearpath, neara, &pt);
    pen->drawCircle(pt.x-300, pt.y-300, 600, 600);
  }
  pen->drawString(10, 10 + 2*14 * 96, "circle: indicates current position");
  pen->drawString(10, 10 + 3*14 * 96, "orange cross: indicates nearest_point_found");
  pen->drawString(10, 10 + 4*14 * 96, "blue crosses: indicate wrong order of intersection entry");
  pen->showPage();
}


TFPath*
fischland::floodfill(TFigureModel &model, TCoord x, TCoord y)
{
#ifdef DEBUG_PDF
  TCairo cairo("test.pdf");
  ::pen = &cairo;
  debug_pdf_print_page_one(pen, x, y, model);
#endif

  // 1. find nearest figure left of x on horizontal line y
  //    in (nearpath, nearx, neara)

  IntersectionPointList linelist;
  intersect_model_with_horizontal_line(model, y, &linelist);

  const TFPath *nearpath; // leftmost figure
  TCoord nearx;		  // intersection position on horizontal line
  TCoord neara;		  // intersection position on path 'nearpath'
  find_nearest_intersection_on_the_left(linelist, x, &nearx, &neara, &nearpath);

  if (!nearpath) {
    cout << "no intersection with line" << endl;
    return 0;
  }

  // 2. find all figures which intersect with 'nearpath'
  //    and whose intersections are near to 'neara' on nearpath

cout << "left of point is path " << nearpath << " at " << neara << endl;
  const TFPath* minp = nearpath;
  TCoord minb = neara;

  TCoord mina = -M_PI;
  TCoord a = 0.0;
  nearpath = 0;

  // segmentstack keeps track of found segments which will enclose the
  // fill path
  segmentstack_t segmentstack;
  segmentstack.push_back(segment_t());
  segmentstack.back().path = nearpath;
  segmentstack.back().u0 = neara;

  // find segment of minp whose degree is nearest to 'a', clockwise
  // from minp to 'a'
  double b = angleAtPoint(minp, minb);
  double b0 = b + 2.0 * M_PI; // +360°
  double b1 = b + 3.0 * M_PI; // +360°+180°
  while(b0>a)
    b0 -= 2.0 * M_PI;
  while(b1>a)
    b1 -= 2.0 * M_PI;

  // we might require the segment before the current back()...
  directionstack_t direction;
  direction.push_back(direction_t(b0,     minp,     minb, true));  // forward
  direction.push_back(direction_t(b1,     minp,     minb, false)); // backward

  IntersectionPointList list(linelist);
  bool oldforward = true;
  for(int loop=0; loop<100; ++loop) {

    cout << "--------------------------- " << loop << " ---------------------------" << endl;

    add_directions_overlapping_with(&direction, list, a, minp, minb);

    debug_pdf_print_directions(pen, x, y, model, direction);

    // outer loop:
    //   1st: inner loop: find a direction or backtrace if there's none
    //   2nd: find all new intersections
    while(true) {
      bool backtrace = false;
      bool forward = true;

      // make a decision which direction to take
cout << "  taking a new direction" << endl;
      while(true) {
        // all direction values are below 'a', now find the largest one
        TCoord dm = -DBL_MAX;
        directionstack_t::iterator dp = direction.end();
        for(auto p = direction.begin(); p != direction.end(); ++p) {
          if (!p->path)
            continue;
cout << "    offered: degree=" << p->degree << ", path=" << p->path << endl;
          if (dm < p->degree) {
            dm = p->degree;
            dp = p;
          }
        }
        
        // when we have a new direction, take it
        if (dp!=direction.end()) {
cout << "    taking : degree=" << dp->degree << ", path=" << dp->path << endl;
          nearpath = dp->path;
          dp->path = 0; // mark this direction as used
          neara    = dp->position;
          forward  = dp->forward;

          debug_pdf_print_taking_direction(pen, x, y, model, direction, dp);

          break;
        }
        
        // all directions were dead ends, backtrace
        if (segmentstack.size()<=1/*empty()*/) {
          cout << "  no minp and can not backtrace (because segmentstack is empty)" << endl;
          return 0;
        }
        cout << "  backtrace, line " << nearpath << " is a dead end" << endl;
        segmentstack.pop_back();
        backtrace = true;
        break;
      } // end of finding a new direction

      oldforward = forward;
      // nearpath:=path to take, neara:=position on nearpath, direction:=...

      // find all beziers intersecting with 'nearpath'
cout << "find all beziers intersecting with " << nearpath << endl;
      list.clear();
      for (auto p = model.begin(); p != model.end(); ++p) {
        const TFPath *path = dynamic_cast<const TFPath*>(*p);
        if (!path)
          continue;
        // filling next to already filled areas failes sometimes, so we
        // do not do this until the real issue is fixed
        if (path->isFilled())
          continue;
        if (*p == nearpath) continue; // TEMPORARY AS bezierIntersection MIGHT FAIL HERE FOR NOW
//cout << "  intersect " << nearpath << " with " << path << endl;
        bezierIntersection(list, nearpath, path);
      }
cout << "got " << list.size() << " intersections" << endl;

      // store the intersection being closest to 'nearpath' at 'neara' in min*
      cout<<"find nearest intersections out of " << list.size()<<" with "<<nearpath<<", neara="<<neara<<", forward="<<forward<<endl;
      if (forward)
        mina = minb = DBL_MAX;
      else
        mina = minb = -DBL_MAX;
      minp = 0;
#ifdef DEBUG_PDF
double xp = 32000, yp = 10, ys=14*96;
{
printPage(model);
ostringstream txt;
txt<<"forward="<<(forward?"true":"false")<<",_neara="<<neara;
pen->drawString(xp, yp, txt.str()); yp+=ys;
}
#endif

      for(auto q = list.begin(); q != list.end(); ++q) {
        if (pointsOverlap(nearpath, q->a, nearpath, neara, 1)) {
         cout << "    overlaps, ignore" << endl;
         continue;
        }
#ifdef DEBUG_PDF
{
ostringstream txt;
txt<<"q->a="<<q->a<<",_neara="<<neara<<",_mina="<<mina;
pen->drawString(xp, yp, txt.str()); yp+=ys;
}
#endif
        if (forward) {
          if (q->a >= neara) {
            if (q->a < mina) {
              if (!backtrace || q->a > segmentstack.back().mina) {
                mina = q->a;
                minb = q->b;
                minp = q->bp;
              }
            }
          }
        } else {
          if (q->a <= neara) {
            if (q->a > mina) {
              if (!backtrace || q->a < segmentstack.back().mina) {
                mina = q->a;
                minb = q->b;
                minp = q->bp;
              }
            }
          }
        }
      }

      debug_pdf_mark_intersections(pen, x, y, list, segmentstack, nearpath, neara, minp, mina, minb, backtrace);
    
      if (minp) {
        cout << "found minp" << endl;
        break;
      }
      cout << "no minp found, try again" << endl;
// ??? what about the directionstack dude ???
#warning "USING INVALID DIRECTIONSTACK AFTER BACKTRACE"
    }

    segmentstack.back().mina = mina;
    segmentstack.back().neara = neara;
    segmentstack.back().nearpath = nearpath;

    if (loop>0) {
      segmentstack.back().path = nearpath;
      segmentstack.back().u0 = neara;
      segmentstack.back().u1 = mina; // end of last segment
      // check for loop in already traversed path
      if (segmentstack.size()>1) {
        for(size_t i=1; i<segmentstack.size()-1; ++i) {
          if (pointsOverlap(segmentstack[i].path, segmentstack[i].u0,
                            segmentstack.back().path, segmentstack.back().u1))
          {
            cout << "found end after " << loop << " iterations" << endl;
/*
            cout << "started at  " << segmentstack[1].path << ", " << segmentstack[1].u0 << endl
                 << "finished at " << segmentstack.back().path << ", " << segmentstack.back().u1 << endl;
*/
            // when the end is not the first point, the start point must be outside
            // the area we have found
            if (i>1) {
              cout << "end is not first point, it is " << i << endl;
              segmentstack.erase(segmentstack.begin(), segmentstack.begin()+i-1);
            }
            if (insidepath(x, linelist, segmentstack))
              return segmentstack2path(segmentstack);
            cout << "not inside path" << endl;
            return 0;
          }
        }
      }
    }

    // divide bez0
    //divideBezier(bez0, split0, mina);
    TPoint split0[7];
    divideBezier(nearpath, split0, mina);
  
    cout << "continue with path " << minp << ", mina=" << mina << ", minb=" << minb << endl;

    cout << "neara = " << neara << ", mina=" << mina << endl;
    a = 0.0;
    if (mina>neara) {
      a = atan2(split0[2].y-split0[3].y, split0[2].x-split0[3].x);
    } else {
      // ∡ between split0[4]→split0[3] and split1[3]→spli1[2]
      //                               and split1[3]→spli1[4]
      a = atan2(split0[4].y-split0[3].y, split0[4].x-split0[3].x);
    }

cout << "  path: " << minp << ", mina=" << mina <<", minb=" << minb << endl;
    segmentstack.push_back(segment_t());
    segmentstack.back().path = minp;
    segmentstack.back().u0 = minb;

    direction.clear();
    direction.push_back(direction_t(a-M_PI, nearpath, mina, oldforward));

    // find segment of split1 whose degree is nearest to 'a', clockwise
    // from split1 to 'a'
#if 0
    pen->setColor(255,0,0);
    pen->drawLine(split1[3].x,
                  split1[3].y,
                  split1[3].x + r * cos(a),
                  split1[3].y + r * sin(a));
    pen->setColor(0,255,0);
#endif
//    cout << "at " << split1[3].x << ", " << split1[3].y << endl;
    double b = angleAtPoint(minp, minb);
    double b0 = b + 2.0 * M_PI;
    double b1 = b + 3.0 * M_PI;
    while(b0>a)
      b0 -= 2.0 * M_PI;
    while(b1>a)
      b1 -= 2.0 * M_PI;

    // we might require the segment before the current back()...
    cout << "collecting possible directions" << endl;
    direction.push_back(direction_t(b0, minp, minb, true));  // forward
    direction.push_back(direction_t(b1, minp, minb, false)); // backward
    
  }
cout << "floodfill exceeded loop maximum" << endl;
  return 0;
}
