#include <oxc_auto.h>

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

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =  20;

  hx711_sck.initHW();
  hx711_dat.initHW();

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

  uint32_t tm0 = HAL_GetTick();

  uint32_t tc0 = tm0, tc00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcc = HAL_GetTick();
    // hx711_sck.toggle();
    // uint16_t h_d = hx711_dat.read();
    int32_t v_f = hx711_read();
    std_out <<  "i= " << i // << "  tick= " << ( tcc - tc00 )
            << " v_f= " <<  HexInt( v_f ) << ' ' << v_f << ' ' << NL;
    leds.toggle( 4 );

    delay_ms_until_brk( &tc0, t_step );
  }

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

  hx711_sck.set();
  delay_hx711();
  hx711_sck.reset();
  delay_hx711();

  return (int32_t)(cnt);
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

