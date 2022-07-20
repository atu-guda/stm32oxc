#ifndef _OXC_STEPMOTOR_GPIO_H
#define _OXC_STEPMOTOR_GPIO_H

// stepmotor control via gpio

#include <oxc_gpio.h>
#include <oxc_stepmotor.h>


class StepMotorDriverGPIO : public StepMotorDriverBase {
  public:
   explicit StepMotorDriverGPIO( PinsOut &a_pins ) noexcept : pins( a_pins ) {}
   virtual void set( uint16_t outs ) noexcept override { pins.write( outs ); };
   virtual void init() noexcept override { pins.initHW(); };
  protected:
   PinsOut &pins;
};

class StepMotorDriverGPIO_e : public StepMotorDriverBase {
  public:
   explicit StepMotorDriverGPIO_e( GpioRegs &gi, uint8_t a_start, uint8_t a_n ) noexcept : pins( gi, a_start, a_n ) {}
   virtual void set( uint16_t outs ) noexcept override { pins.write( outs ); };
   virtual void init() noexcept override { pins.initHW(); };
  protected:
   PinsOut pins;
};


#endif

