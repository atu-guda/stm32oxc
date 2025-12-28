#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as double shot" NL;

TIM_HandleTypeDef tim_h;


void tim_cfg();
unsigned pulse1( uint16_t arr, uint16_t ccr );

const unsigned MAX_TIM_WAIT = 100000000;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test doubleshot"  );



int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 10;
  UVAR_p = calc_TIM_psc_for_cnt_freq( TIM_EXA, 10000  ); // ->1kHz
  UVAR_a = 2000; // tmp: will be recalced

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  tim_cfg();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}

unsigned pulse1( uint16_t arr, uint16_t ccr )
{
  TIM_EXA->ARR  = arr;
  TIM_EXA->CCR1 = ccr;
  TIM_EXA->CNT  = 0;
  TIM_EXA->EGR |= TIM_EGR_UG;

  TIM_EXA->SR &= ~TIM_SR_UIF;
  HAL_TIM_OnePulse_Start( &tim_h, TIM_CHANNEL_1 );
  TIM_EXA->CR1 |= TIM_CR1_CEN;

  unsigned n = 0;
  for( ; n < MAX_TIM_WAIT ; ++n ) {
    if( TIM_EXA->SR & TIM_SR_UIF ) {
      break;
    }
  }
  HAL_TIM_OnePulse_Stop( &tim_h, TIM_CHANNEL_1 );
  return n;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int t_1 = arg2long_d( 1, argc, argv,   200,    1, 2000 );
  int t_d = arg2long_d( 2, argc, argv,   100,    0, 2000 );
  int t_2 = arg2long_d( 3, argc, argv,     0,    0, 2000 );

  uint16_t arr1 = (uint16_t)( t_1 * 10 );
  uint16_t ccr1 = 0;
  uint16_t arr2 = (uint16_t)( ( t_d + t_2 ) * 10 );
  uint16_t ccr2 = (uint16_t)( arr2 - t_2 * 10 );


  std_out << NL "# Test0: t_1= " << t_1 << "  t_d= " << t_d << "  t_2= " << t_2 << NL;
  std_out << "# arr1= " << arr1 << " ccr1= " << ccr1 << "  arr2= " << arr2 << "  ccr2= " << ccr2 << NL;

  leds[1].set();
  unsigned n1 = pulse1( arr1, ccr1 );
  leds[1].reset();

  if( n1 >= MAX_TIM_WAIT ) {
    std_out << "# Error: long wait after pulse 1" NL;
    // return 1; // ???
  }

  unsigned n2 = 0;
  if( t_2 > 0 ) {
    n2 = pulse1( arr2, ccr2 );
  }


  std_out << "# n1= " << n1 << " n2= " << n2 << NL;


  tim_print_cfg( TIM_EXA );

  return 0;
}


//  ----------------------------- configs ----------------

void tim_cfg()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = UVAR_p;
  tim_h.Init.Period            = UVAR_a;
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_OnePulse_Init( &tim_h, TIM_OPMODE_SINGLE ) != HAL_OK ) {
    UVAR_e = 1; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  int pbase = UVAR_a;
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM2; // why 2? works only with it!
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = 50 * pbase / 100; /// TMP: 50

  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR_e = 11;
    return;
  }
  // if( HAL_TIM_OnePulse_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1, 999 ) != HAL_OK ) {
  //   UVAR_e = 13;
  //   return;
  // }

}

// TODO: make common
void HAL_TIM_OnePulse_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_PIN1.cfgAF( TIM_EXA_GPIOAF );

  // if one timer uses different AF/GPIO, like F334:T1
  #ifdef TIM_EXA_PINS_EXT
    TIM_EXA_PIN1_EXT.cfgAF( TIM_EXA_GPIOAF_EXT );
  #endif
}

void HAL_TIM_OnePulse_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_PIN1.cfgIn();
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

