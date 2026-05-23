#include <algorithm>

#include <oxc_pwmctltim.h>

#include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;

bool oxc::PwmCtlTim::setFreq( uint32_t freq )
{
  if( !tim || freq < 1 ) {
    return false;
  }
  auto a =  calc_TIM_arr_for_base_freq( tim, freq );
  UVAR_h = a;
  tim->ARR = a;
  return true;
}

uint32_t oxc::PwmCtlTim::getFreq() const
{
  if( !tim ) {
    return 0;
  }
  return get_TIM_base_freq( tim );
}

bool oxc::PwmCtlTim::setPwmU16( size_t ch, uint16_t pwm )
{
  if( !tim || ch >= n_ch ) {
    return false;
  }
  const auto a = tim->ARR;
  auto c = std::clamp( (uint32_t)( pwm * a / 0xFFFF ), (uint32_t)0, a );
  *(ccrs[ch]) = c;
  return false;
}

bool oxc::PwmCtlTim::setPwm(    size_t ch, float pwm )
{
  if( !tim || ch >= n_ch ) {
    return false;
  }
  const auto a = tim->ARR;
  auto c = std::clamp( (uint32_t)( pwm * a ), (uint32_t)0, a );
  *(ccrs[ch]) = c;
  return false;
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

