/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2006 by Mark-André Hopf <mhopf@mark13.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307,  USA
 */

#include <toad/springlayout.hh>
#include <toad/window.hh>
#include <toad/types.hh>

#include <algorithm>
#include <cassert>

using namespace toad;

#ifdef DEBUG
#undef DEBUG
#endif

// #define DEBUG

/**
 * \class toad::TSpringLayout
 *
 * This layout arranges windows relative to each other.
 *
 * \pre
toad::TSpringLayout {
  <windowname> = {
    [w = <width>]
    [h = <height>]
    top|bottom|left|right = {
      how = border|window|opposite
      [where = <windowname>]
      [distance = <distance>]
    }
    ...
  }
  ...
}
   \endpre
 */

#define HAS_T 1
#define HAS_B 2
#define HAS_L 4
#define HAS_R 8
#define HAS_ALL 15
#define DTOP 0
#define DBOTTOM 1
#define DLEFT 2
#define DRIGHT 3

/*****************************************************************************
 *                                                                           *
 * TFormNode                                                                 *
 *                                                                           *
 *****************************************************************************/
class TSpringLayout::TFormNode
{
    TWindow *wnd;               // pointer to the window to be placed
  public:
    TFormNode(const string &name);
    TWindow* it(TWindow *parent);
    void getShape(TWindow *parent, TRectangle *shape);
    string name;                // name of the window to be placed
    unsigned how[4];            // how to attach
    string whichname[4];
    TWindow *which[4];          // where to attach
    int dist[4];                // minimal distance to neighbours
    int coord[4];               // the left,right,top & bottom during calculation
    int w, h;                   // fixed size (when >= 0)
    unsigned char done;         // flags for attached sides
    unsigned char nflag;                 // flags for sides with undefined attachment
    TFormNode *next, *prev;     // should remove `prev'
};

inline TSpringLayout::TFormNode::TFormNode(const string &name)
{
  this->name = name;
  // cerr << "new node '" << name << "'" << this << endl;
  wnd = NULL;
  for(int i=0; i<4; i++) {
    how[i]=NONE;
    which[i]=NULL;
    dist[i]=0;
  }
  w = h = -1;
  next = prev = NULL;
}

/**
 * return the window associated by this node
 * \param parent The windows parent window.
 */
TWindow*
TSpringLayout::TFormNode::it(TWindow *parent)
{
  if (wnd)
    return wnd;
    
  TInteractor *p = parent->getFirstChild();
  while(p) {
    if (p->getTitle() == name) {
      wnd = dynamic_cast<TWindow*>(p);
      return wnd;
    }
    p = p->getNextSibling();
  }
  cerr << "error: no window found with name '" << name << "'" << this << endl;
  return NULL;
}

void
TSpringLayout::TFormNode::getShape(TWindow *parent, TRectangle *shape)
{
  it(parent)->getShape(shape);
  if (w >= 0)
    shape->size.width = w;
  if (h >= 0)
    shape->size.height = h;
}

TSpringLayout::TSpringLayout()
{
  flist = lastadd = NULL;
  nBorderOverlap = 0;
  bKeepOwnBorder = false;
  running = false;
}

TSpringLayout::~TSpringLayout()
{
  if (flist) {
    flist->prev->next = 0;
    while(flist) {
      TFormNode *on = flist;
      flist = flist->next;
      delete on;
    }
  }
}

#if 0
void
TSpringLayout::addForm(const string &child)
{
  if (flist) {
    lastadd = new TFormNode;
    lastadd->next = flist;
    lastadd->prev = flist->prev;
    flist->prev->next = lastadd;
    flist->prev = lastadd;
  } else {
    lastadd = flist = new TFormNode;
    flist->next = flist;
    flist->prev = flist;
  } 
  lastadd->name = child;
}

void
TSpringLayout::removeForm(const string &child)
{
  if (flist) {
    if (flist == flist->next) {
      delete flist;
      flist=NULL;
    } else {
      TFormNode *ptr = _find(child);
      if (!ptr)
        return;
      ptr->next->prev = ptr->prev;
      ptr->prev->next = ptr->next;
      if (flist==ptr)
        flist = ptr->next;
      delete ptr;
    }
  }
}
#endif

