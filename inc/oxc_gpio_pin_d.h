#ifndef _OXC_GPIO_PIN_D_H
#define _OXC_GPIO_PIN_D_H

#include <oxc_purecaps.h>
#include <oxc_gpio.h>

namespace oxc {

//* PinCapability interface to one GPIO pin
// as workhorse class is small, contain it by value
class Gpio_Pin_Dev : public PinPureCapability
{
  public:
   constexpr explicit Gpio_Pin_Dev( const PinOut& pin_ ) : pin( pin_ )    {}
   constexpr explicit Gpio_Pin_Dev( PortPin portpin_ ) :   pin( portpin_ ) {}
   ReturnCode initHW() noexcept { return pin.initHW();  }
   PinOut* getPin() noexcept { return &pin; } // low-level access: for special init...

   virtual int32_t_er read()    noexcept override { return pin.read_in();  }
   virtual int32_t_er readwr()  noexcept override { return pin.read_out(); }
   virtual void write( bool v ) noexcept override { pin.write( v );        }
   virtual void set()           noexcept override { pin.set();             }
   virtual void reset()         noexcept override { pin.reset();           }
   virtual void toggle()        noexcept override { pin.toggle();          }

  protected:
   PinOut pin;
}; // Gpio_Pins_Dev



}; // namespace oxc

#endif

