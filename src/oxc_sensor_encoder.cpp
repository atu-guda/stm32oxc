#include <oxc_sensor_encoder.h>

using namespace oxc;


ReturnCode oxc::SensorEncoder::measure()
{
  auto vce = enc.get();
  if( !vce ) {
    return (sta = vce.error() ); 
  }
  uint32_t vc { vce.value() };

  dlt = (int32_t)vc - (int32_t)last_raw_v;

  // Phase unwrapping
  if( dlt > half_period ) {
    dlt -= signed_period;
  } else if (dlt < -half_period) {
    dlt += signed_period;
  }

  if( rev_dir ) {
    dlt = -dlt;
  }

  v += dlt;
  last_raw_v = vc;
  return (sta = rcOk);
}

