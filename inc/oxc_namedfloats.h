#ifndef _OXC_NAMEDFLOATS_H
#define _OXC_NAMEDFLOATS_H

class OutStream;

class NamedObj {
  public:
   enum Flags { no = 0, ro = 1 };
   explicit constexpr NamedObj( const char *nm, unsigned n_elm = 1, unsigned flg = Flags::no )
     : name( nm ), ne( n_elm ), flags( flg ) {};
   NamedObj( const NamedObj &rhs ) = delete;
   constexpr const char* getName() const { return name; };
   unsigned size() const { return ne; };
   uint32_t getFlags() const { return flags; }
   bool hasFlags( uint32_t flg ) const { return (flags & flg ); }
   virtual void* getAddr() const = 0;
   virtual int getInt( int idx = 0 ) const = 0;
   virtual float getFloat( int idx = 0 ) const = 0;
   virtual bool getText( char *d, unsigned maxlen, int idx = 0 ) const = 0;
   virtual bool out( OutStream &os, int idx = 0 ) const = 0; // TODO? to where?
  protected:
   const char *name;
   uint32_t ne = 1;
   uint32_t flags = 0;
};

class NamedFloat : public NamedObj {
  public:
   constexpr NamedFloat( const char *nm, float *p_f, unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), p( p_f ) {};
   constexpr NamedFloat( const char *nm, float (*p_get)(int), bool (*p_set)(float,int),
                         unsigned n_elm = 1, unsigned flg = Flags::no )
     : NamedObj( nm, n_elm, flg ), get( p_get ), set( p_set) {};
   virtual void* getAddr() const override { return p; };
   virtual int getInt( int idx = 0 ) const override;
   virtual float getFloat( int idx = 0 ) const override;
   virtual bool getText( char *d, unsigned maxlen, int idx = 0 ) const override;
   virtual bool out( OutStream &os, int idx = 0 ) const override;
   bool do_set( float v, int idx = 0 ) const;
   bool do_get( float &rv, int idx ) const;
  protected:
   float *p = nullptr;
   float (*get)( int idx ) = nullptr;
   bool  (*set)( float v, int idx ) = nullptr;
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
   const char* getName( unsigned i ) const { return ( i<n ) ? fl[i].getName() : nullptr ; }
   bool  set( const char *nm, float v ) const; // change variable, not NamedFloats object
   float get( const char *nm, float def = 0.0f, bool *ok = nullptr ) const;
   bool  text( const char *nm, char *buf, unsigned bufsz ) const;
   bool  fromText( const char *nm, const char *s ) const;
   bool  out( OutStream &os, const char *nm ) const;
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
     for( const auto *f = fl; f->getName() != nullptr; ++f ) {
       ++i;
     }
     return i;
   }
};

#endif
