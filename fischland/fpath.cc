#include "fpath.hh"
#include <toad/pen.hh>
#include <toad/action.hh>
#include <toad/popupmenu.hh>

#include <cmath>
#include "fischland.hh"

void
TFPath::getShape(toad::TRectangle *r)
{
  polygon.getShape(r);
}

void
TFPath::translate(int dx, int dy)
{
  TPolygon::iterator p(polygon.begin()), e(polygon.end());
  while(p!=e) {
    p->x+=dx;
    p->y+=dy;
    ++p;
  }
}

bool 
TFPath::getHandle(unsigned handle, TPoint *p)
{
  if (handle >= polygon.size())
    return false;
  *p = polygon[handle];
  return true;
}

void
TFPath::translateHandle(unsigned handle, int x, int y, unsigned m)
{
  // 0 0  1 1 1  2 2 2  3 3 3
  // 0 1  2 3 4  5 6 7  8 9 10
//  cout << "handle: " << handle << endl;
  
//  cout << "corner: " << (handle+1)/3 << endl;
//  cout << "no    : " << (handle+1)%3 << endl;
  unsigned c = 3;
  unsigned i = (handle+1)/3;
  if (i<corner.size())
    c = corner[i];
//  cout << "i=" << i << ", c=" << c << endl;
  switch( (handle+1)%3 ) {
    case 0:
      if ( c == 4) {
        polygon[handle].x = x;
        polygon[handle].y = y;
        if (handle+2 <= polygon.size()) {
          polygon[handle+2].x = polygon[handle+1].x + (polygon[handle+1].x - polygon[handle].x);
          polygon[handle+2].y = polygon[handle+1].y + (polygon[handle+1].y - polygon[handle].y);
        }
      } else
      if ( c & 1 ) {
        polygon[handle].x = x;
        polygon[handle].y = y;
      } else {
        if (handle+2 < polygon.size()) {
          if ((c & 2) == 0) {
            polygon[handle+2].x = x;
            polygon[handle+2].y = y;
          } else {
            polygon[handle+2].x += x - polygon[handle].x;
            polygon[handle+2].y += y - polygon[handle].y;
          }
        }
        polygon[handle].x = x;
        polygon[handle].y = y;
        if (handle+1 < polygon.size()) {
          polygon[handle+1].x = x;
          polygon[handle+1].y = y;
        }
      }
      break;
    case 1: {
        int dx = x - polygon[handle].x;
        int dy = y - polygon[handle].y;
        if (handle>0) {
          polygon[handle-1].x += dx;
          polygon[handle-1].y += dy;
        } else 
        if (closed) {
          polygon.back().x = x;
          polygon.back().y = y;
          polygon[polygon.size()-2].x += dx;
          polygon[polygon.size()-2].y += dy;
        }
        polygon[handle].x = x;
        polygon[handle].y = y;
        if (handle+1<polygon.size()) {
          polygon[handle+1].x += dx;
          polygon[handle+1].y += dy;
        }
      } break;
    case 2:
      if ( c == 4) {
        polygon[handle].x = x;
        polygon[handle].y = y;
        if (handle>1) {
          polygon[handle-2].x = polygon[handle-1].x + (polygon[handle-1].x - polygon[handle].x);
          polygon[handle-2].y = polygon[handle-1].y + (polygon[handle-1].y - polygon[handle].y);
        }
      } else
      if ( c & 2 ) {
        polygon[handle].x = x;
        polygon[handle].y = y;
      } else {
        if (handle>1) {
          if ((c & 1) == 0) {
            polygon[handle-2].x = x;
            polygon[handle-2].y = y;
          } else {
            polygon[handle-2].x += polygon[handle].x - x;
            polygon[handle-2].y += polygon[handle].y - y;
          }
        }
        polygon[handle].x = x;
        polygon[handle].y = y;
        if (handle>0) {
          polygon[handle-1].x = x;
          polygon[handle-1].y = y;
        }
      }
      break;
  }
  
  TPoint p(x, y);
  polygon[handle]=p;
}

void
TFPath::paintSelection(TPenBase &pen, int handle)
{
}


