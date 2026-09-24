#ifndef _OXC_ROBOCAPS_H
#define _OXC_ROBOCAPS_H

#include <oxc_capabilities.h>

// #include <oxc_debug1.h>

namespace oxc {

// TODO: flag to ignore measure/commit
class RoboObject {
  public:
   enum Flags { noFlag = 0, noInit = 1, noMeasure = 2, noThink = 4, noCommit = 8 };
   explicit RoboObject( uint32_t id_, Flags flags_ = noFlag ) noexcept : id( id_ ), flags ( flags_ ) {};
   virtual ~RoboObject() = default;
   ReturnCode init() noexcept;
   ReturnCode measure() noexcept;
   ReturnCode think() noexcept;
   ReturnCode commit() noexcept;
   uint32_t getId() const noexcept { return id; }
   ReturnCode getStatus() const noexcept { return sta; }
   uint32_t getDirty() const noexcept { return dirty; }
   Flags getFlags() const noexcept { return flags; }
   void  setFlags( Flags flags_ ) noexcept { flags = flags_; }
  protected:
   virtual ReturnCode doInit()    noexcept = 0;
   virtual ReturnCode doMeasure() noexcept = 0;
   virtual ReturnCode doThink()   noexcept = 0;
   virtual ReturnCode doCommit()  noexcept = 0;
  protected:
   const uint32_t id; //* simple id for debug
   Flags flags;
   ReturnCode sta { ReturnCode::rcnErr, 1000 }; // uninitialised
   uint32_t dirty { 0 }; // bit per HW channel, so now no more than 32 channels, here: used in commit
};


//* pack of digital I/O channels + robo interface
class IoRoboCapability : public IoCapability, public RoboObject {
  public:
   explicit constexpr IoRoboCapability( uint32_t id_ = 0 ) noexcept :  RoboObject( id_ ) {};
  protected:
};




class PinsRoboCapability : public IoRoboCapability {
  public:
   enum { // copy?
     ch_read = 0, ch_write = 1, ch_set = 2, ch_reset = 3, ch_toggle = 4,
     ch_setbit = 5, ch_resetbit = 6, ch_togglebit = 7,
     ch_r_bit = 1, ch_w_bit = 2,
     n_ch_int, n_ch_float = 0
   };
   explicit constexpr PinsRoboCapability( PinsPureCapability &pins_, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( id_ ), pins( pins_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[0] = vv[1] = 0;   return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { auto v = pins.read(); if( v ) { vv[0] = v.value();  return rcOk;}; return rcErr; }
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override { if( dirty & ch_w_bit ) { pins.write( vv[1] ); } return rcOk; }
  protected:
   PinsPureCapability &pins;
   int32_t vv[2]; // 0-in 1-out TODO: just 2 vars
};



class PinRoboCapability : public IoRoboCapability {
  public:
   enum { // copy?
     ch_read = 0, ch_write = 1, ch_set = 2, ch_reset = 3, ch_toggle = 4,
     ch_r_bit = 1, ch_w_bit = 2,
   };
   explicit constexpr PinRoboCapability( PinPureCapability &pin_, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( id_ ), pin( pin_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[0] = vv[1] = 0;   return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { auto v = pin.read(); if( v ) { vv[0] = v.value(); return rcOk;}; return rcErr; }
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override { if( dirty & ch_w_bit ) { pin.write( vv[1] ); } return rcOk; }
  protected:
   PinPureCapability &pin;
   int32_t vv[2]; // 0-in 1-out TODO: just 2 vars
};


// channels: in float: 0..sz-1 - duty, 100.. - pulse, 200.. - shift 300 - freq (i/o)
// TODO: default iobuf for 6-8 channels
class PwmRoboCapability : public IoRoboCapability {
  public:
   enum { // copy?
     ch0_pwm = 0, ch0_pulse = 100, ch0_shift = 200, ch_freq = 300
   };
   explicit constexpr PwmRoboCapability( PwmPureCapability &pwm_, size_t n_ch_, std::span<float> io_f_, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( id_ ), pwm( pwm_ ), n_ch( n_ch_ ), io_f( io_f_ )
       {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override { return rcErr; }
   virtual int32_t_er getVal( size_t ch )            noexcept override { return std::unexpected(rcErr); }
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override;
   virtual float_er   getValF( size_t ch )           noexcept override;
  protected:
   virtual ReturnCode doInit()    noexcept override;
   virtual ReturnCode doMeasure() noexcept override;
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override;
  protected:
   PwmPureCapability &pwm;
   size_t n_ch;
   std::span<float> io_f;
   float freq_get { 0 };
   float freq_set { 0 };
   uint32_t dirty_duty {0}, dirty_pulse {0}, dirty_shift {0}, pulse_flag {0};
   bool dirty_f;
};


class EncoderRoboCapability : public IoRoboCapability {
  public:
   enum { // int32_t channels pos: r/w, other - r/o
     ch_pos = 0, ch_posraw = 1, ch_dlt = 2, ch_startpos = 3,
     ch_pos_bit = 1
   };
   explicit constexpr EncoderRoboCapability( EncoderPureCapability &enc_, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( id_ ), enc(enc_) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   virtual ReturnCode doInit()    noexcept override { pos = posraw = dlt = pos_set = 0; return rcOk; }
   virtual ReturnCode doMeasure() noexcept override {
     auto rc = enc.read(); if( rc.isOk() ) {
       pos = enc.getPos(); posraw = enc.getPosRaw(); dlt = enc.getDelta(); start_pos = enc.getStartPos();
       return rcOk;
     };
     return rcErr;
   }
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override { if( dirty & ch_pos_bit ) { enc.setStartPos( pos_set ); } return rcOk; }
  protected:
   EncoderPureCapability &enc;
   int32_t pos {0}, posraw {0}, dlt {0}, start_pos {0};
   int32_t pos_set {0};
};

}; //namespace oxc


#endif

