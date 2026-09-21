#include <ranges>

// #include <oxc_debug1.h>

#include <oxc_bitops.h>
#include <oxc_robocaps.h>

using namespace oxc;


ReturnCode oxc::RoboObject::init() noexcept
{
  if( ! ( flags & noInit ) ) {
    sta = doInit();
  }
  dirty = 0;
  return sta;
}


ReturnCode oxc::RoboObject::measure() noexcept
{
  if( ! ( flags & noMeasure ) ) {
    sta = doMeasure();
  }
  return sta;
}


ReturnCode oxc::RoboObject::think() noexcept
{
  if( ! ( flags & noThink ) ) {
    sta = doThink();
  }
  return sta;
}


ReturnCode oxc::RoboObject::commit() noexcept
{
  if( ! ( flags & noCommit ) ) {
    sta = doCommit();
    if( sta.isOk() ) {
      dirty = 0;
    }
  }
  return sta;
}



int32_t_er oxc::PinsRoboCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return vv[0];
    case ch_write : return vv[1];
  }
  return std::unexpected( rcErr );
}

ReturnCode oxc::PinsRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  auto vvo = vv[1];
  switch( ch ) {
    case ch_write     : vv[1]  =  v;  break;
    case ch_set       : vv[1] |=  v;  break;
    case ch_reset     : vv[1] &= ~v;  break;
    case ch_toggle    : vv[1] ^=  v;  break;
    case ch_setbit    : set_bit(    vv[1], v ); break;
    case ch_resetbit  : reset_bit(  vv[1], v ); break;
    case ch_togglebit : toggle_bit( vv[1], v ); break;
    default           : return rcErr;
  }
  if( vvo != vv[1] ) {
    dirty |= ch_w_bit;
  }
  return rcOk;
}




int32_t_er oxc::PinRoboCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return vv[0];
    case ch_write : return vv[1];
  }
  return std::unexpected( rcErr );
}


ReturnCode oxc::PinRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  auto vvo = vv[1];
  switch( ch ) {
    case ch_write     : vv[1] =  v;     break;
    case ch_set       : vv[1] =  1;     break;
    case ch_reset     : vv[1] =  0;     break;
    case ch_toggle    : vv[1] = !vv[1]; break;
    default           : return rcErr;
  }
  if( vvo != vv[1] ) {
    dirty |= ch_w_bit;
  }
  return rcOk;
}

// ------------------ PwmRoboCapability

ReturnCode oxc::PwmRoboCapability::setValF( size_t ch, float v )  noexcept
{
  if( ch > ch_freq ) {
    return rcErr;
  }

  if( ch == ch_freq ) {
    freq_set = v;
    dirty |= 1;
    dirty_f = true;
    return rcOk;
  }

  // TODO: more generic: function
  if( ch >= ch0_shift ) {
    ch -= ch0_shift;
    if( ch >= n_ch ) {
      return rcErr;
    }
    io_f[2*n_ch+ch] = v;
    set_bit( dirty_shift, ch );
    dirty |= 8;
    return rcOk;
  }

  if( ch >= ch0_pulse ) {
    ch -= ch0_pulse;
    if( ch >= n_ch ) {
      return rcErr;
    }
    io_f[1*n_ch+ch] = v;
    dirty |= 4;
    set_bit( dirty_pulse, ch );
    set_bit( pulse_flag, ch );
    return rcOk;
  }

  if( ch >= n_ch ) {
    return rcErr;
  }
  io_f[ch] = v;
  dirty |= 2;
  set_bit( dirty_duty, ch );
  reset_bit( pulse_flag, ch );

  return rcOk;
}


float_er   oxc::PwmRoboCapability::getValF( size_t ch ) noexcept
{
  if( ch != ch_freq ) { // TODO?? more?
    return std::unexpected( rcErr );
  }
  return freq_get;
}


ReturnCode oxc::PwmRoboCapability::doInit() noexcept
{
  dirty_f = false;
  dirty_duty = dirty_pulse = dirty_shift = pulse_flag = 0;
  return pwm.init();
}


ReturnCode oxc::PwmRoboCapability::doMeasure() noexcept
{
  auto v = pwm.getFreq();
  freq_get = v.value_or( 0.0f );
  return v.error_or( rcOk );
}


ReturnCode oxc::PwmRoboCapability::doCommit()  noexcept
{
  auto was_en = pwm.isEnabled();
  if( dirty_f ) {
    pwm.disable();
    dirty_duty = dirty_pulse = dirty_shift = 0xFFFFFFFF; // assume all changed TODO: but what to keep?
    pwm.setFreq( freq_set );
  }

  for( size_t ch=0; ch < n_ch; ++ch ) {
    if( check_bit( dirty_shift, ch ) ) {
      pwm.setShift( ch, io_f[2*n_ch+ch] );
    }
    if( check_bit( dirty_pulse, ch ) && check_bit( pulse_flag, ch ) ) {
      pwm.setPulse( ch, io_f[1*n_ch+ch] );
    }
    if( check_bit( dirty_duty, ch ) && !check_bit( pulse_flag, ch ) ) {
      pwm.setDuty( ch, io_f[ch] );
    }
  }

  if( dirty_f && was_en ) {
    pwm.enable();
  }

  dirty_f = false;
  dirty_duty = dirty_pulse = dirty_shift = 0;
  // pulse_flag is not reset here - it keeps last operation
  // and if later only freq changes - recalcs in rignt mode
  return rcOk;
}

// ----------------------- EncoderRoboCapability


int32_t_er oxc::EncoderRoboCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_pos    : return pos;
    case ch_posraw : return posraw;
    case ch_dlt    : return dlt;
  }
  return std::unexpected( rcErr );
}


ReturnCode oxc::EncoderRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  auto old_pos = pos_set;
  switch( ch ) {
    case ch_pos       : pos_set = v; break;
    default           : return rcErr;
  }
  if( pos_set != old_pos ) {
    dirty |= ch_pos_bit;
  }
  return rcOk;
}



