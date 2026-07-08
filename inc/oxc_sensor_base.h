#ifndef _OXC_SENSOR_BASE_H
#define _OXC_SENSOR_BASE_H

#include <cmath>

#include <oxc_types.h>
#include <oxc_robo_base.h>
#include <oxc_coordtransform.h>

using std::size_t;

namespace oxc {


//* physycal part of robo sensors with channels
class RoboSensor : public RoboDevice {
  public:
   template<size_t N> // for only string literals as name
     constexpr explicit RoboSensor( const char (&name_)[N], size_t n_ch_ ) noexcept
     : RoboDevice( name_ ), n_ch ( n_ch_ ) {};
   virtual ReturnCode commit() override { return rcOk; }
   virtual int32_t get( size_t ch ) = 0; // single-channel sensors may ignore ch
   size_t size() const { return n_ch; };
   ReturnCode status() { return sta; }

  protected:
   const size_t n_ch;
   ReturnCode sta { ReturnCode::rcnErr, 1 }; // uninitialised
};

//* selects and scale 1 channel of the RoboSensor
class SensorBase {
  public:
   explicit constexpr SensorBase( RoboSensor &psens_, size_t ch_, CoordTransform &coo_tr_ )
     : psens( psens_ ), ch( ch_ ), coo_tr( coo_tr_ ) {}
   virtual ~SensorBase() = default;
   virtual float     get()  { return coo_tr.toPhys( psens.get( ch ) ); }
   virtual int32_t get_i()  { return                psens.get( ch ); }
  protected:
   RoboSensor &psens;
   const size_t ch;
   CoordTransform &coo_tr;
};



}; // namespace oxc;

#endif

