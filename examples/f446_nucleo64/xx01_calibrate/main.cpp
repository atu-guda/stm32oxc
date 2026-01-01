#include <cmath>
#include <cstdio>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_osfun.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;



// --- local commands;
DCL_CMD_REG( test0, 'T', " - "  );




int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 10;
  // UVAR_x = 10000000; // for calibrate
  uint32_t c_v0 = delay_calibrate_value;
  UVAR_x = c_v0 * 500; // for calibrate 500 ms


  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );


  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR_n, 0 );
  uint32_t t_step = UVAR_t;

  std_out <<  NL "Test0: n= "  <<  n  <<  " t= "  <<  t_step   <<  NL;

  std_out <<  "Current delay_calibrate_value= "  <<  delay_calibrate_value  <<  NL;
  do_delay_calibrate();
  std_out <<  "New delay_calibrate_value= "  <<  delay_calibrate_value  <<  NL;

  UVAR_z = _write( 1, "ABCDEFGH", 6 );
  putchar( 'X' );
  puts( "012345" );
  _write( 1, "ZZZ\r\n", 5 );
  printf( "M_PI= %.12g" NL, M_PI );


  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t tcc = HAL_GetTick();

    std_out << "Fake Action i= " <<  i <<  "  tick: " <<  tcc - tm00 << NL;

    // delay_ms_until_brk( &tm0, t_step );
    delay_bad_ms( t_step );
  }

  // delay_bad_n( UVAR_x );
  // // delay_bad_n( 335730317 ); // for 168 Mhz 10 s
  // // delay_bad_n( 33539491 );     // for 168 Mhz 1 s
  // // delay_bad_n( 33539 );     // for 168 Mhz 1 ms
  // uint32_t tm1 = HAL_GetTick(), dlt = tm1 - tm0;
  // uint32_t c_v = UVAR_x / dlt;
  // <<  "n= " pr_d( UVAR_x ); <<  "  delta= "  <<  dlt  <<  "  c_v= "  <<  c_v  <<  NL 

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

