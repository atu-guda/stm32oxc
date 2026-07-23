#ifndef _OXC_SENSOR_AS5600_H
#define _OXC_SENSOR_AS5600_H


#include <oxc_floatfun.h>
#include <oxc_robo_base.h>
#include <oxc_as5600.h>

using oxc::RoboSensor;
using oxc::ReturnCode;

namespace oxc {

class SensorAS5600 : public RoboSensor {
  public:
   template<size_t N>
     SensorAS5600( const char (&name_)[N], AS5600 &dev_ )
         : RoboSensor( name_, 1 ), dev( dev_ ) {};
   virtual ReturnCode initHW() override;
   virtual ReturnCode measure() override;
   virtual int32_t get( size_t ch ) override;
   virtual int32_t getScale( size_t ch ) { return 0x01000; } // 12bit
   static constexpr const float k_i2ph { 2 * pi_f / AS5600::val2turn };
  protected:
   int32_t v { 0 };
   AS5600 &dev;
};

}; // namespace oxc

#endif

