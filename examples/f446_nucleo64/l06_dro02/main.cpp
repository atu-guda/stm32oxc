#include <cstring>
#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include "dro02.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure drone motor params" NL;

PinOut hx711_sck( GpioC, 10 );
PinsIn hx711_dat( GpioC, 11, 1 );
inline void delay_hx711() { delay_bad_mcs( 2 ); }; // TODO: try 1
int32_t hx711_read();

// UsartIO fc_io( &huart1, huart1.Instance );

int motor_pwr( int v );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_test_fc( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST_FC { "test_fc", 'F', cmd_test_fc, " - test FC interaction"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TEST_FC,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 500;
  UVAR('n') =  20;
  UVAR('s') = 5194; // scale, * 1e-10
  UVAR('o') = 0; // offset, g

  hx711_sck.initHW();
  hx711_dat.initHW();

  MX_TIM2_Init();
  MX_FC_UART_Init();
  HAL_UART_Transmit( &huart1, (const uint8_t*)"#\n", 2, 1000 );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;


  float scale = - UVAR('s') * 1e-10f;
  float shift = UVAR('o') * 1e-3f;
  int32_t sum_i { 0 };

  uint32_t tm0 = HAL_GetTick();

  uint32_t tc0 = tm0; //, tc00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    int32_t v_f = hx711_read();
    float v = v_f * scale + shift;
    float f = ( tim2_catch > 0 ) ? ( 1e6f / tim2_catch ) : 0.0f;
    sum_i += v_f;
    std_out <<  "i= " << i // << "  tick= " << ( tcc - tc00 )
            << " v_f= " <<  HexInt( v_f ) << ' ' << v_f << ' ' << v
            << " cap= " << tim2_catch << " f= " << f
            << NL;
    tim2_catch = 0;

    delay_ms_until_brk( &tc0, t_step );
  }
  std_out << "# av: " << float(sum_i) / n << NL;

  return 0;
}

int32_t hx711_read()
{
  uint32_t cnt { 0 };
  bool good { false };
  hx711_sck.reset();

  for( unsigned i=0; i<100000; ++i ) {
    uint16_t t = hx711_dat.read();
    if( !t ) {
      good = true;
      break;
    }
  }
  if( !good ) { return 0x80000000; };

  for( uint8_t i=0; i<24; ++i ) {
    hx711_sck.set();
    delay_hx711();
    uint16_t v = hx711_dat.read();
    cnt <<= 1;
    if( v ) {
      cnt |= 1;
    }
    hx711_sck.reset();
    delay_hx711();
  }

  // TODO: param
  hx711_sck.set();
  delay_hx711();
  hx711_sck.reset();
  delay_hx711();

  if( cnt & 0x00800000 ) {
    cnt |= 0xFF000000;
  }

  return (int32_t)(cnt);
}

int motor_pwr( int v )
{
  char sbuf[64];
  char rbuf[128];

  strcpy( sbuf, "motor 0 " );
  char xbuf[16]; i2dec( v+1000, xbuf );
  strcat( sbuf, xbuf );
  strcat( sbuf, "\n" );
  HAL_UART_Transmit( &huart1, (const uint8_t*)sbuf, strlen(sbuf), 1000 );
  HAL_UART_Receive( &huart1, (uint8_t*)rbuf, sizeof(rbuf), 100 );
  return 0;
}

int cmd_test_fc( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 10, 0, 1000 );

  motor_pwr( v );

  delay_ms( 2000 );

  motor_pwr( 0 );

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