#if 0
void
TSpringLayout::attachLast(unsigned where, unsigned how, TWindow *which)
{
  #ifdef SECURE
  if (!lastadd) {
    fprintf(stderr, "TOAD: error at TFormBase::AttachLast; no children\n");
    return;
  }
  #endif
  for(int i=0; i<4; i++) {
    if( (i==0 && where&TOP   ) ||
        (i==1 && where&BOTTOM) ||
        (i==2 && where&LEFT  ) ||
        (i==3 && where&RIGHT ) )
    {
      lastadd->how[i]=how;
      lastadd->which[i]=which;
    }
  }
}
#endif

/**
 * Attach a window to another window.
 *
 * \param window
 *   Title of the window to be attached to.
 * \param where
 *    A combination of TOP, BOTTOM, LEFT and RIGHT.
 * \param how
 *    One of NONE, FORM, WINDOW or OPPOSITE_WINDOW
 * \param which
 *    Title of the window to be attached.
 */
void 
TSpringLayout::attach(const string &window, unsigned where, EMethod how, const string &which)
{
  TFormNode *node = _find(window);
  if (!node)
    return;
  
  for(int i=0; i<4; i++) {
    if( (i==0 && where&TOP   ) ||
        (i==1 && where&BOTTOM) ||
        (i==2 && where&LEFT  ) ||
        (i==3 && where&RIGHT ) )
    {
      node->how[i]=how;
      node->whichname[i]=which;
    }
  }
}

/**
 * Specify distance to other windows.
 *
 * \param window
 *   Title of the window whose distance is set.
 * \param distance
 *   The distance
 * \param
 *   A combination of TOP, BOTTOM, LEFT and RIGHT.
 */
void
TSpringLayout::distance(const string &window, int distance, unsigned where)
{
  TFormNode *node = _find(window);
  if (!node)
    return;
  
  for(int i=0; i<4; i++)
  {
    if( (i==0 && where&TOP   ) ||
        (i==1 && where&BOTTOM) ||
        (i==2 && where&LEFT  ) ||
        (i==3 && where&RIGHT ) )
    {
      node->dist[i]=distance;
    }
  }
}

void
TSpringLayout::arrange()
{
  arrange(0, 0, window->getWidth(), window->getHeight());
}

/**
 * Arrange
 * arrange all children as described in the 'flist'
 */
