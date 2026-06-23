#ifndef _OXC_SENSOR_AS5600_H
#define _OXC_SENSOR_AS5600_H


#include <oxc_floatfun.h>
#include <oxc_sensor_base.h>
#include <oxc_as5600.h>

using oxc::PhysicalSensor;
using oxc::ReturnCode;

namespace oxc {

class SensorAS5600 : public PhysicalSensor {
  public:
   SensorAS5600( AS5600 &dev_ ) : PhysicalSensor( 1 ), dev( dev_ ) {};
   virtual ReturnCode initHW() override;
   virtual ReturnCode measure() override;
   virtual int32_t get( size_t ch ) override;
   static constexpr const float k_i2ph { 2 * pi_f / AS5600::val2turn };
  protected:
   int32_t v { 0 };
   AS5600 &dev;
};

}; // namespace oxc

#endif

