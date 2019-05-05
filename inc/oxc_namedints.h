#ifndef _OXC_NAMEDINTS_H
#define _OXC_NAMEDINTS_H

#include <oxc_namedobjs.h>


class NamedInt : public NamedObj {
  public:
   constexpr NamedInt( const char *nm, int *p_i, unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), p( p_i ) {};
   constexpr NamedInt( const char *nm, int (*p_get)(int), bool (*p_set)(int,int),
                         unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), fun_get( p_get ), fun_set( p_set ) {};
   virtual void* getAddr() const override { return p; };
   virtual bool  get(    int &v, int idx = 0 ) const override;
   virtual bool  get(  float &v, int idx = 0 ) const override;
   virtual bool  get(   CStr &v, int idx = 0 ) const override;
   virtual bool  out( OutStream &os, int idx = 0, int fmt  = 0) const override;
   virtual bool  set( int v, int idx = 0 ) const override;
   virtual bool  set( float v, int idx = 0 ) const override;
   virtual bool  set( const char *v, int idx = 0 ) const override;
  protected:
   int *p = nullptr;
   int   (*fun_get)( int idx ) = nullptr;
   bool  (*fun_set)( int v, int idx ) = nullptr;

   bool do_set( int v, int idx = 0 ) const;
   bool do_get( int &rv, int idx ) const;
   static void do_out( OutStream &os, int x, int fmt );
};

#endif
