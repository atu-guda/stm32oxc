#ifndef _OXC_SENSOR_BASE_H
#define _OXC_SENSOR_BASE_H

#include <oxc_types.h>

using std::size_t;

namespace oxc {

class PhysicalSensor {
  public:
   PhysicalSensor( size_t n_ch_ ) : n_ch ( n_ch_ ) {};
   virtual ~PhysicalSensor() = default;
   virtual ReturnCode initHW() = 0;
   virtual ReturnCode measure() = 0;
   virtual int32_t get( size_t ch ) = 0;
   size_t size() const { return n_ch; };
   ReturnCode status() { return sta; }

  protected:
   size_t n_ch;
   ReturnCode sta { ReturnCode::rcnErr, 1 }; // uninitialised
};

class SensorBase {
  public:
   virtual ~SensorBase() = default;
   virtual float get() = 0;
   virtual int32_t get_i() = 0;
  protected:
};



}; // namespace oxc;

#endif

