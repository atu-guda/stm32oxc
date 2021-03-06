#ifndef _OXC_NAMEDOBSS_H
#define _OXC_NAMEDOBSS_H

#include <oxc_miscfun.h>

class OutStream;
class NamedObjs;

class NamedObj {
  public:
   enum Flags { no = 0, ro = 1 };
   explicit constexpr NamedObj( const char *nm, unsigned n_elm = 1, unsigned flg = Flags::no )
     : name( nm ), ne( n_elm ), flags( flg ) {};
   NamedObj( const NamedObj &rhs ) = delete;
   constexpr const char* getName() const { return name; };
   constexpr unsigned size() const { return ne; };
   constexpr uint32_t getFlags() const { return flags; }
   constexpr bool hasFlags( uint32_t flg ) const { return (flags & flg ); }
   virtual void* getAddr() const = 0;
   virtual bool  get(   int &v, int idx = 0 ) const = 0;
   virtual bool  get( float &v, int idx = 0 ) const = 0;
   virtual bool  get(  CStr &v, int idx = 0 ) const = 0;
   virtual bool  out( OutStream &os, int idx = 0, int fmt = 0 ) const = 0;
   virtual bool  set(         int v, int idx = 0 ) const = 0;
   virtual bool  set(       float v, int idx = 0 ) const = 0;
   virtual bool  set( const char *v, int idx = 0 ) const = 0;
   virtual const NamedObjs*  getSubObjs() const; // for subobjects w/o = nullptr
  protected:
   const char *name;
   const uint32_t ne = 1;
   const uint32_t flags = 0;
};

class NamedSubObj : public NamedObj {
  public:
   constexpr NamedSubObj( const char *nm, const NamedObjs *a_no, unsigned flg = Flags::no )
     : NamedObj( nm, 1, flg ), no( a_no ) {};
   virtual void* getAddr() const override { return nullptr; };
   virtual bool  get(    int &v, int idx = 0 ) const override;
   virtual bool  get(  float &v, int idx = 0 ) const override;
   virtual bool  get(   CStr &v, int idx = 0 ) const override;
   virtual bool  out( OutStream &os, int idx = 0, int fmt = 0 ) const override;
   virtual bool  set( int v, int idx = 0 ) const override;
   virtual bool  set( float v, int idx = 0 ) const override;
   virtual bool  set( const char *v, int idx = 0 ) const override;
   virtual const NamedObjs*  getSubObjs() const override;
  protected:
   const NamedObjs *no;
};



class NamedObjs {
  public:
   explicit constexpr NamedObjs( const NamedObj *const *const a_objs ) : objs( a_objs ), n( count_elems() ) {};
   constexpr unsigned size() const { return n; }
   const NamedObj*  find( const char *nm, int &idx ) const;
   const NamedObj* const* begin()  const { return objs; } // really const, but need for for( : )
   const NamedObj* const* end()    const { return objs+n; }
   const NamedObj* const* cbegin() const { return objs; }
   const NamedObj* const* cend()   const { return objs+n; }
   const char* getName( unsigned i ) const { return ( i<n ) ? objs[i]->getName() : nullptr ; }
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
   const unsigned n;
   static NamedObjs *global_objs;
   constexpr unsigned count_elems() const
   {
     unsigned i=0;
     for( const auto *o = objs; *o != nullptr; ++o ) {
       ++i;
     }
     return i;
   }
};

#endif