void
TFPath::paint(TPenBase &pen, EPaintType type)
{
  pen.setAlpha(alpha);
  pen.setColor(line_color);
  pen.setLineStyle(line_style);
  pen.setLineWidth(line_width);

  if (!cmat) {
    if (!closed || !filled) {  
      pen.drawPolyBezier(polygon);
    } else {
      pen.setFillColor(fill_color);
      pen.fillPolyBezier(polygon);
    }
  } else {
    TPoint polygon2[polygon.size()];
    TPoint *p2 = polygon2;
    for(TPolygon::const_iterator p = polygon.begin();
        p != polygon.end();
        ++p, ++p2)
    {
      cmat->map(p->x, p->y, &p2->x, &p2->y);
    }
    if (!closed || !filled) {  
      pen.drawPolyBezier(polygon2, polygon.size());
    } else {
      pen.setFillColor(fill_color);
      pen.fillPolyBezier(polygon2, polygon.size());
    }
  }

  if (type!=EDIT && type!=SELECT)
    return;

  pen.setLineWidth(1);

  TMatrix2D _m0;
  const TMatrix2D *m0 = pen.getMatrix();
  if (m0 || cmat) {
    pen.push();
    pen.identity();
    if (cmat) {
      if (!m0) {
        m0 = cmat;
      } else {
        _m0 = *m0;
        _m0 *= *cmat;
        m0 = &_m0;
      }
    }
  }
  
  pen.setLineColor(TColor::FIGURE_SELECTION);
  pen.setFillColor(TColor::WHITE);

  if (type==EDIT || type==SELECT) {
    for(TPolygon::size_type i=0; i<polygon.size(); i+=3) {
      // line before corner
      int x0, y0, x1, y1;
      if (i>0) {
        x0 = polygon[i].x;
        y0 = polygon[i].y;
        x1 = polygon[i-1].x;
        y1 = polygon[i-1].y;
        if (m0) {
          m0->map(x0, y0, &x0, &y0);
          m0->map(x1, y1, &x1, &y1);
        }
        pen.drawLine(x0, y0, x1, y1);
        pen.fillCirclePC(x1-2,y1-2,6,6);
      }
/*
 else
      if (!closed && i+1<polygon.size()) {
        x0 = polygon[i].x;
        y0 = polygon[i].y;
        x1 = polygon[i].x - (polygon[i+1].x - polygon[i].x);
        y1 = polygon[i].y - (polygon[i+1].y - polygon[i].y);
        if (m0) {
          m0->map(x0, y0, &x0, &y0);
          m0->map(x1, y1, &x1, &y1);
        }
        pen.drawLine(x0, y0, x1, y1);
        pen.fillCirclePC(x1-2,y1-2,6,6);
      }
*/    
      // line after corner
      if (i+1<polygon.size()) {
        x0 = polygon[i].x;
        y0 = polygon[i].y;
        x1 = polygon[i+1].x;
        y1 = polygon[i+1].y;
        if (m0) {
          m0->map(x0, y0, &x0, &y0);
          m0->map(x1, y1, &x1, &y1);
        }
        pen.drawLine(x0, y0, x1, y1);
        pen.fillCirclePC(x1-2,y1-2,6,6);
      }
#if 0
 else
      if (!closed && i>0) {
        x0 = polygon[i].x;
        y0 = polygon[i].y;
        x1 = polygon[i].x - (polygon[i-1].x - polygon[i].x);
        y1 = polygon[i].y - (polygon[i-1].y - polygon[i].y);
        if (m0) {
          m0->map(x0, y0, &x0, &y0);
          m0->map(x1, y1, &x1, &y1);
        }
        pen.drawLine(x0, y0, x1, y1);
        pen.fillCirclePC(x1-2,y1-2,6,6);
      }
#endif
    }
    for(TPolygon::size_type i=0; i<polygon.size(); i+=3) {
      int x, y;
      if (m0) {
        m0->map(polygon[i].x, polygon[i].y, &x, &y);
      } else {
        x = polygon[i].x;
        y = polygon[i].y;
      }
      pen.fillRectanglePC(x-2,y-2,5,5);
    }
  }
  
  if (m0) {
    pen.pop();
  }
}

