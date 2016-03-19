#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_debug_i2c.h>
#include <oxc_hd44780_i2c.h>
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


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device
HD44780_i2c lcdt{ &i2ch };

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  __disable_irq();

  leds.initHW();
  leds.write( 0x0F );  delay_bad_ms( 200 );

  MX_I2C1_Init( i2ch );
  i2c_dbg = &i2cd;


  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  __enable_irq();
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
  // int n = UVAR('n');
  uint32_t t_step = UVAR('t');
  uint16_t ch_st = (uint8_t)arg2long_d( 1, argc, argv, 0x30, 0, 256-16 );
  uint16_t ch_en = ch_st + 0x10;

  lcdt.init_4b();
  int state = lcdt.getState();
  pr_sdx( state );

  lcdt.putch( 'X' );
  lcdt.puts( " ptn-hlo!\n\t" );
  lcdt.curs_on();
  delay_ms( t_step );
  lcdt.off();
  delay_ms( t_step );
  lcdt.led_off();
  delay_ms( t_step );
  lcdt.led_on();
  delay_ms( t_step );
  lcdt.on();
  lcdt.gotoxy( 2, 1 );
  for( uint16_t ch = ch_st; ch < ch_en; ++ch ) {
    lcdt.putch( (uint8_t)ch );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

