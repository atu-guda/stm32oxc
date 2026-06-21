#ifndef _OXC_SENSOR_BASE_H
#define _OXC_SENSOR_BASE_H

#include <cmath>

#include <oxc_types.h>

using std::size_t;

namespace oxc {

struct CoordTransform {
  public:
   virtual ~CoordTransform() = default;
   virtual float   toPhys( int32_t iv ) = 0;
   virtual int32_t toInternal( float pv ) = 0;
};

struct LinearCoordTransform : public CoordTransform {
  public:
   struct PhysicalToInternalInit {};
   //* default coeffs is internal to Physical, a!=0, ra!=0
   constexpr LinearCoordTransform( float a_, float b_ )
     : a( not_small(a_) ), b( b_ ), ra (1.0f/a) {}
   //* coeffs for Physical to internal
   constexpr LinearCoordTransform( float ra_, float rb_, PhysicalToInternalInit /*in*/)
     :  a( 1.0f/not_small(ra_) ), b( -rb_ ), ra( not_small(ra_) ) {}
   virtual float   toPhys( int32_t iv )  override  { return iv * a + b;  };
   virtual int32_t toInternal( float pv ) override { return int32_t( ( ( pv - b ) * ra ) + 0.499f ); }
   static constexpr float not_small( float aa ) { return std::fabsf(aa) > 1e-9f ? aa : 1.0f; }
   float a, b, ra;
};

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

