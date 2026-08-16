#ifndef _OXC_ROBOCAPS_H
#define _OXC_ROBOCAPS_H

#include <oxc_capabilities.h>


namespace oxc {

class RoboObject {
  public:
   explicit RoboObject( uint32_t id_ ) noexcept : id( id_ ) {};
   virtual ~RoboObject() = default;
   ReturnCode init() noexcept;
   ReturnCode measure() noexcept;
   ReturnCode think() noexcept;
   ReturnCode commit() noexcept;
   uint32_t getId() const noexcept { return id; }
   ReturnCode getStatus() const noexcept { return sta; }
   uint32_t getDirty() const noexcept { return dirty; }
  protected:
   virtual ReturnCode doInit()    noexcept = 0;
   virtual ReturnCode doMeasure() noexcept = 0;
   virtual ReturnCode doThink()   noexcept = 0;
   virtual ReturnCode doCommit()  noexcept = 0;
  protected:
   const uint32_t id; //* simple id for debug
   ReturnCode sta { ReturnCode::rcnErr, 1000 }; // uninitialised
   uint32_t dirty { 0 }; // bit per HW channel, so now no more than 32 channels, here: used in commit
};


//* pack of digital I/O channels + robo interface
class IoRoboCapability : public IoCapability, public RoboObject {
  public:
   explicit constexpr IoRoboCapability( size_t sz_, size_t szF_, size_t bitsz_,
                                        int32_t_span iobuf_, uint32_t id_ = 0 ) noexcept :
     IoCapability( sz_, szF_, bitsz_ ), RoboObject( id_ ), iobuf( iobuf_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch ) noexcept override;
  protected:
   int32_t_span iobuf;
};




class PinsRoboCapability : public PinsPureCapability, public IoRoboCapability {
  public:
   explicit constexpr PinsRoboCapability( size_t bitsz_, const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( n_ch_int, n_ch_float, bitsz_, vv, id_ ), tr( tr_ ) {};
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return setValF_common( tr, ch, v ); }
   virtual float_er   getValF( size_t ch )           noexcept override { return getValF_common( tr, ch ); }
  protected:
   const ValFiTrans1x1 &tr;
   int32_t vv[n_ch_int]; // 0-out 1-in
};



class PinRoboCapability : public PinPureCapability, public IoRoboCapability {
  public:
   explicit constexpr PinRoboCapability( const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( n_ch_int, n_ch_int, 1, vv, id_ ), tr( tr_ ) {};
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return setVal( ch, (int32_t)v ); }
   virtual float_er   getValF( size_t ch )           noexcept override { return getVal( ch ); } // how converted?
  protected:
   const ValFiTrans1x1 &tr;
   int32_t vv[n_ch_int]; // 0-out 1-in
};


// channels: 0..sz-1 - duty, sz..sz+n_cfg_ch - freq config
class PwmRoboCapability : public PwmPureCapability, public IoRoboCapability {
  public:
   enum { n_cfg_ch = 4 };
   explicit constexpr PwmRoboCapability( size_t sz_, size_t bitsz_, int32_t_span iobuf_,
       const ValFiTrans1xN &tr_f_, const ValFiTrans1x1 &tr_d_ = globalZeroValFiTrans, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( sz_ + n_cfg_ch, sz_ + 1, bitsz_, iobuf_, id_ ),
       tr_f( tr_f_ ), tr_d( tr_d_ ) {};
  protected:
   const ValFiTrans1xN &tr_f; // transformation for frequiency
   const ValFiTrans1x1 &tr_d; // transformation for duty
};


class EncoderRoboCapability : public EncoderPureCapability, public IoRoboCapability {
  public:
   explicit constexpr EncoderRoboCapability( size_t bitsz_, int32_t scale_, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( 1, bitsz_, scale_,  vv, id_ ) {};
  protected:
   int32_t vv[1]; // 0-out
};

}; //namespace oxc


#endif

