#ifndef _OXC_NAMEDFLOATS_H
#define _OXC_NAMEDFLOATS_H

class OutStream;

struct CStr {
  constexpr CStr( char *a_s, unsigned a_n ) : s( a_s ), n( a_n ) {};
  char *s;
  const unsigned n;
};

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
   virtual bool  out( OutStream &os, int idx = 0 ) const = 0;
   virtual bool  set(         int v, int idx = 0 ) const = 0;
   virtual bool  set(       float v, int idx = 0 ) const = 0;
   virtual bool  set( const char *v, int idx = 0 ) const = 0;
  protected:
   const char *name;
   const uint32_t ne = 1;
   const uint32_t flags = 0;
};

class NamedFloat : public NamedObj {
  public:
   constexpr NamedFloat( const char *nm, float *p_f, unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), p( p_f ) {};
   constexpr NamedFloat( const char *nm, float (*p_get)(int), bool (*p_set)(float,int),
                         unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), fun_get( p_get ), fun_set( p_set) {};
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

class NamedObjs {
  public:
   explicit constexpr NamedObjs( const NamedObj *const *const a_objs ) : objs( a_objs ), n( count_elems() ) {};
   constexpr unsigned size() const { return n; }
   const NamedObj*  find( const char *nm, int &idx ) const;
   // const NamedObj* const* begin()  const { return objs; } // really const, but need for for( : )
   // const NamedObj* const* end()    const { return objs+n; }
   // const NamedObj* const* cbegin() const { return objs; }
   // const NamedObj* const* cend()   const { return objs+n; }
   const char* getName( unsigned i ) const { return ( i<n ) ? objs[i]->getName() : nullptr ; }
   bool  get( const char *nm, int   &v ) const;
   bool  get( const char *nm, float &v ) const;
   bool  get( const char *nm, CStr  &v ) const;
   bool  set( const char *nm,         int v ) const; // change variable, not NamedObjs object
   bool  set( const char *nm,       float v ) const;
   bool  set( const char *nm, const char *s ) const;
   bool  out( OutStream &os, const char *nm ) const;
   bool  print( const char *nm ) const;
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
