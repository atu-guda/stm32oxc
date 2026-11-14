#include <climits>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

// #include <oxc_gpio_pin_d.h>
// #include <oxc_gpio_pins_d.h>

// #include <oxc_addr_enco_d.h>
// #include <oxc_as5600_enco_d.h>

#include <oxc_tim_pwm_d.h>

#include <oxc_adcint_adc_d.h>

#include <board_robo_cfg.h>


using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test LWM servo robo iface" NL;

// --- local commands;
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setX,  'X',     " x [t_us] - set x"  );


// -------------------------------------------------------------------------------------

ReturnCode init_hw_all();

// ------------------------ low-level devices -------------------------------------------


// ------------------------ Devices: capabilities ---------------------------------------


TIM_HandleTypeDef  tim_servolwm_h;
Tim_Pwm_Dev        servolwm_hd( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h, 0xFFFF );
float              servolwm_io[3*6];
PwmRoboCapability  servolwm_rd( servolwm_hd, 1, servolwm_io, 600 );

ADC_Info adc_sensor ( ADC_SENSOR, ADC_SENSOR_CHPINS );
Adcint_Adc_Dev adcint_adc_hd( adc_sensor, ADC_SENSOR_N_CH, ADC_resol_to_bits( ADC_SENSOR_Resolution ), 100 );
AdcRoboCapability adcint_adc_rd( adcint_adc_hd, 900 );


// ------------------------ Channels and transforms ---------------------------------------
// output

TransFFLinLim tro_rad2pulse( 2.0e-3f/pi_f, 1.5e-3f, 0.5e-3f, 2.5e-3f ); // angle +/ pi/2 -> 500-2500 μs
OutChFF    oc_servolwm_rad2pulse( servolwm_rd, PwmRoboCapability::ch0_pulse, tro_rad2pulse );

OutChINull oci_null;

// input
TransIFLin tri_adcint_adc( 9.2129e-4f, -1.861f ); // TODO: mech-dependent value
InChFI     ic_adcint_adc_0( adcint_adc_rd, 0, tri_adcint_adc );
InChII     ic_adcint_adc_0i( adcint_adc_rd, 0, globalTransIIUnity );

//   input int
InChIConst ici_const( 42 );

// ------------------------ Channels and transforms end ---------------------------------------


IoRoboCapability* robo_caps[] {
  &servolwm_rd,      // 0
  &adcint_adc_rd,    // 1
};

std::span<IoRoboCapability*> oxc::global_robo_caps { robo_caps };

RoboObject* robo_objs[] {
  &servolwm_rd,      // 0
  &adcint_adc_rd,    // 1
};

std::span<RoboObject*> oxc::global_robo_objs { robo_objs };

OutChFBase* channels_out_f[] {
  &oc_servolwm_rad2pulse, // 0
  // &oc_mpwm_freq,          // 7 ??????????
};

std::span<OutChFBase*> oxc::global_channels_out_f { channels_out_f };

OutChIBase* channels_out_i[] {
  &oci_null,                 // 0
  // &oc_mpwm_r,             // 1
};

std::span<OutChIBase*> oxc::global_channels_out_i { channels_out_i };

InChFBase* channels_in_f[] {
  &ic_adcint_adc_0,    // 7
};

std::span<InChFBase*>  oxc::global_channels_in_f { channels_in_f };

InChIBase* channels_in_i[] {
  &ici_const,          // 0
  &ic_adcint_adc_0i,   // 1
};

std::span<InChIBase*>  oxc::global_channels_in_i { channels_in_i };


RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( robo_objs, robo_joints );

RoboAssembly& oxc::global_robo{ robo };


void idle_main_task()
{
  leds.toggle( 1_mask );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idLe after run
  UVAR_t =   20; // t_step, ms
  UVAR_n =   50;
  UVAR_p =  200; // t_pre,  ms
  UVAR_r = 1000; // t_run,  ms, def
  UVAR_o =  500; // t_post, ms

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  // oxc_add_aux_tick_fun( led_task_nortos ); // tmp disable to test leds

  robo.start_time();

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  TIM_SERVOLWM_CLKEN();
  servolwm_hd.initHW();
  servolwm_hd.setFreq( SERVOLWM_FREQ );
  servolwm_hd.enable();


  adc_sensor_init();

  return robo.init_all();
}




CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  // pwm1.setPwm( 0, pwm_v );

  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_SERVOLWM_BASE );

  // std_out << "# freq:  "  << pwm1.getFreq() << NL;

  dump32( (void*)TIM_SERVOLWM_BASE, 0x60 );

  adc_sensor.pr_state();

  return 0;
}

CMD_FUNCTION( setfreq ) // F
{
  auto freq = arg2float_d( 1, argc, argv, 1, 0.01f );

  // pwm1.setFreq( freq );

  std_out << "# freq: " << freq << " => " << /* pwm1.getFreq() <<*/ NL;

  return 0;
}

CMD_FUNCTION( pulse ) // U
{
  uint32_t pu = arg2ulong_d( 1, argc, argv, 0 );
  // pwm1.setPulse( 0, pu );
  // std_out << '#' << pu << ' ' << pwm1.getPwmRaw( 0 ) << NL;

  tim_print_cfg( TIM_SERVOLWM_BASE );

  return 0;
}


struct Data_setX
{
  float x;
};

ReturnCode run_x_loop( const RunLoopState &rls, const RunLoopData &rld, void *data )
{
  if( !data ) {
    return rcFatal;
  }
  auto d = static_cast<Data_setX*>(data);

  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }
  float x = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->x : 0;

  if( rls.stage & RunLoopState::stage_change_flag  ) {
    // q0_actu.setQ( x );
    robo.commit_all();
  }

  std_out << FmtInt( rls.tc, 8 ) << ' '  << x << ' ' // << pwm1.getPwmRaw( 0 )
    // << ' ' << q0_actu.get_q_int() << ' ' << q0_actu.get_q_phy()
    // << ' ' << sensor_adc1.get(0) << ' ' << r2d( q0_sens.get() )
    << NL;

  return rcOk;
}


CMD_FUNCTION( setX ) // X
{
  struct Data_setX d;
  d.x = arg2float_d( 1, argc, argv, 0 );
  auto t_run = arg2ulong_d( 2, argc, argv, UVAR_r, 0 );

  RunLoopData rld( UVAR_t, UVAR_p, t_run, UVAR_o );

  auto rc = run_periodic( rld, run_x_loop, &d );

  if( UVAR_l ) {
    // q0_actu.idle();
    robo.commit_all();
  }

  return rc.isOk() ? 0 : 2;
}