void
TSpringLayout::arrange(int fx,int fy,int fw,int fh)
{
  // when there is no layout, return
  if (!flist)
    return;

  // FIXME: layout loops cause by multiple layouts should be handled globally
  if (running)
    return;
  running = true;

  // initialize data structures
  TRectangle shape;
  unsigned nChildren=0;
  bool bError=false;
  TFormNode *ptr = flist;
  do {
    #ifdef DEBUG
    printf("! \"%s\" at (%i,%i) with size (%i,%i)\n",
      ptr->it->Title().c_str(),
      ptr->it->XPos(), ptr->it->YPos(),
      ptr->it->Width(), ptr->it->Height());
    #endif
    ++nChildren;
    ptr->done  = 0;
    ptr->nflag = 0;
    ptr->getShape(window, &shape);
    ptr->coord[DTOP]    = shape.origin.y;
    ptr->coord[DBOTTOM] = shape.origin.y+shape.size.height;
    ptr->coord[DLEFT]   = shape.origin.x;
    ptr->coord[DRIGHT]  = shape.origin.x+shape.size.width;
    for(int i=0; i<4; i++) {
      if (ptr->how[i] == NONE ) {
        ptr->nflag|=(1<<i);
      }
    }
    if ((ptr->nflag&3)==3 || (ptr->nflag&12)==12) {
      if(!ptr->it(window)->flagShell && !ptr->it(window)->flagPopup ) {
        cerr << "toad: window '"<<ptr->name<<"' within TSpringLayout has a undefined attachment"<<endl;
        bError = true;
      }
    }

    ptr = ptr->next;
  }while(ptr!=flist);
  
  if (bError) {
    fprintf(stderr, "toad: can't arrange children\n");
//    exit(1);
  }

  // arrange children
  int form[4];
  // form[0]=0; form[1]=Height(); form[2]=0; form[3]=Width();
  form[DTOP]=fy;
  form[DBOTTOM]=fy+fh;
  form[DLEFT]=fx;
  form[DRIGHT]=fx+fw;

  TFormNode *ptr2;

  unsigned count=0;
  unsigned done=0;      // we're done when `done' equals `nChildren'

  while(true) {
    count++;
    if (ptr->done != HAS_ALL) {
      // window has non attached sides

      // 1st strategy:
      // attach all sides where the opposite side of another object is known
      for(int i=0; i<4; i++) {
        if (!(ptr->done & (1<<i))) {
          switch(ptr->how[i])
          {
            case FORM:
              ptr->done |= (1<<i);
              ptr->coord[i] = form[i];
              if (!bKeepOwnBorder) {
                if (i&1)
                  ptr->coord[i] += nBorderOverlap;
                else
                  ptr->coord[i] -= nBorderOverlap;
              }
              if (i&1) {
                ptr->coord[i] -= ptr->dist[i];
              } else {
                ptr->coord[i] += ptr->dist[i];
              }
              count = 0;
              break;
            case WINDOW:
              ptr2=_find(ptr->whichname[i]);      // opposite window
              if ((ptr2->done) & (1<<(i^1))) {    // opposite side is set
                ptr->done |=(1<<i);
                ptr->coord[i] = ptr2->coord[i^1];
                if (i&1) { // bottom & right
                  ptr->coord[i] += nBorderOverlap;
                  ptr->coord[i] -= max(ptr->dist[i], ptr2->dist[i^1]);
                } else { // top & left
                  ptr->coord[i] -= nBorderOverlap;
                  ptr->coord[i] += max(ptr->dist[i], ptr2->dist[i^1]);
                }
                count = 0;
              }
              break;
            case OPPOSITE_WINDOW: // FIXME: CODE IS MISSING FOR DISTANCE !!!
              ptr2=_find(ptr->whichname[i]);
              if ((ptr2->done) & (1<<(i))) {
                ptr->done |=(1<<i);
                ptr->coord[i] = ptr2->coord[i];
                count = 0;
              }
              break;
          }
        }
      } // end of the 1st strategy
      
      if ( (ptr->done|ptr->nflag) == HAS_ALL) {
        // 2nd strategy
        // we're almost done with the window, the missing coordinates
        // can be calculated from the object's size
        ptr->getShape(window, &shape);

        // no top and/or left attachment
        if (ptr->nflag & TOP)
          ptr->coord[DTOP] = ptr->coord[DBOTTOM] - shape.size.height;
        if (ptr->nflag & BOTTOM)
          ptr->coord[DBOTTOM] = ptr->coord[DTOP] + shape.size.height;
        if (ptr->nflag & LEFT)
          ptr->coord[DLEFT] = ptr->coord[DRIGHT] - shape.size.width;
        if (ptr->nflag & RIGHT)
          ptr->coord[DRIGHT] = ptr->coord[DLEFT] + shape.size.width;

        unsigned w,h;
        w = ptr->coord[DRIGHT] - ptr->coord[DLEFT];
        h = ptr->coord[DBOTTOM] - ptr->coord[DTOP];

        //ptr->it(window)->setSize(w,h);
        //ptr->it(window)->setShape(TPOS_PREVIOUS, TPOS_PREVIOUS, w,h);
        ptr->it(window)->getShape(&shape);
        shape.size.width  = w;
        shape.size.height = h;
        ptr->it(window)->setShape(shape);

        // adjust top and/or left after SetSize
        ptr->getShape(window, &shape);
        if (ptr->nflag & TOP)
          ptr->coord[DTOP] = ptr->coord[DBOTTOM] - shape.size.height;
        if (ptr->nflag & BOTTOM)
          ptr->coord[DBOTTOM] = ptr->coord[DTOP] + shape.size.height;
        if (ptr->nflag & LEFT)
          ptr->coord[DLEFT] = ptr->coord[DRIGHT] - shape.size.width;
        if (ptr->nflag & RIGHT)
          ptr->coord[DRIGHT] = ptr->coord[DLEFT] + shape.size.width;
        
        ptr->it(window)->setPosition(ptr->coord[DLEFT],ptr->coord[DTOP]);
        ptr->getShape(window, &shape);

        ptr->coord[DTOP]    = shape.origin.y;
        ptr->coord[DBOTTOM] = shape.origin.y+shape.size.height;
        ptr->coord[DLEFT]   = shape.origin.x;
        ptr->coord[DRIGHT]  = shape.origin.x+shape.size.width;
        ptr->done = HAS_ALL;
        done++;
      }
    }

    if (done>=nChildren) {
      running = false;
      return;
    }
    
    if (count>nChildren) {
      bool bNoGuess = true;
      count=0;
      while(count<nChildren) {
        if (ptr->done != HAS_ALL) {
          ptr->getShape(window, &shape);
          if ( (ptr->nflag&LEFT) && !(ptr->done&LEFT) && (ptr->done&RIGHT) ) {
            #ifdef DEBUG
            printf("guessing left side of %s\n",ptr->name.c_str());
            #endif
            ptr->coord[DLEFT] = ptr->coord[DRIGHT] - shape.size.width;
            ptr->done|=HAS_L;
            bNoGuess = false;
          }
          if ( (ptr->nflag&RIGHT) && !(ptr->done&RIGHT) && (ptr->done&LEFT) ) {
            #ifdef DEBUG
            printf("guessing right side of %s\n",ptr->name.c_str());
            #endif
            ptr->coord[DRIGHT] = ptr->coord[DLEFT] + shape.size.width;
            ptr->done|=HAS_R;
            bNoGuess = false;
          }
          if ( !(ptr->nflag&TOP) && !(ptr->done&TOP) && (ptr->done&BOTTOM) ) {
            #ifdef DEBUG
            printf("guessing top side of %s\n",ptr->name.c_str());
            #endif
            ptr->coord[DTOP] = ptr->coord[DBOTTOM] - shape.size.height;
            ptr->done|=HAS_T;
            bNoGuess = false;
          }
          if ( (ptr->nflag&BOTTOM) && !(ptr->done&BOTTOM) && (ptr->done&TOP) ) {
            #ifdef DEBUG
            printf("guessing bottom side of %s\n",ptr->name.c_str());
            #endif
            ptr->coord[DBOTTOM] = ptr->coord[DTOP] + shape.size.height;
            ptr->done|=HAS_B;
            bNoGuess = false;
          }
          if (ptr->done == HAS_ALL) {
            cerr << "TSpringLayout: looks like recursive attachment" << endl;
          }
        }
        count++;
        ptr = ptr->next;
      }

      if(bNoGuess) {
        cerr << "TSpringLayout: Can't handle recursive attachment. Stopped." << endl;
        #ifdef DEBUG
        count=0;
        while(count<nChildren) {
          printf("%25s : ",ptr->name.c_str());
          printf( ptr->done&HAS_T ? "t" : "-");
          printf( ptr->done&HAS_B ? "b" : "-");
          printf( ptr->done&HAS_L ? "l" : "-");
          printf( ptr->done&HAS_R ? "r" : "-");
          printf("\n");
          count++;
          ptr = ptr->next;
        } 
        #endif
        running = false;
        return;
      }
      #ifdef DEBUG
      printf("trying again\n");
      #endif
      count = 0;
    }
    ptr = ptr->next;
  }
  running=false;
}

