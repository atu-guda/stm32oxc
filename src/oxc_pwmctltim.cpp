#include <algorithm>

#include <oxc_pwmctltim.h>

#include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;

bool oxc::PwmCtlTim::setFreq( float freq )
{
  auto tim = tim_p();
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
    if( ccrs_a[i] ) {
      *pccr( i ) = 0;
    }
  }

  if( was_enabled ) { // not reenable if error
    enable();
  }

  return true;
}

float oxc::PwmCtlTim::getFreq() const
{
  auto tim = tim_p();
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
  auto tim = tim_p();
  const auto a = tim->ARR;
  auto c = std::clamp( (uint32_t)( pwm * a / 0xFFFF ), (uint32_t)0, a );
  *pccr( ch ) = c;
  return true;
}

bool oxc::PwmCtlTim::setPwm(    size_t ch, float pwm )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  auto tim = tim_p();
  const auto a = tim->ARR;
  auto c = std::clamp( (uint32_t)( pwm * a ), (uint32_t)0, a );
  *pccr( ch ) = c;
  return true;
}

bool oxc::PwmCtlTim::setPulse(  std::size_t ch, uint32_t us )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  auto tim = tim_p();
  auto c = (int32_t)( (uint64_t) get_TIM_cnt_freq( tim ) * us / 1000000 );
  *pccr( ch ) = c;
  return true;
}

bool oxc::PwmCtlTim::setPwmRaw(  std::size_t ch, uint32_t v )
{
  if( isBadCh( ch ) ) {
    return false;
  }
  *pccr( ch ) = v;
  return true;
}

uint32_t oxc::PwmCtlTim::getPwmRaw(  std::size_t ch ) const
{
  if( isBadCh( ch ) ) {
    return 0;
  }
  return *pccr( ch );
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

// TODO: need arch-dependent traits // do not really good
ReturnCode oxc::PwmCtlTim::initHW( TIM_HandleTypeDef &t_h, uint32_t psc, uint32_t arr )
{
  auto tim = tim_p();
  if( !tim ) {
    return rcFatal;
  }
  t_h.Instance = tim;
  return tim_pwm_cfg_default( t_h, psc, arr, channels );
}

