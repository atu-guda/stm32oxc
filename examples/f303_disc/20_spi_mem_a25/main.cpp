#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_spimem_at.h>

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


int MX_SPI2_Init( uint32_t prescal = SPI_BAUDRATEPRESCALER_64 );
PinsOut nss_pin( GPIOB, 12, 1 );
SPI_HandleTypeDef spi2_h;
DevSPI spi_d( &spi2_h, &nss_pin );
DevSPIMem_AT memspi( spi_d );

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;
  UVAR('r') = 0x20; // default bytes to read

  if( MX_SPI2_Init() != HAL_OK ) {
    die4led( 0x04 );
  }
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}

// #define DLY_T delay_mcs( 10 );

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
  status = memspi.status(); pr_shx( status ); pr( NL );

  for( int i=0; i<nd; ++i ) {
    gbuf_a[i] = (char)( '0' + i );
  }
  rc = memspi.write( (uint8_t*)gbuf_a, 0x00, nd );
  pr( NL "Write: rc= " ); pr_d( rc ); pr( NL );
  status = memspi.status(); pr_shx( status ); pr( NL );

  rc = memspi.read( (uint8_t*)gbuf_b, 0x00, nd );
  pr( " Read After: rc = " ); pr_d( rc ); pr( NL );
  dump8( gbuf_b, nd );
  status = memspi.status(); pr_shx( status ); pr( NL );

  rc = memspi.read( (uint8_t*)gbuf_b, 0x03, nd );
  pr( " Read After with offset 3: rc = " ); pr_d( rc ); pr( NL );
  dump8( gbuf_b, nd );
  status = memspi.status(); pr_shx( status ); pr( NL );

  return 0;
}

int cmd_spimem_erase( int argc, const char * const * argv )
{
  int rc = memspi.erase_chip();

  return rc;
}

int cmd_spimem_sector0_erase( int argc, const char * const * argv )
{
  int rc = memspi.erase_sector( 0x000000 );

  return rc;
}



//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

