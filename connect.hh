/* DO NOT EDIT - THIS IS A GENERATED FILE (EDIT CONNECT.HH.PM!)
 *
 * TOAD -- A Simple and Powerful C++ GUI Toolkit for the X Window System
 * Copyright (C) 1996-2004 by Mark-André Hopf <mhopf@mark13.org>
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


#ifndef TSignal
#define TSignal TSignal

#include <cstdlib>
#include <functional>

namespace toad {  // connect.hh

/**
 * \ingroup callback
 *
 * A simplified variant of 'connect' which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * Example:
 * \code
   struct MyClass {
     void print(int a) {
       cerr << a << endl;
     }
   };
   
   MyClass *o = new TMyClass();
   TSignal sig;

   CONNECT(sig, o, print, 1);

   sig();
   \endcode
 * 
 * \param S A TSignal object.
 * \param O An object which owns a method to be called.
 * \param M The objects method to be invoked when the signal is triggered.
 * \param ARGS... 
 *   Zero or more arguments which will be used when the method is
 *   invoked.
 * 
 */
#define CONNECT(S, O, M, ARGS...) { typedef typeof(*O) T; connect(S, O, &T::M , ## ARGS ); }

/**
 * \ingroup callback
 *
 * Remove all connections assigned to a signal.
 *
 * \param S The TSignal.
 */
#define DISCONNECT_ALL(S) S.remove();

/**
 * \ingroup callback
 *
 * Remove all connections to a certain object from a signal.
 *
 * \param S
 *   The TSignal.
 * \param O
 *   The object to whose connections shall be removed.
 */
#define DISCONNECT_OBJ(S, O) S.remove((void*)O);

/**
 * \ingroup callback
 *
 * Remove all connections for a certain function from a signal.
 *
 * \param S
 *   The TSignal.
 * \param O
 *   A pointer to the function to be removed.
 */
#define DISCONNECT_FNC(S, O) S.remove((void*)O);

/**
 * \ingroup callback
 *
 * A simplified variant of 'disconnect' which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * Removes a certain method from a signal.
 *
 * \param S
 *   The TSignal.
 * \param O
 *   The Object.
 * \param M
 *   The objects method to be removed.
 */
#define DISCONNECT(S, O, M) { typedef typeof(*O) T; S.remove((void*)O, (TSignalLink::TMethod)&T::M); }
  
/**
 * \ingroup callback
 *
 * Objects of this type are needed to remove single callbacks from a signal.
 *
 * Usually returned when a callback is connected to a signal.
 */
class TSignalLink
{
    class TClass;
  public:
    typedef void(TClass::*TMethod)(void);
    TSignalLink();
    virtual ~TSignalLink();
    virtual void execute() = 0;
    virtual void* objref();
    virtual TMethod metref();
    TSignalLink *next;
    bool lock:1;
    bool dirty:1;    
};

class TClosure: public TSignalLink
{
  public:
    std::function<void()> cb;
    TClosure(std::function<void()> c):cb(c) {}
    virtual void execute() { cb(); }
};

/**
 * \ingroup callback
 *
 * This class is the base to store pointers to callback functions.
 */
class TSignal
{
    /* this class isn't itended to be copied */
    TSignal(const TSignal&) {};
    TSignal& operator=(const TSignal&) {return *this;}
  public:
    TSignal();
    ~TSignal();
    TSignalLink* add(std::function<void()> cb) { return add(new TClosure(cb)); }
    TSignalLink* add(TSignalLink*);
		bool isConnected() const { return _list!=NULL; }
    void remove();
    void remove(TSignalLink*);
    void remove(void*);
    void remove(void(*)(void));
    void remove(void*, TSignalLink::TMethod);
    bool trigger();
    bool delayedTrigger();
    bool operator()() { return trigger(); }
    
    void lock();
    void unlock();
    void print();
#ifdef TOAD_SECURE
    unsigned delayedtrigger;
#endif
  protected:
    TSignalLink *_list;
};

