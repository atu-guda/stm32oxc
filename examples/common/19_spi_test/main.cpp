#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_spi_debug.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to manual control SPI devices" NL;



PinOut dbg_pin( BOARD_SPI_DEFAULT_PIN_EXT2 );
PinOut nss_pin( BOARD_SPI_DEFAULT_PIN_SNSS );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 10;
  UVAR_r = 0x10; // default bytes to read
  UVAR_q =   10; // debug delay

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
    die4led( 0x04_mask );
  }
  dbg_pin.initHW();
  spi_d.setMaxWait( 500 );
  spi_d.setTssDelay_100ns( 10 );
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

