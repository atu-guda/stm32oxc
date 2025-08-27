#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>

#if MC_FLASH_SIZE > (1024*64)
  #define USE_STATDATA 1
  #include <oxc_statdata.h>
#endif

#include <oxc_ina226.h>
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
CmdInfo CMDINFO_GETVIP { "getVIP", 'G', cmd_getVIP, " - get V_bus, I_sh, P"  };
int cmd_setcalibr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETCALIBR { "set_calibr", 'K', cmd_setcalibr, " I_lsb R_sh - calibrate for given shunt"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GETVIP,
  &CMDINFO_SETCALIBR,
  nullptr
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
INA226 ina226( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
xfloat v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };

bool isGoodINA226( INA226 &ina, bool print = true );



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10; // 10 ms
  UVAR('n') = 20;
  UVAR('c') = 4;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ina226;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}

bool isGoodINA226( INA226 &ina, bool print )
{
  uint16_t id_manuf = ina.readReg( INA226::reg_id_manuf );
  uint16_t id_dev   = ina.readReg( INA226::reg_id_dev );
  if( print ) {
    std_out << "# id_manuf= " << HexInt16( id_manuf ) << "  id_dev= " << HexInt16( id_dev ) << NL;
  }
  if( id_manuf != INA226::id_manuf || id_dev != INA226::id_dev ) {
    if( print ) {
      std_out << "# Error: bad ids!" << NL;
    }
    return false;
  }
  return true;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  ina226.setCfg( INA226::cfg_rst );
  uint16_t x_cfg = ina226.getCfg();
  std_out <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  if( ! isGoodINA226( ina226, true ) ) {
    return 3;
  }

  uint16_t cfg = INA226::cfg_default;
  UVAR('e') = ina226.setCfg( cfg );
  x_cfg = ina226.getCfg();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  #ifdef USE_STATDATA
    StatData sdat( 2 );
  #endif

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    int16_t v_sh_raw  = ina226.getVsh();
    int16_t v_bus_raw = ina226.getVbus();
    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    xfloat v[2];
    v[0] = INA226::lsb_V_sh_nv  * v_sh_raw  * 1e-9f;
    v[1] = INA226::lsb_V_bus_uv * v_bus_raw * 1e-6f;

    int dt = tcc - tm00; // ms
    if( do_out ) {
      std_out <<  FltFmt(   0.001f * dt, cvtff_auto, 12, 4 );
    }

    #ifdef USE_STATDATA
      sdat.add( v );
    #endif

    if( do_out ) {
      std_out  << ' '  <<  v[0] <<  ' ' <<  v[1] << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  #ifdef USE_STATDATA
    sdat.calc();
    std_out << sdat << NL;
  #endif

  return rc;
}

#ifdef USE_STATDATA
int cmd_getVIP( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series
  unsigned n_ch = 4;

  StatData sdat( n_ch );

  ina226.setCfg( INA226::cfg_rst );
  uint16_t x_cfg = ina226.getCfg();
  std_out <<  NL "# getVIP: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  if( ! isGoodINA226( ina226, true ) ) {
    return 3;
  }

  uint16_t cfg = INA226::cfg_default;
  UVAR('e') = ina226.setCfg( cfg );
  x_cfg = ina226.getCfg();
  ina226.calibrate();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) <<  " I_lsb_mA= " << ina226.get_I_lsb_mA()
     << " R_sh_uOhm= " << ina226.get_R_sh_uOhm() << NL;

  std_out << "# Coeffs: ";
  for( decltype(n_ch) j=0; j<n_ch; ++j ) {
    std_out << ' ' << v_coeffs[j];
  }
  std_out << NL;

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    xfloat tc = 0.001f * ( tcc - tm00 );
    xfloat v[n_ch];

    // if( UVAR('l') ) {  leds.set( BIT2 ); }

    v[0] = ina226.getVbus_uV()  * (xfloat)1e-6f * v_coeffs[0];
    v[1] = ina226.getI_mA_reg() * (xfloat)1e-3f * v_coeffs[1];
    v[2] = ina226.getI_uA()     * (xfloat)1e-6f * v_coeffs[2];
    v[3] = ina226.getP()                        * v_coeffs[3];

    // if( UVAR('l') ) {  leds.reset( BIT2 ); }

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
    }

    sdat.add( v );

    if( do_out ) {
      for( auto vc : v ) {
        std_out  << ' '  <<  vc;
      }
      std_out << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;

  delay_ms( 10 );

  return rc;
}
#else
int cmd_getVIP( int argc, const char * const * argv )
{
  return 1; // just not enough flash
}
#endif


int cmd_setcalibr( int argc, const char * const * argv )
{
  float calibr_I_lsb = arg2float_d( 1, argc, argv, ina226.get_I_lsb_mA()  * 1e-3f, 1e-20f, 1e10f );
  float calibr_R     = arg2float_d( 2, argc, argv, ina226.get_R_sh_uOhm() * 1e-6f, 1e-20f, 1e10f );
  float V_sh_max =  (int)(INA226::lsb_V_sh_nv) * 1e-9f * 0x7FFF;
  ina226.set_calibr_val( (uint32_t)(calibr_R * 1e6f), (uint32_t)(calibr_I_lsb * 1e3f) );
  std_out << "# calibr_I_lsb= " << calibr_I_lsb << " calibr_R= " << calibr_R
     << " V_sh_max=  " << V_sh_max
     << " I_max= " << ( V_sh_max / calibr_R ) << " / " << ( calibr_I_lsb * 0x7FFF ) << NL;
  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

