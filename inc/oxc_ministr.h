#ifndef _OXC_MINISTR_H
#define _OXC MINISTR_H

// need, in some cases uint32_t not recognized as type
#include <stdint.h>


#include <oxc_miscfun.h>

//* helper classes for MiniStr (hex outout)
class HexInt {
  public:
   explicit HexInt( uint32_t a ) : v(a) {};
   explicit HexInt( void *a ) : v( reinterpret_cast<uint32_t>(a) ) {};
   operator uint32_t() { return v; }
   uint32_t v;
};

class HexInt16 {
  public:
   explicit HexInt16( uint16_t a ) : v(a) {};
   operator uint16_t() { return v; }
   uint16_t v;
};

class HexInt8 {
  public:
   explicit HexInt8( uint8_t a ) : v(a) {};
   operator uint8_t() { return v; }
   uint8_t v;
};

class FmtInt {
  public:
   FmtInt( int a, unsigned _min_sz, char _fill_ch = ' ' )
     : v(a), min_sz( _min_sz ), fill_ch( _fill_ch ) {}
   int v;
   unsigned min_sz;
   char fill_ch;
};

class FixedPoint1 {
  public:
   FixedPoint1( int a ) : v( a ) {};
   int toInt() const { return v / 2 ; };
   int frac() const  { return v  & 1; };
   int v;
   static const char * const fracStr[4];
};

class FixedPoint2 {
  public:
   FixedPoint2( int a ) : v( a ) {};
   int toInt() const { return v / 4; };
   int frac() const  { return ( v >=0 ) ? ( v & 3 ) : ( -v & 3 ) ; };
   int v;
   static const char * const fracStr[6];
};

class BitsStr {
  public:
   BitsStr( uint32_t a, const BitNames *b ) : v( a ), bn( b ) {};
   uint32_t v;
   const BitNames *bn;
};



//* minimal class to represent var-length strings in stack
// to be used in small mc
// no heap allocations, no exceptions ....

class MiniStr {
  public:
   static const unsigned cap_mini =  32;
   MiniStr( char *buf, unsigned _cap ) // place in given buf
     : cap( _cap ), sz(0), s( buf ), e( s ) {
       s[0] = '\0';
     };
   MiniStr( const MiniStr &r ) = delete;
   ~MiniStr() { flush(); s = e = nullptr; cap = sz = 0; }
   unsigned size() const { return sz; }
   unsigned capacity() const { return cap; }
   const char* c_str() const { return s; }
   char* data() { return s; }
   void clear() { sz = 0; e = s; s[0] = '\0'; };
   void flush() { if( flush_fun && sz > 0 ) { flush_fun( s, sz ); } ; clear(); }
   void set_flush_fun( int (*fun)( const char*, unsigned ) ) { flush_fun = fun; };
   bool ensureSpace( unsigned req );
   char  operator[] ( unsigned i ) const { return s[i]; };
   char& operator[] ( unsigned i ) { return s[i]; };
   MiniStr& operator=( const MiniStr &rhs ) = delete; // TODO: make it
   void append( char rhs );
   void add_bitnames( uint32_t b, const BitNames *bn );
   MiniStr& operator+=( char rhs ) { append( rhs ); return *this; }
   MiniStr& operator<<( char rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( const char *rhs );
   MiniStr& operator<<( const char *rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( const MiniStr &rhs ) { return operator+=( rhs.s ); }
   MiniStr& operator+=( int rhs );
   MiniStr& operator<<( int rhs )       { return operator+=( rhs ); }
   MiniStr& operator<<( unsigned rhs )  { return operator+=( (int)rhs ); }
   MiniStr& operator<<( long rhs )      { return operator+=( (int)rhs ); }
   MiniStr& operator<<( unsigned long rhs )  { return operator+=( (int)rhs ); }
   MiniStr& operator+=( HexInt8  rhs );
   MiniStr& operator<<( HexInt8  rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( HexInt16 rhs );
   MiniStr& operator<<( HexInt16 rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( HexInt   rhs );
   MiniStr& operator<<( HexInt   rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( const FmtInt &rhs );
   MiniStr& operator<<( const FmtInt &rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( FixedPoint1 rhs );
   MiniStr& operator<<( FixedPoint1 rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( FixedPoint2 rhs );
   MiniStr& operator<<( FixedPoint2 rhs ) { return operator+=( rhs ); }
   MiniStr& operator+=( const BitsStr &rhs );
   MiniStr& operator<<( const BitsStr &rhs ) { return operator+=( rhs ); }
  private:
   unsigned cap, sz;
   char *s, *e;
   int (*flush_fun)( const char *d, unsigned n ) = nullptr;
   // TODO: flags, mutex
};

#define MSTR(nm,sz) static_assert( sz >= MiniStr::cap_mini ); char nm##_tmp_buf[sz]; MiniStr nm( nm##_tmp_buf, sz );
#define MSTRF(nm,sz,fun) MSTR(nm,sz); nm.set_flush_fun( fun );

#endif
