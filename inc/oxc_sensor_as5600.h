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

class SensorAS5600 : public RoboSensor {
  public:
   template<size_t N>
     SensorAS5600( const char (&name_)[N], AS5600 &dev_, bool rev_dir_ = false )
         : RoboSensor( name_, 1 ), // DUP?
           prox( dev_ ),
           enc( name_, prox, 0x01000, rev_dir_, 0x0FFF ) {};
   virtual ReturnCode initHW() override;
   virtual ReturnCode measure() override { return enc.measure(); }
   virtual int32_t get( size_t ch ) override { return enc.get( ch ); }
   virtual int32_t getScale( size_t ch ) { return enc.getScale( ch ); }
   static constexpr const float k_i2ph { 2 * pi_f / AS5600::val2turn };
  protected:
   EncoderProxyAS5600 prox;
   SensorEncoder enc;
};


}; // namespace oxc

#endif