/**
 * \ingroup callback
 *
 * Remove all callbacks from the signal.
 */
inline void disconnect(TSignal &s) {
	s.remove();
}

/**
 * \ingroup callback
 *
 * Remove the given link from the signal.
 */
template <class T>
inline void disconnect(TSignal &s, T *n) {
	s.remove(n);
}

static inline TSignalLink* connect(TSignal &s, std::function<void()> c) {
  return s.add(c);
}   

// help template for connect_value, connect_value_of, ...
//-------------------------------------------------------
template <class S, class D>
class GSignalCodeLink:
	public TSignalLink
{
	protected:
		typedef GSignalCodeLink<S,D> TThis;
		typedef S* TS;
		typedef D* TD;
		TS src;
		TD dst;
	public:
		GSignalCodeLink(TS s, TD d): src(s), dst(d) {}
};

/**
 * \ingroup callback
 *
 * A simplified variant of 'connect_value_of' for the GNU C++ Compiler.
 *
 * Connect method with method/attribut:
 * \pre
 * CONNECT_VALUE_OF(signal,
 *   destination object, destination method,
 *   source object, source method or attribut)
 * \endpre
 */
#define CONNECT_VALUE_OF(SIG, D, DM, S, SM)\
{\
	typedef typeof(S) TS;\
	typedef typeof(D) TD;\
	class unnamed:\
	  public TSignalLink\
	{\
  		TS src;\
  		TD dst;\
		public:\
      unnamed(TS s, TD d) {\
        src = s;\
        dst = d;\
      }\
			void execute() {\
				this->dst->DM(this->src->SM);\
			}\
	};\
	SIG.add(new unnamed(source, D));\
}

/**
 * \ingroup callback
 *
 * Add a callback that takes one parameter. The value for this parameter
 * is delivered from another callback.
 *
 * \param sig Signal 
 * \param d     Destination's object of the call.
 * \param dm    Destination's method of the call which takes one parameter.
 * \param s     Source's object with a delivering the parameter.
 * \param sm    Source's method delivering the parameter when calling the
 *              destination method.
 */
template <class D, class S, class R, class V>
TSignalLink* connect_value_of(TSignal &sig, D *d, R(D::*dm)(V), S *s, V(S::*sm)())
{
	struct unnamed:
		public GSignalCodeLink<S, D>
	{
	    typedef GSignalCodeLink<S, D> TThis;
			typedef R(D::*DM)(V);
			typedef V(S::*SM)();
      typedef typename GSignalCodeLink<S, D>::TD TD;
      typedef typename GSignalCodeLink<S, D>::TS TS;
			DM _dm;
			SM _sm;
		public:
			unnamed(TD d, DM dm, TS s, SM sm): TThis(s,d), _dm(dm), _sm(sm) {}
			void execute() {
				(this->dst->*_dm)((this->src->*_sm)());
			}
	};
	return sig.add(new unnamed(d, dm, s, sm));
}

// connect with value
/**
 * \ingroup callback
 *
 * Add a callback that takes one parameter. The value for this parameter
 * is delivered from a variable.
 *
 * \param sig Signal
 * \param d     Destination's object of the call.
 * \param dm    Destination's method of the call which takes one parameter.
 * \param v     Pointer to the variable for the parameter.
 */
template <class D, class R, class V>
TSignalLink* connect_value_of(TSignal &sig, D *d, R(D::*dm)(V), V *v)
{
	struct unnamed:
		public TSignalLink
	{
			typedef R(D::*DM)(V);
			D *d;
			DM dm;
			V *v;
		public:
			unnamed(D *d, DM dm, V *v): d(d), dm(dm), v(v) {}
			void execute() {
				(d->*dm)(*v);
			}
	};
	return sig.add(new unnamed(d, dm, v));
}


/**
 * \ingroup callback
 *
 * A simplified variant of 'connect_value' for the GNU C++ Compiler.
 *
 * Connect method with `getValue()' method:
 * \pre
 * CONNECT_VALUE(signal,
 *   destination object, destination method,
 *   source object)
 * \endpre
 */
