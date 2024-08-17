#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test encoder interface in timer" NL;

TIM_HandleTypeDef tim_h;
void init_enco();

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
  UVAR('n') = 2000;


  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  init_enco();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  NL "Test0: n= "  <<  n  <<  " t= "  <<  t_step <<  NL;

  delay_ms( 10 );

  int old_cnt = -1;
  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint16_t t_cnt = tim_h.Instance->CNT;
    if( t_cnt != old_cnt ) {
      std_out <<  "["  <<  i <<  "]  CNT= "  <<  t_cnt <<  "  D= "  <<  t_cnt - old_cnt << NL;
    }
    old_cnt = t_cnt;

    delay_ms_until_brk( &tm0, t_step );
  }

  tim_print_cfg( TIM_EXA );

  return 0;
}

//  ----------------------------- configs ----------------

void init_enco()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Period            = 0xFFFFFFFF;
  tim_h.Init.Prescaler         = 0; // 0?
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;

  TIM_Encoder_InitTypeDef enco_cfg;
  enco_cfg.EncoderMode    = TIM_ENCODERMODE_TI12;
  enco_cfg.IC1Polarity    = TIM_ICPOLARITY_RISING;
  enco_cfg.IC1Selection   = TIM_ICSELECTION_DIRECTTI;
  enco_cfg.IC1Prescaler   = TIM_ICPSC_DIV1;
  enco_cfg.IC1Filter      = 0x0F; /// 0-F
  enco_cfg.IC2Polarity    = TIM_ICPOLARITY_RISING;
  enco_cfg.IC2Selection   = TIM_ICSELECTION_DIRECTTI;
  enco_cfg.IC2Prescaler   = TIM_ICPSC_DIV1;
  enco_cfg.IC2Filter      = 0x0F; /// 0-F

  if( HAL_TIM_Encoder_Init( &tim_h, &enco_cfg ) != HAL_OK ) {
    UVAR('e') = 1;
    return;
  }
  HAL_TIM_Encoder_Start( &tim_h, TIM_CHANNEL_ALL );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

