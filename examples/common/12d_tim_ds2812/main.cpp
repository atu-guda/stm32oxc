#include <cstring>
#include <iterator>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test DS2812 LED controller with timer and DMA" NL;

TIM_HandleTypeDef tim_h;


void tim_cfg();
uint16_t t_min, t_max; // timings got 0/1 on wire (in times ticks) min1-max0 = 0, max1-min0 = 1
const uint32_t max_leds = 256; // each LED require 3*8 = 24 bit, each require halfword
uint16_t ledbuf[max_leds*24];


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10;
  UVAR('n') = 1;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  tim_cfg();
  memset( ledbuf, 0, sizeof(ledbuf) );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  unsigned v = arg2long_d( 1, argc, argv, 0, 0, 100 );
  unsigned td = arg2long_d( 2, argc, argv, UVAR('t'), 0 );
  unsigned n = UVAR('n');

  for( unsigned nled=0; nled < n; ++nled ) {
    unsigned ofs = nled * 24;
    for( unsigned i=0; i<8; ++i ) {
      ledbuf[ofs+i] = t_min;
    }
    ofs += 8;
    for( unsigned i=0; i<8; ++i ) {
      ledbuf[ofs+i] = t_max;
    }
    ofs += 8;
    for( unsigned i=0; i<8; ++i ) {
      ledbuf[ofs+i] = t_min;
    }
  }

  uint16_t s = 0;
  switch( v ) {
    case 0: s = 0;     break;
    case 1: s = t_min; break;
    case 2: s = t_max; break;
    default: s = v; break;
  }

  std_out << "# Test: s= " << s << "  td= " << td <<  NL;
  TIM_EXA->CCR1 = s;

  delay_ms( td );
  TIM_EXA->CCR1 = 0;



  // uint32_t tm0 = HAL_GetTick();
  // break_flag = 0;
  // for( unsigned i=0; i<n && !break_flag; ++i ) {
  //   ch.ccr = ch.v * pbase / 256;
  //   if( UVAR('d') > 0 ) {
  //     std_out << NL;
  //   }
  //
  //   delay_ms_until_brk( &tm0, t_step );
  // }

  return 0;
}

//  ----------------------------- configs ----------------

void tim_cfg()
{
  auto pbase = calc_TIM_arr_for_base_psc( TIM_EXA, 0, 800000 );
  UVAR('a') = pbase;
  t_min = (uint16_t)( pbase * 7 / 25 ); // * 0.28
  t_max = (uint16_t)( pbase - t_min );
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = 0;
  tim_h.Init.Period            = pbase;
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = 0; // pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PINS, TIM_EXA_GPIOAF );

  // if one timer uses different AF/GPIO, like F334:T1
  #ifdef TIM_EXA_PINS_EXT
    TIM_EXA_GPIO_EXT.cfgAF_N( TIM_EXA_PINS_EXT, TIM_EXA_GPIOAF );
  #endif
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PINS );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

