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
//* if freq is changed, drop duty0, shifts to 0
class PwmPureCapability {
  public:
   enum { max_cfg_reg = 8 };
   ReturnCode setDuty(  size_t ch, float duty ) noexcept { return setDutyRaw(  ch,  duty2raw(duty) ); }
   ReturnCode setPulse( size_t ch, float pu_s ) noexcept { return setDutyRaw(  ch, pulse2raw(pu_s) ); }
   ReturnCode setShift( size_t ch, float sh_s ) noexcept { return setShiftRaw( ch, shift2raw(sh_s) ); }
   ReturnCode setFreq( float freq ) noexcept {
     uint32_t cfgs[max_cfg_reg];
     ReturnCode rc = freq2cfgs( freq, cfgs );
     if( rc.isError() ) { return rc; }
     return applyCfg( cfgs );
   }
   float_er getFreq() const noexcept {
     uint32_t cfgs[max_cfg_reg];
     ReturnCode rc = storeCfg( cfgs );
     if( rc.isError() ) { return std::unexpected(rc); }
     return cfg2freq( cfgs );
   };
   // for commonify usage + ll
   virtual uint32_t duty2raw(  float duty ) const noexcept = 0;
   virtual uint32_t pulse2raw( float pu_s ) const noexcept = 0;
   virtual uint32_t shift2raw( float pu_s ) const noexcept = 0;
   virtual ReturnCode freq2cfgs( float freq, std::span<uint32_t> cfgs ) const noexcept = 0;
   virtual float_er  cfg2freq( std::span<const uint32_t> cfgs ) const noexcept = 0;
   // low-level interface
   virtual ReturnCode setDutyRaw(  size_t ch, int32_t dr ) noexcept = 0;
   virtual ReturnCode setShiftRaw( size_t ch, int32_t sr ) noexcept = 0;
   virtual int32_t_er getDutyRaw(  size_t ch )  noexcept = 0;
   virtual int32_t_er getShiftRaw( size_t ch )  noexcept = 0;
   virtual ReturnCode applyCfg( std::span<const uint32_t> cfgs )  noexcept = 0;
   virtual ReturnCode storeCfg( std::span<      uint32_t> cfgs ) const noexcept = 0;
  protected:
};


class EncoderPureCapability {
  public:
};




}; //namespace oxc


#endif

