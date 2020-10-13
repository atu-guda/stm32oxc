#include <iterator>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as double shot" NL;

TIM_HandleTypeDef tim_h;


void tim_cfg();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test doubleshot"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM_EXA, 10000  ); // ->1kHz
  UVAR('a') = 9999; // ARR, 10kHz->1Hz - not now

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  tim_cfg();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int t_1 = arg2long_d( 1, argc, argv,   200,    1, 20000 );
  int t_d = arg2long_d( 2, argc, argv,   100,    0, 20000 );
  int t_2 = arg2long_d( 3, argc, argv,   300,    0, 20000 );

  uint16_t arr1 = (uint16_t)( t_1 * 10 );
  uint16_t ccr1 = 1;
  uint16_t arr2 = (uint16_t)( ( t_d + t_2 ) * 10 );
  uint16_t ccr2 = (uint16_t)( arr2 - t_2 * 10 );


  std_out << NL "# Test0: t_1= " << t_1 << "  t_d= " << t_d << "  t_2= " << t_2 << NL;
  std_out << "# arr1= " << arr1 << "  arr2= " << arr2 << "  ccr2= " << ccr2 << NL;

  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

  tim_print_cfg( TIM_EXA );

  delay_ms( 5000 ); // TMP: BUG: any state
  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );

  return 0;
}


//  ----------------------------- configs ----------------

void tim_cfg()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = UVAR('p');
  tim_h.Init.Period            = UVAR('a');
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

  int pbase = UVAR('a');
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = 50 * pbase / 100; /// TMP: 50
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }

}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PIN1, TIM_EXA_GPIOAF );

  // if one timer uses different AF/GPIO, like F334:T1
  #ifdef TIM_EXA_PINS_EXT
    TIM_EXA_GPIO_EXT.cfgAF_N( TIM_EXA_PIN1_EXT, TIM_EXA_GPIOAF );
  #endif
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PIN1 );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