TSpringLayout::TFormNode* 
TSpringLayout::_find(const string &which)
{
  assert(!which.empty());

  if (!flist) {
    lastadd = flist = new TFormNode(which);
    flist->next = flist;
    flist->prev = flist;
    return flist;
  }
  
  TFormNode *ptr2=flist;
  while(ptr2->name != which) {
    ptr2 = ptr2->next;
    if (ptr2==flist) {
      lastadd = new TFormNode(which);
      lastadd->next = flist;
      lastadd->prev = flist->prev;
      flist->prev->next = lastadd;
      flist->prev = lastadd;
      return lastadd;
    }
  }
  return ptr2;
}

void 
TSpringLayout::store(TOutObjectStream &out) const 
{
  // nBorderOverlap
  // bKeepOwnBorder

  if (!flist)
    return;
  TFormNode *ptr=flist;
  out.indent();
  while(true) {
    out.writeQuoted(ptr->name);
    out<<" = ";
    out.startGroup();
    for(unsigned i=0; i<4; ++i) {
      if (ptr->how[i]!=NONE) {
        out.indent();
        switch(i) {
          case DTOP   : out << "top = "; break;
          case DBOTTOM: out << "bottom = "; break;
          case DLEFT  : out << "left = "; break;
          case DRIGHT : out << "right = "; break;
        }
        out.startGroup();
        switch(ptr->how[i]) {
          case FORM:
            ::store(out, "how", "border");
            break;
          case WINDOW:
            ::store(out, "how", "window");
            ::store(out, "where", ptr->whichname[i]);
            break;
          case OPPOSITE_WINDOW:
            ::store(out, "how", "opposite");
            ::store(out, "where", ptr->whichname[i]);
            break;
        }
        if (ptr->dist[i]) {
          ::store(out, "distance", ptr->dist[i]);
        }
        out.endGroup();
      }
    }
    out.endGroup();
    out.indent();
    ptr = ptr->next;
    if (ptr==flist)
      break;
  }
}

