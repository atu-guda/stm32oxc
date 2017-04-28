#define _GNU_SOURCE
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS_EX;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

extern DAC_HandleTypeDef hdac;
int MX_DAC_Init();
const int dacbuf_sz = 64;
int16_t dacbuf[dacbuf_sz];

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_ofast( int argc, const char * const * argv );
CmdInfo CMDINFO_OFAST { "ofast", 'F', cmd_ofast, " - fast meandre "  };
int cmd_fun( int argc, const char * const * argv );
CmdInfo CMDINFO_FUN { "fun", 'U', cmd_fun, " N type delay - funcs outout "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OFAST,
  &CMDINFO_FUN,
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

  leds.initHW();
  leds.write( BOARD_LEDS_ALL_EX );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL_EX );
    return 0;
  }

  HAL_Delay( 200 ); // delay_bad_ms( 200 );
  leds.write( 0x00 ); delay_ms( 200 );
  leds.write( BOARD_LEDS_ALL_EX );  HAL_Delay( 200 );

  if( ! init_uart( &uah ) ) {
      die4led( 1 );
  }
  leds.write( 0x0A );  delay_bad_ms( 200 );

  UVAR('e') = MX_DAC_Init();

  leds.write( 0x05 );  delay_bad_ms( 200 );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
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

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t v1 = arg2long_d( 1, argc, argv, UVAR('v'), 0 );
  uint32_t v2 = v1;
  int n = arg2long_d( 2, argc, argv, UVAR('n'), 0 );
  pr( NL "Test0: n= " ); pr_d( n ); pr( " v1= " ); pr_d( v1 ); pr( " v2= " ); pr_d( v2 );
  pr( NL );

  auto r1 = HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, v1 );
  auto r2 = HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, v2 );

  pr( "r1= " ); pr_d( r1 ); pr( "  r2= " ); pr_d( r2 ); pr( NL );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_2 );
  uint32_t vv = 3250 * v1 / 4096;
  pr_sdx( vv );

  return 0;
}

int cmd_ofast( int argc, const char * const * argv )
{
  int n   = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int dly = arg2long_d( 2, argc, argv, 0, 0, 1000 );
  pr( "ofast: n= " ); pr_d( n ); pr( " dly= " ); pr_d( dly );
  pr( NL );

  for( int i=0; i<n; ++i ) {
    HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (i&1)*4095 );
    HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
    if( dly ) {
      delay_bad_mcs( dly );
    }
  }

  return 0;
}

int cmd_fun( int argc, const char * const * argv )
{
  int n   = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int tp  = arg2long_d( 2, argc, argv, 0, 0, 5 );
  int dly = arg2long_d( 3, argc, argv, 0, 0, 1000 );
  pr( "funcs: n= " ); pr_d( n ); pr( " tp= " ); pr_d( tp ); pr( " dly= " ); pr_d( dly );
  pr( NL );

  switch( tp ) {
    case 0:
      for( int i=0; i<dacbuf_sz; ++i ) {
        float vf = sinf( 2 * M_PI * (float)(i)/dacbuf_sz );
        dacbuf[i] = 2048 + int16_t( vf * 2047 );
      }
      break;
    case 1:
      for( int i=0; i<dacbuf_sz; ++i ) {
        float x = (float)(i)/dacbuf_sz-0.5;
        dacbuf[i] = int16_t( x * x * 4 * 4095 );
      }
      break;
    case 2:
      for( int i=0; i<dacbuf_sz; ++i ) {
        dacbuf[i] = ( i & 0xFFF0 ) * 4095 / dacbuf_sz;
      }
      break;
    default:
      for( int i=0; i<dacbuf_sz; ++i ) {
        dacbuf[i] = i * 4095 / dacbuf_sz;
      }
      break;
  }

  for( int i=0; i<n && !break_flag; ++i ) {
    HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dacbuf[i%dacbuf_sz] );
    HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
    if( dly ) {
      delay_bad_mcs( dly );
    }
  }

  return 0;
}



//  ----------------------------- configs ----------------



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

