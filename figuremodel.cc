/*
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2005 by Mark-André Hopf <mhopf@mark13.org>
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

#include <toad/figure.hh>
#include <toad/figuremodel.hh>
#include <toad/figureeditor.hh>
#include <toad/undo.hh>
#include <toad/undomanager.hh>
#include <toad/io/binstream.hh>

/**
 * \ingroup figure
 * \class toad::TFigureModel
 *
 * TFigureModel holds a list of TFigure objects, notifies observers
 * about changes and registers undo objects for figures modified with
 * the models methods.
 */

using namespace toad;

TFigureModel::TFigureModel()
{
//  cerr << "new TFigureModel " << this << endl;
}

TFigureModel::TFigureModel(const TFigureModel &m)
{
//  cerr << "copy constructed TFigureModel " << this << " from " << &m << endl;
  for(TStorage::const_iterator p = m.storage.begin();
      p != m.storage.end();
      ++p)
  {
    storage.push_back( static_cast<TFigure*>( (*p)->clone() ) );
  }
}

TFigureModel::~TFigureModel()
{
  type = DELETE;
  sigChanged();
  clear();
}

class TUndoGroup:
  public TUndo
{
    TFigureAtDepthList figures;
    TFigureModel *model;
    TFGroup *group;
  public:
    TUndoGroup(TFigureModel *model) {
      this->model = model;
    }
    ~TUndoGroup() {
      figures.drop(); // clear to avoid deleting the figures
    }
    void insert(TFigure *f, unsigned d)
    {
//cerr << "TUndoRemove: store figure " << f << " at depth " << d << endl;
      figures.push_back(f, d);
    }
    void setGroup(TFGroup *group) {
      this->group = group;
    }
  protected:
    void undo() {
      group->drop();
      model->_undoGroup(group, figures);
      figures.drop();
    }
    bool getUndoName(string *name) const {
      *name = "Undo: Group";
      return true;
    }
    bool getRedoName(string *name) const {
      *name = "Redo: Ungroup";
      return true;
    }
};

class TUndoUngroup:
  public TUndo
{
    TFigureModel *model;
    TFigureSet figures;
  public:
    TUndoUngroup(TFigureModel *model) {
      this->model = model;
    }
    void insert(TFigure *f) {
      figures.insert(f);
    }
  protected:
    void undo() {
      model->group(figures);
    }
    bool getUndoName(string *name) const {
      *name = "Undo: Ungroup";
      return true;
    }
    bool getRedoName(string *name) const {
      *name = "Redo: Group";
      return true;
    }
};

TFigureAtDepthList::~TFigureAtDepthList()
{
  for(TStore::iterator p=store.begin();
      p!=store.end();
      ++p)
  {
    delete p->figure;
  }
}

class TUndoTranslateHandle:
  public TUndo
{
    TFigureModel *model;
    TFigure* figure;
    unsigned handle;
    TCoord dx, dy;
    unsigned m;
  public:
    TUndoTranslateHandle(TFigureModel *model, TFigure *figure, unsigned handle, TCoord dx, TCoord dy, unsigned m) {
      this->model = model;
      this->figure = figure;
      this->handle = handle;
      this->dx = dx;
      this->dy = dy;
      this->m  = m;
    }
  protected:
    void undo() {
//cout << "undo translate " << dx << ", " << dy << endl;
      model->translateHandle(figure, handle, dx, dy, m);
    }
    bool getUndoName(string *name) const {
      *name = "Undo: Move Handle";
      return true;
    }
    bool getRedoName(string *name) const {
      *name = "Redo: Move Handle";
      return true;
    }
};

/*****************************************************************************
 *                                                                           *
 *                                  I N S E R T                              *
 *                                                                           *
 *****************************************************************************/

namespace {

class TUndoInsert:
  public TUndo
{
    TFigureModel *model;
  public:
    TFigureSet figures;
    TUndoInsert(TFigureModel *model) {
      this->model = model;
    }
    void insert(TFigure *f) {
      figures.insert(f);
    }
  protected:
    void undo() {
      model->erase(figures);
      figures.clear();
    }
    bool getUndoName(string *name) const {
      *name = "Undo: Insert";
      return true;
    }
    bool getRedoName(string *name) const {
      *name = "Redo: Remove";
      return true;
    }
};

}

