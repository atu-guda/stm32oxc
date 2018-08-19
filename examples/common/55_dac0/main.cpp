#define _GNU_SOURCE
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use DAC in manual mode" NL;


extern DAC_HandleTypeDef hdac;
int MX_DAC_Init();
const int dacbuf_sz = 64;
int16_t dacbuf[dacbuf_sz];

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_ofast( int argc, const char * const * argv );
CmdInfo CMDINFO_OFAST { "ofast", 'F', cmd_ofast, " - fast meandre "  };
int cmd_fun( int argc, const char * const * argv );
CmdInfo CMDINFO_FUN { "fun", 'U', cmd_fun, " N type delay - funcs outout "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OFAST,
  &CMDINFO_FUN,
  nullptr
};




int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;

  MX_DAC_Init();

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
  uint32_t v1 = arg2long_d( 1, argc, argv, UVAR('v'), 0 );
  uint32_t v2 = v1;
  int n = arg2long_d( 2, argc, argv, UVAR('n'), 0 );
  STDOUT_os;
  os << NL "Test0: n= " <<  n <<  " v1= " <<  v1 <<  " v2= " <<  v2 <<  NL;

  auto r1 = HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, v1 );
  auto r2 = HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, v2 );

  os <<  "r1= " <<  r1 <<  "  r2= " <<  r2 <<  NL;
  HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_2 );
  uint32_t vv = 3250 * v1 / 4096;
  os << " vv= " << vv << NL;

  return 0;
}

int cmd_ofast( int argc, const char * const * argv )
{
  int n   = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int dly = arg2long_d( 2, argc, argv, 0, 0, 1000 );
  STDOUT_os;
  os <<  "ofast: n= " <<  n <<  " dly= " <<  dly <<  NL;

  for( int i=0; i<n; ++i ) {
    HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (i&1)*4095 );
    HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
    if( dly ) {
      delay_bad_mcs( dly );
    }
  }

  return 0;
}

int cmd_fun( int argc, const char * const * argv )
{
  int n   = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int tp  = arg2long_d( 2, argc, argv, 0, 0, 5 );
  int dly = arg2long_d( 3, argc, argv, 0, 0, 1000 );
  STDOUT_os;
  os <<  "funcs: n= " <<  n <<  " tp= " <<  tp <<  " dly= " <<  dly <<  NL;

  switch( tp ) {
    case 0:
      for( int i=0; i<dacbuf_sz; ++i ) {
        float vf = sinf( 2 * M_PI * (float)(i)/dacbuf_sz );
        dacbuf[i] = 2048 + int16_t( vf * 2047 );
      }
      break;
    case 1:
      for( int i=0; i<dacbuf_sz; ++i ) {
        float x = (float)(i)/dacbuf_sz-0.5;
        dacbuf[i] = int16_t( x * x * 4 * 4095 );
      }
      break;
    case 2:
      for( int i=0; i<dacbuf_sz; ++i ) {
        dacbuf[i] = ( i & 0xFFF0 ) * 4095 / dacbuf_sz;
      }
      break;
    default:
      for( int i=0; i<dacbuf_sz; ++i ) {
        dacbuf[i] = i * 4095 / dacbuf_sz;
      }
      break;
  }

  for( int i=0; i<n && !break_flag; ++i ) {
    HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dacbuf[i%dacbuf_sz] );
    HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
    if( dly ) {
      delay_bad_mcs( dly );
    }
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