#define CONNECT_VALUE(SIG, D, DM, S)\
{\
	typedef typeof(*D) TD;\
	connect_value(SIG, D, &TD::DM, S);\
}

/**
 * \ingroup callback
 *
 * Add a callback that takes one parameter. The value for this parameter
 * is delivered from another objects 'getValue()' method.
 *
 * \param sig Signal 
 * \param d     Destination's object of the call.
 * \param dm    Destination's method of the call which takes one parameter.
 * \param s     Source's object whose 'getValue()' method will be called to
 *              deliver the parameter.
 */
template <class D, class S, class R, class V>
TSignalLink* connect_value(TSignal &sig, D *d, R(D::*dm)(V), S *s)
{
	struct unnamed:
		public GSignalCodeLink<S, D>
	{
	    typedef GSignalCodeLink<S, D> TThis;
			typedef R(D::*DM)(V);
			typedef typename GSignalCodeLink<S, D>::TD TD;
			typedef typename GSignalCodeLink<S, D>::TS TS;
			DM _dm;
		public:
			unnamed(TD d, DM dm, TS s): TThis(s,d), _dm(dm) {}
			void execute() {
				(this->dst->*_dm)(this->src->getValue());
			}
	};
	return sig.add(new unnamed(d, dm, s));
}

/**
 * \ingroup callback
 *
 * This macro provides a substitute for 'closures' known from Smalltalk.
 *
 * Example:
 * \code
void foo(int a) {
  cerr << a << endl;
}

int main() {
  TSignal sig;

  CLOSURE(sig,
    foo(8);
  )

  sig();
}
\endcode
 *
 * \param SIG TSignal which will invokes the closure.
 * \param DEF A code block which defines the closure.
 * \sa TCLOSURE
 */
#define CLOSURE(SIG, DEF) \
{ struct closure { \
  static void __f() { DEF } \
}; \
connect(SIG, closure::__f); }

/**
 * \ingroup callback
 *
 * This macro provides a substitute for 'closures' known from Smalltalk.
 *
 * Example:
 * \code
int
foo(int a) {
  cerr << a << endl;
}

int main() {
  TSignal sig;

  CLOSURE1(sig, int a, 17,
    foo(a);
  );

  sig();
}\endcode
 *
 * \param SIG TSignal which will invoke the closure.
 * \param P  Type and name of variable inside closure.
 * \param V  Value for P
 * \param DEF An argument list with one argument and code block.
 * \sa TCLOSURE1
 */
#define CLOSURE1(SIG, P, V, DEF) \
{ struct closure { \
  static void __f(P) { DEF } \
}; \
connect(SIG, closure::__f, V); }

/**
 * \ingroup callback
 *
 * This macro provides a substitute for 'closures' known from Smalltalk.
 *
 * Example:
 * \code
int bar(int a, int b) {
  cerr << a << ", " << b << endl;
}

int main() {
  TSignal sig;

  CLOSURE2(sig, int a, 20, int b, 25,
    bar(a, b);
  )

  sig();
}\endcode
 *
 * \param SIG TSignal which will invoke the closure.
 * \param P1  Type and name of variable inside closure.
 * \param V1  Value for P1
 * \param P2  Type and name of variable inside closure.
 * \param V2  Value for P2
 * \param DEF An argument list with two arguments and code block.
 * \sa TCLOSURE2
 */
#define CLOSURE2(SIG, P1, V1, P2, V2, DEF) \
{ struct closure { \
  static void __f(P1, P2) { DEF } \
}; \
connect(SIG, closure::__f, V1, V2); }

/**
 * \ingroup callback
 *
 * This macro provides a substitute for 'closures' known from Smalltalk.
 *
 * \param SIG TSignal which will invoke the closure.
 * \param P1  Type and name of variable inside closure.
 * \param V1  Value for P1
 * \param P2  Type and name of variable inside closure.
 * \param V2  Value for P2
 * \param P3  Type and name of variable inside closure.
 * \param V3  Value for P3
 * \param DEF An argument list with two arguments and code block.
 * \sa CLOSURE2
 */