void
TFigureModel::insert(TFigureAtDepthList &figuresAtDepth)
{
  pureInsert(figuresAtDepth);

  TUndoInsert *undo = new TUndoInsert(this);
  for(auto &&p: figuresAtDepth.store)
    undo->figures.insert(p.figure);
  TUndoManager::registerUndo(this, undo);
  
  type = ADD;
  figures.clear();
  for(auto &&p: figuresAtDepth.store)
    figures.insert(p.figure);
  sigChanged();

  figuresAtDepth.drop();
}

void 
TFigureModel::pureInsert(const TFigureAtDepthList &figuresAtDepth)
{
  for(auto p=figuresAtDepth.store.begin();
      p!=figuresAtDepth.store.end();
      ++p)
  {
    storage.insert(
      storage.begin() + p->depth,
      p->figure);
  }
}

/**
 * Append a figure to the model
 *
 * This function notifies all its views about the modification and
 * registers an undo object.
 */
void 
TFigureModel::add(TFigure *figure) {
  TUndoInsert *undo = new TUndoInsert(this);
  undo->insert(figure);
  TUndoManager::registerUndo(this, undo);

  storage.push_back(figure);

  type = ADD;
  figures.clear();
  figures.insert(figure);
  sigChanged();
  
  TFigureEditEvent ee;
  ee.model = this;
  ee.type = TFigureEditEvent::ADDED;
  figure->editEvent(ee);
}

/**
 * Append figures to the model.
 *
 * This function notifies all its views about the modification and
 * registers an undo object.
 */
void 
TFigureModel::add(TFigureVector &newfigures) {
  TFigureEditEvent ee;
  ee.model = this;
  ee.type = TFigureEditEvent::ADDED;
  figures.clear();
  type = ADD;
  TUndoInsert *undo = new TUndoInsert(this);
  for(TFigureVector::iterator p = newfigures.begin();
      p != newfigures.end();
      ++p)
  {
    storage.push_back(*p);
    figures.insert(*p);
    undo->insert(*p);
    (*p)->editEvent(ee);
  }
  TUndoManager::registerUndo(this, undo);
  sigChanged();
}

/*****************************************************************************
 *                                                                           *
 *                                   E R A S E                               *
 *                                                                           *
 *****************************************************************************/

namespace {

class TUndoRemove:
  public TUndo
{
    TFigureModel *model;
  public:
    TFigureAtDepthList figures;
    TUndoRemove(TFigureModel *model) {
      this->model = model;
    }
    void insert(TFigure *f, unsigned d)
    {
//cerr << "TUndoRemove: store figure " << f << " at depth " << d << endl;
      figures.push_back(f, d);
    }
  protected:
    void undo() {
      model->insert(figures);
      figures.drop();
    }
    bool getUndoName(string *name) const {
      *name = "Undo: Remove";
      return true;
    }
    bool getRedoName(string *name) const {
      *name = "Redo: Insert";
      return true;
    }
};

} // namespace

void
TFigureModel::erase(TFigureSet &set, TFigureAtDepthList *placement)
{
  // FIXME: create TFigureEditEvents
  TUndoRemove *undo = new TUndoRemove(this);
  pureErase(set, &undo->figures);
  TUndoManager::registerUndo(this, undo);
}

/**
 * Erase a set of figures from the model.
 *
 * After the call the figures are owned by the undo manager.
 *
 * This function notifies all its views about the modification and
 * registers an undo object.
 */
