#ifndef _OXC_CAPABILITIES_H
#define _OXC_CAPABILITIES_H

#include <cmath>

#include <oxc_types.h>

using std::size_t;
using std::uint32_t;
using std::int32_t;

using oxc::ReturnCode;

//* Abstract classes to realize common capabilities + mix to real HW classes
// or may be RoboDevice children - with buffers

// TODO: split file: pure and robo capabilities, may be rename robo =? Buf

namespace oxc {

//* try: separate robo functions
class RoboObject {
  public:
   virtual ~RoboObject() = default;
   ReturnCode init() noexcept;
   ReturnCode measure() noexcept;
   ReturnCode think() noexcept;
   ReturnCode commit() noexcept;
   ReturnCode getStatus() const noexcept { return sta; }
   bool isDirty() const noexcept { return dirty; }
  protected:
   virtual ReturnCode doInit()    noexcept = 0;
   virtual ReturnCode doMeasure() noexcept = 0;
   virtual ReturnCode doThink()   noexcept = 0;
   virtual ReturnCode doCommit()  noexcept = 0;
  protected:
   ReturnCode sta { ReturnCode::rcnErr, 1000 }; // uninitialised
   bool dirty { false };
};

//* pack of digital I/O channels
class IoCapability {
  public:
   explicit constexpr IoCapability( size_t sz_, size_t bitsz_ ) noexcept : sz( sz_ ), bitsz( bitsz_ ) {};
   virtual constexpr size_t size()    const noexcept { return sz; }
   virtual constexpr size_t bitsize() const noexcept { return bitsz; }
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch ) noexcept = 0;
   //* just convenience functions
   ReturnCode setVals( cint32_t_span vs ) noexcept;
   ReturnCode getVals(  int32_t_span vs ) noexcept;
  protected:
   const size_t sz;
   const size_t bitsz;
};



//* pack of digital I/O channels + robo interface
class IoRoboCapability : public IoCapability, public RoboObject {
  public:
   explicit constexpr IoRoboCapability( size_t sz_, size_t bitsz_, int32_t_span buf_ ) noexcept :
     IoCapability( sz_, bitsz_ ),
     buf( buf_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch ) noexcept override;
  protected:
   int32_t_span buf;
   // TODO: dirty, fresh?
};


class AnalogCapability : public IoCapability {
  public:
   explicit constexpr AnalogCapability( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept
     : IoCapability( sz_, bitsz_ ), scale( scale_ )   {};
   virtual constexpr size_t     getScale() noexcept { return scale; }
   virtual ReturnCode setValF( size_t ch, float v ) noexcept { // non-virtual?
     return setVal( ch, (int32_t) std::lroundf( v * scale ) );
   }
   virtual float_er   getValF( size_t ch ) noexcept {
     auto vi_er = getVal( ch );
     if( !vi_er ) {
       return vi_er;
     }
     return vi_er.value() / scale;
   }
   ReturnCode setValFs( cfloat_span vs ) noexcept;
   ReturnCode getValFs(  float_span vs ) noexcept;
  protected:
   const int32_t scale;
};

class AnalogRoboCapability : public AnalogCapability, public RoboObject {
  public:
   explicit constexpr AnalogRoboCapability( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept
     : AnalogCapability( sz_, bitsz_, scale_ ) {};
  protected:
};



class PinsCapability : public IoCapability {
  public:
   explicit constexpr PinsCapability( size_t bitsz_ ) noexcept : IoCapability( 1, bitsz_ )  {};
   virtual int32_t_er read()             = 0;
   virtual void write(     int32_t v )   = 0;
   virtual void set(       int32_t v )   = 0;
   virtual void reset(     int32_t v )   = 0;
   virtual void toggle(    int32_t v )   = 0;
   virtual void setbit(    int32_t pos ) = 0;
   virtual void resetbit(  int32_t pos ) = 0;   // reset given (by pos) 1 bit to '0' (AND~)
   virtual void togglebit( int32_t pos ) = 0;
};

class PinsRoboCapability : public PinsCapability, public RoboObject {
  public:
   explicit constexpr PinsRoboCapability( size_t bitsz_ ) noexcept
     : PinsCapability( bitsz_ ) {};
  protected:
};


class PinCapability : public IoCapability {
  public:
   explicit constexpr PinCapability() noexcept : IoCapability( 1, 1 )  {};
   virtual int32_t_er read()    = 0;
   virtual void write()         = 0;
   virtual void set()           = 0;
   virtual void reset()         = 0;
   virtual void toggle()        = 0;
};

class PinRoboCapability : public PinCapability, public RoboObject {
  public:
   explicit constexpr PinRoboCapability() noexcept
     : PinCapability() {};
  protected:
};


class PwmCapability : public AnalogCapability {
  public:
    explicit constexpr PwmCapability( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept
     : AnalogCapability( sz_, bitsz_, scale_ ) {};
    virtual ReturnCode setFreq( float freq ) = 0;
    constexpr float getFreq() const noexcept { return freq; }
    ReturnCode setDuty( size_t ch, float duty ) { return setValF( ch, duty ); }
    virtual ReturnCode setPulse(  size_t ch, float p_t ) = 0;
  protected:
    float freq {1};
};

class PwmRoboCapability : public PwmCapability, public RoboObject {
  public:
   explicit constexpr PwmRoboCapability( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept
     : PwmCapability( sz_, bitsz_, scale_ ) {};
  protected:
};


class EncoderCapability : public IoCapability { // or AnalogCapability?
  public:
   explicit constexpr EncoderCapability( size_t bitsz_, int32_t scale_ ) noexcept
     : IoCapability( 1, bitsz_ ), scale( scale_ )   {};
  protected:
   const int32_t scale;
};

class EncoderRoboCapability : public EncoderCapability {
  public:
   explicit constexpr EncoderRoboCapability( size_t bitsz_, int32_t scale_ ) noexcept
     : EncoderCapability( bitsz_, scale_ ) {};
  protected:
};

}; //namespace oxc


#endif

