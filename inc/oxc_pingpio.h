#ifndef _OXC_PINGPIO_H
#define _OXC_PINGPIO_H

#include <oxc_pinbase.h>
#include <oxc_gpio.h>

// -------------- PinGpio ---------------------------------------
// for generic interface
namespace oxc {
class PinGpio : public PinBase
{
  public:
   constexpr explicit PinGpio( const PinOut& pin_ ) : pin( pin_ )    {}
   constexpr explicit PinGpio( PortPin portpin    ) : pin( portpin ) {}
   virtual void set()           override { pin.set();            }
   virtual void reset()         override { pin.reset();          }
   virtual void write( bool v ) override { pin.write( v );       }
   virtual void toggle()        override { pin.toggle();         }
   virtual bool get()           override { return pin.read_in(); }
   virtual ReturnCode initHW()  override { return pin.initHW();  }
  protected:
   PinOut pin;
};

}; // namespace oxc

#endif
