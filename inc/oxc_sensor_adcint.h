#ifndef _OXC_SENSOR_ADCINT_H
#define _OXC_SENSOR_ADCINT_H

//* RoboSensor interface to inner ADC via ADC_Info

#include <oxc_floatfun.h>
#include <oxc_adc.h>
#include <oxc_robo_base.h>



namespace oxc {


class SensorAdcInt : public RoboSensor {
  public:
   enum { max_inner_ch = 16 };
   template<size_t N>
     SensorAdcInt( const char (&name_)[N], ADC_Info &adc_, uint32_t n_aver_, uint32_t clock_div_, uint32_t sample_time_,
          uint32_t resol_ = ADC_RESOLUTION_12B )
         : RoboSensor( name_, adc_.n_ch_max ), adc( adc_ ), n_aver( n_aver_ ), clock_div( clock_div_), sample_time( sample_time_ ),
            resol( resol_), scale( ADC_resol_to_scale( resol_ ) )
           {};
   virtual ReturnCode initHW()  override;
   virtual ReturnCode measure() override;
   virtual int32_t get( size_t ch ) override { return data[ch]; }
   virtual int32_t getScale( size_t ch ) override { return scale; }
  protected:
   ADC_Info &adc;
   const uint32_t n_aver;
   const uint32_t clock_div;
   const uint32_t sample_time;
   const uint32_t resol;
   const uint32_t scale;
   uint16_t buf[max_inner_ch];
   int32_t  data[max_inner_ch];
};


} // namespace oxc

#endif // _OXC_SENSOR_ADCIN_H


