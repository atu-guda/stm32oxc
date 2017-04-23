#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [N] [01] - test step 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

PinsOut motor { GPIOE, 0, 4 };

const uint8_t half_steps4[] = { 1, 3, 2, 6, 4, 12, 8, 9 };
const uint8_t full_steps4[] = { 1, 2, 4, 8 };
const uint8_t half_steps3[] = { 1, 3, 2, 6, 4, 5 };

struct MotorMode {
  int n_steps;
  const uint8_t *steps;
};

MotorMode m_modes[] = {
  { 8, half_steps4 },
  { 4, full_steps4 },
  { 6, half_steps3 }, // 3-phase modes
  { 3, full_steps4 }  // part of 4-phase
};
const int n_modes = sizeof(m_modes)/sizeof(MotorMode);


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  delay_bad_ms( 200 );  leds.write( 0 );

  motor.initHW();

  leds.write( 0x01 );  delay_bad_ms( 200 );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_USBCDC_AS_STDIO(usbcdc);

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  static int ph = 0;
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

  int m = UVAR('m');
  if( m >= n_modes ) {
    m = 0;
  }

  const uint8_t *steps = m_modes[m].steps;
  int ns = m_modes[m].n_steps;
  if( ph >= ns ||  ph < 0 ) { ph = 0; }

  int d = 1;
  if( argc > 2  && argv[2][0] == '-' ) {
    d = ns - 1; // % no zero
  }
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( " m= " ); pr_d( m );
  pr( " d= " ); pr_d( d );
  pr( NL );

  if( n < 1 ) {
    motor.write( 0 );
    ph = 0;
    pr( NL "Motor off." NL );
    return 0;
  }


  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  for( int i=0; i<n && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    if( t_step > 500 ) {
      pr( " Step  i= " ); pr_d( i );
      pr( " ph: " ); pr_d( ph );
      pr( " v: " ); pr_h( steps[ph] );
      pr( "  tick: "); pr_d( tcc - tc00 );
      pr( NL );
    }
    motor.write( steps[ph] );
    ph += d;
    ph %= ns;
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }

  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

