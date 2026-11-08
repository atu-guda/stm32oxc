#include <cstring>
#include <ranges>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>

#include <oxc_robo_base.h>

// #include <oxc_gpio_pin_d.h>
// #include <oxc_gpio_pins_d.h>

#include <oxc_tim_pwm_d.h>

#include <oxc_adcint_adc_d.h>

#include <board_robo_cfg.h>

#include "main.h"
#include "lab1_common.h"


using namespace oxc;
using namespace SMLRL;
namespace ranges = std::ranges;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

// auto out_q_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };

uint32_t      t_lab_max { 0 };
float pha_0 {0};
float pha_1 {0};

// ------------------------ named global vars -----------------------------------

#define NAMED_VARS_LIST    \
  IX(                 debug,         0 ) \
  IX(               dry_run,         0 ) \
  UX(  off_motor_idle_ticks,     60000 ) \
  UX(                t_pre,        200 ) \
  UX(                t_run_c,   100000 ) \
  UX(                t_post,       500 ) \
  UX(                t_step,        20 ) \
  UX(                adc_n,         20 ) \
  FX(                a_0,         0.4f ) \
  FX(                a_1,         0.4f ) \
  FX(                T_0,         5.1f ) \
  FX(                T_1,         9.0395f )


// global vars itself
#define IX( name, init)      int   name { init };
#define UX( name, init) uint32_t   name { init };
#define FX( name, init)    float   name { init };
NAMED_VARS_LIST
#undef IX
#undef UX
#undef FX

// objects to global vars
#define IX(name, init) constexpr NamedInt   ob_##name { #name, &name };
#define UX(name, init) constexpr NamedInt   ob_##name { #name, &name };
#define FX(name, init) constexpr NamedFloat ob_##name { #name, &name };
NAMED_VARS_LIST
#undef IX
#undef UX
#undef FX

#define IX(name, init) &ob_##name,
#define UX(name, init) &ob_##name,
#define FX(name, init) &ob_##name,
constexpr const NamedObj *const objs_info[] = {
    NAMED_VARS_LIST
    nullptr
};
#undef IX
#undef UX
#undef FX

NamedObjs objs( objs_info );
// print/set hook functions
DEFINE_NAMEDOBJ_FUNCS(objs)


// ------------------------ end: named global vars -----------------------------------

// ------------------------ easing functions  -----------------------------------

const EasingFunInfo easing_fun_info[] = {
  { easing_one,       1.0f },    // 0
  { easing_poly2_in,  0.5f },    // 1
  { easing_poly2_out, 0.5f },    // 2
  { easing_poly3_io,  0.6f },    // 3
  { easing_trig_in,   0.6f },    // 4 0.6 approx 1/pi_half_f
  { easing_trig_out,  0.6f },    // 5
  { easing_trig_io,   0.6f },    // 6
  { easing_step_01,   1.0f },    // 7
  { easing_one,       1.0f }     // protect // 8
};
constexpr auto easing_fun_n { std::size( easing_fun_info) };

// ------------------------ end: easing functions -----------------------------------


// ------------------------ low-level devices -------------------------------------------

// ------------ LWM timer
TIM_HandleTypeDef  tim_servolwm_h;

// ------------ ADC
ADC_HandleTypeDef hadc_sensor;
DMA_HandleTypeDef hdma_adc_sensor;
ADC_Info adc_s1 ( ADC_SENSOR, ADC_SENSOR_CHPINS );

ReturnCode init_hw_all();

// ------------------------ end: low-level devices -------------------------------------------


// ------------------------ Capabilities + RoboCap (C) ---------------------------------------

Tim_Pwm_Dev        servolwm_hd( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h, 0xFFFF );
float              servolwm_io[3*6*3]; // TODO???
PwmRoboCapability  servolwm_rd( servolwm_hd, 3, servolwm_io, 100 );

Adcint_Adc_Dev adcint_adc_hd( adc_s1, ADC_SENSOR_N_CH, ADC_resol_to_bits( ADC_SENSOR_Resolution ), adc_n );
AdcRoboCapability adcint_adc_rd( adcint_adc_hd, 200 );

// ------------------------ Channels and transforms (T) ---------------------------------------

