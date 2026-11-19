#include <climits>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_pin_d.h>
// #include <oxc_gpio_pins_d.h>

// #include <oxc_addr_enco_d.h>
#include <oxc_as5600_enco_d.h>
#include <oxc_motordir_1v2d_roboctl.h>

#include <oxc_tim_pwm_d.h>

// #include <oxc_adcint_adc_d.h>

#include <board_robo_cfg.h>


using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test PWM+2pin controlled DC motor" NL;

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

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 as5600_sens( i2cd );


// ------------------------ Devices: capabilities ---------------------------------------


TIM_HandleTypeDef  tim_mpwm_h;
Tim_Pwm_Dev        mpwm_hd( TIM_MPWM_BASE, tim_MPWM_chspins, tim_mpwm_h, 0xFFFF ); // TODO: timer props
float              mpwm_io[3*6];
PwmRoboCapability  mpwm_rd( mpwm_hd, 1, mpwm_io, 500 );
//
Gpio_Pin_Dev      pin_mpwm_l_hd( MPWM_CtlPin_L );
PinRoboCapability pin_mpwm_l_rd( pin_mpwm_l_hd, 501 );
Gpio_Pin_Dev      pin_mpwm_r_hd( MPWM_CtlPin_R );
PinRoboCapability pin_mpwm_r_rd( pin_mpwm_r_hd, 502 );

AS5600_Enco_Dev    as5600_enco_hd( as5600_sens, true );
EncoderRoboCapability as5600_enco_rd( as5600_enco_hd, 800 );
constexpr int32_t as5600_zero { 2739 }; // Beware: mech-dependent constant



// ------------------------ Channels and transforms ---------------------------------------
// output

OutChFF    oc_mpwm_duty(  mpwm_rd, PwmRoboCapability::ch0_pwm,   globalTransFFUnity );
OutChFF    oc_mpwm_pulse( mpwm_rd, PwmRoboCapability::ch0_pulse, globalTransFFUnity );
OutChFF    oc_mpwm_freq(  mpwm_rd, PwmRoboCapability::ch_freq,   globalTransFFUnity );
OutChII    oci_mpwm_l( pin_mpwm_l_rd, PinRoboCapability::ch_write, globalTransIIUnity );
OutChII    oci_mpwm_r( pin_mpwm_r_rd, PinRoboCapability::ch_write, globalTransIIUnity );

OutChINull oci_null;

// input

InChFConst ic_const( 3.1415f );

TransIFLin tri_as5600_enco( /* 2* */ pi_f/AS5600::val2turn, 0 );
InChFI     ic_as5600_enco(   as5600_enco_rd, EncoderRoboCapability::ch_pos, tri_as5600_enco );
InChFI     ic_as5600_enco_d( as5600_enco_rd, EncoderRoboCapability::ch_dlt, tri_as5600_enco );

//   input int
InChIConst ici_const( 42 );

// for motdir0
//  in:
InChFConst ic_motdir0_v( 0.1f );
InChIConst ici_motdir0_brk( 0 );
InChIConst ici_motdir0_idle( 0 );
//  out:

// ------------------------ Channels and transforms end ---------------------------------------

// ------------------------ RoboCtl --------------------------------------------------

MotorDir_1V2D_RoboCtl motdir0( ic_motdir0_v, ici_motdir0_idle, ici_motdir0_brk,
       oc_mpwm_duty, oci_mpwm_l, oci_mpwm_r, 1000  );

// ------------------------ RoboCtl end --------------------------------------------------


IoRoboCapability* robo_caps[] {
  &mpwm_rd,          // 0
  &pin_mpwm_l_rd,    // 1
  &pin_mpwm_r_rd,    // 2
  &as5600_enco_rd,   // 3
  &motdir0,          // 4
};

std::span<IoRoboCapability*> oxc::global_robo_caps { robo_caps };

RoboObject* robo_objs[] {
  &mpwm_rd,          // 0
  &pin_mpwm_l_rd,    // 1
  &pin_mpwm_r_rd,    // 2
  &as5600_enco_rd,   // 3
  &motdir0,          // 4
};

std::span<RoboObject*> oxc::global_robo_objs { robo_objs };