double
TFPath::_distance(TFigureEditor *fe, int x, int y)
{
  if (!polygon.isInside(x, y)) {
    int x1,y1,x2,y2;
    double min = OUT_OF_RANGE;
    TPolygon::const_iterator p(polygon.begin());
    x2=p->x;
    y2=p->y;
    ++p;
    while(p!=polygon.end()) {
      x1=x2;
      y1=y2;
      x2=p->x;
      y2=p->y;
      double d = distance2Line(x,y, x1,y1, x2,y2);
      if (d<min)
        min = d;
      ++p;
    }
    if (min > 0.5*fe->fuzziness*TFigure::RANGE)
      return OUT_OF_RANGE;
  }
   
  TPolygon p2;
  TPenBase::poly2Bezier(polygon, p2);
  if (closed && filled) {
    if (p2.isInside(x, y))
      return INSIDE;
  }
  
  TPolygon::const_iterator p(p2.begin()), e(p2.end());
  int x1,y1,x2,y2;
  double min = OUT_OF_RANGE, d;
  assert(p!=e);
  x2=p->x;
  y2=p->y;
  ++p;
  assert(p!=e);
  while(p!=e) {
    x1=x2;
    y1=y2;
    x2=p->x;
    y2=p->y;
    d = distance2Line(x,y, x1,y1, x2,y2);
    if (d<min)
      min = d;
    ++p;
  }
  return min;
}

namespace {
class TMyPopupMenu:
  public TPopupMenu
{
  public:
    TMyPopupMenu(TWindow *p, const string &t): TPopupMenu(p, t)
    {
//cerr << "create menu " << this << endl;
    }
    ~TMyPopupMenu() {
//cerr << "delete tree " << tree << endl;
      delete tree;
    }
 
    void closeRequest() {  
      TPopupMenu::closeRequest();
//cerr << "delete menu " << this << endl;
      delete this;
    }

    TInteractor *tree;
};
}

unsigned
TFPath::mouseRDown(TFigureEditor *editor, int x, int y, unsigned modifier)
{
//  cerr << "TFBezierline::mouseRDown" << endl;
//cerr << " at (" << x << ", " << y << ")\n";
//cerr << " 1s point at (" << polygon[0].x << ", " << polygon[0].y << ")\n";

//cerr << "editor->fuzziness = " << editor->fuzziness << endl;
  unsigned i=0;
  bool found=false;
  for(TPolygon::iterator p=polygon.begin();
      p!=polygon.end();
      ++p, ++i)
  {
    if (p->x-editor->fuzziness<=x && x<=p->x+editor->fuzziness &&
        p->y-editor->fuzziness<=y && y<=p->y+editor->fuzziness)
    {
//      cerr << "found handle " << i << endl;
      found = true;
      break;
    }
  }
  
  if (found && (i%3)!=0)
    return NOTHING;

  TInteractor *dummy = new TInteractor(0, "dummy interactor");
//cerr << "create tree " << dummy << endl;
  TAction *action;
  if (!found) {   
    action = new TAction(dummy, "add point", TAction::ALWAYS);
    TCLOSURE4(
      action->sigClicked,
      figure, this,
      edit, editor,
      _x, x,
      _y, y,
      edit->invalidateFigure(figure);
      figure->insertPointNear(_x, _y);
      edit->invalidateFigure(figure); 
    )
    action = new TAction(dummy, "split");
  } else {
    action = new TAction(dummy, "delete point", TAction::ALWAYS);
    TCLOSURE3(
      action->sigClicked,
      figure, this,
      edit, editor,
      _i, i,
      if (figure->polygon.size()<=4) {
        edit->deleteFigure(figure);   
      } else {
        edit->invalidateFigure(figure);
        figure->deletePoint(_i);
      }
      edit->invalidateFigure(figure);
    )
    action = new TAction(dummy, "split");
  }
  // action = new TAction(dummy, "sharp edge");
  // action = new TAction(dummy, "no edge");   
  TMyPopupMenu *menu;
  menu = new TMyPopupMenu(editor, "popup");
  menu->tree = dummy;
  menu->setScopeInteractor(dummy);
  menu->open(x, y, modifier);
  return NOTHING;
}
 