#define CLOSURE3(SIG, P1, V1, P2, V2, P3, V3, DEF) \
{ struct closure { \
  static void __f(P1, P2, P3) DEF \
}; \
connect(SIG, closure::__f, V1, V2, V3); }

/**
 * \ingroup callback
 *
 * This macro provides a substitute for 'closures' known from Smalltalk.
 *
 * \param SIG TSignal which will invoke the closure.
 * \param P1  Type and name of variable inside closure.
 * \param V1  Value for P1
 * \param P2  Type and name of variable inside closure.
 * \param V2  Value for P2
 * \param P3  Type and name of variable inside closure.
 * \param V3  Value for P3
 * \param P4  Type and name of variable inside closure.
 * \param V4  Value for P4
 * \param DEF An argument list with two arguments and code block.
 * \sa CLOSURE2
 */
#define CLOSURE4(SIG, P1, V1, P2, V2, P3, V3, P4, V4, DEF) \
{ struct closure { \
  static void __f(P1, P2, P3, P4) DEF \
}; \
connect(SIG, closure::__f, V1, V2, V3, V4); }

/**
 * \ingroup callback
 *
 * A simplified variant of CLOSURE which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * Example:
 * \code
void foo(int a) {
  cerr << a << endl;
}

int main() {
  TSignal sig;

  TCLOSURE(sig,
    foo(8);
  )

  sig();
}\endcode
 *
 * Code Connection
 * \param SIG TSignal which will invokes the closure.
 * \param DEF A code block which defines the closure.
 * \sa CLOSURE
 */
#define TCLOSURE(SIG, DEF) \
{ struct closure { \
  static void __f() { DEF } \
}; \
connect(SIG, closure::__f); }

/**
 * \ingroup callback
 *
 * A simplified variant of CLOSURE1 which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * Example:
 * \code
int foo(int a) {
  cerr << a << endl;
}

int main() {
  TSignal sig;

  TCLOSURE1(sig, a, f(),
    foo(a);
  )

  sig();

}\endcode
 *
 * Code Connection
 * \param SIG TSignal which will invokes the closure.
 * \param P  Name of variable inside closure.
 * \param V  Value for P.
 * \param DEF A code block which defines the closure.
 * \sa CLOSURE1
 */
#define TCLOSURE1(SIG, P, V, DEF) \
{ typedef typeof(V) __t1; \
  struct closure { \
    static void __f(__t1 P) { DEF } \
  }; \
  connect(SIG, closure::__f, V); }

/**
 * \ingroup callback
 *
 * A simplified variant of CLOSURE2 which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * Example:
 * \code
int bar(int a, int b) {
  cerr << a << ", " << b << endl;
}

int main() {
  TSignal sig;

  TCLOSURE2(sig, 
            a, f(),
            b, g(),
    bar(a, b);
  )

  sig();
}\endcode
 *
 * Code Connection
 * \param SIG TSignal which will invokes the closure.
 * \param P1  Name of variable inside closure.
 * \param V1  Value for P1.
 * \param P2  Name of variable inside closure.
 * \param V2  Value for P2.
 * \param DEF A code block which defines the closure.
 * \sa CLOSURE2
 */
#define TCLOSURE2(SIG, P1, V1, P2, V2, DEF) \
{ typedef typeof(V1) __t1; \
  typedef typeof(V2) __t2; \
  struct closure { \
    static void __f(__t1 P1, __t2 P2) { DEF } \
  }; \
  connect(SIG, closure::__f, V1, V2); }

