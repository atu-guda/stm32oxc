#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

BOARD_DEFINE_LEDS;



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

void MX_I2C3_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART1 );
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART1_SEND_TASK( usartio );
// STD_USART1_RECV_TASK( usartio );
STD_USART1_IRQ( usartio );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  init_uart( &uah );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  MX_I2C3_Init( i2ch );
  i2c_dbg = &i2cd;


  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart1_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
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
  SET_UART_AS_STDIO( usartio );

  // usartio.sendStrSync( "0123456789ABCDEF" NL );
  // delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int st_a = arg2long_d( 1, argc, argv,   2,    0, 127 );
  int en_a = arg2long_d( 2, argc, argv, 127, st_a, 127 );
  pr( NL "Test0: st_a= " ); pr_h( st_a ); pr( " en_a= " ); pr_h( en_a );
  pr( NL );
  i2cd.resetDev();

  // uint8_t val;
  int i_err;
  for( uint8_t ad = (uint16_t)st_a; ad <= (uint16_t)en_a && ! break_flag; ++ad ) {
    bool rc = i2cd.ping( ad );
    i_err = i2cd.getErr();
    delay_ms( 10 );
    if( !rc ) {
      continue;
    }
    pr( "ad = " ); pr_h( ad ); pr( "  rc= " ); pr_d( rc ) ; pr( "  err= " ); pr_d( i_err ); pr(NL);
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