namespace {

inline double
mid(double a, double b)
{
  return (a + b) / 2.0;
}
 
inline double
distance(double x, double y, double x1, double y1)
{
  double ax = x-x1;
  double ay = y-y1;
  return sqrt(ax*ax+ay*ay);
}
 
double
bezpoint(
  double px, double py,
  double x0, double y0,
  double x1, double y1,
  double x2, double y2,
  double x3, double y3,
  double min=0.0, double max=1.0,
  double *dist = 0)
{
  double vx0 = x1-x0;
  double vx1 = x2-x1;
  double vx2 = x3-x2;
  double vy0 = y1-y0;
  double vy1 = y2-y1;
  double vy2 = y3-y2;

  double w0 = vx0 * vy1 - vy0 * vx1;
  double w1 = vx1 * vy2 - vy1 * vx2;

  double vx3 = x2 - x0;
  double vx4 = x3 - x0;
  double vy3 = y2 - y0;
  double vy4 = y3 - y0;

  double w2 = vx3 * vy4 - vy3 * vx4;
  double w3 = vx0 * vy4 - vy0 * vx4;

  if (fabs(w0)+fabs(w1)+fabs(w2)+fabs(w3)<1.0) {
    double mind, d, f;
    mind = distance(px, py, x0, y0);
    f = 0.0;
    d = distance(px, py, x1, y1);
    if (d<mind) {
      mind = d;  
      f = 1.0;   
    }
    d = distance(px, py, x2, y2);
    if (d<mind) {
      mind = d;  
      f = 2.0;   
    }
    d = distance(px, py, x3, y3);
    if (d<mind) {
      mind = d;  
      f = 3.0;   
    }

    if (dist)
      *dist = mind;
    return min + (max-min)*f/3.0;
  }
   
  double xx  = mid(x1, x2);
  double yy  = mid(y1, y2);
  double x11 = mid(x0, x1);
  double y11 = mid(y0, y1);
  double x22 = mid(x2, x3);
  double y22 = mid(y2, y3);
  double x12 = mid(x11, xx);
  double y12 = mid(y11, yy);
  double x21 = mid(xx, x22);
  double y21 = mid(yy, y22);
  double cx  = mid(x12, x21);
  double cy  = mid(y12, y21);
  double d1, d2, t1, t2;
  t1 = bezpoint(px, py, x0, y0, x11, y11, x12, y12, cx, cy, min, min+(max-min)/2.0, &d1);
  t2 = bezpoint(px, py, cx, cy, x21, y21, x22, y22, x3, y3, min+(max-min)/2.0, max, &d2);
  if (dist) {
    *dist = (d1<d2) ? d1 : d2;
  }
  return (d1<d2) ? t1 : t2;
}
 
} // namespace

/**
 * Insert an additional point near the point given by x, y.
 */
void
TFPath::insertPointNear(int x, int y)
{
//  cerr << "add point near " << x << ", " << y << endl;

  unsigned i=0, j;
  double f, min;

  for(j=0; j+3 <= polygon.size(); j+=3) {
    double u, d;
    u = bezpoint(x, y,
                 polygon[j  ].x, polygon[j  ].y,
                 polygon[j+1].x, polygon[j+1].y,
                 polygon[j+2].x, polygon[j+2].y,
                 polygon[j+3].x, polygon[j+3].y,
                 0.0, 1.0, &d);
    if (j==0) {
      i = j;   
      f = u;   
      min = d; 
    } else {   
      if (d<min) {
        min = d;  
        f = u;    
        i = j;    
      }
    }  
  }    
       
  int x0 = f*(polygon[i+1].x-polygon[i+0].x) + polygon[i+0].x;
  int y0 = f*(polygon[i+1].y-polygon[i+0].y) + polygon[i+0].y;
  int x1 = f*(polygon[i+2].x-polygon[i+1].x) + polygon[i+1].x;
  int y1 = f*(polygon[i+2].y-polygon[i+1].y) + polygon[i+1].y;
  int x2 = f*(polygon[i+3].x-polygon[i+2].x) + polygon[i+2].x;
  int y2 = f*(polygon[i+3].y-polygon[i+2].y) + polygon[i+2].y;

  int x3 = f*(x1-x0) + x0;
  int y3 = f*(y1-y0) + y0;
  int x4 = f*(x2-x1) + x1;
  int y4 = f*(y2-y1) + y1;

  int x5 = f*(x4-x3) + x3;
  int y5 = f*(y4-y3) + y3;

  j = (i+1) / 3;
//  cout << "insert corner " << j << endl;
  if (j<=corner.size())
    corner.insert(corner.begin()+j, 4);

  polygon[i+1].set(x0,y0);
  polygon.insert(polygon.begin()+i+2, TPoint(x3,y3));
  polygon.insert(polygon.begin()+i+3, TPoint(x5,y5));
  polygon.insert(polygon.begin()+i+4, TPoint(x4,y4));
  polygon[i+5].set(x2,y2);
}
 
