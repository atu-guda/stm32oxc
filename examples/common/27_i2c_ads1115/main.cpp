#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_ads1115.h>
using namespace std;
using namespace SMLRL;

using sreal = StatData::sreal;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADS155 ADC I2C device" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test one channel"  };
int cmd_getNch( int argc, const char * const * argv );
CmdInfo CMDINFO_GETNCH { "getNch", 'G', cmd_getNch, " - test n ('c') channel"  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GETNCH,
  &CMDINFO_SET_COEFFS,
  nullptr
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
ADS1115 adc( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100; // 100 ms
  UVAR('n') = 20;
  UVAR('c') = 4;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &adc;

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

  uint16_t x_cfg = adc.getDeviceCfg();
  std_out <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;
  bool is_cont = UVAR('o');

  adc.setDefault();

  uint16_t cfg;
  cfg = ADS1115::cfg_in_0 | ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  // oneShot may be removed by startCont()
  UVAR('e') = adc.setCfg( cfg );
  x_cfg = adc.getDeviceCfg();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  StatData sdat( 1 );

  std_out << "# Coeffs: " << v_coeffs[0] << NL;

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  if( is_cont ) {
    adc.startCont();
  }
  x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  std_out <<  "#  cfg= " <<  HexInt16( x_cfg ) <<  " scale_mv = " << scale_mv << NL;

  int v0 = 0;

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
    if( is_cont ) {
      v0 = adc.getContValue();
    } else {
      v0 = adc.getOneShot();
    }
    if( UVAR('l') ) {  leds.reset( BIT2 ); }
    int dt = tcc - tm00; // ms
    if( do_out ) {
      std_out <<  FltFmt( 0.001f * dt, cvtff_auto, 12, 4 );
    }

    sreal vf0 = 0.001f * scale_mv * v0  * v_coeffs[0] / 32678;

    sdat.add( &vf0 );

    if( do_out ) {
      std_out  << ' '  <<  v0 << ' ' << vf0 << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  if( is_cont ) {
    adc.stopCont();
  }

  sdat.calc();
  std_out << sdat << NL;

  x_cfg = adc.getDeviceCfg();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  return rc;
}

int cmd_getNch( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series
  unsigned n_ch = (uint8_t)clamp( ( UVAR('c') ), 1, 4 );
  uint8_t e_ch = (uint8_t)(n_ch-1);
  uint16_t x_cfg = adc.getDeviceCfg();
  std_out <<  NL "# getNch: n= " <<  n << " n_ch= " << n_ch << " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  StatData sdat( n_ch );

  std_out << "# Coeffs: ";
  for( decltype(n_ch) j=0; j<n_ch; ++j ) {
    std_out << ' ' << v_coeffs[j];
  }

  adc.setDefault();

  uint16_t cfg =  ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  UVAR('e') = adc.setCfg( cfg );
  x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) << " scale_mv = " << scale_mv << NL;

  int16_t vi[4];
  sreal kv = 0.001f * scale_mv / 0x7FFF;

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
    float tc = 0.001f * ( tcc - tm00 );
    sreal v[n_ch];

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    int no = adc.getOneShotNch( 0, e_ch, vi );
    if( UVAR('l') ) {  leds.reset( BIT2 ); }


    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
    }

    for( decltype(no) j=0; j<no; ++j ) {
      v[j] = kv * vi[j] * v_coeffs[j];
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

  return rc;
}

int cmd_set_coeffs( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    v_coeffs[0] = arg2float_d( 1, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[1] = arg2float_d( 2, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[2] = arg2float_d( 3, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[3] = arg2float_d( 4, argc, argv, 1, -1e10f, 1e10f );
  }
  std_out << "# Coefficients: "
     << v_coeffs[0] << ' ' << v_coeffs[1] << ' ' << v_coeffs[2] << ' ' << v_coeffs[3] << NL;
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

