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

#ifndef __TOAD_ACTION_HH
#define __TOAD_ACTION_HH 1

#include <toad/window.hh>
#include <toad/connect.hh>
#include <toad/model.hh>
#include <set>

namespace toad {

class TAction;
class TBitmap;

class TActionStorage:
  protected set<TAction*>
{
    typedef set<TAction*> super;
  public:
    typedef super::iterator iterator;
    typedef super::const_iterator const_iterator;
    iterator begin() { return super::begin(); }
    iterator end() { return super::end(); }
    void push_back(TAction *a) { super::insert(a); sigChanged(); }
    void erase(TAction *a) {
      iterator p = super::find(a);
      if (p!=super::end()) {
        super::erase(p);
        sigChanged();
      }
    }
    TSignal sigChanged;
};

class TAction:
  public TInteractor
{
  public:
    enum EActivation {
      ALWAYS,
      DOMAIN_FOCUS,
      PARENT_FOCUS
    };
    enum EType { // FIXME: remove type from action, create 3 different classes instead?
      BUTTON,
      CHECKBUTTON,
      RADIOBUTTON
    };

    static TActionStorage actions;

    TAction(TInteractor *, const string&, EType type = BUTTON, EActivation activation = PARENT_FOCUS);
    virtual ~TAction();
    
    // enable/disable
    void setEnabled(bool b);
    bool isEnabled() const;
    
    //! this signal is triggered when the action has to be performed
    TSignal sigClicked;
    
    virtual bool trigger(unsigned idx=0);
    virtual bool delayedTrigger(unsigned idx=0);

    //! the status of the action (enabled/disabled) has changed
    TSignal sigChanged;
    
    EType type;
    
    virtual unsigned getSize() const;
    virtual const string& getID(unsigned idx) const;
    virtual unsigned getSelection() const { return 0; }
    virtual bool getState(string *text, bool *active) const { return false; }
    void setActivationType(EActivation a) { activation = a; }
    EActivation getActivationType() const { return activation; }

  protected:
    void focus(bool);
    void domainFocus(bool);

  private:
    TBitmap *bitmap;
    bool has_focus:1;
    bool has_domain_focus:1;
    bool enabled:1;
    EActivation activation:2;
//    EType type:3;
};

class TChoiceModel:
  public TModel
{
  public:
    TChoiceModel() {
      idx = 0;
    }
    virtual void select(unsigned idx) {
      // cout << "selected " << data[idx]->id << endl;
      this->idx = idx;
      sigChanged();
    }
    unsigned getSelection() const {
      return idx;
    }
    TSignal sigChanged;
  protected:
    unsigned idx;
};

/**
 * Class to hold the data for a choice.
 */
template <class T>
class GChoiceModel:
  public TChoiceModel
{
  public:
    void add(const string &id, T what) {
      TData *d = new TData();
      d->id = id;
      d->what = what;
      data.push_back(d);
    }
    unsigned getSize() const { return data.size(); }
    const string& getID(unsigned idx) const { return data[idx]->id; }
    const T& getValue() const { return data[idx]->what; }
    const T& getValue(unsigned idx) const { return data[idx]->what; }
    void setValue(T value) {
      if (value==getValue())
        return;
      unsigned idx=0;
      for(auto &&p: data) {
        if (p->what==value) {
          select(idx);
          break;
        }
        ++idx;
      }
    }
    
  protected:
    struct TData {
      string id;
      T what;
    };
    static const TData empty;
    vector<TData*> data;
};

class TAbstractChoice:
  public TAction
{
  public:
    TAbstractChoice(TWindow *parent,
                    const string &title,
                    EType type = BUTTON,
                    EActivation activation = PARENT_FOCUS)
      :TAction(parent, title, type, activation)
    {}

    virtual ~TAbstractChoice();
    virtual TChoiceModel* getModel() = 0;
};

/**
 * Class to map a choice model into the display.
 */
template <class T>
class GChoice:
  public TAbstractChoice
{
  public:
    GChoiceModel<T> *model;
    TSignalLink *signalLink;

    GChoice<T>(TWindow *p, 
               const string &id,
               GChoiceModel<T> *m=new GChoiceModel<T>())
      :TAbstractChoice(p, id, TAction::RADIOBUTTON), model(m)
    {
      signalLink = connect(model->sigChanged, [=] {
        this->sigChanged();
      });
      // FIXME: TAction already added this object, but then the type was still TAction
      //        update when GChoice isn't a subclass of TAction anymore
      TAction::actions.sigChanged();
    }
    
    ~GChoice<T>() {
      // FIXME: this must be easier, update when TSlot is finished
      model->sigChanged.remove(signalLink);
    }
    
    TChoiceModel* getModel() { return model; }
  
    void add(const string &id, T what) { model->add(id, what); }
    const T& getValue() const { return model->getValue(); }
    void setValue(T t) { model->setValue(t); }
    unsigned getSize() const { return model->getSize(); }
    virtual const string& getID(unsigned idx) const { return model->getID(idx); }
    virtual void select(unsigned idx=0) {
      model->select(idx);
      sigClicked();
    }
    virtual bool trigger(unsigned idx=0)
    {
      if (!isEnabled())
        return false;
      model->select(idx);
      return sigClicked.trigger();
    }
    virtual bool delayedTrigger(unsigned idx=0)
    {
      if (!isEnabled())
        return false;
      model->select(idx);
      return sigClicked.delayedTrigger();
    }
    virtual unsigned getSelection() const { return model->getSelection(); }
};

}

#endif
