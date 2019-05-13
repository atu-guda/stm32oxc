#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test US-014 ultrasonic sensor with timer" NL;

TIM_HandleTypeDef tim_h;
void init_usonic();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test sensor"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;


  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  init_usonic();

  delay_ms( 50 );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  NL "Test0: n= "  <<  n  <<  " t= "  <<  t_step  <<  NL;
  tim_print_cfg( TIM_EXA );

  delay_ms( 10 );

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    std_out <<  "["  <<  i  <<  "]  l= "  << UVAR('l')  <<  NL;
    std_out.flush();
    delay_ms_until_brk( &tm0, t_step );
  }


  return 0;
}

//  ----------------------------- configs ----------------

void init_usonic()
{
  // 5.8 mks approx 1mm 170000 = v_c/2 in mm/s, 998 or 846
  tim_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM_EXA, 170000 );
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Period            = 8500; // F approx 20Hz: for future motor PWM
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

  TIM_OC_InitTypeDef  tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse        = 3; // 3 = approx 16 us
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  if( HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 12;
    return;
  }

  TIM_IC_InitTypeDef  tim_ic_cfg;
  // tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_RISING;
  tim_ic_cfg.ICPolarity  = TIM_ICPOLARITY_BOTHEDGE; // rising - start, falling - stop
  tim_ic_cfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_ic_cfg.ICPrescaler = TIM_ICPSC_DIV1;
  tim_ic_cfg.ICFilter    = 0; // 0 - 0x0F

  if( HAL_TIM_IC_ConfigChannel( &tim_h, &tim_ic_cfg, TIM_CHANNEL_2 ) != HAL_OK ) {
    UVAR('e') = 21;
    return;
  }

  HAL_NVIC_SetPriority( TIM_EXA_IRQ, 7, 0 );
  HAL_NVIC_EnableIRQ( TIM_EXA_IRQ );

  if( HAL_TIM_IC_Start_IT( &tim_h, TIM_CHANNEL_2 ) != HAL_OK ) {
    UVAR('e') = 23;
  }
}

void TIM_EXA_IRQHANDLER(void)
{
  HAL_TIM_IRQHandler( &tim_h );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  uint32_t cap2;
  static uint32_t c_old = 0xFFFFFFFF;
  if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2 )  {
    cap2 = HAL_TIM_ReadCapturedValue( htim, TIM_CHANNEL_2 );
    if( cap2 > c_old ) {
      UVAR('l') = cap2 - c_old ;
      leds.reset( BIT2 );
    } else {
      leds.set( BIT2 );
    }
    c_old = cap2;

    UVAR('m') = cap2;
    UVAR('z') = htim->Instance->CNT;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

