#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_ina226.h>
#include <oxc_statdata.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test INA226 I2C device" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test V_sh, V_bus"  };
int cmd_getVIP( int argc, const char * const * argv );
CmdInfo CMDINFO_GETNCH { "getVIP", 'G', cmd_getVIP, " - get V_bus, I_sh, P"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GETNCH,
  &CMDINFO_SETADDR,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
INA226 ina226( i2cd );

bool isGoodINA226( INA226 &ina, bool print = true );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10;
  UVAR('n') = 20;
  UVAR('c') = 4;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}

bool isGoodINA226( INA226 &ina, bool print )
{
  STDOUT_os;
  uint16_t id_manuf = ina.readReg( INA226::reg_id_manuf );
  uint16_t id_dev   = ina.readReg( INA226::reg_id_dev );
  if( print ) {
    os << "# id_manuf= " << HexInt16( id_manuf ) << "  id_dev= " << HexInt16( id_dev ) << NL;
  }
  if( id_manuf != INA226::id_manuf || id_dev != INA226::id_dev ) {
    if( print ) {
      os << "# Error: bad ids!" << NL;
    }
    return false;
  }
  return true;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  int t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  ina226.setCfg( INA226::cfg_rst );
  uint16_t x_cfg = ina226.getCfg();
  os <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  if( ! isGoodINA226( ina226, true ) ) {
    return 3;
  }

  uint16_t cfg = INA226::cfg_default;
  UVAR('e') = ina226.setCfg( cfg );
  x_cfg = ina226.getCfg();
  os <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;
  StatData sdat( 2 );

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');

  break_flag = 0;
  for( unsigned i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    int16_t v_sh_raw  = ina226.getVsh();
    int16_t v_bus_raw = ina226.getVbus();
    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    double v[2];
    v[0] = INA226::lsb_V_sh_nv  * v_sh_raw  * 1e-9;
    v[1] = INA226::lsb_V_bus_nv * v_bus_raw * 1e-9;

    int dt = tcc - tm00; // ms
    if( do_out ) {
      os <<  FloatFmt( 0.001 * dt, "%-10.4f "  );
    }

    sdat.add( v );

    if( do_out ) {
      os  << ' '  <<  FloatFmt( v[0], "%#12.6g" ) <<  ' ' <<  FloatFmt( v[1], "%#12.6g" )<<  NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  os << sdat << NL;

  return rc;
}

int cmd_getVIP( int argc, const char * const * argv )
{
  // int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  // uint32_t t_step = UVAR('t');

  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    STDOUT_os;
    os <<  "Need addr [1-127]" NL;
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  ina226.setAddr( addr );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

