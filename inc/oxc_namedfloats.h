#ifndef _OXC_NAMEDFLOATS_H
#define _OXC_NAMEDFLOATS_H

#include <oxc_namedobjs.h>


class NamedFloat : public NamedObj {
  public:
   constexpr NamedFloat( const char *nm, float *p_f, unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), p( p_f ) {};
   constexpr NamedFloat( const char *nm, float (*p_get)(int), bool (*p_set)(float,int),
                         unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), fun_get( p_get ), fun_set( p_set ) {};
   virtual void* getAddr() const override { return p; };
   virtual bool  get(    int &v, int idx = 0 ) const override;
   virtual bool  get(  float &v, int idx = 0 ) const override;
   virtual bool  get(   CStr &v, int idx = 0 ) const override;
   virtual bool  out( OutStream &os, int idx = 0 ) const override;
   virtual bool  set( int v, int idx = 0 ) const override;
   virtual bool  set( float v, int idx = 0 ) const override;
   virtual bool  set( const char *v, int idx = 0 ) const override;
  protected:
   float *p = nullptr;
   float (*fun_get)( int idx ) = nullptr;
   bool  (*fun_set)( float v, int idx ) = nullptr;

   bool do_set( float v, int idx = 0 ) const;
   bool do_get( float &rv, int idx ) const;
};

#endif
