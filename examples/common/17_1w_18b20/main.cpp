#include <cstring>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_onewire.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test some oneWire device" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test oneWire"  );
DCL_CMD_REG( wire0, 'w', " - test 1-wire"  );



IoPin pin_wire1( BOARD_1W_DEFAULT_GPIO, BOARD_1W_DEFAULT_PIN );
OneWire wire1( pin_wire1 );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;

  // UVAR('a') = delay_calibrate_value;
  // approx_delay_calibrate();
  // UVAR('b') = delay_calibrate_value;
  // do_delay_calibrate();
  UVAR('d') = delay_calibrate_value;

  pin_wire1.initHW();
  wire1.initHW();

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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  NL "Test0: n= "  <<  n  <<  " t= "  <<  t_step  <<  NL;
  uint8_t buf[12];


  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  // bool out_flag = UVAR('o');
  bool ok = false;
  for( int i=0; i<n && !break_flag; ++i ) {
    // pin_wire1.set_sw0( i & 1 );
    // if( out_flag ) {
    //   uint16_t iv = pin_wire1.rw();
    //   std_out <<  "i= "  <<  i   <<  "  iv= "  << HexInt( iv ) <<  NL;
    // }
    ok = wire1.skipRom( 0x44, nullptr, 0 ); // convert T
    if( !ok ) {
      std_out <<  "Err: 0x44 fail" NL;
    }

    delay_ms( 750 );
    memset( buf, 0, sizeof(buf) );
    ok = ok && wire1.skipRom( OneWire::CMD_READ_SPAD, buf, 9 ); // read buf
    if( !ok ) {
      std_out <<  "Err: skipRom::READ_SPAD fail" NL;
    } else {
      int te0 = buf[0] + (buf[1] << 8);
      // pr_sdx( te );
      int te = te0 * 10000 / 16;
      // ifcvt( te, 10000, obuf, 4 ); // TODO: hex
      // std_out <<  obuf  <<  NL;
      std_out <<  FloatMult( te, 4 )  << ' ' << te0 << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }
  pin_wire1.sw1();

  return 0;
}

int cmd_wire0( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  uint8_t buf[12], addr[12];
  std_out <<  NL "1wire test start." NL;

  // pin_wire1.sw1();
  // delay_ms( 200 );
  // pin_wire1.sw0();
  // delay_ms( 200 );
  // pin_wire1.sw1();

  // wire1.initHW();
  // delay_ms( 100 );
  UVAR('i') = pin_wire1.rw_raw();

  bool have_dev = wire1.reset();
  if( !have_dev ) {
    std_out << "Fail to find device" << NL;
    return 1;
  }

  memset( addr, 0, sizeof(addr) );
  delay_ms( 1 ); // for logic analizator
  bool ok =  wire1.readRom( addr, 8 );
  std_out <<  ( ok ? ( "readRom OK. addr:" NL)  : ( "readRom FAIL!!!" NL ) );
  dump8( addr, 8 );


  memset( buf, 0, sizeof(buf) );
  ok = wire1.skipRom( OneWire::CMD_READ_SPAD, buf, 9 ); //  really need 9
  std_out << ( ok ? ( "READ_SPAD OK. buf:" NL ) : ( "READSPAD FAIL!!!" NL ) );
  dump8( buf, 12 );

  ok = wire1.skipRom( 0x44, nullptr, 0 ); // convert T
  std_out << "ConvertT(44), waitung...  ok= " << ok <<  NL;
  delay_ms( 1000 );

  memset( buf, 0, sizeof(buf) );
  wire1.skipRom( OneWire::CMD_READ_SPAD, buf, 9 ); // read buf
  std_out <<  ( ok ? ( "READ_SPAD OK. buf:" NL ) : ( "READSPAD FAIL!!!" NL ) );
  dump8( buf, 12 );

  int te = buf[0] + (buf[1] << 8);
  pr_sdx( te );
  te *= 1000; te /= 16;
  pr_sdx( te );

  memset( buf, 0, sizeof(buf) );
  ok = wire1.matchRom( addr, OneWire::CMD_READ_SPAD, buf, 9 ); // read buf with addr
  std_out << "MatchROM.READ_SPAD  ok= " << ok <<  NL;
  dump8( buf, 12 );

  memset( buf, 0, sizeof(buf) );
  addr[1] = 0xFF; // bad addr
  ok = wire1.matchRom( addr, OneWire::CMD_READ_SPAD, buf, 9 ); // read buf with addr
  std_out <<  ( ok ? ( "Bad addr: READ_SPAD OK" NL ) : ( "Bad addr: READSPAD FAIL!!!" NL ) );
  dump8( buf, 12 );

  wire1.eot();
  delay_ms( 100 );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

