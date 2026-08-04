#ifndef _OXC_ROBOCAPS_H
#define _OXC_ROBOCAPS_H

#include <oxc_capabilities.h>


namespace oxc {

class RoboObject {
  public:
   explicit RoboObject( uint32_t id_, int32_t_span iobuf_ ) noexcept : id( id_ ), iobuf( iobuf_ ) {};
   virtual ~RoboObject() = default;
   ReturnCode init() noexcept;
   ReturnCode measure() noexcept;
   ReturnCode think() noexcept;
   ReturnCode commit() noexcept;
   uint32_t getId() const noexcept { return id; }
   ReturnCode getStatus() const noexcept { return sta; }
   bool isDirty() const noexcept { return dirty; }
  protected:
   virtual ReturnCode doInit()    noexcept = 0;
   virtual ReturnCode doMeasure() noexcept = 0;
   virtual ReturnCode doThink()   noexcept = 0;
   virtual ReturnCode doCommit()  noexcept = 0;
  protected:
   const uint32_t id; //* simple id for debug
   int32_t_span iobuf;
   ReturnCode sta { ReturnCode::rcnErr, 1000 }; // uninitialised
   bool dirty { false };
};


//* pack of digital I/O channels + robo interface
class IoRoboCapability : public IoCapability, public RoboObject {
  public:
   explicit constexpr IoRoboCapability( size_t sz_, size_t bitsz_, int32_t scale_,
                                        int32_t_span iobuf_, uint32_t id_ = 0 ) noexcept :
     IoCapability( sz_, bitsz_, scale_ ), RoboObject( id_, iobuf_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch ) noexcept override;
  protected:
};




class PinsRoboCapability : public PinsPureCapability, public IoRoboCapability {
  public:
   explicit constexpr PinsRoboCapability( size_t bitsz_, uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( 2, bitsz_, (1<<bitsz)-1, vv, id_ ) {};
  protected:
   int32_t vv[2]; // 0-out 1-in
};



class PinRoboCapability : public PinPureCapability, public IoRoboCapability {
  public:
   explicit constexpr PinRoboCapability( uint32_t id_ = 0 ) noexcept
     : IoRoboCapability( 2, 1, 1, vv, id_ ) {};
  protected:
   int32_t vv[2]; // 0-out 1-in
};



class PwmRoboCapability : public PwmCapability, public RoboObject {
  public:
   explicit constexpr PwmRoboCapability( size_t sz_, size_t bitsz_, int32_t scale_,
       int32_t_span iobuf_, uint32_t id_ = 0 ) noexcept
     : PwmCapability( sz_, bitsz_, scale_ ), RoboObject( id_, iobuf_ ) {};
  protected:
};


class EncoderRoboCapability : public EncoderCapability, public RoboObject {
  public:
   explicit constexpr EncoderRoboCapability( size_t bitsz_, int32_t scale_, uint32_t id_ = 0 ) noexcept
     : EncoderCapability( bitsz_, scale_ ), RoboObject( id_, vv ) {};
  protected:
   int32_t vv[1]; // 0-out
};

}; //namespace oxc


#endif

