#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_onewire.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;



const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_1wire0( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG );
CmdInfo CMDINFO_WIRE0 { "wire0", 'w', cmd_1wire0, " - test 1-wire"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_WIRE0,
  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

IoPin pin_wire1( GPIOC, GPIO_PIN_0 );
OneWire wire1( pin_wire1 );

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

  pin_wire1.initHW();
  wire1.initHW();

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 1*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_UART_AS_STDIO(usartio);


  // wire1.set_check_crc( 0 ); // TEST?

  default_main_loop();
  vTaskDelete(NULL);
}



// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  uint8_t buf[12];

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount();

  break_flag = 0;
  // bool out_flag = UVAR('o');
  bool ok = false;
  for( int i=0; i<n && !break_flag; ++i ) {
    // pin_wire1.set_sw0( i & 1 );
    // if( out_flag ) {
    //   uint16_t iv = pin_wire1.rw();
    //   pr( "i= " ); pr_d( i );
    //   pr( "  iv= " ); pr_h( iv );
    //   pr( NL );
    // }
    ok = wire1.skipRom( 0x44, nullptr, 0 ); // convert T
    if( !ok ) {
      pr( "Err: 0x44 fail" NL );
    }
    delay_ms( 750 );
    memset( buf, 0, sizeof(buf) );
    ok = ok && wire1.skipRom( OneWire::CMD_READ_SPAD, buf, 9 ); // read buf
    if( !ok ) {
      pr( "Err: skipRom::READ_SPAD fail" NL );
    } else {
      int te = buf[0] + (buf[1] << 8);
      // pr_sdx( te );
      te *= 1000; te /= 16;
      pr_sdx( te );
    }

    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;
  pin_wire1.sw1();

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_1wire0( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  uint8_t buf[12], addr[12];
  pr( NL "1wire test start." NL );

  bool have_dev = wire1.reset();
  pr_sdx( have_dev );
  if( !have_dev ) {
    return 1;
  }

  memset( addr, 0, sizeof(addr) );
  delay_ms( 1 ); // for logic analizator
  bool ok =  wire1.readRom( addr, 8 );
  pr( ok ? "readRom OK. addr:" NL  : "readRom FAIL!!!" NL );
  dump8( addr, 8 );


  memset( buf, 0, sizeof(buf) );
  ok = wire1.skipRom( OneWire::CMD_READ_SPAD, buf, 9 ); //  really need 9
  pr( ok ? "READ_SPAD OK. buf:" NL  : "READSPAD FAIL!!!" NL );
  dump8( buf, 12 );

  ok = wire1.skipRom( 0x44, nullptr, 0 ); // convert T
  pr( "ConvertT(44), waitung...  ok=" ); pr_d ( ok ); pr( NL );
  delay_ms( 1000 );

  memset( buf, 0, sizeof(buf) );
  wire1.skipRom( OneWire::CMD_READ_SPAD, buf, 9 ); // read buf
  pr( ok ? "READ_SPAD OK. buf:" NL  : "READSPAD FAIL!!!" NL );
  dump8( buf, 12 );

  int te = buf[0] + (buf[1] << 8);
  pr_sdx( te );
  te *= 1000; te /= 16;
  pr_sdx( te );

  memset( buf, 0, sizeof(buf) );
  ok = wire1.matchRom( addr, OneWire::CMD_READ_SPAD, buf, 9 ); // read buf with addr
  pr( "MatchROM.READ_SPAD  ok=" ); pr_d ( ok ); pr( NL );
  dump8( buf, 12 );

  memset( buf, 0, sizeof(buf) );
  addr[1] = 0xFF; // bad addr
  ok = wire1.matchRom( addr, OneWire::CMD_READ_SPAD, buf, 9 ); // read buf with addr
  pr( ok ? "Bad addr: READ_SPAD OK" NL  : "Bad addr: READSPAD FAIL!!!" NL );
  dump8( buf, 12 );

  wire1.eot();
  delay_ms( 100 );

  pr( NL "1wire test end." NL );
  return 0;
}

//  ----------------------------- configs ----------------
//

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

