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
   constexpr explicit Gpio_OPin_RDev( const PinOut& pin_, uint32_t id_ = 0 ) :
     PinRoboCapability( id_ ), pin( pin_ )    {}
   constexpr explicit Gpio_OPin_RDev( PortPin portpin_, uint32_t id_ = 0 ) :
     PinRoboCapability( id_ ), pin( portpin_ )    {}

   // Pin[Robo]Capability:
   virtual int32_t_er read()    noexcept override { return vv[1];             }
   virtual void write( bool v ) noexcept override { setVal( 0, (int32_t)v );  }
   virtual void set()           noexcept override { setVal( 0, 1 );           }
   virtual void reset()         noexcept override { setVal( 0, 0 );           }
   virtual void toggle()        noexcept override { setVal( 0, !vv[0] );      }

   ReturnCode initHW() noexcept { return pin.initHW();  }

   // RoboObject:
  protected:
   virtual ReturnCode doInit()    noexcept override { vv[0] = 0; vv[1] = 0;   return rcOk; }
   virtual ReturnCode doMeasure() noexcept override { return rcOk; }
   virtual ReturnCode doThink()   noexcept override { return rcOk; }
   virtual ReturnCode doCommit()  noexcept override { if( dirty & 1 ) { pin.write( vv[0] ); }; return rcOk; }
  protected:
   PinOut pin;

};

class Gpio_Pin_RDev : public Gpio_OPin_RDev
{
  public:
   constexpr explicit Gpio_Pin_RDev( const PinOut& pin_, uint32_t id_ = 0 ) :
     Gpio_OPin_RDev( pin_, id_ )   {}
   constexpr explicit Gpio_Pin_RDev( PortPin portpin_, uint32_t id_ = 0 ) :
     Gpio_OPin_RDev( portpin_, id_ )   {}
  protected:
   virtual ReturnCode doMeasure() noexcept override { vv[1] = pin.read_out(); return rcOk; } // real read

};

}; // namespace oxc

#endif
