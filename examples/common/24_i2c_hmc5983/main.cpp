#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_hmc5983.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test HMC5983 compas" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr (see 'C')"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SETADDR,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HMC5983 mag( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 500;
  UVAR('n') = 50;

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
  constexpr auto n_scales = HMC5983::getNScales();
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int scale = arg2long_d( 2, argc, argv, 1, 0, n_scales-1 );
  uint32_t t_step = UVAR('t');

  STDOUT_os;
  os <<  NL "Test0: n= "  <<  n  <<  " t= "  <<  t_step
     <<  " scale= "  <<  scale   <<  NL;

  int scale_min = -1000000; // INT16_MIN
  int scale_max =  1000000; // INT16_MAX

  BarHText bar_x( 1, 1, 100, scale_min, scale_max );
  BarHText bar_y( 1, 2, 100, scale_min, scale_max );
  BarHText bar_z( 1, 3, 100, scale_min, scale_max );

  // mag.resetDev();

  if( ! mag.init(  HMC5983::cra_odr_75_Hz, HMC5983::Scales( scale ) ) ) {
    os <<  "Fail to init HMC5983, Error= " <<  mag.getErr() << NL;
    return 1;
  }

  int16_t temp;
  const int32_t *xyz;

  term_clear();

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    mag.read1( 10 );
    xyz = mag.getMagAllmcGa();
    temp = mag.getTemp();
    uint32_t tcc = HAL_GetTick();
    bar_x.draw( xyz[0] );
    bar_y.draw( xyz[1] );
    bar_z.draw( xyz[2] );
    term_set_xy( 10, 5 );
    os <<  "i= "  <<  i  <<  "  tick: " <<  (tcc - tm00 )
       <<  " [ "  <<  xyz[0] <<  " ; "  <<  xyz[1]  <<  " ; "  <<  xyz[2]
       <<  " ] T= " /* NL  */  <<  temp  <<  " " NL;

    os.flush();
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
  mag.setAddr( addr );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

