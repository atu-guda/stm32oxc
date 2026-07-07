#ifndef _OXC_COORDTRANSFORM_H
#define _OXC_COORDTRANSFORM_H

#include <cmath>

#include <oxc_types.h>

using std::size_t;

namespace oxc {

struct CoordTransform {
  public:
   virtual ~CoordTransform() = default;
   virtual float   toPhys(   float iv ) = 0;
   virtual float toInternal( float pv ) = 0;
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
   virtual float toPhys( float iv )     override { return iv * a + b;      }
   virtual float toInternal( float pv ) override { return ( pv - b ) * ra; }
   static constexpr float not_small( float aa ) { return std::fabsf(aa) > 1e-9f ? aa : 1.0f; }
   float a, b, ra;
};

}; // namespace oxc;

#endif

