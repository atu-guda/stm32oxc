#ifndef _OXC_GPIO_RD_H
#define _OXC_GPIO_RD_H

#include <oxc_capabilities.h>
#include <oxc_gpio.h>

namespace oxc {

//* PinRoboCapability interface to one GPIO pin
//* here: read and write ignored
// channels: 0 - write, 1 - read
class Gpio_XPin_RDev : public PinRoboCapability
{
  public:
   constexpr explicit Gpio_XPin_RDev( const PinOut& pin_, const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     PinRoboCapability( tr_, id_ ), pin( pin_ )    {}
   constexpr explicit Gpio_XPin_RDev( PortPin portpin_, const ValFiTrans1x1 &tr_ = globalUnityValFiTrans,  uint32_t id_ = 0 ) :
     PinRoboCapability( tr_, id_ ), pin( portpin_ )    {}

   // Pin[Robo]Capability:
   // not here: OPin

   ReturnCode initHW() noexcept { return pin.initHW();  }
   PinOut* getPin() noexcept { return &pin; } // low-level access: for special init...

   // RoboObject:
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[ch_out] = 0; vv[ch_in] = 0; return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { return rcOk; } // Ignore
   virtual ReturnCode doThink()   noexcept override { return rcOk; } // Not here
   virtual ReturnCode doCommit()  noexcept override { return rcOk; } // Ignore
  protected:
   PinOut pin;
}; // Gpio_XPin_RDev


//* only input implemented
class Gpio_IPin_RDev : public Gpio_XPin_RDev
{
  public:
   constexpr explicit Gpio_IPin_RDev( const PinOut& pin_, const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_XPin_RDev( pin, tr_, id_ ) {}
   constexpr explicit Gpio_IPin_RDev( PortPin portpin_, const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_XPin_RDev( portpin_, tr_, id_ ) {}

   // Pin[Robo]Capability: part
   virtual int32_t_er read()    noexcept override { return vv[ch_in];               }
   virtual void write( bool v ) noexcept override { /* NOP */                       }
   virtual void set()           noexcept override { /* NOP */                       }
   virtual void reset()         noexcept override { /* NOP */                       }
   virtual void toggle()        noexcept override { /* NOP */                       }

   // RoboObject:
   virtual ReturnCode doMeasure() noexcept override { vv[ch_in] = pin.read_in(); return rcOk; } // real read
  protected:
}; // Gpio_IPin_RDev


//* only output implemented
class Gpio_OPin_RDev : public Gpio_XPin_RDev
{
  public:
   constexpr explicit Gpio_OPin_RDev( const PinOut& pin_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_XPin_RDev( pin, tr_, id_ ) {}
   constexpr explicit Gpio_OPin_RDev( PortPin portpin_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans,  uint32_t id_ = 0 ) :
     Gpio_XPin_RDev( portpin_, tr_, id_ ) {}

   // Pin[Robo]Capability:
   virtual int32_t_er read()    noexcept override { return vv[ch_in];               }
   virtual void write( bool v ) noexcept override { setVal( ch_out, (int32_t)v );   }
   virtual void set()           noexcept override { setVal( ch_out, 1 );            }
   virtual void reset()         noexcept override { setVal( ch_out, 0 );            }
   virtual void toggle()        noexcept override { setVal( ch_out, !vv[ch_out] );  }

   // RoboObject:
  protected:
   virtual ReturnCode doCommit()  noexcept override { if( dirty & ch_out_bit ) { pin.write( vv[ch_out] ); }; return rcOk; }
}; // Gpio_OPin_RDev


//* Both I/O implemented
class Gpio_Pin_RDev : public Gpio_OPin_RDev
{
  public:
   constexpr explicit Gpio_Pin_RDev( const PinOut& pin_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_OPin_RDev( pin_, tr_, id_ )   {}
   constexpr explicit Gpio_Pin_RDev( PortPin portpin_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_OPin_RDev( portpin_, tr_, id_ )   {}
  protected:
   virtual ReturnCode doMeasure() noexcept override { vv[ch_in] = pin.read_in(); return rcOk; } // real read

}; // Gpio_Pin_RDev




//* PinsRoboCapability interface to sequence of GPIO pins
//* here: read and write ignored
// channels: 0 - write, 1 - read
class Gpio_XPins_RDev : public PinsRoboCapability
{
  public:
   constexpr explicit Gpio_XPins_RDev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans,  uint32_t id_ = 0 ) :
     PinsRoboCapability( a_n_, tr_, id_ ), pins( gi_, a_start_, a_n_ )    {}
   constexpr explicit Gpio_XPins_RDev( PortPin pp_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     PinsRoboCapability( a_n_, tr_, id_ ), pins( pp_, a_n_ )    {}

   // Pins[Robo]Capability:
   virtual int32_t_er read()             noexcept override { return vv[ch_out];             }

   ReturnCode initHW() noexcept { pins.initHW(); return rcOk; }
   PinsOut* getPins() noexcept { return &pins; } // low-level access: for special init...

   // RoboObject:
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[ch_out] = 0; vv[ch_in] = 0; return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { return rcOk; } // ignored here
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override { return rcOk; } // ignored here
  protected:
   PinsOut pins;
}; // Gpio_XPins_RDev


//* only output implemented
class Gpio_OPins_RDev : public Gpio_XPins_RDev
{
  public:
   constexpr explicit Gpio_OPins_RDev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans,  uint32_t id_ = 0 ) :
     Gpio_XPins_RDev( gi_, a_start_, a_n_, tr_, id_ ) {}
   constexpr explicit Gpio_OPins_RDev( PortPin pp_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_XPins_RDev( pp_, a_n_, tr_, id_ ) {}

   // Pins[Robo]Capability:
   // virtual int32_t_er read()             noexcept override { return vv[ch_out];             } // keep
   virtual void write(     int32_t v   ) noexcept override { setVal( ch_out, v );           }
   virtual void set(       int32_t v   ) noexcept override { setVal( ch_out, vv[ch_out] |  v );  }
   virtual void reset(     int32_t v   ) noexcept override { setVal( ch_out, vv[ch_out] & ~v );  }
   virtual void toggle(    int32_t v   ) noexcept override { setVal( ch_out, vv[ch_out] ^  v );  }
   virtual void setbit(    int32_t pos ) noexcept override { setVal( ch_out, vv[ch_out] |  ( 1 << pos ) );  }
   virtual void resetbit(  int32_t pos ) noexcept override { setVal( ch_out, vv[ch_out] & ~( 1 << pos ) );  }
   virtual void togglebit( int32_t pos ) noexcept override { setVal( ch_out, vv[ch_out] ^  ( 1 << pos ) );  }

   // RoboObject:
  protected:
   virtual ReturnCode doCommit()  noexcept override {
     if( dirty & ch_out_bit ) {
       pins.write( PinMask(vv[ch_out]) );
     };
     return rcOk;
   }
  protected:
}; // Gpio_OPins_RDev


//* only input implemented
class Gpio_IPins_RDev : public Gpio_XPins_RDev
{
  public:
   constexpr explicit Gpio_IPins_RDev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans,  uint32_t id_ = 0 ) :
     Gpio_XPins_RDev( gi_, a_start_, a_n_, tr_, id_ ) {}
   constexpr explicit Gpio_IPins_RDev( PortPin pp_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_XPins_RDev( pp_, a_n_, tr_, id_ ) {}

   // Pins[Robo]Capability:
   // virtual int32_t_er read()             noexcept override { return vv[ch_out];             } // keep
   virtual void write(     int32_t v   ) noexcept override { /* NOP */ }
   virtual void set(       int32_t v   ) noexcept override { /* NOP */ }
   virtual void reset(     int32_t v   ) noexcept override { /* NOP */ }
   virtual void toggle(    int32_t v   ) noexcept override { /* NOP */ }
   virtual void setbit(    int32_t pos ) noexcept override { /* NOP */ }
   virtual void resetbit(  int32_t pos ) noexcept override { /* NOP */ }
   virtual void togglebit( int32_t pos ) noexcept override { /* NOP */ }

   // RoboObject:
  protected:
   virtual ReturnCode doMeasure() noexcept override { vv[ch_in] = pins.readUint(); return rcOk; }
  protected:
}; // Gpio_IPins_RDev


//* Both I/O implemented
class Gpio_Pins_RDev : public Gpio_OPins_RDev {
  public:
   constexpr explicit Gpio_Pins_RDev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_OPins_RDev( gi_, a_start_, a_n_, tr_, id_ )    {}
   constexpr explicit Gpio_Pins_RDev( PortPin pp_, uint8_t a_n_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans, uint32_t id_ = 0 ) :
     Gpio_OPins_RDev( pp_, a_n_, tr_, id_ )    {}

   // RoboObject:
  protected:
   virtual ReturnCode doMeasure() noexcept override { vv[ch_in] = pins.readUint(); return rcOk; }
}; // Gpio_OPins_RDev



}; // namespace oxc

#endif
