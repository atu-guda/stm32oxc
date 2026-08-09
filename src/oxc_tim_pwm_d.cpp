#include <algorithm>

#include <oxc_tim_pwm_d.h>

#include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;

bool oxc::Tim_Pwm_Dev::setFreq( float freq )
{
  auto tim = tim_p();
  if( !tim || freq < 1 ) {
    return false;
  }

  auto was_enabled = isEnabled();
  disable(); // TODO: store?

  if( allowPSCadj ) {
    uint32_t freq_in = get_TIM_in_freq( tim );
    auto [ psc_, arr_ ] = calc_tim_psc_arr( freq_in, freq, arr_max, 0xFFFF ); // TODO: timer traits
    if( psc == 0xFFFFFFFF ) {
      return false;
    }
    psc = psc_; arr = arr_;
    tim->PSC = psc; tim->ARR = arr; tim->CNT = 0;

  } else {
    arr =  calc_TIM_arr_for_base_freq( tim, freq );
    tim->ARR = arr; tim->CNT = 0;
  }

  for( size_t i=0; i<n_ch; ++i ) { // Reset ccrs
    if( ccrs_a[i] ) {
      *pccr( i ) = 0;
    }
  }

  if( was_enabled ) { // not reenable if error
    enable();
  }

  return true;
}

float oxc::Tim_Pwm_Dev::getFreq() const
{
  auto tim = tim_p();
  if( !tim ) {
    return 0;
  }
  return get_TIM_base_freq_f( tim );
}


// uint32_t oxc::Tim_Pwm_Dev::pwm2raw( float pwm )
// {
//   auto tim = tim_p();
//   const auto a = tim->ARR;
//   return std::clamp( (uint32_t)( pwm * a ), (uint32_t)0, a );
// }
//
// bool oxc::Tim_Pwm_Dev::setPwm(    size_t ch, float pwm )
// {
//   return setPwmRaw( ch, pwm2raw( pwm ) );
// }

// uint32_t oxc::Tim_Pwm_Dev::pulse2raw( uint32_t us )
// {
//   auto tim = tim_p();
//   return (int32_t)( (uint64_t) get_TIM_cnt_freq( tim ) * us / 1000000 ); // TODO cache cnt_freq
// }

// bool oxc::Tim_Pwm_Dev::setPulse(  std::size_t ch, uint32_t us )
// {
//   return setPwmRaw( ch, pulse2raw( us ) );
// }
//
// bool oxc::Tim_Pwm_Dev::setPwmRaw( std::size_t ch, uint32_t v )
// {
//   if( isBadCh( ch ) ) {
//     return false;
//   }
//   *pccr( ch ) = v;
//   return true;
// }
//
// uint32_t oxc::Tim_Pwm_Dev::getPwmRaw(  std::size_t ch ) const
// {
//   if( isBadCh( ch ) ) {
//     return 0;
//   }
//   return *pccr( ch );
// }

void oxc::Tim_Pwm_Dev::initPins()
{
  for( auto chp : channels ) {
    chp.pin.enableClk();
    chp.pin.cfgAF( chp.af );
  }
}

// TODO: need arch-dependent traits // do not really good
ReturnCode oxc::Tim_Pwm_Dev::setHardParams( uint32_t psc_, uint32_t arr_, uint32_t cmode_ )
{
  psc = psc_; arr = arr_; cmode = cmode_;
  return rcOk;
}

// TODO: need arch-dependent traits // do not really good
ReturnCode oxc::Tim_Pwm_Dev::initHW()
{
  auto tim = tim_p();
  if( !tim ) {
    return rcFatal;
  }
  t_h.Instance = tim;
  auto rc = tim_pwm_cfg_default( t_h, psc, arr, channels, cmode );
  if( rc != rcOk ) {
    return rc;
  }
  initPins();
  return rcOk;
}

