#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usartio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;


TIM_HandleTypeDef tim8h;
int pwm_vals[] = { 25, 50, 75, 90 };
void tim8_cfg();
void pwm_recalc();
void pwm_update();
void pwm_print_cfg();

const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TINIT,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
int init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  if( !init_uart( &uah ) ) {
    die4led( 0x08 );
  }
  leds.write( 0x0A );  delay_bad_ms( 200 );

  leds.write( 0x00 );

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('p') = 14399;// prescaler, 144MHz->10kHz
  UVAR('a') = 9999; // ARR, 10kHz->1Hz
  UVAR('r') = 0;    // flag: raw values

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 1*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  delay_ms( 50 );

  usartio.itEnable( UART_IT_RXNE );
  usartio.setOnSigInt( sigint );
  devio_fds[0] = &usartio; // stdin
  devio_fds[1] = &usartio; // stdout
  devio_fds[2] = &usartio; // stderr

  tim8_cfg();
  delay_ms( 50 );
  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while(1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      srl.redraw();
    }
    idle_flag = 0;
    delay_ms( 60000 );
    // delay_ms( 1 );

  }
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  for( int i=0; i<4; ++i ) {
    if( argc > i+1 ) {
      pwm_vals[i] = strtol( argv[i+1], 0, 0 );
    }
  }

  pr( NL "Test0: pwm_vals[]= " );
  for( int i=0; i<4; ++i ) {
    pr_d( pwm_vals[i] ); pr( " " );
  }
  pr( NL );

  pwm_print_cfg();
  // pwm_recalc();
  pwm_update();

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_tinit( int argc, const char * const * argv )
{
  pwm_print_cfg();

  tim8_cfg();

  pr( NL "tinit end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

void tim8_cfg()
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  // TIM_ClearInputConfigTypeDef sClearInputConfig;
  // TIM_IC_InitTypeDef sConfigIC;
  // TIM_OC_InitTypeDef tim_oc_cfg;
  // TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  // TIM_MasterConfigTypeDef sMasterConfig;

  tim8h.Instance = TIM8;
  tim8h.Init.Prescaler =  UVAR('p');
  tim8h.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim8h.Init.Period =  UVAR('a');
  tim8h.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim8h.Init.RepetitionCounter = 0;

  if( HAL_TIM_PWM_Init( &tim8h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }

  // if( HAL_TIM_OC_Init( &tim8h ) != HAL_OK ) {
  //   UVAR('e') = 5;
  //   return;
  // }
  //
  // if( HAL_TIM_IC_Init( &tim8h ) != HAL_OK ) {
  //   UVAR('e') = 7;
  //   return;
  // }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim8h, &sClockSourceConfig );


  // sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  // sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  // sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  // sConfigIC.ICFilter = 0;
  // HAL_TIM_IC_ConfigChannel(&tim8h, &sConfigIC, TIM_CHANNEL_2);

  // tim_oc_cfg.OCMode = TIM_OCMODE_TIMING;
  // tim_oc_cfg.Pulse = 0;
  // tim_oc_cfg.OCPolarity = TIM_OCPOLARITY_HIGH;
  // tim_oc_cfg.OCIdleState = TIM_OCIDLESTATE_RESET;
  // HAL_TIM_OC_ConfigChannel(&tim8h, &tim_oc_cfg, TIM_CHANNEL_4);

  // tim_oc_cfg.OCMode = TIM_OCMODE_PWM1;
  // tim_oc_cfg.OCFastMode = TIM_OCFAST_DISABLE;
  // tim_oc_cfg.Pulse = tim8h.Init.Period /2;
  // if( HAL_TIM_PWM_ConfigChannel( &tim8h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
  //   UVAR('e') = 9;
  //   return;
  // };
  // HAL_TIM_PWM_ConfigChannel( &tim8h, &tim_oc_cfg, TIM_CHANNEL_2 );
  // HAL_TIM_PWM_ConfigChannel( &tim8h, &tim_oc_cfg, TIM_CHANNEL_3 );
  // HAL_TIM_PWM_ConfigChannel( &tim8h, &tim_oc_cfg, TIM_CHANNEL_4 );


  // sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  // sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  // sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  // sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  // sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  // sBreakDeadTimeConfig.BreakFilter = 0;
  // sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  // sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  // sBreakDeadTimeConfig.Break2Filter = 0;
  // sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  // HAL_TIMEx_ConfigBreakDeadTime( &tim8h, &sBreakDeadTimeConfig );

  // sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  // sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  // sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  // HAL_TIMEx_MasterConfigSynchronization(&tim8h, &sMasterConfig);

  pwm_recalc();

}

void pwm_recalc()
{
  TIM_OC_InitTypeDef tim_oc_cfg;
  int pbase = UVAR('a');
  tim_oc_cfg.OCMode = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode = TIM_OCFAST_DISABLE;

  const int nch = 4;
  const int channels[nch] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

  for( int i=0; i<nch; ++i ) {
    HAL_TIM_PWM_Stop( &tim8h, channels[i] );
    tim_oc_cfg.Pulse = pwm_vals[i] * pbase / 100;
    if( HAL_TIM_PWM_ConfigChannel( &tim8h, &tim_oc_cfg, channels[i] ) != HAL_OK ) {
      UVAR('e') = 11+i;
      return;
    }
    HAL_TIM_PWM_Start( &tim8h, channels[i] );
  }

}

void pwm_update()
{
  tim8h.Instance->PSC  = UVAR('p');
  int pbase = UVAR('a');
  tim8h.Instance->ARR  = pbase;
  int scl = pbase;
  if( UVAR('r') ) { // raw values
    scl = 100;
  }
  tim8h.Instance->CCR1 = pwm_vals[0] * scl / 100;
  tim8h.Instance->CCR2 = pwm_vals[1] * scl / 100;
  tim8h.Instance->CCR3 = pwm_vals[2] * scl / 100;
  tim8h.Instance->CCR4 = pwm_vals[3] * scl / 100;
}

void pwm_print_cfg()
{
  int presc = UVAR('p');
  int arr   = UVAR('a');
  int freq1 = 72000000 * 2  / ( presc + 1 ); // *2 : if APB2 prescaler != 1 (=2)
  int freq2 = freq1 / ( arr + 1 );
  pr( NL "TIM8 reinit: prescale: " ); pr_d( presc );
  pr( " ARR: " ); pr_d( arr );
  pr( " freq1: " ); pr_d( freq1 );
  pr( " freq2: " ); pr_d( freq2 ); pr( NL );
}


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

