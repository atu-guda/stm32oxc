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
   virtual int32_t get( size_t ch ) = 0; // single-channel sensors may ignore ch
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

class SensorSimple : public SensorBase {
  public:
   SensorSimple( PhysicalSensor &psens_, size_t ch_, float k_a_, float k_b_ ) : // TODO: object for conversion?
     psens( psens_ ), ch( ch_ ), k_a( k_a_ ), k_b( k_b_ ) {}
   virtual float get() override     { return psens.get( ch ) * k_a + k_b; };
   virtual int32_t get_i() override { return psens.get( ch ); }
  protected:
   PhysicalSensor &psens;
   size_t ch;
   float k_a, k_b;
};



}; // namespace oxc;

#endif

