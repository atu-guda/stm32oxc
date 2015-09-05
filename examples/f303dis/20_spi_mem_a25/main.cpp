#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usartio.h>
#include <oxc_console.h>
#include <oxc_spi.h>
#include <oxc_spimem_at.h>
#include <oxc_debug1.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;


const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_spimem_erase( int argc, const char * const * argv );
CmdInfo CMDINFO_ERASR { "erase",  0, cmd_spimem_erase, "- ERASE CHIP!"  };

int cmd_spimem_sector0_erase( int argc, const char * const * argv );
CmdInfo CMDINFO_ERAS0 { "era0",  0, cmd_spimem_sector0_erase, "- erase sector 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_ERASR,
  &CMDINFO_ERAS0,
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

int MX_SPI2_Init();
PinsOut nss_pin( GPIOB, 12, 1 );
SPI_HandleTypeDef spi2_h;
DevSPI spi_d( &spi2_h, &nss_pin );
DevSPIMem_AT memspi( spi_d );

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

  if( MX_SPI2_Init() != HAL_OK ) {
    die4led( 0x04 );
  }
  spi_d.initSPI();

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('r') = 0x20; // default bytes to read/write

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

#define DLY_T delay_mcs( 10 );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int nd     = imin( UVAR('r'), sizeof(gbuf_b) );
  pr( NL "Test0: nd= " ); pr_d( nd );
  pr( NL );

  uint16_t chip_id = memspi.read_id();
  pr_shx( chip_id );

  int status = memspi.status();
  pr_shx( status );

  memset( gbuf_b, '\x00', sizeof( gbuf_b ) );

  int rc = memspi.read( (uint8_t*)gbuf_b, 0x00, nd );
  pr( " Read Before: rc = " ); pr_d( rc ); pr( NL );
  dump8( gbuf_b, nd );

  for( int i=0; i<nd; ++i ) {
    gbuf_a[i] = (char)( '0' + i );
  }
  rc = memspi.write( (uint8_t*)gbuf_a, 0x00, nd );
  pr( NL "Write: rc= " ); pr_d( rc ); pr( NL );

  rc = memspi.read( (uint8_t*)gbuf_b, 0x00, nd );
  pr( " Read After: rc = " ); pr_d( rc ); pr( NL );
  dump8( gbuf_b, nd );

  rc = memspi.read( (uint8_t*)gbuf_b, 0x03, nd );
  pr( " Read After with offset 3: rc = " ); pr_d( rc ); pr( NL );
  dump8( gbuf_b, nd );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_spimem_erase( int argc, const char * const * argv )
{
  pr( NL "Erase chips " NL );

  TickType_t tc0 = xTaskGetTickCount();

  int rc = memspi.erase_chip();

  TickType_t tc1 = xTaskGetTickCount();

  pr( NL "rc= " ); pr_d( rc ); pr( " ticks: " ); pr_d( tc1 - tc0 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "Erase end." NL );
  return 0;
}

int cmd_spimem_sector0_erase( int argc, const char * const * argv )
{
  pr( NL "Erase sector 0 " NL );

  TickType_t tc0 = xTaskGetTickCount();

  int rc = memspi.erase_sector( 0x000000 );

  TickType_t tc1 = xTaskGetTickCount();

  pr( NL "rc= " ); pr_d( rc ); pr( " ticks: " ); pr_d( tc1 - tc0 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "Erase end." NL );
  return 0;
}



//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc
