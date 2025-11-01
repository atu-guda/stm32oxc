#include <algorithm>
// #include <cmath>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_atleave.h>
#include <oxc_outstr.h>
#include <oxc_as5600.h>

#include "main.h"

// using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

int debug {0};

PinsOut ledsx( LEDSX_GPIO, LEDSX_START, LEDSX_N );



const char* common_help_string = "hand0 " __DATE__ " " __TIME__ NL;


TIM_HandleTypeDef tim_lwm_h;
uint32_t tim_lwm_arr { 19999 }; // near init value, will, be recalculated
int lwm_t_min {  500 }; // min pulse width in us
int lwm_t_max { 1500 }; // max pulse width in us

// --- local commands;
DCL_CMD ( test0, 'T', " - test something timers" );
DCL_CMD ( stop,  'P', " - stop pwm" );

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_test0,
  &CMDINFO_stop,
  nullptr
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );

// TaskData td;


#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_IOBJ_TD(x) constexpr NamedInt   ob_##x { #x, &td.x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
#define ADD_FOBJ_TD(x) constexpr NamedFloat ob_##x { #x, &td.x }

// ADD_IOBJ_TD( n_total );
// ADD_FOBJ_TD( d_wire  );
ADD_IOBJ   ( debug   );
ADD_IOBJ   ( lwm_t_min   );
ADD_IOBJ   ( lwm_t_max   );

#undef ADD_IOBJ
#undef ADD_IOBJ_TD


constexpr const NamedObj *const objs_info[] = {
  & ob_debug,
  & ob_lwm_t_min,
  & ob_lwm_t_max,
  nullptr
};

NamedObjs objs( objs_info );

// print/set hook functions

bool print_var_ex( const char *nm, int fmt )
{
  return objs.print( nm, fmt );
}

bool set_var_ex( const char *nm, const char *s )
{
  auto ok =  objs.set( nm, s );
  print_var_ex( nm, 0 );
  return ok;
}


void idle_main_task()
{
  // if( cstate_go == 0 && ostate_go != 0 ) {
  //   uint32_t cur_start_tick = HAL_GetTick();
  //   if( cur_start_tick - last_start_tick > 100 ) {
  //     leds.toggle( 1 );
  //     if( global_smallrl != nullptr && global_smallrl->get()[0] == '\0' ) {
  //       ungets( 0, "G\n" );
  //     }
  //     last_start_tick = cur_start_tick;
  //   }
  // }
  // ostate_go = cstate_go;
}


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') =   100;
  UVAR('n') =     2;
  UVAR('c') = AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off;

  ledsx.initHW();
  ledsx.reset( 0xFF );

  UVAR('v') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

  // pins_tower.initHW();

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  if( ! tim_lwm_cfg() ) {
    std_out << "Err: timer LWM init"  NL;
    die4led( 2 );
  }
  tim_lwm_start();

  init_EXTI();

  ang_sens.setCfg( UVAR('c') );
  ang_sens.setStartPosCurr();


  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}



void init_EXTI()
{
  //   ei.gpio.setEXTI( ei.pin, ei.dir );
  //   HAL_NVIC_SetPriority( ei.exti_n, EXTI_IRQ_PRTY, 0 );
  //   HAL_NVIC_EnableIRQ(   ei.exti_n );
}

int cmd_test0( int argc, const char * const * argv )
{
  float v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int  ch = arg2long_d(  2, argc, argv, 1, 0, 3 );
  int n = UVAR('n');
  uint32_t t_step = UVAR('t');

  uint32_t vi = (uint32_t) lwm_t_min + (lwm_t_max-lwm_t_min) * v;
  vi = std::clamp( vi, (uint32_t)lwm_t_min, (uint32_t) lwm_t_max );
  uint32_t ccr = (uint32_t) tim_lwm_arr * vi / 10000;

  std_out
    <<  "# Test0: ch= " << ch << " v= " << v << " vi=" << vi
    << " ccr= " << ccr << " dt= " << t_step << NL;

  static const decltype( &TIM_LWM->CCR1 ) ccrs[] { &TIM_LWM->CCR1, &TIM_LWM->CCR2, &TIM_LWM->CCR3, &TIM_LWM->CCR4 };

  tim_lwm_start();

  *ccrs[ch] = ccr;


  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;


  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcb = HAL_GetTick();

    ledsx[0].set();

    uint32_t  tcc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 ) << " dt = " << ( tcc - tcb ) << NL;

    delay_ms_until_brk( &tc0, t_step );
    ledsx[0].reset();
  }

  tim_print_cfg( TIM_LWM );

  return 0;
}

