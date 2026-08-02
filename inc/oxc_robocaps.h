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
   bool isDirty() const noexcept { return dirty; }
  protected:
   virtual ReturnCode doInit()    noexcept = 0;
   virtual ReturnCode doMeasure() noexcept = 0;
   virtual ReturnCode doThink()   noexcept = 0;
   virtual ReturnCode doCommit()  noexcept = 0;
  protected:
   const uint32_t id; //* simple id for debug
   ReturnCode sta { ReturnCode::rcnErr, 1000 }; // uninitialised
   bool dirty { false };
};


//* pack of digital I/O channels + robo interface
class IoRoboCapability : public IoCapability, public RoboObject {
  public:
   explicit constexpr IoRoboCapability( size_t sz_, size_t bitsz_, int32_t_span buf_, uint32_t id_ = 0 ) noexcept :
     IoCapability( sz_, bitsz_ ), RoboObject( id_ ),
     buf( buf_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch ) noexcept override;
  protected:
   int32_t_span buf;
   // TODO: dirty, fresh?
};



class AnalogRoboCapability : public AnalogCapability, public RoboObject {
  public:
   explicit constexpr AnalogRoboCapability( size_t sz_, size_t bitsz_, int32_t scale_, uint32_t id_ = 0 ) noexcept
     : AnalogCapability( sz_, bitsz_, scale_ ), RoboObject( id_ ) {};
  protected:
};



class PinsRoboCapability : public PinsCapability, public RoboObject {
  public:
   explicit constexpr PinsRoboCapability( size_t bitsz_, uint32_t id_ = 0 ) noexcept
     : PinsCapability( bitsz_ ), RoboObject( id_ ) {};
  protected:
};



class PinRoboCapability : public PinCapability, public RoboObject {
  public:
   explicit constexpr PinRoboCapability( uint32_t id_ = 0 ) noexcept
     : PinCapability(), RoboObject( id_ ) {};
  protected:
};



class PwmRoboCapability : public PwmCapability, public RoboObject {
  public:
   explicit constexpr PwmRoboCapability( size_t sz_, size_t bitsz_, int32_t scale_, uint32_t id_ = 0 ) noexcept
     : PwmCapability( sz_, bitsz_, scale_ ), RoboObject( id_ ) {};
  protected:
};


class EncoderRoboCapability : public EncoderCapability, public RoboObject {
  public:
   explicit constexpr EncoderRoboCapability( size_t bitsz_, int32_t scale_, uint32_t id_ = 0 ) noexcept
     : EncoderCapability( bitsz_, scale_ ), RoboObject( id_ ) {};
  protected:
};

}; //namespace oxc


#endif