/**
 * \ingroup callback
 *
 * A simplified variant of CLOSURE3 which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * \param SIG TSignal which will invokes the closure.
 * \param P1  Name of variable inside closure.
 * \param V1  Value for P1.
 * \param P2  Name of variable inside closure.
 * \param V2  Value for P2.
 * \param P3  Name of variable inside closure.
 * \param V3  Value for P3.
 * \param DEF A code block which defines the closure.
 * \sa TCLOSURE2
 */
#define TCLOSURE3(SIG, P1, V1, P2, V2, P3, V3, DEF) \
{ typedef typeof(V1) __t1; \
  typedef typeof(V2) __t2; \
  typedef typeof(V3) __t3; \
  struct closure { \
    static void __f(__t1 P1, __t2 P2, __t3 P3) { DEF } \
  }; \
  connect(SIG, closure::__f, V1, V2, V3); }

/**
 * \ingroup callback
 *
 * A simplified variant of CLOSURE4 which requires a C++ compiler
 * with a 'typeof' operator like GNU C++.
 *
 * \param SIG TSignal which will invokes the closure.
 * \param P1  Name of variable inside closure.
 * \param V1  Value for P1.
 * \param P2  Name of variable inside closure.
 * \param V2  Value for P2.
 * \param P3  Name of variable inside closure.
 * \param V3  Value for P3.
 * \param P4  Name of variable inside closure.
 * \param V4  Value for P4.
 * \param DEF A code block which defines the closure.
 * \sa TCLOSURE2
 */
#define TCLOSURE4(SIG, P1, V1, P2, V2, P3, V3, P4, V4, DEF) \
{ typedef typeof(V1) __t1; \
  typedef typeof(V2) __t2; \
  typedef typeof(V3) __t3; \
  typedef typeof(V4) __t4; \
  struct closure { \
    static void __f(__t1 P1, __t2 P2, __t3 P3, __t4 P4) { DEF } \
  }; \
  connect(SIG, closure::__f, V1, V2, V3, V4); }

/*
 * various signal nodes & connect's
 */
template <class A>
class GSignalLinkF0:
  public TSignalLink
{
  protected:
    A(*_m)(void);
   public:
    GSignalLinkF0(A(*m)(void)):_m(m) {}
    void execute() { (*_m)(); }
};

/**
 * \ingroup callback
 */
template <class R>
TSignalLink* connect(TSignal &s, R(*m)(void)) {
  return s.add(new GSignalLinkF0<R>(m));
}

/**
 * \ingroup callback
 */
template <class R>
void disconnect(TSignal &s, R(*m)(void)) {
	s.remove( (void(*)())m );
}

template <class A, class T>
class GSignalLinkO0:
  public TSignalLink
{
  protected:
    A(T::*_m)(void);
    T *_o;
   public:
    GSignalLinkO0(T *o, A(T::*m)(void)):_m(m), _o(o) {}
    void execute() { (_o->*_m)(); }
    virtual void* objref() {return (void*)_o;}
    virtual TMethod metref() {return (TMethod)_m;}
};

/**
 * \ingroup callback
 */
template <class R, class T1, class T2>
TSignalLink* connect(TSignal &s, T1 *o, R(T2::*m)(void)) {
  return s.add(new GSignalLinkO0<R, T1>(o, m));
}

/**
 * \ingroup callback
 */
template <class R, class T1, class T2>
void disconnect(TSignal &s, T1 *o, R(T2::*m)(void)) {
	T2 *co=o; s.remove( (void*)o, (TSignalLink::TMethod)m);
}

template <class A, class U1>
class GSignalLinkF1:
  public TSignalLink
{
  protected:
    A(*_m)(U1);
    U1 _p1;
   public:
    GSignalLinkF1(A(*m)(U1), U1 p1):_m(m), _p1(p1) {}
    void execute() { (*_m)(_p1); }
};

/**
 * \ingroup callback
 */
template <class R, class U1, class V1>
TSignalLink* connect(TSignal &s, R(*m)(U1), V1 p1) {
  return s.add(new GSignalLinkF1<R, U1>(m, p1));
}

/**
 * \ingroup callback
 */