//   auto alp_r = ang_sens.getAngleN();
//     uint32_t tcc = HAL_GetTick();
//     auto alp_mDeg = AS5600::to_mDeg( alp_r );
//     auto sta = ang_sens.getStatus();
//
//     std_out <<  (tcc - tm00)
//     << ' ' << alp_r << ' ' << FloatMult( alp_mDeg, 3 )
//     << ' ' << HexInt8( sta )
//     << ' ' << ang_sens.getN_turn() << ' ' << ang_sens.getOldVal() << NL;
//
//     std_out.flush();
//     delay_ms_until_brk( &tm0, t_step );
// }
//
// std_out << "=== " << ang_sens.getAGCSetting() << ' ' <<  ang_sens.getCORDICMagnitude()
// << ' '    << ang_sens.isMagnetDetected() << ' ' << HexInt8( ang_sens.getStatus() ) << NL;

int cmd_stop( int argc, const char * const * argv )
{
  tim_lwm_stop();
  return 0;
}

int tim_lwm_cfg()
{
  const uint32_t psc { calc_TIM_psc_for_cnt_freq( TIM_LWM, tim_lwm_psc_freq ) };
  tim_lwm_arr = calc_TIM_arr_for_base_psc( TIM_LWM, psc, tim_lwm_freq );
  UVAR('a') = psc;
  UVAR('b') = tim_lwm_arr;

  auto &t_h { tim_lwm_h };
  t_h.Instance               = TIM_LWM;
  t_h.Init.Prescaler         = psc;
  t_h.Init.Period            = tim_lwm_arr;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &t_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &t_h, &sClockSourceConfig );

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig ) != HAL_OK ) {
    UVAR('e') = 2;
    return 0;
  }

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.Pulse        = 0; // tim_lwm_arr / 2; // TMP to test, need 0;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Stop( &t_h, ch );
    if( HAL_TIM_PWM_ConfigChannel( &t_h, &tim_oc_cfg, ch ) != HAL_OK ) {
      UVAR('e') = 3;
      return 0;
    }
  }
  return 1;
}


void tim_lwm_start()
{
  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Start( &tim_lwm_h, ch );
  }
}

void tim_lwm_stop()
{
  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Stop( &tim_lwm_h, ch );
  }
}



bool read_sensors()
{
  return true;
}


void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_LWM ) {
    TIM_LWM_EN;
    GpioA.cfgAF_N( TIM_LWM_GPIO_PIN_0, TIM_LWM_GPIO_AF );
    GpioA.cfgAF_N( TIM_LWM_GPIO_PIN_1, TIM_LWM_GPIO_AF );
    GpioA.cfgAF_N( TIM_LWM_GPIO_PIN_2, TIM_LWM_GPIO_AF );
    GpioA.cfgAF_N( TIM_LWM_GPIO_PIN_3, TIM_LWM_GPIO_AF );
    // HAL_NVIC_SetPriority( TIM_LWM_IRQn, 8, 0 );
    // HAL_NVIC_EnableIRQ( TIM_LWM_IRQn );
    return;
  }


}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_LWM ) {
    TIM_LWM_DIS;
    GpioA.cfgIn_N( TIM_LWM_GPIO_PIN_0 );
    GpioA.cfgIn_N( TIM_LWM_GPIO_PIN_1 );
    GpioA.cfgIn_N( TIM_LWM_GPIO_PIN_2 );
    GpioA.cfgIn_N( TIM_LWM_GPIO_PIN_3 );
    // HAL_NVIC_DisableIRQ( TIM_LWM_IRQn );
    return;
  }

}

// void TIM_LWM_IRQ_HANDLER()
// {
//   HAL_TIM_IRQHandler( &tim_lwm_h );
// }


void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  // read_sensors();
  // uint32_t pa = porta_sensors_bits & sensor_flags;
  //
  // if( htim->Instance == TIM_LWM ) {
  //   ++UVAR('y');
  //   // ledsx.toggle( 2 );
  //   ++tim_lwm_pulses;
  //   if( tim_lwm_need > 0 && tim_lwm_pulses >= tim_lwm_need ) {
  //     tim_lwm_stop();
  //   }
  //   return;
  // }

}


void HAL_GPIO_EXTI_Callback( uint16_t pin_bit )
{
  ++UVAR('i');
  // bool need_stop { false };

  switch( pin_bit ) {
    // case USER_STOP_BIT:
    //   need_stop = true;
    //   break_flag = (int)(BreakNum::cbreak);
    //   break;

    default:
      ledsx.toggle( 1 );
      ++UVAR('j');
      break;
  }

  // if( need_stop ) {
  //   tims_stop( TIM_BIT_ALL );
  // }
}


void EXTI2_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler( TOWER_BIT_UP );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