TransFFLinLim tro_rad2pulse( 2.0e-3f/pi_f, 1.5e-3f, 0.5e-3f, 2.5e-3f ); // angle +/ pi/2 -> 500-2500 μs
OutChFF    oc_servolwm_rad2pulse0( servolwm_rd, PwmRoboCapability::ch0_pulse  , tro_rad2pulse );
OutChFF    oc_servolwm_rad2pulse1( servolwm_rd, PwmRoboCapability::ch0_pulse+1, tro_rad2pulse );
OutChFF    oc_servolwm_rad2pulse2( servolwm_rd, PwmRoboCapability::ch0_pulse+2, tro_rad2pulse );

TransIFLin tri_adcint_adc0( 9.166e-4f,  -1.915f ); // TODO: mech-dependent value
TransIFLin tri_adcint_adc1( 9.346e-4f,  -1.933f );
InChFI     ic_adcint_adc_0( adcint_adc_rd, 0, tri_adcint_adc0 );
InChFI     ic_adcint_adc_1( adcint_adc_rd, 1, tri_adcint_adc1 );
InChFI     ic_adcint_adc_2( adcint_adc_rd, 2, globalTransIFUnity ); // unused for now
InChFI     ic_adcint_adc_0x( adcint_adc_rd, 0, globalTransIFUnity ); // pure int for calibrate
InChFI     ic_adcint_adc_1x( adcint_adc_rd, 1, globalTransIFUnity );
InChFI     ic_adcint_adc_2x( adcint_adc_rd, 2, globalTransIFUnity );

// ------------------------ Channels and transforms end ---------------------------------------

// ------------------------ Robo (R) ---------------------------------------

IoRoboCapability* rcaps[] {
  &servolwm_rd,      // 0
  &adcint_adc_rd,    // 1
};

RoboObject* robo_objs[] {
  &servolwm_rd,      // 0
  &adcint_adc_rd,    // 1
};

OutChFBase* outchfs[] {
  &oc_servolwm_rad2pulse0, // 0
  &oc_servolwm_rad2pulse1, // 1
  &oc_servolwm_rad2pulse2, // 2
};

InChFBase* inchfs[] {
  &ic_adcint_adc_0,    // 0
  &ic_adcint_adc_1,    // 1
  &ic_adcint_adc_2,    // 2
  &ic_adcint_adc_0x,   // 3
  &ic_adcint_adc_1x,   // 4
  &ic_adcint_adc_2x,   // 5
};


RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( robo_objs, robo_joints );


// ------------------------ Robo end ---------------------------------------

const char* common_help_string = "robo_lab02_ " __DATE__ " " __TIME__ NL;

// --- local commands;

DCL_CMD_REG(     test0,       'T',    " alp_0 alp_1 - test " );
DCL_CMD_REG(      run0,       'R',    " [i] - run " );
DCL_CMD_REG(      init,       '\0',   " - init"  );
DCL_CMD_REG(      outf,       'F',    " i_och v - output to float ch"  );
DCL_CMD_REG(       inf,       'I',    " read all float ch"  );
DCL_CMD_REG(       prt,       '\0',   " - print devs info " );

// -------------------------------------------------------------------------------------


void idle_main_task()
{
  robo.at_main_idle();
  // leds[1].toggle();
}

void on_sigint( int /* c */ )
{
  // tim_lwm_stop();
  break_flag = 1;
}

void out_inputs();

int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR_t =    50;
  UVAR_n =    1000;

  SET_NAMEDOBJ_FUNCS;

  leds.initHW();   leds.reset( 0xFF_mask );

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );
  dev_console.setOnSigInt( on_sigint );
  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  TIM_SERVOLWM_CLKEN();
  servolwm_hd.initHW();
  servolwm_hd.setFreq( SERVOLWM_FREQ );
  servolwm_hd.enable();

  // TODO: move to device
  adc_s1.prepare_multi_ev( ADC_SENSOR_N_CH, ADC_SENSOR_ClockPrescaler, ADC_SENSOR_SAMPLETIME, ADC_SOFTWARE_START, ADC_SENSOR_Resolution );
  adc_s1.init_common();

  return robo.init_all();
}

void out_inputs()
{
  robo.measure_all();
  for( auto [i,pich] : std::views::enumerate( inchfs ) ) {
    auto v = pich->in();
    std_out << i << ' ' << v << NL;
  }

  // test
  // int32_t v = adcint_adc_rd.getVal( 0 ).value_or( 0 );
  // float a = tri_adcint_adc0.a;
  // float b = tri_adcint_adc0.b;
  // float vv = v * a + b;
  // std_out << '#' << v << ' ' << a << ' ' << b << ' ' << vv << NL;
}