void
TFPath::deletePoint(unsigned i)
{
  // don't delete curve handles
  if ((i%3)!=0)
    return;
  if (polygon.size()<=4)
    return;

  unsigned j = (i+1) / 3;
//  cout << "delete corner " << j << endl;
  if (j<corner.size())
   corner.erase(corner.begin()+j);

  if (i==0) {
    polygon.erase(polygon.begin(), polygon.begin()+3);
  } else 
  if (i==polygon.size()-1) {
    polygon.erase(polygon.end()-3, polygon.end());
  } else {
    polygon.erase(polygon.begin()+i-1, polygon.begin()+i+2);
  }
}  

void
TFPath::store(TOutObjectStream &out) const
{
  TColoredFigure::store(out);
  ::store(out, "closed", closed);
  unsigned i = 0;
  for(TPolygon::const_iterator p = polygon.begin();
      p != polygon.end();
      ++p, ++i)
  {
    if (i==0 || i%3 == 2 ) {
      out.indent();
      unsigned c = 3;
      unsigned j = (i+1)/3;
      if (j<corner.size())
        c = corner[j];
      out << c;
    }
    out << ' ' << p->x << ' ' << p->y;
  }
}

// 0 1 2 3 4 5 6 7 8 9
// --- ----- ----- ----

bool
TFPath::restore(TInObjectStream &in)
{
  if (in.what == ATV_VALUE && in.attribute.empty() && in.type.empty()) {
//    cerr << "corner: " << in.value << endl;
    unsigned n;
    if (polygon.empty()) {
      n = 2;
    } else {
      n = 3;
    }
    corner.push_back(atoi(in.value.c_str()));
    
    in.setInterpreter(0);
    for(unsigned i=0; i<n; ++i) {
      if (!in.parse())
        break;
      if (in.what == ATV_FINISHED) {
        in.putback('}');
        break;
      }
      int x, y;
      x = atoi(in.value.c_str());
//      cerr << in.value << ", ";
      in.parse();
      y = atoi(in.value.c_str());
      polygon.addPoint(x, y);
//      cerr << in.value << ", ";
    }
//    cerr << endl;
    in.setInterpreter(this);
    return true;
  }
  if (::restore(in, "closed", &closed))
    return true;
  if (TColoredFigure::restore(in))
    return true;
  ATV_FAILED(in)
  return false;
}

TPenTool*
TPenTool::getTool()
{
  static TPenTool* tool = 0;
  if (!tool)
    tool = new TPenTool();
  return tool;
}

void
TPenTool::cursor(TFigureEditor *fe, int x, int y)
{
  if (!path) {
    fe->getWindow()->setCursor(fischland::cursor[0]);
    return;
  }
  if (down) {
    fe->getWindow()->setCursor(fischland::cursor[3]);
    return;
  }
  if (!path->polygon.empty() &&
       path->polygon.front().x-fe->fuzziness<=x && x<=path->polygon.front().x+fe->fuzziness &&
       path->polygon.front().y-fe->fuzziness<=y && y<=path->polygon.front().y+fe->fuzziness)
  {
    fe->getWindow()->setCursor(fischland::cursor[1]);
    return;
  }
  if (!path->polygon.empty() &&
       path->polygon.back().x-fe->fuzziness<=x && x<=path->polygon.back().x+fe->fuzziness &&
       path->polygon.back().y-fe->fuzziness<=y && y<=path->polygon.back().y+fe->fuzziness)
  {
    fe->getWindow()->setCursor(fischland::cursor[2]);
    return;
  }
  fe->getWindow()->setCursor(fischland::cursor[0]);
}