void
TFigureModel::pureErase(TFigureSet &set, TFigureAtDepthList *placement)
{
  if (set.empty())
    return;

//  TFigureEditEvent ee;
//  ee.model = this;

//  ee.type = TFigureEditEvent::RELATION_REMOVED;
  for(auto &figureToBeRemoved: set) {
    auto relation = TFigureEditor::relatedTo.find(figureToBeRemoved);
    if (relation==TFigureEditor::relatedTo.end()) {
      continue;
    }
    figures.clear();
    figures.insert(figureToBeRemoved);
/*
    for(auto &p: relation->second) {
      const_cast<TFigure*>(p)->editEvent(ee);
    }
*/
    TFigureEditor::relatedTo.erase(relation); // FIXME: there might be no guarantee that the iterator isn't invalidated
  }
/*
  type = REMOVE;
  figures.clear();
  figures.insert(set.begin(), set.end());
  sigChanged();
    
  ee.type = TFigureEditEvent::REMOVED;
*/
  unsigned depth = 0;
  auto iterator=storage.begin();
  while(iterator!=storage.end()) {
    TFigureSet::iterator q = set.find(*iterator);
    if (q!=set.end()) {
      if (placement)
        placement->push_back(*iterator, depth);
//      (*p)->editEvent(ee);
      iterator = storage.erase(iterator);
    } else {
      ++iterator;
    }
    ++depth;
  }
}

void
TFigureModel::erase(TFigure *figure)
{
  TFigureSet set;
  set.insert(figure);
  erase(set);
}

void
TFigureModel::erase(const iterator &p)
{
  TFigureSet set;
  set.insert(*p);
  erase(set);
}

void
TFigureModel::erase(const iterator &p, const iterator &e)
{
  TFigureSet set;
  set.insert(p, e);
  erase(set);
}

/*****************************************************************************
 *                                                                           *
 *                               T R A N S F O R M                           *
 *                                                                           *
 *****************************************************************************/

class TUndoTransform:
  public TUndo
{
    TFigureModel *model;
    TFigureSet figures;
    TMatrix2D matrix;
    bool invert; // to avoid floating point errors in matrix when going through multiple undo/redo events

  public:
    TUndoTransform(TFigureModel *model, const TFigureSet &set, const TMatrix2D &matrix, bool invert) {
      this->model = model;
      this->figures.insert(set.begin(), set.end());
      this->matrix = matrix;
      this->invert = invert;
    }
  protected:
    void undo() override {
cout << "TUndoTransform::undo()" << endl;
      model->transform(&figures, matrix, invert);
    }
    bool getUndoName(string *name) const override {
      *name = "Undo: Transform";
      return true;
    }
    bool getRedoName(string *name) const override {
      *name = "Redo: Transform";
      return true;
    }
};

/**
 * Apply the affine transformation 'matrix' to all figures listed in 'selection'
 * and create an undo event. the object in selection may change, please see
 * pureTransform() for full details.
 *
 * \sa TFigureModel::pureTransform
 */
void
TFigureModel::transform(TFigureSet *selection, const TMatrix2D &matrix, bool invert)
{
  figures = *selection;
  type = MODIFY;
  sigChanged();

  if (!invert) {
    pureTransform(selection, matrix);
  } else {
    TMatrix2D invertedMatrix(matrix);
    invertedMatrix.invert();
    pureTransform(selection, invertedMatrix);
  }

  figures = *selection;
  type = MODIFIED;
  sigChanged();
  
  TFigureEditEvent ee;
  ee.model = this;
/*
  ee.type = TFigureEditEvent::TRANSFORMED;
  ee.transform = matrix;
  for(auto &&figure: *selection)
    ...
*/
    
  ee.type = TFigureEditEvent::RELATION_MODIFIED;
  for(auto &figure: *selection) {
    auto relation = TFigureEditor::relatedTo.find(figure);
    if (relation==TFigureEditor::relatedTo.end())
      continue;
    for(auto &relatedFigure: relation->second)
      const_cast<TFigure*>(relatedFigure)->editEvent(ee);
  }

  TUndoManager::registerUndo(this,
    new TUndoTransform(this, *selection, matrix, !invert)
  );
}

/**
 * Apply the affine transformation 'matrix' to all figures listed in 'selection'
 * by invoking their 'transform' method.
 *
 * Figures which can not be transformed, are prefixed with TFTransform objects
 * or if they already are prefixed in TFTransform objects and the transform became
 * an identity, the TFTransform object will be removed.
 *
 * Selection will be updated to the TFTransform objected added/removed.
 *
 * FIXME?: how do we handle TFigureEditor::relatedTo???
 */
