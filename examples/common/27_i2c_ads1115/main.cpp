#include <cstring>
#include <cstdlib>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_ads1115.h>

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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  uint16_t x_cfg = adc.getDeviceCfg();
  STDOUT_os;
  os <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;
  bool is_cont = UVAR('o');

  adc.setDefault();

  uint16_t cfg;
  cfg = ADS1115::cfg_in_0 | ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  // oneShot may be removed by startCont()
  UVAR('e') = adc.setCfg( cfg );
  x_cfg = adc.getDeviceCfg();
  os <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  if( is_cont ) {
    adc.startCont();
  }
  x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  os <<  "#  cfg= " <<  HexInt16( x_cfg ) <<  " scale_mv = " << scale_mv << NL;

  int v0 = 0;

  uint32_t tm0 = HAL_GetTick();
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    if( is_cont ) {
      v0 = adc.getContValue();
    } else {
      v0 = adc.getOneShot();
    }

    float vf0 = 0.001f * scale_mv * v0 / 32678;
    os <<  "[" <<  i <<  "]  " <<  v0 <<  "  vf0= " <<  vf0 <<  NL;
    os.flush();

    delay_ms_until_brk( &tm0, t_step );
  }

  if( is_cont ) {
    adc.stopCont();
  }
  x_cfg = adc.getDeviceCfg();
  os <<  "# cfg= " << HexInt16( x_cfg ) <<  NL;

  return 0;
}

int cmd_getNch( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint8_t e_ch = (uint8_t)clamp( ( UVAR('c') - 1 ), 0, 3 );
  uint32_t t_step = UVAR('t');
  uint16_t x_cfg = adc.getDeviceCfg();
  STDOUT_os;
  os <<  NL "# getNch: n= " <<  n << " e_ch= " << e_ch << " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  adc.setDefault();

  uint16_t cfg;
  cfg =  ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  UVAR('e') = adc.setCfg( cfg );
  x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  os <<  "# cfg= " << HexInt16( x_cfg ) << " scale_mv = " << scale_mv << NL;

  int16_t v[4];
  float vf[4];

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t tmc = HAL_GetTick();
    float tc = 0.001f * ( tmc - tm00 );

    int no = adc.getOneShotNch( 0, e_ch, v );
    os <<  tc << ' ';
    for( int j=0; j<no; ++j ) {
      vf[j] = 0.001f * scale_mv * v[j] / 32678;
      if( UVAR('d') ) {
        os <<  v[j] <<  ' ' <<  vf[j] <<  ' ';
      } else {
        os << vf[j] << ' ';
      }
    }
    if( UVAR('d') ) {
      os << no;
    }
    os << NL;

    delay_ms_until_brk( &tm0, t_step );
  }

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
  adc.setAddr( addr );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

