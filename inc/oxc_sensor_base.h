#ifndef _OXC_SENSOR_BASE_H
#define _OXC_SENSOR_BASE_H

#include <cmath>

#include <oxc_types.h>
#include <oxc_robo_base.h>
#include <oxc_coordtransform.h>

using std::size_t;

namespace oxc {


// TODO: remove: use RoboDevice or its childs
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
   explicit constexpr SensorBase( PhysicalSensor &psens_, size_t ch_, CoordTransform &coo_tr_ )
     : psens( psens_ ), ch( ch_ ), coo_tr( coo_tr_ ) {}
   virtual ~SensorBase() = default;
   virtual float get()      { return coo_tr.toPhys( psens.get( ch ) ); }
   virtual int32_t get_i()  { return                psens.get( ch ); }
  protected:
   PhysicalSensor &psens;
   size_t ch;
   CoordTransform &coo_tr;
};



}; // namespace oxc;

#endif

