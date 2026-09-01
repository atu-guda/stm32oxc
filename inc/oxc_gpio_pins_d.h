#ifndef _OXC_GPIO_PINS_D_H
#define _OXC_GPIO_PINS_D_H

#include <oxc_purecaps.h>
#include <oxc_gpio.h>

namespace oxc {

//* PinsCapability interface to sequence for the GPIO pins
class Gpio_Pins_Dev : public PinsPureCapability
{
  public:
   constexpr explicit Gpio_Pins_Dev( GpioRegs &gi_, PinNum a_start_, uint8_t a_n_ ) : pins( gi_, a_start_, a_n_ )  {}
   constexpr explicit Gpio_Pins_Dev( PortPin pp_, uint8_t a_n_ ) : pins( pp_, a_n_ ) {}
   ReturnCode initHW() noexcept { pins.initHW(); return rcOk; }
   PinsOut* getPins() noexcept { return &pins; } // low-level access: for special init...

   virtual int32_t_er read()             noexcept override { return pins.readUint();       }
   virtual int32_t_er readwr()           noexcept override { return pins.read_outUint();   }
   virtual void write(     int32_t v   ) noexcept override { pins.write(  PinMask( v ) );  }
   virtual void set(       int32_t v   ) noexcept override { pins.set(    PinMask( v ) );  }
   virtual void reset(     int32_t v   ) noexcept override { pins.reset(  PinMask( v ) );  }
   virtual void toggle(    int32_t v   ) noexcept override { pins.toggle( PinMask( v ) );  }
   virtual void setbit(    int32_t pos ) noexcept override { pins.setbit(    PinNum( pos ) );  }
   virtual void resetbit(  int32_t pos ) noexcept override { pins.resetbit(  PinNum( pos ) );  }
   virtual void togglebit( int32_t pos ) noexcept override { pins.togglebit( PinNum( pos ) );  }

  protected:
   PinsOut pins;
}; // Gpio_Pins_Dev

}; // namespace oxc

#endif

