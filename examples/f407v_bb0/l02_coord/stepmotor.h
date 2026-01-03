#ifndef _STEPMOTOR_H
#define _STEPMOTOR_H

#include <oxc_gpio.h>


//* abstract class for different controlled stepmotors
class StepMotor {
  public:
   virtual ReturnCode initHW() = 0;
   virtual void step() = 0;
   virtual void set_dir( int d ) = 0;
   int get_dir() const { return dir; }
  protected:
   int dir = 0;
};

//* fake motor
class StepMotorFake : public StepMotor{
  public:
   virtual ReturnCode initHW() override { return rcOk; }
   virtual void step() override {};
   virtual void set_dir( int d ) override { dir = d; }
  protected:
};


//* stepmotor, controlled by 2 squential GPIO pins, first - step, second - dir
class StepMotorGpio2 : public StepMotor {
  public:
   enum { pinStep = 0x01, pinDir = 0x02 };
   StepMotorGpio2( PortPin start )
     : pins( start, 2 ) {}
   virtual ReturnCode initHW() override { pins.initHW(); pins.write( 0_mask ); return rcOk; }
   virtual void step() override;
   virtual void set_dir( int d ) override;
  protected:
   PinsOut pins;
   static const unsigned pulse_us { 2 };
};

#endif


// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc
