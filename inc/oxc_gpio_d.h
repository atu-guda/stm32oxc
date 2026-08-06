#ifndef _OXC_GPIO_D_H
#define _OXC_GPIO_D_H

#include <oxc_capabilities.h>
#include <oxc_gpio.h>

namespace oxc {

//* PinCapability interface to one GPIO pin
// as workhorse class is small, contain it by value
class Gpio_Pin_Dev : public PinCapability
{
  public:
   constexpr explicit Gpio_Pin_Dev( const PinOut& pin_ ) : pin( pin_ )    {}
   constexpr explicit Gpio_Pin_Dev( PortPin portpin_   ) : pin( portpin_ ) {}

   // IOCapability:
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override
     { if( ch != 0 ) { return rcErr; }; pin.write((bool)v); return rcOk; }
   virtual int32_t_er getVal( size_t ch ) noexcept override
     { if( ch == 1 ) { return pin.read_in(); }
       if( ch == 0 ) { return pin.read_out(); }
       return std::unexpected( rcErr );
     }

   // PinCapability:
   virtual int32_t_er read()    noexcept override { return pin.read_in(); }
   virtual void write( bool v ) noexcept override { pin.write( v );        }
   virtual void set()           noexcept override { pin.set();             }
   virtual void reset()         noexcept override { pin.reset();           }
   virtual void toggle()        noexcept override { pin.toggle();          }

   ReturnCode initHW() noexcept { return pin.initHW();  }
   PinOut* getPin() noexcept { return &pin; } // low-level access: for special init...
  protected:
   PinOut pin;
};


//* PinsCapability interface to sequence for the GPIO pins
class Gpio_Pins_Dev : public PinsCapability
{
  public:
   constexpr explicit Gpio_Pins_Dev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_ )
     : PinsCapability( a_n_ ), pins( gi_, a_start_, a_n_ )  {}
   constexpr explicit Gpio_Pins_Dev( PortPin pp_, uint8_t a_n_ )
     : PinsCapability( a_n_ ), pins( pp_, a_n_ ) {}

   // IOCapability:
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override
     { if( ch != 0 ) { return rcErr; }; pins.write( PinMask(v) ); return rcOk; }
   virtual int32_t_er getVal( size_t ch ) noexcept override
     { if( ch == 1 ) { return pins.readUint(); }
       if( ch == 0 ) { return pins.read_outUint(); }
       return std::unexpected( rcErr );
     }

   // PinsCapability:
   virtual int32_t_er read()             noexcept override { return pins.readUint();       }
   virtual void write(     int32_t v   ) noexcept override { pins.write( PinMask( v ) );   }
   virtual void set(       int32_t v   ) noexcept override { pins.set( PinMask( v ) );     }
   virtual void reset(     int32_t v   ) noexcept override { pins.reset( PinMask( v ) );   }
   virtual void toggle(    int32_t v   ) noexcept override { pins.toggle( PinMask( v ) );  }
   virtual void setbit(    int32_t pos ) noexcept override { pins.setbit(    PinNum( pos ) );  }
   virtual void resetbit(  int32_t pos ) noexcept override { pins.resetbit(  PinNum( pos ) );  }
   virtual void togglebit( int32_t pos ) noexcept override { pins.togglebit( PinNum( pos ) );  }

   ReturnCode initHW() noexcept { pins.initHW(); return rcOk; }
   PinsOut* getPins() noexcept { return &pins; } // low-level access: for special init...
  protected:
   PinsOut pins;
};

}; // namespace oxc

#endif
