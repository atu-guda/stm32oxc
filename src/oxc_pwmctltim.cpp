#include <algorithm>

#include <oxc_pwmctltim.h>

#include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;

bool oxc::PwmCtlTim::setFreq( float freq )
{
  if( !tim || freq < 1 ) {
    return false;
  }

  auto was_enabled = isEnabled();
  disable(); // TODO: store?

  if( allowPSCadj ) {
    uint32_t freq_in = get_TIM_in_freq( tim );
    auto [ psc, arr ] = calc_tim_psc_arr( freq_in, freq, arr_max, 0xFFFF ); // TODO: timer traits
    if( psc == 0xFFFFFFFF ) {
      return false;
    }
    tim->PSC = psc; tim->ARR = arr; tim->CNT = 0;
  } else {
    auto arr =  calc_TIM_arr_for_base_freq( tim, freq );
    tim->ARR = arr; tim->CNT = 0;
  }

  for( size_t i=0; i<n_ch; ++i ) { // Reset ccrs
    if( ccrs[i] ) {
      *ccrs[i] = 0;
    }
  }

  if( was_enabled ) { // not reenable if error
    enable();
  }

  return true;
}

float oxc::PwmCtlTim::getFreq() const
{
  if( !tim ) {
    return 0;
  }
  return get_TIM_base_freq_f( tim );
}

bool oxc::PwmCtlTim::setPwmU16( size_t ch, uint16_t pwm )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  const auto a = tim->ARR;
  auto c = std::clamp( (uint32_t)( pwm * a / 0xFFFF ), (uint32_t)0, a );
  *(ccrs[ch]) = c;
  return true;
}

bool oxc::PwmCtlTim::setPwm(    size_t ch, float pwm )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  const auto a = tim->ARR;
  auto c = std::clamp( (uint32_t)( pwm * a ), (uint32_t)0, a );
  *(ccrs[ch]) = c;
  return true;
}

bool oxc::PwmCtlTim::setPulse(  std::size_t ch, uint32_t us )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  auto c = (int32_t)( (uint64_t) get_TIM_cnt_freq( tim ) * us / 1000000 );
  *(ccrs[ch]) = c;
  return true;
}

bool oxc::PwmCtlTim::setPwmRaw(  std::size_t ch, uint32_t v )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  *(ccrs[ch]) = v;
  return true;
}

uint32_t oxc::PwmCtlTim::getPwmRaw(  std::size_t ch ) const
{
  if( isBadCh( ch ) ) {
    return 0;
  }
  return *(ccrs[ch]);
}

size_t oxc::PwmCtlTim::initPins()
{
  size_t n { 0 };
  for( auto chp : channels ) {
    chp.pin.enableClk();
    chp.pin.cfgAF( chp.af );
    ++n;
  }
  return n;
}

