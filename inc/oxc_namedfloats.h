#ifndef _OXC_NAMEDFLOATS_H
#define _OXC_NAMEDFLOATS_H

// class NamedObj {
//   public:
//    enum Flags { no = 0, ro = 1 };
//    explicit constexpr NamedObj( const char *nm, unsigned n_elm = 1, unsigned flg = Flags::no )
//      : name( nm ), ne( n_elm ), flags( flg ) {};
//    NamedObj( const NamedObj &rhs ) = delete;
//    const char* getName() const { return name; };
//    unsigned getNe() const { return ne; };
//    virtual void* getAddr() const = 0;
//    virtual int getInt( int idx = 0 ) const = 0;
//    virtual float getFloat( int idx = 0 ) const = 0;
//    virtual bool getText( char *d, unsigned maxlen, int idx = 0 ) const = 0;
//    virtual bool print( int idx = 0 ) const = 0; // TODO? to where?
//   protected:
//    const char *name;
//    uint32_t ne = 1;
//    uint32_t flags = 0;
//    // float *p;
//    // float (*get)();
//    // bool  (*set)( float v );
// };

struct NamedFloat {
  enum Flags { no = 0, ro = 1 };
  const char *name;
  float *p;
  float (*get)();
  bool  (*set)( float v );
  uint32_t flags = 0;
  uint32_t ne = 1;
};

class NamedFloats {
  public:
   explicit constexpr NamedFloats( const NamedFloat *a_flts ) : fl( a_flts ), n( count_elems() ) {};
   constexpr unsigned size() const { return n; }
   const NamedFloat* find( const char *nm, int &idx ) const;
   const NamedFloat* begin() const  { return fl;   } // really const, but need for for( : )
   const NamedFloat* end() const    { return fl+n; }
   const NamedFloat* cbegin() const { return fl;   }
   const NamedFloat* cend() const   { return fl+n; }
   const char* getName( unsigned i ) const { return ( i<n ) ? fl[i].name : nullptr ; }
   bool  set( const char *nm, float v ) const; // change variable, not NamedFloats object
   float get( const char *nm, float def = 0.0f, bool *ok = nullptr ) const;
   bool  text( const char *nm, char *buf, unsigned bufsz ) const;
   bool  fromText( const char *nm, const char *s ) const;
   bool  print( const char *nm ) const;
   static void set_global_floats( NamedFloats *gfl ) { global_floats = gfl; }
   static bool  g_print( const char *nm );
   static bool  g_set( const char *nm, float v );
   static float g_get( const char *nm, float def = 0.0f, bool *ok = nullptr );
   static bool  g_fromText( const char *nm, const char *s );
   static const char* g_getName( unsigned i );
  private:
   const NamedFloat *fl;
   const unsigned n;
   static NamedFloats *global_floats;
   constexpr unsigned count_elems() const
   {
     unsigned i=0;
     for( const auto *f = fl; f->name != nullptr; ++f ) {
       ++i;
     }
     return i;
   }
};

#endif
