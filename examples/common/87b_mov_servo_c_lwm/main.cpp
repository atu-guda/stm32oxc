#include <climits>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

// #include <oxc_gpio_pin_d.h>
// #include <oxc_gpio_pins_d.h>

#include <oxc_addr_enco_d.h>
// #include <oxc_as5600_enco_d.h>

#include <oxc_tim_pwm_d.h>

// #include <oxc_adcint_adc_d.h>

#include <board_robo_cfg.h>


using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test contigous LWM servo" NL;

// --- local commands;
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setV,  'V',     " v [t_us] - set v"  );
DCL_CMD_REG(     setRef,  'Z',     " [v] - set [zero] encoder value"  );


// -------------------------------------------------------------------------------------

ReturnCode init_hw_all();

// ------------------------ low-level devices -------------------------------------------


// ------------------------ Devices: capabilities ---------------------------------------


TIM_HandleTypeDef  tim_servolwm_h;
Tim_Pwm_Dev        servolwm_hd( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h, 0xFFFF );
float              servolwm_io[3*6];
PwmRoboCapability  servolwm_rd( servolwm_hd, 1, servolwm_io, 600 );

TIM_HandleTypeDef  tim_enco_h;
Addr_Enco_Dev      tim_enco_hd( TIM_ENCODER_CNTBASE, 0xFFFF, true ); // TODO: separate device to init
EncoderRoboCapability tim_enco_rd( tim_enco_hd, 700 );


// ------------------------ Channels and transforms ---------------------------------------
// output

TransFFLinLim tro_rad2pulse( 2.0e-3f/pi_f, 1.5e-3f, 0.5e-3f, 2.5e-3f ); //  calibration required, not angle +/ pi/2 -> 500-2500 μs
OutChFF    oc_servolwm_v2pulse( servolwm_rd, PwmRoboCapability::ch0_pulse, tro_rad2pulse );
// TODO: us to ??? to direct measure calibration

OutChINull oci_null;

// input
TransIFLin tri_tim_enco( pi_f/1200, 0 );
InChFI     ic_tim_enco(  tim_enco_rd, EncoderRoboCapability::ch_pos, tri_tim_enco );
InChFI     ic_tim_enco_v(  tim_enco_rd, EncoderRoboCapability::ch_dlt, tri_tim_enco );
InChII     ici_tim_enco( tim_enco_rd, EncoderRoboCapability::ch_pos, globalTransIIUnity );

//   input int
InChIConst ici_const( 42 );

// ------------------------ Channels and transforms end ---------------------------------------


IoRoboCapability* robo_caps[] {
  &servolwm_rd,      // 0
  &tim_enco_rd,      // 1
  //&adcint_adc_rd,    // 2
};

std::span<IoRoboCapability*> oxc::global_robo_caps { robo_caps };

RoboObject* robo_objs[] {
  &servolwm_rd,      // 0
  &tim_enco_rd,      // 1
  // &adcint_adc_rd,    // 2
};

std::span<RoboObject*> oxc::global_robo_objs { robo_objs };

OutChFBase* channels_out_f[] {
  &oc_servolwm_v2pulse, // 0
  // &oc_mpwm_freq,          // 7 ??????????
};

std::span<OutChFBase*> oxc::global_channels_out_f { channels_out_f };

OutChIBase* channels_out_i[] {
  &oci_null,                 // 0
  // &oc_mpwm_r,             // 1
};

std::span<OutChIBase*> oxc::global_channels_out_i { channels_out_i };

InChFBase* channels_in_f[] {
  &ic_tim_enco,        // 0
  &ic_tim_enco_v,      // 1
  // &ic_adcint_adc_0,    // 2
};

std::span<InChFBase*>  oxc::global_channels_in_f { channels_in_f };

InChIBase* channels_in_i[] {
  &ici_const,          // 0
  &ici_tim_enco,       // 1
  //&ic_adcint_adc_0i,   // 2
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


  // adcint_adc_hd.init();
  // adc_sensor_init();

  TIM_ENCODER_CLKEN(); // manual config, as not in device
  tim_enco_h.Instance = TIM_ENCO;
  tim_enco_cfg_default( tim_enco_h, tim_ENCODER_chspins );

  return robo.init_all();
}




CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  servolwm_hd.setDuty( 0, pwm_v );

  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_SERVOLWM_BASE );

  dump32( (void*)TIM_SERVOLWM_BASE, 0x60 );

  // std_out << "# Encoder:  " << TIM_ENCO->CNT
  //         <<  " hw:" << q0_sens_hw.get(0) << " log: " << q0_sens.get() <<NL;
  tim_print_cfg( TIM_ENCODER_BASE );

  return 0;
}

CMD_FUNCTION( setfreq ) // F
{
  auto freq = arg2float_d( 1, argc, argv, 1, 0.01f );

  // servolwm_hd.setFreq( freq );
  servolwm_rd.setValF( PwmRoboCapability::ch_freq, freq );
  robo.commit_all();

  std_out << "# freq: " << freq << " => " << servolwm_hd.getFreq().value_or( 0.0f ) << NL;

  return 0;
}

CMD_FUNCTION( pulse ) // U
{
  float pu = arg2float_d( 1, argc, argv, 0 );

  //servolwm_hd.setPulse( 0, pu );
  servolwm_rd.setValF( PwmRoboCapability::ch0_pulse, pu );
  robo.commit_all();

  tim_print_cfg( TIM_SERVOLWM_BASE );

  return 0;
}


struct Data_setV
{
  float v;
};

ReturnCode run_v_loop( const RunLoopState &rls, const RunLoopData &rld, void *data )
{
  if( !data ) {
    return rcFatal;
  }
  auto d = static_cast<Data_setV*>(data);

  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }
  float v = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->v : 0;


  if( rls.stage & RunLoopState::stage_change_flag  ) {
    channels_out_f[0]->out( v );
    robo.commit_all();
  }

  float x_m    = channels_in_f[0]->in();
  float d_m    = channels_in_f[1]->in();
  float v_m    = d_m * 1.0e3f / rld.t_step;
  int32_t x_mi = channels_in_i[1]->in();

  std_out << FmtInt( rls.tc, 8 ) << ' '  << v << ' ' << v_m << ' ' << x_m << ' ' << d_m << ' ' << x_mi << NL;

  return rcOk;
}


CMD_FUNCTION( setV ) // V
{
  struct Data_setV d;
  d.v        = arg2float_d( 1, argc, argv, 0 );
  auto t_run = arg2ulong_d( 2, argc, argv, UVAR_r, 0 );

  RunLoopData rld( UVAR_t, UVAR_p, t_run, UVAR_o );

  auto rc = run_periodic( rld, run_v_loop, &d );

  if( UVAR_l ) { // unbreak
    servolwm_rd.setValF( 0, 0.0f );
    robo.commit_all();
  }

  return rc.isOk() ? 0 : 2;
}

CMD_FUNCTION( setRef ) // Z
{
  std_out << "#  pre: " << tim_enco_hd.getStartPos() << ' ' << tim_enco_hd.getPos() << NL;
  auto nv = arg2long_d( 1, argc, argv, 0 );
  tim_enco_hd.setStartPos( nv - tim_enco_hd.getPosRaw() );
  std_out << "# post: " << tim_enco_hd.getStartPos() << ' ' << tim_enco_hd.getPos() << NL;
  return 0;
}



