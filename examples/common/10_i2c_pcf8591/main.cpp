#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_pcf8591.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to control PCF8591 - ADC (4ch) + DAC (1ch)" NL;

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
PCF8591 adc( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

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
  STDOUT_os;
  os << NL "Test0: n= " << n << " t= " << t_step << NL; os.flush();

  int v_end = UVAR('e');

  adc.setMode( PCF8591::autoinc | PCF8591::mode_4in | PCF8591::out_en );

  constexpr const int n_ch = 4;
  uint8_t d_in[n_ch] = { 0, 0, 0, 0 };

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    adc.getIn( d_in, n_ch );
    os << "[" << i << "]  ";
    for( int j=0; j<n_ch; ++j ) {
      os << d_in[j] << ' ';
    }
    adc.setOut( i & 0xFF );

    os << NL; os.flush();
    delay_ms_until_brk( &tm0, t_step );
  }
  adc.setOut( v_end );

  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc < 2 ) {
    os<< "Need addr [1-127]" NL;
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  adc.setAddr( addr );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

