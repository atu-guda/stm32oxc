#include <cstring>
#include <cstdlib>
#include <iterator>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>
#include <oxc_menu4b.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MAX31855 thermocouple control device + LCD output" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_menu( int argc, const char * const * argv );
CmdInfo CMDINFO_MENU { "menu", 'M', cmd_menu, " N - menu action"  };

int cmd_reset_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RESETSPI { "reset_spi", 'Z', cmd_reset_spi, " - reset spi"  };


  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_RESETSPI,
  &CMDINFO_MENU,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );


PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

const unsigned MAX31855_SIZE = 4; // 32 bit per packet

const uint32_t MAX31855_FAIL = 0x00010000;
const uint32_t MAX31855_BRK  = 0x00000001;
const uint32_t MAX31855_GND  = 0x00000002;
const uint32_t MAX31855_VCC  = 0x00000004;

int T_off = 100, T_hyst = 10, t_dt = 1000;
int fun_x1( int n );

int fun_x1( int n )
{
  std_out << "#-- fun_x1: n= " << n << NL;
  return n;
}


const Menu4bItem menu_main[] = {
  { "T_off",   &T_off,    1, -100,   5000, nullptr },
  { "T_hyst" , &T_hyst,   1,    0,   1000, nullptr },
  { "t_dt",      &t_dt, 100,  100, 100000, nullptr },
  { "fun_x1",  nullptr,   1,    0, 100000,  fun_x1 }
};

MenuState menu4b_state { menu_main, size( menu_main), "T\n" };



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10000000;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;

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
  oxc_add_aux_tick_fun( menu4b_ev_dispatch );

  lcdt.init_4b();
  lcdt.puts_xy( 0, 1, "Ready!" );

  init_menu4b_buttons();

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 0xFFFFFF );
  uint32_t t_step = t_dt;


  std_out << NL "# go: n= " << n << " t= " << t_step << NL;
  std_out.flush();
  lcdt.cls();

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
    if( tif & 0x0800 ) { // sign propagation
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

    char buf0[32];
    ifcvt( tod4, 100, buf0, 2, 4 );
    lcdt.puts_xy( 0, 0, buf0 );


    delay_ms_until_brk( &tm0, t_step );
  }

  if( UVAR('d') > 0 ) {
    spi_d.pr_info();
  }

  lcdt.puts_xy( 0, 1, "Stop!  " );

  return 0;
}


int cmd_menu( int argc, const char * const * argv )
{
  int cmd_i = arg2long_d( 1, argc, argv, 0 );
  return menu4b_cmd( cmd_i );
}

int menu4b_output( const char *s1, const char *s2 )
{
  // lcdt.cls();
  if( s1 ) {
    lcdt.puts_xy( 0, 0, s1 );
  }
  if( s2 ) {
    lcdt.puts_xy( 0, 1, s2 );
  }
  return 1;
}




int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d.resetDev();

  spi_d.pr_info();

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

