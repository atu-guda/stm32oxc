#ifndef _OXC_OUTFMT_H
#define _OXC_OUTFMT_H

#include <cstdint>

#include <oxc_miscfun.h>

class OutStream;

class OutStreamFmt {
  public:
   virtual void out( OutStream &os ) const = 0;
};

//* helper classes for stream output
class HexInt : public OutStreamFmt {
  public:
   explicit constexpr HexInt( uint32_t a, bool prefix_0x = false ) : v(a), pr( prefix_0x ) {};
   static_assert( sizeof(void*) == sizeof(uint32_t), "Only for 32-bit pointers" );
   explicit constexpr HexInt( const void *a, bool prefix_0x = true )  : v( reinterpret_cast<uint32_t>(a) ), pr( prefix_0x )  {};
   constexpr operator uint32_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   const uint32_t v;
   const bool pr; // output (0x before)
};

class HexInt64 : public OutStreamFmt {
  public:
   explicit constexpr HexInt64( uint64_t a, bool prefix_0x = false ) : v(a), pr( prefix_0x ) {};
   constexpr operator uint64_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   const uint64_t v;
   const bool pr; // output (0x before)
};

class HexInt16 : public OutStreamFmt {
  public:
   explicit constexpr HexInt16( uint16_t a ) : v(a) {};
   constexpr operator uint16_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   const uint16_t v;
};

class HexInt8 : public OutStreamFmt {
  public:
   explicit constexpr HexInt8( uint8_t a ) : v(a) {};
   constexpr operator uint8_t() const { return v; }
   virtual void out( OutStream &os ) const override;
   const uint8_t v;
};

class FmtInt : public OutStreamFmt {
  public:
   constexpr FmtInt( int a, unsigned _min_sz, char _fill_ch = ' ' )
     : v(a), min_sz( _min_sz ), fill_ch( _fill_ch ) {}
   virtual void out( OutStream &os ) const override;
   const int v;
   const unsigned min_sz;
   const char fill_ch;
};

class FixedPoint1 : public OutStreamFmt {
  public:
   explicit constexpr FixedPoint1( int a ) : v( a ) {};
   constexpr int toInt() const { return v / 2 ; };
   constexpr int frac()  const { return v & 1; };
   virtual void out( OutStream &os ) const override;
   const int v;
   static const char * const fracStr[4];
};

class FixedPoint2 : public OutStreamFmt {
  public:
   explicit constexpr FixedPoint2( int a ) : v( a ) {};
   constexpr int toInt() const { return v / 4; };
   constexpr int frac()  const { return ( v >=0 ) ? ( v & 3 ) : ( -v & 3 ) ; };
   virtual void out( OutStream &os ) const override;
   const int v;
   static const char * const fracStr[6];
};

class FloatMult : public OutStreamFmt {
  public:
   constexpr FloatMult( int a, unsigned a_min_sz_frac = 1,  unsigned a_min_sz_int = 1, char a_plus_ch = ' '  )
      : v( a ), min_sz_frac( a_min_sz_frac ), min_sz_int( a_min_sz_int ),
        mult( exp10i(min_sz_frac) ), plus_ch( a_plus_ch )
      {};
   virtual void out( OutStream &os ) const override;
   const int v;
   unsigned min_sz_frac, min_sz_int;
   const int mult;
   const char plus_ch;
};

class BitsStr : public OutStreamFmt {
  public:
   constexpr BitsStr( uint32_t a, const BitNames *b ) : v( a ), bn( b ) {};
   virtual void out( OutStream &os ) const override;
   const uint32_t v;
   const BitNames *const bn;
};

inline const char * const FixedPoint1::fracStr[4] = { ".0", ".5", ".?", ".X" };
inline const char * const FixedPoint2::fracStr[6] = { ".00", ".25", ".50", ".75", ".??", ".XX" };


#endif

