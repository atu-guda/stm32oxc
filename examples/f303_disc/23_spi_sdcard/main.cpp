#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_spi_sdcard.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_sendr_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_SENDR { "sendr", 'S', cmd_sendr_spi, "[0xXX ...] - send bytes, recv UVAR('r')"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SENDR,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

int MX_SPI2_Init( uint32_t prescal = SPI_BAUDRATEPRESCALER_64 );
PinsOut nss_pin( GPIOB, 12, 1 ); // 4 - to test GPIO
SPI_HandleTypeDef spi2_h;
DevSPI spi_d( &spi2_h, &nss_pin );

DevSPI_SdCard card( spi_d );
uint8_t cardbuf[512];

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('r') = 0x20; // default bytes to read

  if( MX_SPI2_Init() != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_UART_AS_STDIO(usartio);

  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}

#define DLY_T delay_mcs( 10 );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint8_t sv = arg2long_d( 1, argc, argv, 0x15, 0, 0xFF );
  int nd     = arg2long_d( 2, argc, argv,    2, 0, sizeof(gbuf_a) );
  pr( NL "Test0: sv= " ); pr_h( sv ); pr( " nd= " ); pr_d( nd );
  pr( NL );

  // // for logic analizer
  // nss_pin.reset( 1 );
  // DLY_T;
  // nss_pin.set( 1 );
  // DLY_T;

  // spi_d.resetDev();

  card.init();


  // int rc = spi_d.send_recv( sv, (uint8_t*)gbuf_a, nd );
  // int rc = spi_d.send( (uint8_t)sv );
  // int rc = spi_d.recv( (uint8_t*)gbuf_a, imin(UVAR('r'),sizeof(gbuf_a)) );


  //pr_sdx( rc );
  //if( rc > 0 ) {
  //  dump8( gbuf_a, rc );
  // }
  pr( NL "SPI2" NL );
  pr_shx( SPI2->CR1 );
  pr_shx( SPI2->CR2 );
  pr_shx( SPI2->SR );
  pr_shx( SPI2->DR );

  return 0;
}

int cmd_sendr_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[16]; // really used not more then 9 - max args
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  int nd = imin( UVAR('r'), sizeof(gbuf_a) );
  pr( NL "Send/recv: ns= " ); pr_d( ns ); pr( " nd= " ); pr_d( nd );
  pr( NL );
  dump8( sbuf, ns );

  int rc = spi_d.send_recv( sbuf, ns, (uint8_t*)gbuf_a, nd );

  pr_sdx( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  }

  return 0;
}


//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

