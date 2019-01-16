#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_ads1115.h>
#include <oxc_statdata.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADS155 ADC I2C device" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test one channel"  };
int cmd_getNch( int argc, const char * const * argv );
CmdInfo CMDINFO_GETNCH { "getNch", 'G', cmd_getNch, " - test n ('c') channel"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr (see 'C')"  };

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
ADS1115 adc( i2cd );
I2CClient *i2c_client_def = &adc;


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
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


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  uint16_t x_cfg = adc.getDeviceCfg();
  os <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;
  bool is_cont = UVAR('o');

  adc.setDefault();

  uint16_t cfg;
  cfg = ADS1115::cfg_in_0 | ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  // oneShot may be removed by startCont()
  UVAR('e') = adc.setCfg( cfg );
  x_cfg = adc.getDeviceCfg();
  os <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  StatData sdat( 1 );

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  if( is_cont ) {
    adc.startCont();
  }
  x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  os <<  "#  cfg= " <<  HexInt16( x_cfg ) <<  " scale_mv = " << scale_mv << NL;

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
      os <<  FloatFmt( 0.001 * dt, "%-10.4f "  );
    }

    double vf0 = 0.001f * scale_mv * v0 / 32678;

    sdat.add( &vf0 );

    if( do_out ) {
      os  << ' '  <<  v0 << ' ' << FloatFmt( vf0, "%#12.6g" ) << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  if( is_cont ) {
    adc.stopCont();
  }

  sdat.calc();
  os << sdat << NL;

  x_cfg = adc.getDeviceCfg();
  os <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  return rc;
}

int cmd_getNch( int argc, const char * const * argv )
{
  STDOUT_os;
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series
  unsigned n_ch = (uint8_t)clamp( ( UVAR('c') ), 1, 4 );
  uint8_t e_ch = (uint8_t)(n_ch-1);
  uint16_t x_cfg = adc.getDeviceCfg();
  os <<  NL "# getNch: n= " <<  n << " n_ch= " << n_ch << " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  StatData sdat( n_ch );

  adc.setDefault();

  uint16_t cfg =  ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  UVAR('e') = adc.setCfg( cfg );
  x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  os <<  "# cfg= " << HexInt16( x_cfg ) << " scale_mv = " << scale_mv << NL;

  int16_t v[4];
  double vf[4];
  double kv = 0.001 * scale_mv / 0x7FFF;

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

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    int no = adc.getOneShotNch( 0, e_ch, v );
    if( UVAR('l') ) {  leds.reset( BIT2 ); }


    if( do_out ) {
      os <<  FloatFmt( tc, "%-10.4f "  );
    }

    for( decltype(no) j=0; j<no; ++j ) {
      vf[j] = kv * v[j];
      os << vf[j] << ' ';
    }

    sdat.add( vf );
    os << NL;

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  os << sdat << NL;

  return rc;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc < 2 ) {
    os <<  "Need addr [1-127]" NL;
    return 1;
  }
  if( !i2c_client_def ) {
    os << "# Error: I2C default client is not set!" << NL;
    return 2;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  i2c_client_def->setAddr( addr );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

