#include <cstring>
#include <cstdlib>

// #include <string>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test 2 tcoubples"  };


int cmd_reset_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RESETSPI { "reset_spi", 'Z', cmd_reset_spi, " - reset spi"  };

  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_RESETSPI,
  nullptr
};



PinsOut nss_pin1( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
PinsOut nss_pin2( BOARD_SPI_DEFAULT_GPIO_EXT1, BOARD_SPI_DEFAULT_GPIO_PIN_EXT1, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d1( &spi_h, &nss_pin1 );
DevSPI spi_d2( &spi_h, &nss_pin2 );

const uint16_t MAX6675_BRK  = 0x0004;

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin1.initHW();
  //nss_pin1.set(1);
  nss_pin2.initHW();
  spi_d1.setMaxWait( 500 );
  spi_d1.initSPI();

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
  MSTRF( os, 128, prl1 );

  os <<  NL "Test0: n= " << n << " t_step= " <<  t_step << NL;
  os.flush();

  uint16_t v1, v2;
  int rc1, rc2;
  spi_d1.setTssDelay( 200 );
  spi_d2.setTssDelay( 200 );
  TickType_t tc0, tc00 = tc0;

  for( int i=0; i<n && !break_flag; ++i ) {

    TickType_t tcc = xTaskGetTickCount();
    if( i == 0 ) {
      tc0 = tc00 = tcc;
    }

    rc1 = spi_d1.recv( (uint8_t*)(&v1), sizeof(v1) );
    rc2 = spi_d2.recv( (uint8_t*)(&v2), sizeof(v2) );
    v1 = rev16( v1 ); v2 = rev16( v2 );

    os << FmtInt( tcc - tc00, 8, '0' ) << ' ';

    os << FixedPoint2( v1 >> 3 ) << ' ' << FixedPoint2( v2 >> 3 ) << ' ';

    if( v1 & MAX6675_BRK ) {
      os << " B1 ";
    }
    if( v2 & MAX6675_BRK ) {
      os << " B2 ";
    }

    if( UVAR('d') > 0 ) {
      os << NL << HexInt( v1 ) << ' ' << HexInt( v2 ) << ' '
         << rc1 << ' ' << rc2 << ' ';
    }

    os << NL;
    os.flush();
    delay_ms_until_brk( &tc0, t_step );
  }

  spi_d1.pr_info();

  return 0;
}



int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d1.resetDev();

  spi_d1.pr_info();

  return 0;
}





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

