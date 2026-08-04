#ifndef _OXC_GPIO_D_H
#define _OXC_GPIO_D_H

#include <oxc_capabilities.h>
#include <oxc_gpio.h>

namespace oxc {

//* PinCapability interfae to one GPIO pin
// as workhorse class is small, contain it by value
class Gpio_Pin_Dev : public PinCapability
{
  public:
   constexpr explicit Gpio_Pin_Dev( const PinOut& pin_ ) : pin( pin_ )    {}
   constexpr explicit Gpio_Pin_Dev( PortPin portpin    ) : pin( portpin ) {}

   // IOCapability:
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override { pin.write((bool)v); return rcOk; }
   virtual int32_t_er getVal( size_t ch ) noexcept override { return pin.read_out(); }

   // PinCapability:
   virtual int32_t_er read()    noexcept override { return pin.read_out(); }
   virtual void write( bool v ) noexcept override { pin.write( v );        }
   virtual void set()           noexcept override { pin.set();             }
   virtual void reset()         noexcept override { pin.reset();           }
   virtual void toggle()        noexcept override { pin.toggle();          }

   ReturnCode initHW() noexcept { return pin.initHW();  }
  protected:
   PinOut pin;
};

}; // namespace oxc

#endif
