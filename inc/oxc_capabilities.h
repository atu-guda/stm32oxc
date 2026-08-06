#ifndef _OXC_CAPABILITIES_H
#define _OXC_CAPABILITIES_H

#include <cmath>

#include <oxc_types.h>

using std::size_t;
using std::uint32_t;
using std::int32_t;

using oxc::ReturnCode;

//* Abstract classes to realize common capabilities + mix to real HW classes

namespace oxc {


//* pack of digital I/O channels
class IoCapability {
  public:
   explicit constexpr IoCapability( size_t sz_, size_t szF_, size_t bitsz_ ) noexcept :
     sz( sz_ ), szF( szF_ ), bitsz( bitsz_ ) {};
   virtual ~IoCapability() = default; // really unneeded now
   // main interface
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch )            noexcept = 0;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept = 0;
   virtual float_er   getValF( size_t ch )           noexcept = 0;
   constexpr size_t size()     const noexcept { return sz;    }
   constexpr size_t sizeF()    const noexcept { return szF;    }
   constexpr size_t bitsize()  const noexcept { return bitsz; }
   // constexpr size_t getScale() const noexcept { return scale; }
   //* just convenience functions
   // may be useless now
   // ReturnCode setVals( cint32_t_span vs ) noexcept;
   // ReturnCode getVals(  int32_t_span vs ) noexcept;
   // ReturnCode setValFs( cfloat_span vs )  noexcept;
   // ReturnCode getValFs(  float_span vs )  noexcept;

  protected:
   const size_t sz;     //* number of integer channels
   const size_t szF;    //* number of floating channels - may be different
   const size_t bitsz;  //* integer channel bitsize - why here? -many different usages?
   // const int32_t scale; not here
};



class PinsPureCapability {
  public:
   enum { n_ch_int = 2, n_ch_float = 2 };
   virtual int32_t_er read()             noexcept = 0;
   virtual void write(     int32_t v   ) noexcept = 0;
   virtual void set(       int32_t v   ) noexcept = 0;
   virtual void reset(     int32_t v   ) noexcept = 0;
   virtual void toggle(    int32_t v   ) noexcept = 0;
   virtual void setbit(    int32_t pos ) noexcept = 0;
   virtual void resetbit(  int32_t pos ) noexcept = 0;
   virtual void togglebit( int32_t pos ) noexcept = 0;
};


//* pack of pins.
// channels: [0] - set, [1] - get, floats: just copy for now
class PinsCapability : public IoCapability, public PinsPureCapability {
  public:
   explicit constexpr PinsCapability( size_t bitsz_ ) noexcept : IoCapability( n_ch_int, n_ch_float, bitsz_ )  {};
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return setVal( ch, (int32_t)v ); }
   virtual float_er   getValF( size_t ch )           noexcept override { return getVal( ch ); } // how converted?
};


class PinPureCapability {
  public:
   enum { n_ch_int = 2, n_ch_float = 2 };
   virtual int32_t_er read()    noexcept  = 0;
   virtual void write( bool v ) noexcept  = 0;
   virtual void set()           noexcept  = 0;
   virtual void reset()         noexcept  = 0;
   virtual void toggle()        noexcept  = 0;
};


//* Single pin.
// channels: [0] - set, [1] - get, floats: just copy for now
class PinCapability : public IoCapability, public PinPureCapability {
  public:
   explicit constexpr PinCapability() noexcept : IoCapability( n_ch_int, n_ch_float, 1 )  {};
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return setVal( ch, (int32_t)v ); }
   virtual float_er   getValF( size_t ch )           noexcept override { return getValF( ch ); }
};


class PwmPureCapability {
  public:
   virtual ReturnCode setFreq( float freq ) = 0;
   virtual ReturnCode setDuty( size_t ch, float duty ) = 0;
   virtual ReturnCode setPulse( size_t ch, float p_t ) = 0;
   virtual float getFreq() const noexcept = 0;
};

class PwmCapability : public IoCapability, public PwmPureCapability { // + PinsPureCapability?
  public:
    explicit constexpr PwmCapability( size_t sz_, size_t bitsz_ ) noexcept
     : IoCapability( sz_, bitsz_, (1<<bitsz_)-1 ) {};
  protected:
};



class EncoderPureCapability {
  public:
};


class EncoderCapability : public IoCapability, public EncoderPureCapability {
  public:
   explicit constexpr EncoderCapability( size_t bitsz_, int32_t scale_ ) noexcept
     : IoCapability( 1, bitsz_, scale_ ) {};
  protected:
};


}; //namespace oxc


#endif