void
TFigureModel::pureTransform(TFigureSet *selection, const TMatrix2D &matrix)
{
cout << "TFigureModel::pureTransform()" << endl;
cout << "  number of figures: " << selection->size() << endl;
  TFigureSet addTransform, removeTransform;
  
  for(auto &&figure: *selection) {
    TFTransform *transform = dynamic_cast<TFTransform*>(figure);
    if (transform) {
      transform->matrix = matrix * transform->matrix;
      if (transform->matrix.isIdentity()) {
        removeTransform.insert(figure);
      }
    } else
    if (!figure->transform(matrix)) {
      addTransform.insert(figure);
    }
  }

cout << "  number of new TFTransforms: " << addTransform.size() << endl;
for(auto &&p: addTransform)
  cout << "    " << p << endl;

  TFigureSet replace(addTransform);
  replace.insert(removeTransform.begin(), removeTransform.end());

cout << "  number of replacements : " << replace.size() << endl;

  TFigureAtDepthList replaceAtDepth;
  pureErase(replace, &replaceAtDepth);

cout << "  actually to be replaced: " << replaceAtDepth.size() << endl;

  for(auto &&place: replaceAtDepth) {
cout << "  replace " << place.figure << endl;
    if (addTransform.contains(place.figure)) {
cout << "    insert TFTransform" << endl;
      TFTransform *transform = new TFTransform();
      transform->matrix = matrix;
      transform->figure = place.figure;
      place.figure = transform;
      selection->erase(transform->figure);
      selection->insert(transform);
    } else {
cout << "    remove TFTransform" << endl;
      TFTransform *transform = dynamic_cast<TFTransform*>(place.figure);
      place.figure = transform->figure;
      selection->erase(transform);
      selection->insert(transform->figure);
      delete transform;
    }
  }
  pureInsert(replaceAtDepth); // insert: putBack
  replaceAtDepth.drop(); // do not delete the figures FIXME: TFigureAtDepthList should not take ownership, the undo events should
}

void
TFigureModel::translate(TFigureSet *selection, const TPoint &p)
{
  TMatrix2D matrix;
  matrix.translate(p);
  transform(selection, matrix);
}

/*****************************************************************************
 *                                                                           *
 *                         T R A N S L A T E H A N D L E                     *
 *                                                                           *
 *****************************************************************************/


/**
 * Translate a figures handle.
 *
 * This function notifies all its views about the modification and
 * registers an undo object.
 *
 * \param figure
 *   The figure to be notified.
 * \param handle
 *   The figures handle to be modified.
 * \param x
 *   horizontal position of the figures handle
 * \param y
 *   vertical position of the figures handle
 *
 * NOTE: Not using a TMouseEvent for (x, y, modifier) is intentional as only these values
 *       are required for handle modification (by now) and that TUndoTranslateHandle only
 *       stores these values (by now).
 */
void
TFigureModel::translateHandle(TFigure *figure, unsigned handle, TCoord x, TCoord y, unsigned modifier)
{
  if (x==0 && y==0)
    return;
  
  figures.clear();
  figures.insert(figure);
  
  type = MODIFY;
  sigChanged();

  TPoint p;
  figure->getHandle(handle, &p);
  
  figure->translateHandle(handle, x, y, modifier);
  
  type = MODIFIED;
  sigChanged();
  
  TUndoTranslateHandle *undo = new TUndoTranslateHandle(this, figure, handle, x, y, modifier);
  TUndoManager::registerUndo(this, undo);
}

/**
 * Tell the figure to start in-place-editing after a double click.
 *
 * For example: A text figure will begin to display a cursor or a
 * file browser will try to load or execute the file.
 *
 * \param figure
 *   The figure which will receive the message.
 * \param fe
 *   Sometimes the figure may need to know which figure editor caused
 *   the event. May be NULL.
 * \return
 *   'true' when the figure wants to start the in-place-editing mode.
 *   It will then receive keyboard events.
 */
bool
TFigureModel::startInPlace(TFigure *figure, TFigureEditor *fe) {
  TFigureEditEvent ee;
  ee.model = this;
  ee.editor = fe;
  ee.type = TFigureEditEvent::START_IN_PLACE;
  return figure->editEvent(ee);
}

/**
 * Group a given set of figures.
 *
 * This function notifies all its views about the modification and
 * registers an undo object.
 *
 * \param set
 *   A list of figures to group
 * \return
 *   The new group object or NULL in case the group would have only
 *   contained one or no objects.
 */
