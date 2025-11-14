#ifndef _OXC_OUTFMT_H
#define _OXC_OUTFMT_H

#include <stdint.h>

#include <oxc_miscfun.h>

class OutStream;

class OutStreamFmt {
  public:
   virtual void out( OutStream &os ) const = 0;
};

//* helper classes for stream output
class HexInt : public OutStreamFmt {
  public:
   explicit HexInt( uint32_t a, bool prefix_0x = false ) : v(a), pr( prefix_0x ) {};
   explicit HexInt( const void *a, bool prefix_0x = true )  : v( reinterpret_cast<uint32_t>(a) ), pr( prefix_0x )  {};
   operator uint32_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   uint32_t v;
   bool pr; // output (0x before)
};

class HexInt64 : public OutStreamFmt {
  public:
   explicit HexInt64( uint64_t a, bool prefix_0x = false ) : v(a), pr( prefix_0x ) {};
   operator uint64_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   uint64_t v;
   bool pr; // output (0x before)
};

class HexInt16 : public OutStreamFmt {
  public:
   explicit HexInt16( uint16_t a ) : v(a) {};
   operator uint16_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   uint16_t v;
};

class HexInt8 : public OutStreamFmt {
  public:
   explicit HexInt8( uint8_t a ) : v(a) {};
   operator uint8_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   uint8_t v;
};

class FmtInt : public OutStreamFmt {
  public:
   FmtInt( int a, unsigned _min_sz, char _fill_ch = ' ' )
     : v(a), min_sz( _min_sz ), fill_ch( _fill_ch ) {}
   virtual void out( OutStream &os ) const override;
   int v;
   unsigned min_sz;
   char fill_ch;
};

class FixedPoint1 : public OutStreamFmt {
  public:
   FixedPoint1( int a ) : v( a ) {};
   int toInt() const { return v / 2 ; };
   int frac()  const { return v & 1; };
   virtual void out( OutStream &os ) const override;
   int v;
   static const char * const fracStr[4];
};

class FixedPoint2 : public OutStreamFmt {
  public:
   FixedPoint2( int a ) : v( a ) {};
   int toInt() const { return v / 4; };
   int frac()  const { return ( v >=0 ) ? ( v & 3 ) : ( -v & 3 ) ; };
   virtual void out( OutStream &os ) const override;
   int v;
   static const char * const fracStr[6];
};

class FloatMult : public OutStreamFmt {
  public:
   FloatMult( int a, unsigned a_min_sz_frac = 1,  unsigned a_min_sz_int = 1, char a_plus_ch = ' '  )
      : v( a ), min_sz_frac( a_min_sz_frac ), min_sz_int( a_min_sz_int ), plus_ch( a_plus_ch )
      { for( unsigned i=0; i<min_sz_frac; ++i ) { mult *= 10; } };
   virtual void out( OutStream &os ) const override;
   int v, mult = 1;
   unsigned min_sz_frac, min_sz_int;
   char plus_ch;
};

class BitsStr : public OutStreamFmt {
  public:
   BitsStr( uint32_t a, const BitNames *b ) : v( a ), bn( b ) {};
   virtual void out( OutStream &os ) const override;
   uint32_t v;
   const BitNames *bn;
};

inline const char * const FixedPoint1::fracStr[4] = { ".0", ".5", ".?", ".X" };
inline const char * const FixedPoint2::fracStr[6] = { ".00", ".25", ".50", ".75", ".??", ".XX" };


#endif

