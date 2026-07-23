#include <oxc_base.h>
#include <oxc_gpio.h> // for debug leds
#include <oxc_robo_base.h>

using namespace oxc;
// using oxc::ReturnCode;

oxc::ReturnCode oxc::RoboAssembly::for_all_till_err( ReturnCode (RoboDevice::*fun)() )
{
  last_err_dev = nullptr;
  for( auto dev : pdevs ) {
    auto rc = (dev->*fun)();
    if( rc.isError() ) {
      last_err_dev = dev;
      return rc;
    }
  }
  return rcOk;
}


void oxc::RoboAssembly::start_time()
{
  t_start_i = GET_OS_TICK();
  t_cur_i = 0;  t_cur_f = 0; t_meas_i = 0;
  first_measure = true;
}

void oxc::RoboAssembly::calc_current_time()
{
  t_cur_i = GET_OS_TICK() - t_start_i;
  t_cur_f = t_cur_i * 1e-3f;
}

ReturnCode oxc::RoboAssembly::init_all()
{
  return for_all_till_err( &RoboDevice::initHW  );
}

ReturnCode oxc::RoboAssembly::measure_all()
{
  auto old_meas = t_meas_i;
  calc_current_time();
  t_meas_i = t_cur_i;
  if( first_measure ) {
    t_dt = 0;
    t_dt_f = 0;
  } else {
    t_dt = std::max( t_meas_i - old_meas, 1_u32 );
    t_dt_f = t_dt * 1e-3f;
  }

  leds[1].set();
  auto rc =  for_all_till_err( &RoboDevice::measure );
  leds[1].reset();
  first_measure = false;
  return rc;
}

ReturnCode oxc::RoboAssembly::commit_all()
{
  return for_all_till_err( &RoboDevice::commit  );
}

void oxc::RoboAssembly::at_main_idle()
{
  calc_current_time();
  if( ( t_cur_i - t_meas_i ) >= measure_idle_ticks ) {
    measure_all();
  }
}