TFigure*
TFigureModel::group(TFigureSet &set)
{
  if (set.size()<2)
    return 0;
    
  TUndoGroup *undo = new TUndoGroup(this);
  
  unsigned count = 0;
  unsigned depth = 0;
  TFGroup *group = new TFGroup();
  undo->setGroup(group);
  TStorage::iterator last;
  for(TStorage::iterator p = storage.begin();
      p!=storage.end(); ++depth)
  {
    if (set.find(*p)!=set.end()) {
      group->gadgets.add(*p);
      undo->insert(*p, depth);
      unsigned pi = p - storage.begin();
      storage.erase(p);
      last = p = storage.begin() + pi;
      ++count;
    } else {
      ++p;
    }
  }
  if (count<2) {
    cerr << "count < 2" << endl;
    // delete group;
    // return 0;
  }
  
  TUndoManager::registerUndo(this, undo);
  
  group->calcSize();
  storage.insert(last, group);
  
  type = GROUP;
  figures.clear();
  figure = group;
  sigChanged();
  
  return group;
}

TFigure*
TFigureModel::transform(TFigureSet &set, TFPerspectiveTransform *transform)
{
//  TUndoGroup *undo = new TUndoGroup(this);
  
  unsigned count = 0;
  unsigned depth = 0;
//  TFGroup *group = new TFGroup();
//  undo->setGroup(group);
  TStorage::iterator last;
  for(TStorage::iterator p = storage.begin();
      p!=storage.end(); ++depth)
  {
    if (set.find(*p)!=set.end()) {
      transform->add(*p);
//      undo->insert(*p, depth);
      unsigned pi = p - storage.begin();
      storage.erase(p);
      last = p = storage.begin() + pi;
      ++count;
    } else {
      ++p;
    }
  }
  
//  TUndoManager::registerUndo(this, undo);
  
  transform->init();
  storage.insert(last, transform);
  
  type = GROUP;
  figures.clear();
  figure = transform;
  sigChanged();
  
  return transform;
}

/**
 * Ungroup a given set of figures.
 *
 * This function notifies all its views about the modification and
 * registers an undo object.
 *
 * \param grouped
 *    A set of figures from which all groups will be ungrouped.
 * \param ungrouped
 *    A pointer to set a of figures, which will take all non-groups
 *    and ungrouped groups from 'grouped'.
 *
 * grouped and ungrouped can be the same object.
 */
void
TFigureModel::ungroup(TFigureSet &grouped, TFigureSet *ungrouped)
{
  TFigureSet memo;
  for(TStorage::iterator p = storage.begin();
      p != storage.end();
      ++p)
  {
    if (grouped.find(*p)!=grouped.end()) {
      TFGroup *group = dynamic_cast<TFGroup*>(*p);
      if (group) {
        int pi = p - storage.begin();
        storage.erase(p);
        p = storage.begin() + pi;

        storage.insert(p, group->gadgets.storage.begin(), group->gadgets.storage.end());
        p = storage.begin() + pi + group->gadgets.storage.size()-1;
        memo.insert(group->gadgets.begin(), group->gadgets.end());
        group->gadgets.erase(group->gadgets.begin(),group->gadgets.end());
        delete group;
      } else {
        memo.insert(*p);
      }
    }
  }
  ungrouped->clear();
  ungrouped->insert(memo.begin(), memo.end());
  
  type = UNGROUP;
  figures.clear();
  figures.insert(memo.begin(), memo.end());
  sigChanged();
}

