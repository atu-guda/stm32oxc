#ifndef _OXC_PURECAPS_H
#define _OXC_PURECAPS_H

#include <oxc_types.h>



namespace oxc {

//* single pin
class PinPureCapability {
  public:
   virtual int32_t_er read()    noexcept  = 0; // 0 - r
   virtual int32_t_er readwr()  noexcept  = 0; // 1 - r
   virtual void write( bool v ) noexcept  = 0; // 1 - w
   virtual void set()           noexcept  = 0; // 2 - w
   virtual void reset()         noexcept  = 0; // 3 - w
   virtual void toggle()        noexcept  = 0; // 4 - w
};


class PinsPureCapability {
  public:
   virtual int32_t_er read()             noexcept = 0; // 0 r - index for PinsCapability
   virtual int32_t_er readwr()           noexcept = 0; // 1 r -|
   virtual void write(     int32_t v   ) noexcept = 0; // 1 w -|
   virtual void set(       int32_t v   ) noexcept = 0; // 2 w
   virtual void reset(     int32_t v   ) noexcept = 0; // 3
   virtual void toggle(    int32_t v   ) noexcept = 0; // 4
   virtual void setbit(    int32_t pos ) noexcept = 0; // 5
   virtual void resetbit(  int32_t pos ) noexcept = 0; // 6
   virtual void togglebit( int32_t pos ) noexcept = 0; // 7
};



//* frequiency in Hz, duty: [0:1]
class PwmPureCapability {
  public:
   virtual ReturnCode setDuty(  size_t ch, float duty ) noexcept = 0;
   virtual ReturnCode setPulse( size_t ch, float pu_s ) noexcept = 0;
   virtual ReturnCode setFreq( float freq )             noexcept = 0;
   virtual float getFreq() const                        noexcept = 0;
};


class EncoderPureCapability {
  public:
};




}; //namespace oxc


#endif

