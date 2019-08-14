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

// int cmd_testx( int argc, const char * const * argv );
// CmdInfo CMDINFO_TESTX { "testX", 'X', cmd_testx, " - test output conversion"  };

  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_RESETSPI,
//  &CMDINFO_TESTX,
  nullptr
};



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

const unsigned MAX31855_SIZE = 4; // 32 bit per packet

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

  std_out <<  NL "# Test0: n= "  <<  n <<  " t_step= "  <<  t_step <<  NL;

  uint32_t vl;
  int rc;
  spi_d.setTssDelay( 200 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    rc = spi_d.recv( (uint8_t*)(&vl), sizeof(vl) );
    vl = __builtin_bswap32( vl );// or __REV(vl)
    uint32_t tcc = HAL_GetTick();
    std_out <<  FloatMult( tcc - tm00, 3, 5 )  <<  ' ';


    int32_t tif = ( vl >> 4 ) & 0x0FFF;
    if( tif & 0x0800 ) { // sign propagete
      tif |= 0xFFFFF000;
    }
    int32_t tid4 = tif * 625; // 4 bit for fraction part
    std_out << FloatMult( tid4, 4 );

    int32_t tof =  ( vl >> 18 ) & 0x3FFF; // Temperature out: 14 bit 18:31
    if( tof & 0x2000 ) {
      tof |= 0xFFFFC000;
    }
    int tod4 = tof * 25; // 2 bit for fraction
    std_out <<  ' ' << FloatMult( tod4, 2 ) << ' ' << ( vl & 0x07 ) << ' '; // err


    if( vl & MAX31855_FAIL ) {
      std_out <<  'F';
    };
    if( vl & MAX31855_BRK ) {
      std_out <<  'B';
    }
    if( vl & MAX31855_GND ) {
      std_out <<  'G';
    }
    if( vl & MAX31855_VCC ) {
      std_out <<  'V';
    }

    if( UVAR('d') > 0 ) {
      std_out <<  " vl= " << HexInt( vl ) << " tif= "  << HexInt( tif ) <<  " tof= "  << HexInt( tof ) << " rc= " << rc;
    }

    std_out << NL;
    delay_ms_until_brk( &tm0, t_step );
  }

  if( UVAR('d') > 0 ) {
    spi_d.pr_info();
  }

  return 0;
}



int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d.resetDev();

  spi_d.pr_info();

  return 0;
}

// int cmd_testx( int argc, const char * const * argv )
// {
//   std_out << "# test output" NL;
//   const int mult = 100;
//   for( int i=-104; i<125; i+=5 ) {
//     int i1 = i / mult;
//     int i2 = i - i1 * mult;
//     std_out << i << ' ' << FloatMult( i, 2 ) << ' ' << i1 << ' ' << i2 << NL;
//   }
//   return 0;
// }


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