void
TPenTool::stop(TFigureEditor *fe)
{
//cout << "stop pen" << endl;
//  fe->getWindow()->ungrabMouse();
  fe->getWindow()->setAllMouseMoveEvents(true);
  fe->getWindow()->setCursor(0);
  fe->state = TFigureEditor::STATE_NONE;
  if (path) {
/*
cout << "---------------" << endl;
for(unsigned i=0; i<path->corner.size(); ++i)
  cout << i << ": " << (unsigned)path->corner[i] << endl;
cout << "---------------" << endl;
*/
    if (path->polygon.size()>=4)
      fe->addFigure(path);
    else
      delete path;
    path = 0;
  }
  fe->invalidateWindow();
}

void
TPenTool::mouseEvent(TFigureEditor *fe, TMouseEvent &me)
{
  int x, y;
  fe->mouse2sheet(me.x, me.y, &x, &y);

  switch(me.type) {
    case TMouseEvent::ENTER:
      cursor(fe, x, y);
      break;
    case TMouseEvent::LDOWN:
      if (fe->state == TFigureEditor::STATE_NONE) {
        // start creation
        fe->state = TFigureEditor::STATE_CREATE;
        fe->getWindow()->setAllMouseMoveEvents(true);
        path = new TFPath();
        path->removeable = true;
        fe->getAttributes()->reason = TFigureAttributes::ALLCHANGED;
        path->setAttributes(fe->getAttributes());
      } else
      if (me.modifier() & MK_CONTROL || me.modifier() & MK_DOUBLE) {
        // end with open path
        stop(fe);
        fe->getWindow()->setCursor(fischland::cursor[0]);
        return;
      } else
      if (!path->polygon.empty() &&
          path->polygon.front().x-fe->fuzziness<=x && x<=path->polygon.front().x+fe->fuzziness &&
          path->polygon.front().y-fe->fuzziness<=y && y<=path->polygon.front().y+fe->fuzziness)
      {
        // end with closed path
        TPolygon::iterator p0, p1;
        if (path->polygon.size()%3 == 1) {
          p0 = path->polygon.end();
          --p0;
          p1 = p0;
          --p0;
          if (p0->x == p1->x && p0->y == p1->y) {
            path->corner.push_back(0);
          } else {
//            cout << "last corner is " << (unsigned)path->corner.back() << endl;
            if (path->corner.back()!=2) // ==1
              path->corner.back() = 4;
            path->corner.push_back(1);
          }
          path->polygon.addPoint(p1->x - ( p0->x - p1->x ),
                                 p1->y - ( p0->y - p1->y ));
        } else {
          path->corner.push_back(1);
        }
        p0 = p1 = path->polygon.begin();
        ++p0;
        int x1 = p1->x, y1 = p1->y;
        path->polygon.addPoint(p1->x - ( p0->x - p1->x ),
                               p1->y - ( p0->y - p1->y ));
        path->polygon.addPoint(x1, y1);
        path->closed = true;
        stop(fe);
        fe->getWindow()->setCursor(fischland::cursor[0]);
        return;
      }
      //    )(      )(      )
      // 1 2  3 4  5  6 7  8
      // 1 2  0 1  2  0 1  2
      // 0 1  2 3  4  5 6  7
      // ^      ^       ^
//      cout << "going to add points: " << path->polygon.size() << endl;
      if (path->polygon.size()%3 == 1) {
        // if (x == polygon.back().x && y==polygon.back().y) {
        if (path->polygon.back().x-fe->fuzziness<=x && x<=path->polygon.back().x+fe->fuzziness &&
            path->polygon.back().y-fe->fuzziness<=y && y<=path->polygon.back().y+fe->fuzziness)
        {
          // corner after smooth curve
          path->polygon.addPoint(x, y);
//cout << "corner after smooth curve" << endl;
        } else {
          // smooth curve after smooth curve
//cout << "smooth curve after smooth curve" << endl;
          TPolygon::iterator p0, p1;
          p0 = path->polygon.end();
          --p0;
          p1 = p0;
          --p0;
          if (p0->x != p1->x || p0->y != p1->y)
            path->corner.back() = 4;
          path->polygon.addPoint(p1->x - ( p0->x - p1->x ),
                                 p1->y - ( p0->y - p1->y ));
          path->polygon.addPoint(x, y);
          path->polygon.addPoint(x, y);
          path->corner.push_back(0);
        }
      } else {
//cout << "hmm 1: add two start points ?o|" << endl;
        // this one add's point 0,1 and 2,3
        path->polygon.addPoint(x, y);
        path->polygon.addPoint(x, y);
        path->corner.push_back(0);
      }
//      cout << "points now: " << path->polygon.size() << endl;
      down = true;
      break;
    case TMouseEvent::MOVE:
      if (down) {
//        cout << "move with " << path->polygon.size() << ", " << path->polygon.size()%3 << endl;
        if (path->polygon.size()%3 == 2) {
//cout << "hmm 2" << endl;
          // make points 0,1 a smooth point
          path->corner.back() |= 2; // 2nd point has curve
          path->polygon.back().x = x;
          path->polygon.back().y = y;
        } else {
//cout << "hmm 3" << endl;
          path->corner.back() |= 1; // 1st point has curve
          TPolygon::iterator p0, p1;
          p0 = path->polygon.end();
          --p0;
          p1 = p0;
          --p0;
          p0->x = p1->x - ( x - p1->x );
          p0->y = p1->y - ( y - p1->y );
        }
      }
      cursor(fe, x, y);
      break;
    case TMouseEvent::LUP:

      down = false;
      cursor(fe, x, y);
      break;
  }
  fe->invalidateWindow();
}

