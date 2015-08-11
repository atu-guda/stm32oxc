#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;

TIM_HandleTypeDef tim1h;
TIM_OC_InitTypeDef tim_oc_cfg;
int pwm_vals[] = { 25, 50, 75, 90 };
void tim1_cfg();
void pwm_recalc();
void pwm_update();
void pwm_print_cfg();

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

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

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x00 );

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('p') = 16799;// prescaler, 168MHz->10kHz
  UVAR('a') = 9999; // ARR, 10kHz->1Hz
  UVAR('r') = 0;    // flag: raw values

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  usbcdc.init();
  usbcdc.setOnSigInt( sigint );
  devio_fds[0] = &usbcdc; // stdin
  devio_fds[1] = &usbcdc; // stdout
  devio_fds[2] = &usbcdc; // stderr
  delay_ms( 50 );

  tim1_cfg();

  delay_ms( 10 );
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

  tim1_cfg();

  pr( NL "tinit end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

void tim1_cfg()
{
  tim1h.Instance = TIM1;
  tim1h.Init.Prescaler = UVAR('p');
  tim1h.Init.Period    = UVAR('a');
  tim1h.Init.ClockDivision = 0;
  tim1h.Init.CounterMode = TIM_COUNTERMODE_UP;
  if( HAL_TIM_PWM_Init( &tim1h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }

  pwm_recalc();

}

void pwm_recalc()
{
  int pbase = UVAR('a');
  tim_oc_cfg.OCMode = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode = TIM_OCFAST_DISABLE;
  tim_oc_cfg.Pulse = pwm_vals[0] * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim1h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  tim_oc_cfg.Pulse =  pwm_vals[1] * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim1h, &tim_oc_cfg, TIM_CHANNEL_2 ) != HAL_OK ) {
    UVAR('e') = 12;
    return;
  }
  tim_oc_cfg.Pulse =  pwm_vals[2] * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim1h, &tim_oc_cfg, TIM_CHANNEL_3 ) != HAL_OK ) {
    UVAR('e') = 13;
    return;
  }
  tim_oc_cfg.Pulse =  pwm_vals[3] * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim1h, &tim_oc_cfg, TIM_CHANNEL_4 ) != HAL_OK ) {
    UVAR('e') = 14;
    return;
  }

  if( HAL_TIM_PWM_Start( &tim1h, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 21;
    return;
  }
  if( HAL_TIM_PWM_Start( &tim1h, TIM_CHANNEL_2 ) != HAL_OK ) {
    UVAR('e') = 22;
    return;
  }
  if( HAL_TIM_PWM_Start( &tim1h, TIM_CHANNEL_3 ) != HAL_OK ) {
    UVAR('e') = 23;
    return;
  }
  if( HAL_TIM_PWM_Start( &tim1h, TIM_CHANNEL_4 ) != HAL_OK ) {
    UVAR('e') = 24;
    return;
  }
}

void pwm_update()
{
  tim1h.Instance->PSC  = UVAR('p');
  int pbase = UVAR('a');
  tim1h.Instance->ARR  = pbase;
  int scl = pbase;
  if( UVAR('r') ) { // raw values
    scl = 100;
  }
  tim1h.Instance->CCR1 = pwm_vals[0] * scl / 100;
  tim1h.Instance->CCR2 = pwm_vals[1] * scl / 100;
  tim1h.Instance->CCR3 = pwm_vals[2] * scl / 100;
  tim1h.Instance->CCR4 = pwm_vals[3] * scl / 100;
}

void pwm_print_cfg()
{
  int presc = UVAR('p');
  int arr   = UVAR('a');
  int freq1 = 84000000 * 2  / ( presc + 1 ); // *2 : if APB2 prescaler != 1 (=2)
  int freq2 = freq1 / ( arr + 1 );
  pr( NL "TIM1 reinit: prescale: " ); pr_d( presc );
  pr( " ARR: " ); pr_d( arr );
  pr( " freq1: " ); pr_d( freq1 );
  pr( " freq2: " ); pr_d( freq2 ); pr( NL );
}


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