bool
TSpringLayout::restore(TInObjectStream &in)
{
  if (in.what==ATV_START || in.what==ATV_FINISHED)
    return true;
  unsigned depth=0;
  in.setInterpreter(0);
  TFormNode *node = 0;
  unsigned pos = 0;

  do {
    switch(depth) {
      case 0:
        if (in.what == ATV_GROUP &&
            !in.attribute.empty() &&
            in.type.empty())
        {
          node = _find(in.attribute);
          if (!node) {
            cerr << "failed to locate node '" << in.attribute << "'" << endl;
            break;
          }
          // cout << "  new node " << node << endl;
        } else {
          node = 0;
        }
        break;
      case 1:
        if (!node) {
          cerr << "TSpringLayout: no node" << endl;
          return false;
        }
        if (in.what == ATV_GROUP &&
            !in.attribute.empty() &&
            in.type.empty())
        {
          if (in.attribute=="top")
            pos = DTOP;
          else if (in.attribute=="bottom")
            pos = DBOTTOM;
          else if (in.attribute=="left")
            pos = DLEFT;
          else if (in.attribute=="right")
            pos = DRIGHT;
        } else
        if (in.what == ATV_VALUE &&
            !in.attribute.empty() &&
            in.type.empty() &&
            !in.value.empty())
        {
          if (in.attribute=="w") {
            node->w = atoi(in.value.c_str());
          } else if (in.attribute=="h") {
            node->h = atoi(in.value.c_str());
          }
        }
        break;
      case 2:
        if (in.what == ATV_VALUE &&
            in.attribute == "how" &&
            in.type.empty())
        {
          if (in.value=="border")
            node->how[pos]=FORM;
          else if (in.value=="window")
            node->how[pos]=WINDOW;
          else if (in.value=="opposite")
            node->how[pos]=OPPOSITE_WINDOW;
        }
        if (in.what == ATV_VALUE &&
            in.attribute == "where" &&
            in.type.empty())
        {
          node->whichname[pos]=in.value;
        }
        if (in.what == ATV_VALUE &&
            in.attribute == "distance" &&
            in.type.empty())
        {
          node->dist[pos]=atoi(in.value.c_str());
        }
    }
    if (in.what==ATV_GROUP) {
      ++depth;
    } else if (in.what==ATV_FINISHED) {
      if (!depth) {
        in.putback('}');
        break;
      }
      --depth;
    }
  } while(in.parse());
  in.setInterpreter(this);
  return true;
}
