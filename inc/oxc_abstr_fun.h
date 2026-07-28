#ifndef _OXC_ABSTR_FUN_H
#define _OXC_ABSTR_FUN_H

#include <oxc_types.h>

using std::size_t;
using std::uint32_t;
using std::int32_t;

using oxc::ReturnCode;

//* Abstract classes to realize common functions + mix to real HW classes
// or may be RoboDevice children - with buffers

namespace oxc {

//* pack of digital I/O channels
class F_IO {
  public:
   explicit constexpr F_IO( size_t sz_, size_t bitsz_ ) noexcept : sz( sz_ ), bitsz( bitsz_ ) {};
   virtual constexpr size_t size()    const noexcept { return sz; }
   virtual constexpr size_t bitsize() const noexcept { return bitsz; }
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch ) noexcept = 0;
   ReturnCode setVals( cint32_t_span vs ) noexcept;
   ReturnCode getVals(  int32_t_span vs ) noexcept;
  protected:
   const size_t sz;
   const size_t bitsz;
};


class F_Analog : public F_IO {
  public:
   explicit constexpr F_Analog( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept
     : F_IO( sz_, bitsz_ ), scale( scale_ )   {};
   virtual constexpr size_t     getScale() noexcept { return scale; }
   virtual ReturnCode setValF( size_t ch, float v ) noexcept { // non-virtual?
     auto vi = (int32_t)(v*scale); // round?
     return setVal( ch, vi );
   }
   virtual float_er   getValF( size_t ch ) noexcept {
     auto vi_er = getVal( ch );
     if( !vi_er ) {
       return vi_er;
     }
     return vi_er.value() / scale;
   }
  protected:
   const int32_t scale;
};


class F_Pins : public F_IO {
  public:
   explicit constexpr F_Pins( size_t bitsz_ ) noexcept : F_IO( 1, bitsz_ )  {};
   virtual int32_t_er read()             = 0;
   virtual void write(     int32_t v )   = 0;
   virtual void set(       int32_t v )   = 0;
   virtual void reset(     int32_t v )   = 0;
   virtual void toggle(    int32_t v )   = 0;
   virtual void setbit(    int32_t pos ) = 0;
   virtual void resetbit(  int32_t pos ) = 0;   // reset given (by pos) 1 bit to '0' (AND~)
   virtual void togglebit( int32_t pos ) = 0;
};

class F_Pin : public F_IO {
  public:
   explicit constexpr F_Pin() noexcept : F_IO( 1, 1 )  {};
   virtual int32_t_er read()    = 0;
   virtual void write()         = 0;
   virtual void set()           = 0;
   virtual void reset()         = 0;
   virtual void toggle()        = 0;
};

class F_Pwm : public F_Analog {
  public:
    virtual ReturnCode setFreq( float freq ) = 0;
    constexpr float getFreq() const noexcept { return freq; }
    ReturnCode setDuty( size_t ch, float duty ) { return setValF( ch, duty ); }
    virtual ReturnCode setPulse(  size_t ch, float p_t ) = 0;
  protected:
    float freq {1};
};

class F_Encoder : public F_IO {
  public:
   explicit constexpr F_Encoder( size_t bitsz_, int32_t scale_ ) noexcept
     : F_IO( 1, bitsz_ ), scale( scale_ )   {};
  protected:
   const int32_t scale;
};

}; //namespace oxc


#endif