CMD_FUNCTION( test0 )
{
  float alp0  = arg2float_d( 1, argc, argv, 0,  -pi_f/2, pi_f/2 );
  float alp1  = arg2float_d( 2, argc, argv, 0,  -pi_f/2, pi_f/2 );
  oc_servolwm_rad2pulse0.out( alp0 );
  oc_servolwm_rad2pulse1.out( alp1 );
  robo.commit_all();
  delay_ms( 1000 );
  out_inputs();
  return 0;
}

struct Data_setAlp
{
  float alp_0;
  float alp_1;
};

ReturnCode run_v_loop( const RunLoopState &rls, const RunLoopData &rld, void *data )
{
  if( !data ) {
    return rcFatal;
  }
  // auto d = static_cast<Data_setAlp*>(data);

  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }

  float dbuf[2];
  uint32_t tc_i = std::max( rls.tc - rld.t_pre, 0_u32 );
  float t = (float) tc_i * 1e-3f; // DUP?

  int rc2 = lab1_step( tc_i, dbuf );
  if( rc2 != 0 ) {
    return rc2;
  }


  if( !dry_run ) {
    oc_servolwm_rad2pulse0.out( dbuf[0] );
    oc_servolwm_rad2pulse1.out( dbuf[1] );
    robo.commit_all();
  }

  std_out
    << FmtInt( rls.tc, 8 ) << ' '
    << t << ' '
    << pha_0 << ' ' << pha_1 << ' '
    << dbuf[0] << ' ' << dbuf[1] << ' '
    << inchfs[0]->in() << ' ' << inchfs[1]->in()
    // << FltFmt( robo.get_t_cur(), cvtff_auto, 12, 3 )
    << NL;

  return rcOk;
}



CMD_FUNCTION( run0 ) // R
{
  int idx = arg2long_d( 1, argc, argv, 3, 0, easing_fun_n-1 );
  struct Data_setAlp d;
  d.alp_0      = a_0;
  d.alp_1      = a_1;

  int rc1 = lab1_init( idx );
  if( rc1 != 0 ) {
    std_out << "# Error: init failed! rc=" << rc1 << NL;
  }

  RunLoopData rld( t_step, t_pre, t_run_c , t_post );
  t_lab_max = t_run_c;

  robo.start_time();
  auto rc = run_periodic( rld, run_v_loop, &d );

  robo.commit_all();

  return rc.isOk() ? 0 : 2;
}

CMD_FUNCTION( init )
{
  auto rc = robo.init_all();
  std_out << " rc= [" << rc.code << ", " << rc.data << "]" NL;
  return 0;
}

CMD_FUNCTION( outf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(outchfs)-1 );
  float v  = arg2float_d( 2, argc, argv, 0 );
  outchfs[idx]->out( v );
  if( UVAR_a ) {
    robo.commit_all();
  }
  return 0;
}


CMD_FUNCTION( inf )
{
  out_inputs();
  return 0;
}


CMD_FUNCTION( prt )
{
  std_out << "# SERVOLWM: " NL;
  tim_print_cfg( TIM_SERVOLWM_BASE );
  adc_s1.pr_state();
  return 0;
}


// ------------------------------------ ADC ------------------------------------------------
// arch-dependent part: TODO: ???

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC_SENSOR ) {
    return;
  }
  ADC_SENSOR_CLK_EN();
  ADC_SENSOR_DMA_CLK_EN();

  adc_s1.init_gpio_channels();
  adc_s1.DMA_reinit( DMA_NORMAL );

  HAL_NVIC_SetPriority( ADC_SENSOR_DMA_IRQ, ADC_SENSOR_DMA_IRQ_PRTY, 0 );
  HAL_NVIC_EnableIRQ(   ADC_SENSOR_DMA_IRQ );
  // UVAR_x = 1;
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC_SENSOR ) {
    return;
  }
  ADC_SENSOR_CLK_DIS();
  ADC_SENSOR_DMA_CLK_DIS();
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds[1].set();
  // adc_s1.convHalfCpltCallback( hadc );
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds[1].set();
  // ++UVAR_i;
  adc_s1.convCpltCallback( hadc );
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // leds[0].set();
  // ++UVAR_j;
  adc_s1.errorCallback( hadc );
}

void BOARD_ADC_DMA_IRQHANDLER(void)
{
  // leds[0].set();
  // ++UVAR_k;
  HAL_DMA_IRQHandler( &adc_s1.hdma_adc );
}

// ------------------------------------  ------------------------------------------------


