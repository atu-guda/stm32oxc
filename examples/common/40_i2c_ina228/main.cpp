#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_ina228.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test INA228 I2C device" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test V_sh, V_bus"  };
int cmd_getVIP( int argc, const char * const * argv );
CmdInfo CMDINFO_GETVIP { "getVIP", 'G', cmd_getVIP, " - get V_bus, I_sh, P"  };
int cmd_setcalibr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETCALIBR { "set_calibr", 'K', cmd_setcalibr, " I_lsb R_sh - calibrate for given shunt"  };
int cmd_calc_acfg( int argc, const char * const * argv );
CmdInfo CMDINFO_CALC_ACFG { "calc_acfg", 'A', cmd_calc_acfg, "ct_b ct_s ct_t avg - calc ADCconfig val ->'a' "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GETVIP,
  &CMDINFO_SETCALIBR,
  &CMDINFO_CALC_ACFG,
  nullptr
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
INA228 ina228( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
xfloat v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100; // 100 ms
  UVAR('n') = 20;
  UVAR('c') = 4;
  UVAR('g') = INA228::cfg_default;
  UVAR('a') = INA228::acfg_mode_def; // 0xFB68; // default
  UVAR('p') = 0x40; // default addr for debug
  UVAR('s') = 0;    // scale 0 - TODO

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ina228;

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
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  ina228.setCfg( INA228::cfg_rst );
  uint16_t x_cfg = ina228.getCfg();
  std_out <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  if( uint32_t id = ina228.isBad() ) {
    std_out << "# Bad INA228 id: " << HexInt(id) << NL;
    return 3;
  }

  uint16_t cfg = UVAR('g');
  UVAR('e') = ina228.setCfg( cfg );
  ina228.setAdcCfg( UVAR('a') );
  x_cfg = ina228.getCfg();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) << ' ' << HexInt16( ina228.getAdcCfg() ) << NL;
  const xfloat k_i = ( x_cfg & INA228::cfg_adcrange ) ? 78.125e-9f : 312.5e-9f;

  const unsigned n_ch { 4 };
  StatData sdat( n_ch );

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');
  delay_ms( t_step );

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    int wrc = ina228.waitEOC();
    auto[v_sh_raw,v_bus_raw]  = ina228.getVV();
    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    xfloat v[n_ch];
    v[0] = v_sh_raw  * k_i;
    v[1] = v_bus_raw * 195.3124e-6f;
    v[2] = v_sh_raw;
    v[3] = v_bus_raw;

    int dt = tcc - tm00; // ms
    if( do_out ) {
      std_out <<  FltFmt(   0.001f * dt, cvtff_auto, 12, 4 );
    }

    sdat.add( v );

    if( do_out ) {
      std_out  << ' '  <<  v[0] <<  ' ' <<  v[1]
          << ' ' << HexInt(v_sh_raw) << ' ' << HexInt(v_bus_raw)
          << ' ' << HexInt16( ina228.getLastDiag() ) << ' ' << wrc
          << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;
  auto T_raw = ina228.getT();
  auto I_raw = ina228.getI();
  std_out << "# T_raw= " << T_raw << ' ' << (7.8125e-3f*T_raw) << " I_raw= " << I_raw << NL;

  return rc;
}

int cmd_getVIP( int argc, const char * const * argv )
{
  // uint32_t t_step = UVAR('t');
  // uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series
  // unsigned n_ch = 4;
  //
  // StatData sdat( n_ch );
  //
  // ina228.setCfg( INA228::cfg_rst );
  // uint16_t x_cfg = ina228.getCfg();
  // std_out <<  NL "# getVIP: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;
  //
  //
  // uint16_t cfg = INA228::cfg_default;
  // UVAR('e') = ina228.setCfg( cfg );
  // x_cfg = ina228.getCfg();
  // ina228.calibrate();
  // std_out <<  "# cfg= " << HexInt16( x_cfg ) <<  " I_lsb_mA= " << ina228.get_I_lsb_mA()
  //    << " R_sh_uOhm= " << ina228.get_R_sh_uOhm() << NL;
  //
  // std_out << "# Coeffs: ";
  // for( decltype(n_ch) j=0; j<n_ch; ++j ) {
  //   std_out << ' ' << v_coeffs[j];
  // }
  // std_out << NL;
  //
  // leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  // leds.reset( BIT0 | BIT1 | BIT2 );
  //
  // uint32_t tm0, tm00;
  // int rc = 0;
  // bool do_out = ! UVAR('b');
  //
  // break_flag = 0;
  // for( decltype(n) i=0; i<n && !break_flag; ++i ) {
  //
  //   uint32_t tcc = HAL_GetTick();
  //   if( i == 0 ) {
  //     tm0 = tcc; tm00 = tm0;
  //   }
  //
  //   xfloat tc = 0.001f * ( tcc - tm00 );
  //   xfloat v[n_ch];
  //
  //   if( UVAR('l') ) {  leds.set( BIT2 ); }
  //
  //   v[0] = ina228.getVbus_uV()  * (xfloat)1e-6f * v_coeffs[0];
  //   v[1] = ina228.getI_mA_reg() * (xfloat)1e-3f * v_coeffs[1];
  //   v[2] = ina228.getI_uA()     * (xfloat)1e-6f * v_coeffs[2];
  //   v[3] = ina228.getP()                        * v_coeffs[3];
  //
  //   if( UVAR('l') ) {  leds.reset( BIT2 ); }
  //
  //   if( do_out ) {
  //     std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
  //   }
  //
  //   sdat.add( v );
  //
  //   if( do_out ) {
  //     for( auto vc : v ) {
  //       std_out  << ' '  <<  vc;
  //     }
  //     std_out << NL;
  //   }
  //
  //   delay_ms_until_brk( &tm0, t_step );
  // }
  //
  // sdat.calc();
  // std_out << sdat << NL;
  //
  // delay_ms( 10 );

  return 0;
  //return rc;
}


int cmd_setcalibr( int argc, const char * const * argv )
{
  // float calibr_I_lsb = arg2float_d( 1, argc, argv, ina228.get_I_lsb_mA()  * 1e-3f, 1e-20f, 1e10f );
  // float calibr_R     = arg2float_d( 2, argc, argv, ina228.get_R_sh_uOhm() * 1e-6f, 1e-20f, 1e10f );
  // float V_sh_max =  (int)(INA228::lsb_V_sh_nv) * 1e-9f * 0x7FFF;
  // ina228.set_calibr_val( (uint32_t)(calibr_R * 1e6f), (uint32_t)(calibr_I_lsb * 1e3f) );
  // std_out << "# calibr_I_lsb= " << calibr_I_lsb << " calibr_R= " << calibr_R
  //    << " V_sh_max=  " << V_sh_max
  //    << " I_max= " << ( V_sh_max / calibr_R ) << " / " << ( calibr_I_lsb * 0x7FFF ) << NL;
  return 0;
}

int cmd_calc_acfg( int argc, const char * const * argv )
{
  auto md   = (uint8_t)arg2long_d( 1, argc, argv, 0x0B, 0, 0x0F );
  auto ct_b = (uint8_t)arg2long_d( 2, argc, argv,    5, 0, 7 );
  auto ct_s = (uint8_t)arg2long_d( 3, argc, argv,    5, 0, 7 );
  auto ct_t = (uint8_t)arg2long_d( 4, argc, argv,    5, 0, 7 );
  auto avg  = (uint8_t)arg2long_d( 5, argc, argv,    0, 0, 7 );
  UVAR('a') =  INA228::calc_acfg( md, ct_b, ct_s, ct_t, avg );
  std_out << "# acfg= " << HexInt16( UVAR('a') ) << NL;
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

