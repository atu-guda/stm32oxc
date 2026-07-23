#ifndef _OXC_SENSOR_AS5600_H
#define _OXC_SENSOR_AS5600_H


#include <oxc_floatfun.h>
#include <oxc_robo_base.h>
#include <oxc_sensor_encoder.h>
#include <oxc_as5600.h>

using oxc::RoboSensor;
using oxc::ReturnCode;

namespace oxc {

//* class to encoder counter access to/from AS5600
class EncoderProxyAS5600 : public EncoderProxy {
  public:
   explicit constexpr EncoderProxyAS5600( AS5600 &dev_ ) noexcept: dev ( dev_ ) {};
   exprc_uint32_t get()             override;
   ReturnCode     set( uint32_t v ) override;
   AS5600& getDev() { return dev; }
  protected:
   AS5600 &dev;
};

//* utility class to get correct initialization order in the SensorAS5600
class SensorAS5600Base {
  protected:
   EncoderProxyAS5600 prox;
     SensorAS5600Base( AS5600 &dev_ ) noexcept : prox( dev_ ) {}
};

class SensorAS5600 : public SensorAS5600Base, public SensorEncoder {
  public:
   template<size_t N>
     SensorAS5600( const char (&name_)[N], AS5600 &dev_, bool rev_dir_ = false )
         : SensorAS5600Base( dev_ ),
           SensorEncoder( name_, prox, AS5600::val2turn, rev_dir_, AS5600::val2turn - 1 )
           {};
   virtual ReturnCode initHW() override;
   static constexpr const float k_i2ph { 2 * pi_f / AS5600::val2turn };
  protected:
};


}; // namespace oxc

#endif