void
TPenTool::keyEvent(TFigureEditor *fe, TKeyEvent &ke)
{
  if (ke.type != TKeyEvent::DOWN)
    return;
  if (!path)
    return;
  TPolygon &polygon = path->polygon;
  vector<byte> &corner = path->corner;
  switch(ke.getKey()) {
    case TK_ESCAPE:
      delete path;
      path = 0;
      stop(fe);
      break;
    case TK_DELETE:
    case TK_BACKSPACE:
#if 0
      cout << "delete with " << polygon.size() 
           << " (" << polygon.size()%3 << "), "
           << corner.size() << endl;
#endif
      // fe->invalidateFigure(path);
      fe->invalidateWindow();
      if (polygon.size()==2) {
        if (corner[0] & 2) {
          polygon[1].x = polygon[0].x;
          polygon[1].y = polygon[0].y;
          corner[0] = 0;
        } else {
          polygon.erase(polygon.end()-2, polygon.end());
          corner.erase(corner.end()-1);
        }
      } else
      if (polygon.size()%3 == 1) { // 4 7 10 ...
#if 0
        if (corner[polygon.size()/3] & 1) {
          polygon[polygon.size()-2].x = polygon[polygon.size()-1].x;
          polygon[polygon.size()-2].y = polygon[polygon.size()-1].y;
          corner[polygon.size()/3] = 0;
        } else {
#endif
          polygon.erase(polygon.end()-2, polygon.end());
          corner.erase(corner.end()-1);
//        }
      } else
      if (polygon.size()%3 == 2) { // 5 8 11 ...
        polygon.erase(polygon.end()-1);
        corner.back() &= 1;
      }
#if 0
      cout << "delete with " << polygon.size() 
           << " (" << polygon.size()%3 << "), "
           << corner.size() << endl;
#endif
      break;
  }
}

void
TPenTool::paintSelection(TFigureEditor *fe, TPenBase &pen)
{
  if (!path)
    return;
  path->paint(pen, TFigure::EDIT);

  const TMatrix2D *m0 = pen.getMatrix();
  if (m0) {
    pen.push();
    pen.identity();
  }
  TPolygon &polygon = path->polygon;
  int i = polygon.size();

  if (i<4 || (i%3)!=1)  
    return;
  --i;
  int x0 = polygon[i].x;
  int y0 = polygon[i].y;
  int x1 = polygon[i].x - (polygon[i-1].x - polygon[i].x);
  int y1 = polygon[i].y - (polygon[i-1].y - polygon[i].y);
  if (m0) {
    m0->map(x0, y0, &x0, &y0);
    m0->map(x1, y1, &x1, &y1);
  }
  pen.drawLine(x0, y0, x1, y1);
  pen.fillCirclePC(x1-2,y1-2,6,6);

  if (m0) {
    pen.pop();
  }
}

TPencilTool*
TPencilTool::getTool()
{
  static TPencilTool* tool = 0;
  if (!tool)
    tool = new TPencilTool();
  return tool;
}

void
TPencilTool::stop(TFigureEditor *fe)
{
  fe->getWindow()->setAllMouseMoveEvents(false);
  fe->getWindow()->flagCompressMotion = true;
  fe->getWindow()->setCursor(0);
  fe->state = TFigureEditor::STATE_NONE;
  fe->invalidateWindow();
}

void fitCurve(const TPolygon &in, TPolygon *out);

void
TPencilTool::keyEvent(TFigureEditor *fe, TKeyEvent &ke)
{
  if (ke.getKey() != TK_CONTROL_L &&
      ke.getKey() != TK_CONTROL_R)
    return;
  if (ke.type != TKeyEvent::UP)
    fe->getWindow()->setCursor(fischland::cursor[4]);
  else
    fe->getWindow()->setCursor(fischland::cursor[5]);
}
 

void 
TPencilTool::mouseEvent(TFigureEditor *fe, TMouseEvent &me)
{
  int x, y;
  fe->mouse2sheet(me.x, me.y, &x, &y);

  switch(me.type) {
    case TMouseEvent::ENTER:
      if (me.modifier() & MK_CONTROL)
        fe->getWindow()->setCursor(fischland::cursor[4]);
      else
        fe->getWindow()->setCursor(fischland::cursor[5]);
      break;
    case TMouseEvent::LDOWN:
      fe->getWindow()->flagCompressMotion = false;
      polygon.clear();
      polygon.addPoint(x, y);
      closed = false;
      break;
    case TMouseEvent::MOVE: {
      if (! (me.modifier() & MK_LBUTTON))
        break;
      if (!polygon.empty() &&
          polygon.front().x-fe->fuzziness<=x && x<=polygon.front().x+fe->fuzziness &&
          polygon.front().y-fe->fuzziness<=y && y<=polygon.front().y+fe->fuzziness)
      {
        closed = true;
        fe->getWindow()->setCursor(fischland::cursor[4]);
      } else {
        closed = false;
        fe->getWindow()->setCursor(fischland::cursor[5]);
      }
      polygon.addPoint(x,y);

      //TCairo pen(fe->getWindow()); // too slow
      TPen pen(fe->getWindow());
      TFigureAttributes *a = fe->getAttributes();
      pen.setAlpha(a->alpha);
      pen.setColor(a->linecolor);
      pen.setLineStyle(a->linestyle);
      pen.setLineWidth(1 /*a->linewidth*/);

      // pen.setClipRect(fe->getVisible());
      pen &= fe->getVisible();
      TWindow *window = fe->getWindow();
      pen.translate(fe->getVisible().x,
                    fe->getVisible().y);
      pen.multiply(fe->getMatrix());
      TPolygon::size_type i = polygon.size();
      pen.drawLine(polygon[i-2].x, polygon[i-2].y,
                   polygon[i-1].x, polygon[i-1].y);
    } break;
    case TMouseEvent::LUP:
      if (polygon.size()>1) {
        TFPath *f = new TFPath();
        if (me.modifier() & MK_CONTROL || closed) {
          polygon.addPoint(polygon[0].x, polygon[0].y);
          f->closed = true;
        }
        fe->getAttributes()->reason = TFigureAttributes::ALLCHANGED;
        f->setAttributes(fe->getAttributes());
        fitCurve(polygon, &f->polygon);
        fe->addFigure(f);
        fe->invalidateFigure(f);
        polygon.clear();
      }
      polygon.clear();
      fe->getWindow()->flagCompressMotion = true;
      break;
  }
}