class TUndoAttributes:
  public TUndo
{
    TFigureModel *model;
    struct TNode {
      TFigure *figure;
      TFigureAttributeModel attributes;
      TNode *next;
    };
    TNode *list;
  public:
    TUndoAttributes(TFigureModel *model) {
      this->model = model;
      list = 0;
    }
    ~TUndoAttributes() {
      while(list) {
        TNode *n = list;
        list = list->next;
        delete n;
      }
    }
    void insert(TFigure *f) {
      TNode *node = new TNode;
      node->figure = f;
      f->getAttributes(&node->attributes);
      node->attributes.setAllReasons();
      node->next = list;
      list = node;
    }
  protected:
    void undo() {
      TUndoAttributes *undo = new TUndoAttributes(model);
      
      model->figures.clear();
      TNode *node = list;
      while(node) {
        model->figures.insert(node->figure);
        node = node->next;
      }
      model->type = TFigureModel::MODIFY;
      model->sigChanged();
      
      node = list;
      while(node) {
        undo->insert(node->figure);
        node->figure->setAttributes(&node->attributes);
        node = node->next;
      }
      TUndoManager::registerUndo(model, undo);

      model->type = TFigureModel::MODIFIED;
      model->sigChanged();
    }
    bool getUndoName(string *name) const {
      *name = "Undo: Figure change";
      return true;
    }
    bool getRedoName(string *name) const {
      *name = "Redo: Figure change";
      return true;
    }
};

void
TFigureModel::setAttributes(TFigureSet &set, const TFigureAttributeModel *attributes)
{
  if (set.empty())
    return;

  figures.clear();
  figures.insert(set.begin(), set.end());

  type = MODIFY;
  sigChanged();
  
  TUndoAttributes *undo = new TUndoAttributes(this);

  for(TFigureSet::iterator p=set.begin();
      p!=set.end();
      ++p)
  {
    undo->insert(*p);
    (*p)->setAttributes(attributes);
  }
  
  TUndoManager::registerUndo(this, undo);

  type = MODIFIED;
  sigChanged();
}

/**
 * Kludge for TUndoGroup (to be renamed into ungroup)
 */
void
TFigureModel::_undoGroup(TFGroup *group, TFigureAtDepthList &store)
{
  type = _UNDO_GROUP;
  figure = group;
  figures.clear();
  
  TUndoUngroup *undo = new TUndoUngroup(this);
  
  for(TFigureAtDepthList::TStore::iterator p = store.store.begin();
      p!=store.store.end();
      ++p)
  {
    storage.insert(
      storage.begin() + p->depth,
      p->figure);
    figures.insert(p->figure);
    undo->insert(p->figure);
  }
  sigChanged();

  for(TStorage::iterator p=storage.begin();
      p!=storage.end();
      ++p)
  {
    if (*p == group) {
      storage.erase(p);
      break;
    }
  }
  TUndoManager::registerUndo(this, undo);
}


void
TFigureModel::insert(const iterator &p, TFigure *g)
{
  type = MODIFIED;
  sigChanged();
  storage.insert(p, g);
}

void
TFigureModel::insert(const iterator &at, const iterator &from, const iterator &to)
{
  type = MODIFIED;
  sigChanged();
  storage.insert(at, from, to);
}

/**
 * Remove all figures from the model.
 *
 * This method doesn't create an undo object yet...
 */
void
TFigureModel::clear()
{
  type = MODIFIED;
  sigChanged();
//  cerr << "clear TFigureModel " << this << endl;
  TStorage::iterator p, e;
  p = storage.begin();
  e = storage.end();
  while(p!=e) {
    delete *p;
    ++p;
  }
  storage.erase(storage.begin(), storage.end());
}

void
TFigureModel::store(TOutObjectStream &out) const
{
  TStorage::const_iterator p, e;
  p = storage.begin();
  e = storage.end();
  while(p!=e) {
    ::store(out, *p);
    ++p;
  }
}

bool
TFigureModel::restore(TInObjectStream &in)
{
  switch(in.what) {
    case ATV_START:
      clear();
      return true;
    case ATV_GROUP: {
        TSerializable *s = in.clone(in.type);
        if (!s)
          return false;
//        cerr << "okay got " << s->name() << endl;
        TFigure *g = dynamic_cast<TFigure*>(s);
        if (!g) {
//          cerr << "  wasn't a gadget" << endl;
          delete s;
          return false;
        }
//        cerr << "adding new gadget to TFigureModel " << this << endl;
        storage.push_back(g);
//        cerr << "new storage size is " << storage.size() << endl;
        in.setInterpreter(s);
        return true;
      }
      break;
    case ATV_FINISHED:
//      cerr << "restored TFigureModel of size " << storage.size() << endl;
      type = MODIFIED;
      sigChanged();
      return true;
  }
  return false;
}
