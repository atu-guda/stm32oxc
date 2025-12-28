#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_mcp4725.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MCP4725 DAC" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " [v] - test DAC output (12bit)"  );
DCL_CMD_REG( outV, 'V', " [v] - output in mV"  );
DCL_CMD_REG( outEEPROM, 'E', " [v] - output in mV and store to EEPROM"  );
DCL_CMD_REG( sleep1, 'S', " - sleep with 1k gound"  );



I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
MCP4725 dac( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t =  100;
  UVAR_n =  100;
  UVAR_v = 3300; // mV

  UVAR_e = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &dac;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  delay_ms( 50 );

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 0x0800, 0, 0x0FFF );

  std_out << NL "# Test0: v= " << v << NL;
  std_out.flush();

  dac.setOutFast( v );

  return 0;
}

int cmd_outV( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 1000, 0  );
  int va = v * 0xFFF / UVAR_v;

  std_out << NL "# Out: v= " << v << " va= " << va << NL;
  std_out.flush();

  dac.setOutFast( va );

  return 0;
}

int cmd_outEEPROM( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 1000, 0  );
  int va = v * 0xFFF / UVAR_v;

  std_out << NL "# Out: v= " << v << " va= " << va << NL;
  std_out.flush();

  dac.setEEPROM( va );

  return 0;
}


int cmd_sleep1( int argc, const char * const * argv )
{
  std_out << NL "# sleep1: " << NL;
  std_out.flush();

  dac.sleep1k();

  return 0;
}

int cmd_rect( int argc, const char * const * argv )
{
  int v0 = arg2long_d( 1, argc, argv,  500, 0  );
  int v1 = arg2long_d( 2, argc, argv, 2500, 0  );
  int va0 = v0 * 0xFFF / UVAR_v;
  int va1 = v1 * 0xFFF / UVAR_v;

  int n = UVAR_n;
  int t_step = UVAR_t;

  std_out << NL "# Rect: v0= " << v0 << " v1= " << v1 << NL;


  uint32_t tm0 = HAL_GetTick();
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    dac.setOutFast( (i&1) ? va1 : va0 );
    if( t_step > 0 ) {
      delay_ms_until_brk( &tm0, t_step );
    }
  }

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

