#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };


int cmd_reset_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RESETSPI { "reset_spi", 'Z', cmd_reset_spi, " - reset spi"  };

  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_RESETSPI,
  nullptr
};



int SPI1_Init_common( uint32_t baud_presc  = SPI_BAUDRATEPRESCALER_256 );
PinsOut nss_pin( GPIOA, 4, 1 ); //  to test GPIO
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

const uint32_t MAX31855_FAIL = 0x00010000;
const uint32_t MAX31855_BRK  = 0x00000001;
const uint32_t MAX31855_GND  = 0x00000002;
const uint32_t MAX31855_VCC  = 0x00000004;

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  if( SPI1_Init_common() != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
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

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n      = arg2long_d( 1, argc, argv,    UVAR('n'), 1, 0xFFFFFF );
  int t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n );
  pr( NL " t_step= " ); pr_d( t_step );
  pr( NL );

  uint8_t v[4];
  int rc;
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  for( int i=0; i<n; ++i ) {
    rc = spi_d.recv( (uint8_t*)(v), sizeof(v) );
    TickType_t tcc = xTaskGetTickCount();
    pr_d( tcc - tc00 ); pr( " " );
    pr_d( rc ); pr( " " );
    // pr( NL );
    // dump8( v, sizeof(v) );

    if( v[0] & MAX31855_FAIL ) {
      pr( "FAIL, " );
      if( v[0] & MAX31855_BRK ) {
        pr( "BREAK" );
      }
      if( v[0] & MAX31855_GND ) {
        pr( "GND" );
      }
      if( v[0] & MAX31855_VCC ) {
        pr( "VCC" );
      }
    } else {
      int32_t tif =  ( v[3] >> 4 ) | ( v[2] << 4 );
      if( tif & 0x0800 ) {
        tif |= 0xFFFFF000;
      }
      // pr_d( tif ); pr( " = " ); pr_h( tif );
      int32_t tid4 = tif * 625;
      // pr( " = " ) ;
      pr_d( tid4 / 10000 ); pr( "." ); pr_d( tid4 % 10000 );
      pr( " " );

      int32_t tof =  ( v[1] >> 2 ) | ( v[0] << 6 );
      if( tof & 0x2000 ) {
        tof |= 0xFFFFC000;
      }
      int tod4 = tof * 25;
      pr( " " );
      pr_d( tod4 / 100 ); pr( "." ); pr_d( tod4 % 100 );
    }

    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
  }

  spi_d.pr_info();

  return 0;
}



int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d.resetDev();

  spi_d.pr_info();

  return 0;
}





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