OutChFBase* channels_out_f[] {
  &oc_mpwm_duty,  // 0
  &oc_mpwm_pulse, // 1
  &oc_mpwm_freq,  // 2
};

std::span<OutChFBase*> oxc::global_channels_out_f { channels_out_f };

OutChIBase* channels_out_i[] {
  &oci_null,                 // 0
  &oci_mpwm_l,               // 1
  &oci_mpwm_r,               // 2
};

std::span<OutChIBase*> oxc::global_channels_out_i { channels_out_i };

InChFBase* channels_in_f[] {
  &ic_const,               // 0
  &ic_as5600_enco,         // 1
  &ic_as5600_enco_d,       // 2
  &ic_motdir0_v,           // 3
};

std::span<InChFBase*>  oxc::global_channels_in_f { channels_in_f };

InChIBase* channels_in_i[] {
  &ici_const,          // 0
  &ici_motdir0_idle,   // 1
  &ici_motdir0_brk,    // 2
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

  TIM_MPWM_CLKEN(); // TODO: auto
  mpwm_hd.initHW();
  mpwm_hd.setFreq( MPWM_FREQ );
  mpwm_hd.enable();
  pin_mpwm_l_hd.initHW();
  pin_mpwm_r_hd.initHW();

  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &as5600_sens;
  // TODO: to bsp
  as5600_sens.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  as5600_enco_hd.read();
  as5600_enco_hd.setStartPos( as5600_zero );

  return robo.init_all();
}




CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  mpwm_hd.setDuty( 0, pwm_v );

  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_MPWM_BASE );

  // dump32( (void*)TIM_MPWM_BASE, 0x60 );

  // std_out << "# Encoder:  " << TIM_ENCO->CNT
  //         <<  " hw:" << q0_sens_hw.get(0) << " log: " << q0_sens.get() <<NL;
  // tim_print_cfg( TIM_ENCODER_BASE );

  return 0;
}

CMD_FUNCTION( setfreq ) // F
{
  auto freq = arg2float_d( 1, argc, argv, 1, 0.01f );

  // mpwm_hd.setFreq( freq );
  mpwm_rd.setValF( PwmRoboCapability::ch_freq, freq );
  robo.commit_all();

  std_out << "# freq: " << freq << " => " << mpwm_hd.getFreq().value_or( 0.0f ) << NL;

  return 0;
}

CMD_FUNCTION( pulse ) // U
{
  float pu = arg2float_d( 1, argc, argv, 0 );

  //mpwm_hd.setPulse( 0, pu );
  mpwm_rd.setValF( PwmRoboCapability::ch0_pulse, pu );
  robo.commit_all();

  tim_print_cfg( TIM_MPWM_BASE );

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

  robo.think_all();


  if( rls.stage & RunLoopState::stage_change_flag  ) {
    channels_out_f[0]->out( v );
    robo.commit_all();
  }

  float x_m    = channels_in_f[1]->in();
  float d_x    = channels_in_f[2]->in();
  float v_m    = d_x * 1.0e3f / rld.t_step;
  // int32_t x_mi = channels_in_i[1]->in();

  std_out << FmtInt( rls.tc, 8 ) << ' '  << v << ' ' << x_m
    << ' ' << v_m << ' ' << d_x
    << NL;

  return rcOk;
}


CMD_FUNCTION( setV ) // V
{
  struct Data_setV d;
  d.v        = arg2float_d( 1, argc, argv, 0 );
  auto t_run = arg2ulong_d( 2, argc, argv, UVAR_r, 0 );

  RunLoopData rld( UVAR_t, UVAR_p, t_run, UVAR_o );

  auto rc = run_periodic( rld, run_v_loop, &d );

  return rc.isOk() ? 0 : 2;
}

CMD_FUNCTION( setRef ) // Z
{
  std_out << "#  pre: " <<  as5600_enco_hd.getStartPos() << ' ' << as5600_enco_hd.getPos() << NL;
  auto nv = arg2long_d( 1, argc, argv, 0 );
  as5600_enco_hd.setStartPos( nv - as5600_enco_hd.getPosRaw() );
  std_out << "# post: " << as5600_enco_hd.getStartPos() << ' ' << as5600_enco_hd.getPos() << NL;
  return 0;
}