template <class R, class U1>
void disconnect(TSignal &s, R(*m)(U1)) {
	s.remove( (void(*)())m );
}

template <class A, class T, class U1>
class GSignalLinkO1:
  public TSignalLink
{
  protected:
    A(T::*_m)(U1);
    T *_o;
    U1 _p1;
   public:
    GSignalLinkO1(T *o, A(T::*m)(U1), U1 p1):_m(m), _o(o), _p1(p1) {}
    void execute() { (_o->*_m)(_p1); }
    virtual void* objref() {return (void*)_o;}
    virtual TMethod metref() {return (TMethod)_m;}
};

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class V1>
TSignalLink* connect(TSignal &s, T1 *o, R(T2::*m)(U1), V1 p1) {
  return s.add(new GSignalLinkO1<R, T1, U1>(o, m, p1));
}

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1>
void disconnect(TSignal &s, T1 *o, R(T2::*m)(U1)) {
	T2 *co=o; s.remove( (void*)o, (TSignalLink::TMethod)m);
}

template <class A, class U1, class U2>
class GSignalLinkF2:
  public TSignalLink
{
  protected:
    A(*_m)(U1, U2);
    U1 _p1; U2 _p2;
   public:
    GSignalLinkF2(A(*m)(U1, U2), U1 p1, U2 p2):_m(m), _p1(p1), _p2(p2) {}
    void execute() { (*_m)(_p1, _p2); }
};

/**
 * \ingroup callback
 */
template <class R, class U1, class U2, class V1, class V2>
TSignalLink* connect(TSignal &s, R(*m)(U1, U2), V1 p1, V2 p2) {
  return s.add(new GSignalLinkF2<R, U1, U2>(m, p1, p2));
}

/**
 * \ingroup callback
 */
template <class R, class U1, class U2>
void disconnect(TSignal &s, R(*m)(U1, U2)) {
	s.remove( (void(*)())m );
}

template <class A, class T, class U1, class U2>
class GSignalLinkO2:
  public TSignalLink
{
  protected:
    A(T::*_m)(U1, U2);
    T *_o;
    U1 _p1; U2 _p2;
   public:
    GSignalLinkO2(T *o, A(T::*m)(U1, U2), U1 p1, U2 p2):_m(m), _o(o), _p1(p1), _p2(p2) {}
    void execute() { (_o->*_m)(_p1, _p2); }
    virtual void* objref() {return (void*)_o;}
    virtual TMethod metref() {return (TMethod)_m;}
};

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class U2, class V1, class V2>
TSignalLink* connect(TSignal &s, T1 *o, R(T2::*m)(U1, U2), V1 p1, V2 p2) {
  return s.add(new GSignalLinkO2<R, T1, U1, U2>(o, m, p1, p2));
}

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class U2>
void disconnect(TSignal &s, T1 *o, R(T2::*m)(U1, U2)) {
	T2 *co=o; s.remove( (void*)o, (TSignalLink::TMethod)m);
}

template <class A, class U1, class U2, class U3>
class GSignalLinkF3:
  public TSignalLink
{
  protected:
    A(*_m)(U1, U2, U3);
    U1 _p1; U2 _p2; U3 _p3;
   public:
    GSignalLinkF3(A(*m)(U1, U2, U3), U1 p1, U2 p2, U3 p3):_m(m), _p1(p1), _p2(p2), _p3(p3) {}
    void execute() { (*_m)(_p1, _p2, _p3); }
};

/**
 * \ingroup callback
 */
template <class R, class U1, class U2, class U3, class V1, class V2, class V3>
TSignalLink* connect(TSignal &s, R(*m)(U1, U2, U3), V1 p1, V2 p2, V3 p3) {
  return s.add(new GSignalLinkF3<R, U1, U2, U3>(m, p1, p2, p3));
}

/**
 * \ingroup callback
 */
template <class R, class U1, class U2, class U3>
void disconnect(TSignal &s, R(*m)(U1, U2, U3)) {
	s.remove( (void(*)())m );
}

