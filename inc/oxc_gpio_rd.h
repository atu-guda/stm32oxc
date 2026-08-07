#ifndef _OXC_GPIO_RD_H
#define _OXC_GPIO_RD_H

#include <oxc_capabilities.h>
#include <oxc_gpio.h>

namespace oxc {

//* PinRoboCapability interface to one GPIO pin
//* here: read ignored
// channels: 0 - write, 1 - read
class Gpio_OPin_RDev : public PinRoboCapability
{
  public:
   constexpr explicit Gpio_OPin_RDev( const PinOut& pin_, const ValueTransform1x1 &tr_ = globalUnityValueTransform, uint32_t id_ = 0 ) :
     PinRoboCapability( tr_, id_ ), pin( pin_ )    {}
   constexpr explicit Gpio_OPin_RDev( PortPin portpin_, const ValueTransform1x1 &tr_ = globalUnityValueTransform,  uint32_t id_ = 0 ) :
     PinRoboCapability( tr_, id_ ), pin( portpin_ )    {}

   // Pin[Robo]Capability:
   virtual int32_t_er read()    noexcept override { return vv[ch_in];               }
   virtual void write( bool v ) noexcept override { setVal( ch_out, (int32_t)v );   }
   virtual void set()           noexcept override { setVal( ch_out, 1 );            }
   virtual void reset()         noexcept override { setVal( ch_out, 0 );            }
   virtual void toggle()        noexcept override { setVal( ch_out, !vv[ch_out] );  }

   ReturnCode initHW() noexcept { return pin.initHW();  }
   PinOut* getPin() noexcept { return &pin; } // low-level access: for special init...

   // RoboObject:
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[ch_out] = 0; vv[ch_in] = 0; return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { return rcOk; }
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override { if( dirty & ch_in_bit ) { pin.write( vv[ch_out] ); }; return rcOk; }
  protected:
   PinOut pin;
}; // Gpio_OPin_RDev

class Gpio_Pin_RDev : public Gpio_OPin_RDev
{
  public:
   constexpr explicit Gpio_Pin_RDev( const PinOut& pin_, const ValueTransform1x1 &tr_ = globalUnityValueTransform, uint32_t id_ = 0 ) :
     Gpio_OPin_RDev( pin_, tr_, id_ )   {}
   constexpr explicit Gpio_Pin_RDev( PortPin portpin_, const ValueTransform1x1 &tr_ = globalUnityValueTransform, uint32_t id_ = 0 ) :
     Gpio_OPin_RDev( portpin_, tr_, id_ )   {}
  protected:
   virtual ReturnCode doMeasure() noexcept override { vv[1] = pin.read_out(); return rcOk; } // real read

}; // Gpio_Pin_RDev


//* PinsRoboCapability interface to sequence of GPIO pins
//* here: read ignored
// channels: 0 - write, 1 - read
class Gpio_OPins_RDev : public PinsRoboCapability
{
  public:
   constexpr explicit Gpio_OPins_RDev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_,
       const ValueTransform1x1 &tr_ = globalUnityValueTransform,  uint32_t id_ = 0 ) :
     PinsRoboCapability( a_n_, tr_, id_ ), pins( gi_, a_start_, a_n_ )    {}
   constexpr explicit Gpio_OPins_RDev( PortPin pp_, uint8_t a_n_,
       const ValueTransform1x1 &tr_ = globalUnityValueTransform, uint32_t id_ = 0 ) :
     PinsRoboCapability( a_n_, tr_, id_ ), pins( pp_, a_n_ )    {}

   // Pins[Robo]Capability:
   virtual int32_t_er read()             noexcept override { return vv[ch_out];             }
   virtual void write(     int32_t v   ) noexcept override { setVal( ch_out, v );           }
   virtual void set(       int32_t v   ) noexcept override { setVal( ch_out, vv[ch_out] |  v );  }
   virtual void reset(     int32_t v   ) noexcept override { setVal( ch_out, vv[ch_out] & ~v );  }
   virtual void toggle(    int32_t v   ) noexcept override { setVal( ch_out, vv[ch_out] ^  v );  }
   virtual void setbit(    int32_t pos ) noexcept override { setVal( ch_out, vv[ch_out] |  ( 1 << pos ) );  }
   virtual void resetbit(  int32_t pos ) noexcept override { setVal( ch_out, vv[ch_out] & ~( 1 << pos ) );  }
   virtual void togglebit( int32_t pos ) noexcept override { setVal( ch_out, vv[ch_out] ^  ( 1 << pos ) );  }

   ReturnCode initHW() noexcept { pins.initHW(); return rcOk; }
   PinsOut* getPins() noexcept { return &pins; } // low-level access: for special init...

   // RoboObject:
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[ch_out] = 0; vv[ch_in] = 0; return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { return rcOk; } // ignored here
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override {
     if( dirty & ch_out_bit ) {
       pins.write( PinMask(vv[ch_out]) );
     };
     return rcOk;
   }
  protected:
   PinsOut pins;
}; // Gpio_OPins_RDev


//* the same but with real read
class Gpio_Pins_RDev : public Gpio_OPins_RDev {
  public:
   constexpr explicit Gpio_Pins_RDev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_,
       const ValueTransform1x1 &tr_ = globalUnityValueTransform, uint32_t id_ = 0 ) :
     Gpio_OPins_RDev( gi_, a_start_, a_n_, tr_, id_ )    {}
   constexpr explicit Gpio_Pins_RDev( PortPin pp_, uint8_t a_n_,
       const ValueTransform1x1 &tr_ = globalUnityValueTransform, uint32_t id_ = 0 ) :
     Gpio_OPins_RDev( pp_, a_n_, tr_, id_ )    {}

   // RoboObject:
  protected:
   virtual ReturnCode doMeasure() noexcept override { vv[ch_in] = pins.readUint(); return rcOk; }
}; // Gpio_OPins_RDev



}; // namespace oxc

#endif
