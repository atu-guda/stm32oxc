#include <oxc_sensor_adcint.h>

//* RoboSensor interface to inner ADC via ADC_Info: realization

using namespace oxc;

using std::begin;
using std::end;

ReturnCode oxc::SensorAdcInt::initHW()
{

  adc.prepare_multi_ev( n_ch, clock_div, sample_time, ADC_SOFTWARE_START, BOARD_ADC_DEFAULT_RESOLUTION );

  adc.data = buf;
  adc.reset_cnt();
  sta = rcOk;

  if( ! adc.init_common() ) {
    sta = rcErr;
  }

  return sta;
}

ReturnCode oxc::SensorAdcInt::measure()
{
  std::fill( begin(data), end(data), 0 );

  const uint32_t t_wait0 = 2; // ms?

  for( uint32_t av=0; av<n_aver; ++av ) {
    std::fill( begin(buf), end(buf), 0 );

    uint32_t r = adc.start_DMA_wait( n_ch, 1, t_wait0 );
    if( r != 0 ) {
      return ( sta = rcErr );
    }
    for( uint32_t i = 0; i<n_ch; ++i ) {
      data[i] += buf[i];
    }
  }

  for( uint32_t i = 0; i<n_ch; ++i ) {
    data[i] /= n_aver;
  }

  return (sta = rcOk);
}

