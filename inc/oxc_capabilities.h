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
   explicit constexpr IoCapability( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept :
     sz( sz_ ), bitsz( bitsz_ ), scale( scale_ ) {};
   virtual ~IoCapability() = default; // really unneeded now
   // main interface
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch )            noexcept = 0;
   constexpr size_t size()     const noexcept { return sz;    }
   constexpr size_t bitsize()  const noexcept { return bitsz; }
   constexpr size_t getScale() const noexcept { return scale; }
   //* just convenience functions
   ReturnCode setVals( cint32_t_span vs ) noexcept;
   ReturnCode getVals(  int32_t_span vs ) noexcept;
   ReturnCode setValFs( cfloat_span vs )  noexcept;
   ReturnCode getValFs(  float_span vs )  noexcept;
   ReturnCode setValF( size_t ch, float v ) noexcept {
     return setVal( ch, (int32_t) std::lroundf( v * scale ) );
   }
   float_er   getValF( size_t ch ) noexcept {
     auto vi_er = getVal( ch );
     if( !vi_er ) {
       return vi_er;
     }
     return vi_er.value() / scale;
   }
  protected:
   const size_t sz;
   const size_t bitsz;
   const int32_t scale;
};



class PinsPureCapability {
  public:
   virtual int32_t_er read()             noexcept = 0;
   virtual void write(     int32_t v )   noexcept = 0;
   virtual void set(       int32_t v )   noexcept = 0;
   virtual void reset(     int32_t v )   noexcept = 0;
   virtual void toggle(    int32_t v )   noexcept = 0;
   virtual void setbit(    int32_t pos ) noexcept = 0;
   virtual void resetbit(  int32_t pos ) noexcept = 0;
   virtual void togglebit( int32_t pos ) noexcept = 0;
};



class PinsCapability : public IoCapability, public PinsPureCapability {
  public:
   explicit constexpr PinsCapability( size_t bitsz_ ) noexcept : IoCapability( 1, bitsz_, (1<<bitsz)-1 )  {};
};


class PinPureCapability {
  public:
   virtual int32_t_er read()    noexcept  = 0;
   virtual void write( bool v ) noexcept  = 0;
   virtual void set()           noexcept  = 0;
   virtual void reset()         noexcept  = 0;
   virtual void toggle()        noexcept  = 0;
};


class PinCapability : public IoCapability, public PinPureCapability {
  public:
   explicit constexpr PinCapability() noexcept : IoCapability( 2, 1, 1 )  {};
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

