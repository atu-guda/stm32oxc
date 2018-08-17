#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MAX31855 thermocouple control device" NL;

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



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
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

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n      = arg2long_d( 1, argc, argv,    UVAR('n'), 1, 0xFFFFFF );
  int t_step = UVAR('t');

  STDOUT_os;
  os <<  NL "Test0: n= "  <<  n <<  " t_step= "  <<  t_step <<  NL;

  uint8_t v[4];
  int rc;
  spi_d.setTssDelay( 200 );
  // TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    rc = spi_d.recv( (uint8_t*)(v), sizeof(v) );
    uint32_t tcc = HAL_GetTick();
    os <<  tcc - tm00  <<  ' ' <<  rc  << ' ';

    if( v[0] & MAX31855_FAIL ) {
      os <<  "FAIL, ";
      if( v[0] & MAX31855_BRK ) {
        os <<  "BREAK";
      }
      if( v[0] & MAX31855_GND ) {
        os <<  "GND";
      }
      if( v[0] & MAX31855_VCC ) {
        os <<  "VCC";
      }
    }; // even if fail

    int32_t tif =  ( v[3] >> 4 ) | ( v[2] << 4 );
    if( tif & 0x0800 ) {
      tif |= 0xFFFFF000;
    }
    // os <<  tif  <<  " = "  HexInt( tif );
    int32_t tid4 = tif * 625;
    // os << " = ";
    // os <<  tid4 / 10000  <<  '.'  <<  tid4 % 10000   <<  " ";
    os << FloatMult( tid4, 4 );

    int32_t tof =  ( v[1] >> 2 ) | ( v[0] << 6 );
    if( tof & 0x2000 ) {
      tof |= 0xFFFFC000;
    }
    int tod4 = tof * 25;
    os <<  ' ' << FloatMult( tod4, 2 );
    // <<  tod4 / 100  <<  "."  <<  tod4 % 100 

    if( UVAR('d') > 0 ) {
      os <<  " tif= "  << HexInt( tif ) <<  " tof= "  << HexInt( tof );
      dump8( v, sizeof(v) );
    }

    os << NL; os.flush();
    delay_ms_until_brk( &tm0, t_step );
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

