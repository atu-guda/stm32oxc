#include <algorithm>

#include <oxc_tim_pwm_d.h>

#include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;

uint32_t   oxc::Tim_Pwm_Dev::duty2raw(  float duty ) const noexcept
{
  return std::clamp( (uint32_t)( duty * arr ), (uint32_t)0, arr );
}


uint32_t   oxc::Tim_Pwm_Dev::pulse2raw( float pu_s ) const noexcept
{
  return  (int32_t)( pu_s * freq_cnt );
}


uint32_t   oxc::Tim_Pwm_Dev::shift2raw( float pu_s ) const noexcept
{
  return 0;
}


ReturnCode oxc::Tim_Pwm_Dev::freq2cfgs( float freq, std::span<uint32_t> cfgs ) const noexcept
{
  auto tim = tim_p();
  if( !tim || freq < 0.001f ) {
    return false;
  }

  if( allowPSCadj ) {
    auto [ psc_, arr_ ] = calc_tim_psc_arr( freq_in, freq, arr_min, arr_max );
    if( psc == 0xFFFFFFFF ) {
      return rcErr;
    }
    cfgs[0] = arr_; cfgs[1] = psc_;

  } else {
    cfgs[0] =  calc_TIM_arr_for_base_freq( tim, freq );
    cfgs[1] = psc;
  }

  return rcOk;
}


float_er   oxc::Tim_Pwm_Dev::cfg2freq( std::span<const uint32_t> cfgs ) const noexcept
{
  return (float)freq_in / ( (cfgs[0]+1) * (cfgs[1]+1) );
}


ReturnCode oxc::Tim_Pwm_Dev::setDutyRaw(  size_t ch, int32_t dr ) noexcept
{
  if( isBadCh( ch ) ) {
    return rcErr;
  }
  *pccr( ch ) = dr;
  return rcOk;
}


ReturnCode oxc::Tim_Pwm_Dev::setShiftRaw( size_t ch, int32_t sr ) noexcept
{
  return rcErr;
}


int32_t_er oxc::Tim_Pwm_Dev::getDutyRaw(  size_t ch )  noexcept
{
  if( isBadCh( ch ) ) {
    return std::unexpected( rcErr );
  }
  return *pccr( ch );
}


int32_t_er oxc::Tim_Pwm_Dev::getShiftRaw( size_t ch )  noexcept
{
  return 0;
}


ReturnCode oxc::Tim_Pwm_Dev::readCfg()  noexcept
{
  auto tim = tim_p();
  if( !tim ) {
    return rcFatal;
  }
  arr = tim->ARR;
  psc = tim->PSC;
  freq_in   = get_TIM_in_freq( tim );
  freq_cnt  = (float)freq_in / ( 1 + psc );
  freq_base = freq_cnt / ( 1 + arr );
  return rcOk;
}

ReturnCode oxc::Tim_Pwm_Dev::applyCfg( std::span<const uint32_t> cfgs )  noexcept
{
  if( cfgs.size() < 2 ) { // user controlled
    return rcErr;
  }

  auto was_enabled = isEnabled();
  disable();

  auto tim = tim_p();
  bool was_changed { false };


  if( cfgs[0] != arr ) {
    arr      = cfgs[0];
    tim->ARR = arr;
    was_changed = true;
  }
  if( cfgs[1] != psc ) {
    psc      = cfgs[1];
    tim->PSC = psc;
    was_changed = true;
  }

  if( was_changed ) {
    for( size_t i=0; i<n_ch; ++i ) { // Reset ccrs
      if( ccrs_a[i] ) {
        *pccr( i ) = 0;
      }
    }
    tim->CNT = 0;
    readCfg();
  }

  if( was_enabled ) { // not reenable if error
    enable();
  }
  return rcOk;
}


ReturnCode oxc::Tim_Pwm_Dev::storeCfg( std::span<      uint32_t> cfgs ) const noexcept
{
  if( cfgs.size() < 2 ) {
    return rcErr;
  }
  cfgs[0] = arr;
  cfgs[1] = psc;

  return rcOk;
}


void oxc::Tim_Pwm_Dev::initPins()
{
  for( auto chp : channels ) {
    chp.pin.enableClk();
    chp.pin.cfgAF( chp.af );
  }
}


ReturnCode oxc::Tim_Pwm_Dev::initHW()
{
  auto tim = tim_p();
  if( !tim ) {
    return rcFatal;
  }
  t_h.Instance = tim;
  auto rc = tim_pwm_cfg_default( t_h, psc, arr, channels, cmode /* TODO? */ );
  if( rc.isError() ) {
    return rc;
  }
  readCfg();
  initPins();
  return rcOk;
}

