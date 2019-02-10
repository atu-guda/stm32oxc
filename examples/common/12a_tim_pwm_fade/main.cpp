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

const char* common_help_string = "App to test timer as PWM source with fade" NL;

TIM_HandleTypeDef tim_h;


PwmCh pwmc[] = {
 // idx          ch              ccr    v
  { 0, TIM_CHANNEL_1, (TIM_EXA->CCR1),  1 },
  { 1, TIM_CHANNEL_2, (TIM_EXA->CCR2),  1 },
  { 2, TIM_CHANNEL_3, (TIM_EXA->CCR3),  1 },
  { 3, TIM_CHANNEL_4, (TIM_EXA->CCR4),  1 }
};
const auto n_pwm_ch = size(pwmc);
void tim_cfg();
void pwm_update();
void pwm_update_ccr();
extern const int8_t sin_table_8x8[256];

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
  UVAR('n') = 10000;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM_EXA, 10000000  ); // ->10 MHz
  UVAR('a') = 999; // ARR, 10 MHz -> 10 kHz
  // UVAR('m') = 0;    // mode: 0: up, 1: down, 2: updown

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
  unsigned n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  unsigned t_step = UVAR('t');
  for( auto &ch : pwmc ) {
    ch.v = 0;
  }
  pwm_update();

  unsigned pbase = tim_h.Instance->ARR;
  uint32_t phas[n_pwm_ch] { 0, 0, 0, 0 };
  uint32_t pdlt[n_pwm_ch] {  0x01112222, 0x02002222, 0x01701111, 0x00705555 };

  for( unsigned i=0; i<n_pwm_ch; ++i ) {
    if( argc <= (int)(i+1) ) {
      break;
    }
    pdlt[i] = strtol( argv[i+1], 0, 0 );
  }

  STDOUT_os;


  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( unsigned i=0; i<n && !break_flag; ++i ) {
    for( auto ch : pwmc ) {
      phas[ch.idx] += pdlt[ch.idx];
      ch.v = 127 + sin_table_8x8[ phas[ch.idx] >> 24 ];
      ch.ccr = ch.v * pbase / 256;
      if( UVAR('d') > 0 ) {
        os << ch.v <<  ' ' << ch.ccr << ' ' << HexInt(phas[ch.idx]) << ' ';
      }
    }
    if( UVAR('d') > 0 ) {
      os << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

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

  for( auto ch : pwmc ) {
    HAL_TIM_PWM_Stop( &tim_h, ch.ch );
    tim_oc_cfg.Pulse = ch.v * pbase / 100;
    if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, ch.ch ) != HAL_OK ) {
      UVAR('e') = 11 + ch.idx;
      return;
    }
    HAL_TIM_PWM_Start( &tim_h, ch.ch );
  }

}

void pwm_update()
{
  tim_h.Instance->PSC  = UVAR('p');
  tim_h.Instance->ARR  = UVAR('a');
  pwm_update_ccr();
}

void pwm_update_ccr()
{
  unsigned pbase = tim_h.Instance->ARR;
  for( auto ch : pwmc ) {
    ch.ccr = ch.v * pbase / 256;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

