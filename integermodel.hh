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

#ifndef _TOAD_INTEGERMODEL_HH
#define _TOAD_INTEGERMODEL_HH

#include <toad/textmodel.hh>
#include <iostream>

namespace toad {

using namespace std;

class TIntegerModel:
  public TModel
{
  protected:
    int minimum;
    int maximum;
    int value;
    int extent;
    
    bool adjusting;

    /** true, when the last setValue call had an out of range value */    
    bool out_of_range;

    virtual void changed();

  public:
    TIntegerModel();
    
    int getExtent() const { return extent; }
    int getMaximum() const { return maximum; }
    int getMinimum() const { return minimum; }
    int getValue() const { return value; }
    // name inconsistence: isAdjusting() const would be right
    bool getValueIsAdjusting() const { return adjusting; }
    
    void setExtent(int extent);
    void setMaximum(int max);
    void setMinimum(int min);
    void setRangeProperties(int value, int extent, int min, int max, bool adjusting=false);
    void setValue(int value);
    void setValueIsAdjusting(bool b);
    
    bool wasOutOfRange() const { return out_of_range; }
    
    TIntegerModel& operator=(const TIntegerModel &m) {
      setValue(m.value);
      return *this;
    }
    TIntegerModel& operator=(int v) {
      setValue(v);
      return *this;
    }
    operator int() const {
      return value;
    }
};

inline ostream& operator<<(ostream &s, const TIntegerModel &m) {
  return s<<m.getValue();
}

typedef GSmartPointer<TIntegerModel> PIntegerModel;

TTextModel * createTextModel(TIntegerModel *);

} // namespace toad

bool restore(atv::TInObjectStream &in, toad::TIntegerModel *value);

#endif
