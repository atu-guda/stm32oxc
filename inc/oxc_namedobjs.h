#ifndef _OXC_NAMEDOBSS_H
#define _OXC_NAMEDOBSS_H

#include <cstdint>

#include <oxc_miscfun.h>

class OutStream;
class NamedObjs;

class NamedObj {
  public:
   enum Flags { no = 0, ro = 1 };
   enum class types { NO = 0, STRUCT= 1,
      INT = 2, UINT = 3, INT16 = 4, UINT16 = 5, INT8 = 6, UINT8 = 7,
      FLOAT = 10, DOUBLE = 11 };
   explicit constexpr NamedObj( const char *nm, void *p_, unsigned n_elm = 1, Flags flg = Flags::no, types tp = types::NO )
     : name( nm ), p( p_ ), ne( n_elm ), flags( flg ), type( tp ) {};
   NamedObj( const NamedObj &rhs ) = delete;
   constexpr const char* getName() const { return name; };
   constexpr std::size_t size() const { return ne; };
   constexpr uint32_t getFlags() const { return flags; }
   constexpr bool hasFlags( uint32_t flg ) const { return (flags & flg ); }
   constexpr types getType() const { return type; }
   void* getAddr() const { return p; }; // beware: we can change *p, ignoring flags!
   virtual bool  get(   int &v, int idx = 0 ) const = 0;
   virtual bool  get( float &v, int idx = 0 ) const = 0;
   virtual bool  get(  CStr &v, int idx = 0 ) const = 0;
   virtual bool  out( OutStream &os, int idx = 0, int fmt = 0 ) const = 0;
   virtual bool  set(         int v, int idx = 0 ) const = 0;
   virtual bool  set(       float v, int idx = 0 ) const = 0;
   virtual bool  set( const char *v, int idx = 0 ) const = 0;
   virtual NamedObjs*  getSubObjs() const { return nullptr; }// for subobjects w/o = nullptr TODO: span + no nullptr
  protected:
   const char *name;
   void *p;
   const std::size_t ne { 1 };
   const Flags flags { Flags::no };
   const types type;
};

class NamedSubObj : public NamedObj {
  public:
   constexpr NamedSubObj( const char *nm, NamedObjs *par_obj_, Flags flg = Flags::no )
     : NamedObj( nm, par_obj_, 1, flg, types::STRUCT ) {};
   virtual bool  get(    int &v, int idx = 0 ) const override;
   virtual bool  get(  float &v, int idx = 0 ) const override;
   virtual bool  get(   CStr &v, int idx = 0 ) const override;
   virtual bool  out( OutStream &os, int idx = 0, int fmt = 0 ) const override;
   virtual bool  set( int v, int idx = 0 ) const override;
   virtual bool  set( float v, int idx = 0 ) const override;
   virtual bool  set( const char *v, int idx = 0 ) const override;
   virtual NamedObjs*  getSubObjs() const override;
  protected:
};



class NamedObjs {
  public:
   explicit constexpr NamedObjs( const NamedObj *const *const a_objs ) : objs( a_objs ), n( count_elems() ) {};
   constexpr std::size_t size() const { return n; }
   const NamedObj*  find( const char *nm, int &idx ) const; // TODO: pair?
   const NamedObj* const* begin()  const { return objs; } // really const, but need for for( : )
   const NamedObj* const* end()    const { return objs+n; }
   const NamedObj* const* cbegin() const { return objs; }
   const NamedObj* const* cend()   const { return objs+n; }
   const char* getName( std::size_t i ) const { return ( i<n ) ? objs[i]->getName() : nullptr ; }
   bool  get( const char *nm, int   &v ) const;
   bool  get( const char *nm, float &v ) const;
   bool  get( const char *nm, CStr  &v ) const;
   bool  set( const char *nm,         int v ) const; // change variable, not NamedObjs object
   bool  set( const char *nm,       float v ) const;
   bool  set( const char *nm, const char *s ) const;
   bool  out( OutStream &os, const char *nm, int fmt = 0 ) const;
   bool  print( const char *n, int fmt = 0 ) const;
  private:
   const NamedObj *const *const objs;
   const std::size_t n;
   static NamedObjs *global_objs;
   constexpr std::size_t count_elems() const
   {
     unsigned i=0;
     for( const auto *o = objs; *o != nullptr; ++o ) {
       ++i;
     }
     return i;
   }
};

#define DEFINE_NAMEDOBJ_FUNCS(objs) \
bool print_var_ex( const char *nm, int fmt ) { return objs.print( nm, fmt ); } \
bool set_var_ex( const char *nm, const char *s ) { auto ok =  objs.set( nm, s ); print_var_ex( nm, 0 );  return ok; }
#define SET_NAMEDOBJ_FUNCS print_var_hook = print_var_ex; set_var_hook   = set_var_ex;

#endif
