#ifndef _ENDSTOPGPIO_H
#define _ENDSTOPGPIO_H

#include <oxc_gpio.h>
#include "endstop.h"

// 2+ pins with positive clear status
class EndStopGpioPos : public EndStop {
  public:
   enum StopBits { minusBit = 0x01, plusBit = 0x02, extraBit = 0x04, mainBits = 0x03 };
   EndStopGpioPos( GpioRegs &a_gi, uint8_t a_start, uint8_t a_n = 2 )
     : pins( a_gi, a_start, a_n, GpioRegs::Pull::down ) {}
   virtual ReturnCode initHW() override { pins.initHW(); return rcOk; };
   virtual uint16_t read() override { v = pins.read(); return v; }
   virtual bool is_minus_stop() const override { return ( ( v & minusBit ) == 0 ) ; }
   virtual bool is_minus_go()   const override { return ( ( v & minusBit ) != 0 ) ; }
   virtual bool is_plus_stop()  const override { return ( ( v & plusBit  ) == 0 ) ; }
   virtual bool is_plus_go()    const override { return ( ( v & plusBit  ) != 0 ) ; }
   virtual bool is_any_stop()   const override { return ( ( v & mainBits ) != mainBits ) ; }
   virtual bool is_clear()      const override { return ( ( v & mainBits ) == mainBits ) ; }
   virtual bool is_bad()        const override { return ( ( v & mainBits ) == 0 ) ; }
   virtual bool is_clear_for_dir( int dir ) const override;
   virtual char toChar() const override {
     static const char es_chars[] { "X+-.??" };
     return es_chars[ v & mainBits];
   }
  protected:
   PinsIn pins;
};



#endif

