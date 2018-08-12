#ifndef _OXC_OUTFMT_H
#define _OXC_OUTFMT_H

#include <stdint.h>

#include <oxc_miscfun.h>

//* helper classes for stream output
class HexInt {
  public:
   explicit HexInt( uint32_t a ) : v(a) {};
   explicit HexInt( void *a ) : v( reinterpret_cast<uint32_t>(a) ) {};
   operator uint32_t() const { return v; }
   uint32_t v;
};

class HexInt16 {
  public:
   explicit HexInt16( uint16_t a ) : v(a) {};
   operator uint16_t() const { return v; }
   uint16_t v;
};

class HexInt8 {
  public:
   explicit HexInt8( uint8_t a ) : v(a) {};
   operator uint8_t() const { return v; }
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
   int frac()  const { return v & 1; };
   int v;
   static const char * const fracStr[4];
};

class FixedPoint2 {
  public:
   FixedPoint2( int a ) : v( a ) {};
   int toInt() const { return v / 4; };
   int frac()  const { return ( v >=0 ) ? ( v & 3 ) : ( -v & 3 ) ; };
   int v;
   static const char * const fracStr[6];
};

class FloatMult {
  public:
   FloatMult( int a, unsigned a_min_sz_frac = 1,  unsigned a_min_sz_int = 1, char a_plus_ch = ' '  )
      : v( a ), min_sz_frac( a_min_sz_frac ), min_sz_int( a_min_sz_int ), plus_ch( a_plus_ch )
      { for( unsigned i=0; i<min_sz_frac; ++i ) { mult *= 10; } };
   int v;
   unsigned mult = 1, min_sz_frac, min_sz_int;
   char plus_ch;
};

class BitsStr {
  public:
   BitsStr( uint32_t a, const BitNames *b ) : v( a ), bn( b ) {};
   uint32_t v;
   const BitNames *bn;
};

inline const char * const FixedPoint1::fracStr[4] = { ".0", ".5", ".?", ".X" };
inline const char * const FixedPoint2::fracStr[6] = { ".00", ".25", ".50", ".75", ".??", ".XX" };


#endif