template <class A, class T, class U1, class U2, class U3>
class GSignalLinkO3:
  public TSignalLink
{
  protected:
    A(T::*_m)(U1, U2, U3);
    T *_o;
    U1 _p1; U2 _p2; U3 _p3;
   public:
    GSignalLinkO3(T *o, A(T::*m)(U1, U2, U3), U1 p1, U2 p2, U3 p3):_m(m), _o(o), _p1(p1), _p2(p2), _p3(p3) {}
    void execute() { (_o->*_m)(_p1, _p2, _p3); }
    virtual void* objref() {return (void*)_o;}
    virtual TMethod metref() {return (TMethod)_m;}
};

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class U2, class U3, class V1, class V2, class V3>
TSignalLink* connect(TSignal &s, T1 *o, R(T2::*m)(U1, U2, U3), V1 p1, V2 p2, V3 p3) {
  return s.add(new GSignalLinkO3<R, T1, U1, U2, U3>(o, m, p1, p2, p3));
}

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class U2, class U3>
void disconnect(TSignal &s, T1 *o, R(T2::*m)(U1, U2, U3)) {
	T2 *co=o; s.remove( (void*)o, (TSignalLink::TMethod)m);
}

template <class A, class U1, class U2, class U3, class U4>
class GSignalLinkF4:
  public TSignalLink
{
  protected:
    A(*_m)(U1, U2, U3, U4);
    U1 _p1; U2 _p2; U3 _p3; U4 _p4;
   public:
    GSignalLinkF4(A(*m)(U1, U2, U3, U4), U1 p1, U2 p2, U3 p3, U4 p4):_m(m), _p1(p1), _p2(p2), _p3(p3), _p4(p4) {}
    void execute() { (*_m)(_p1, _p2, _p3, _p4); }
};

/**
 * \ingroup callback
 */
template <class R, class U1, class U2, class U3, class U4, class V1, class V2, class V3, class V4>
TSignalLink* connect(TSignal &s, R(*m)(U1, U2, U3, U4), V1 p1, V2 p2, V3 p3, V4 p4) {
  return s.add(new GSignalLinkF4<R, U1, U2, U3, U4>(m, p1, p2, p3, p4));
}

/**
 * \ingroup callback
 */
template <class R, class U1, class U2, class U3, class U4>
void disconnect(TSignal &s, R(*m)(U1, U2, U3, U4)) {
	s.remove( (void(*)())m );
}

template <class A, class T, class U1, class U2, class U3, class U4>
class GSignalLinkO4:
  public TSignalLink
{
  protected:
    A(T::*_m)(U1, U2, U3, U4);
    T *_o;
    U1 _p1; U2 _p2; U3 _p3; U4 _p4;
   public:
    GSignalLinkO4(T *o, A(T::*m)(U1, U2, U3, U4), U1 p1, U2 p2, U3 p3, U4 p4):_m(m), _o(o), _p1(p1), _p2(p2), _p3(p3), _p4(p4) {}
    void execute() { (_o->*_m)(_p1, _p2, _p3, _p4); }
    virtual void* objref() {return (void*)_o;}
    virtual TMethod metref() {return (TMethod)_m;}
};

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class U2, class U3, class U4, class V1, class V2, class V3, class V4>
TSignalLink* connect(TSignal &s, T1 *o, R(T2::*m)(U1, U2, U3, U4), V1 p1, V2 p2, V3 p3, V4 p4) {
  return s.add(new GSignalLinkO4<R, T1, U1, U2, U3, U4>(o, m, p1, p2, p3, p4));
}

/**
 * \ingroup callback
 */
template <class R, class T1, class T2, class U1, class U2, class U3, class U4>
void disconnect(TSignal &s, T1 *o, R(T2::*m)(U1, U2, U3, U4)) {
	T2 *co=o; s.remove( (void*)o, (TSignalLink::TMethod)m);
}

} // namespace toad  connect.hh

#endif
