#ifndef _OXC_NAMEDFLOATS_H
#define _OXC_NAMEDFLOATS_H

#include <oxc_namedobjs.h>


class NamedFloat : public NamedObj {
  public:
   constexpr NamedFloat( const char *nm, float *p_f, std::size_t n_elm = 1, Flags flg = Flags::no )
     : NamedObj( nm, p_f, n_elm, flg, types::FLOAT ) {};
   constexpr NamedFloat( const char *nm, float (*p_get)(int), bool (*p_set)(float,int),
                         std::size_t n_elm = 1, Flags flg = Flags::no )
     : NamedObj( nm, nullptr, n_elm, flg ), fun_get( p_get ), fun_set( p_set ) {};
   virtual bool  get(    int &v, int idx = 0 ) const override;
   virtual bool  get(  float &v, int idx = 0 ) const override;
   virtual bool  get(   CStr &v, int idx = 0 ) const override;
   virtual bool  out( OutStream &os, int idx = 0, int fmt = 0 ) const override;
   virtual bool  set( int v, int idx = 0 ) const override;
   virtual bool  set( float v, int idx = 0 ) const override;
   virtual bool  set( const char *v, int idx = 0 ) const override;
  protected:
   float (*fun_get)( int idx ) = nullptr;
   bool  (*fun_set)( float v, int idx ) = nullptr;

   bool do_set( float v, int idx = 0 ) const;
   bool do_get( float &rv, int idx ) const;
   static void do_out( OutStream &os, float x, int fmt );
};

#endif
