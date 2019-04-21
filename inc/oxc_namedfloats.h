#ifndef _OXC_NAMEDFLOATS_H
#define _OXC_NAMEDFLOATS_H

struct NamedFloat {
  enum Flags { flg_no = 0, flg_ro = 1 };
  const char *name;
  float *p;
  float (*get)();
  bool  (*set)( float v );
  uint32_t flags = 0;
  uint32_t ne = 1;
};

class NamedFloats {
  public:
   NamedFloats( const NamedFloat *a_flts ) : fl( a_flts ), n( count_elems() ) {};
   constexpr unsigned size() const { return n; }
   const NamedFloat* find( const char *nm ) const;
   const NamedFloat* begin() const  { return fl;   } // really const, but need for for( : )
   const NamedFloat* end() const    { return fl+n; }
   const NamedFloat* cbegin() const { return fl;   }
   const NamedFloat* cend() const   { return fl+n; }
   const char* getName( unsigned i ) const { return ( i<n ) ? fl[i].name : nullptr ; }
   bool  set( const char *nm, float v ) const; // change variable, not NamedFloats object
   float get( const char *nm, float def = 0.0f, bool *ok = nullptr ) const;
   bool  text( const char *nm, char *buf, unsigned bufsz ) const;
   bool  fromText( const char *nm, const char *s ) const;
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
