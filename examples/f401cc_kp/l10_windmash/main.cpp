#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

PinsOut ledsx( GpioB, 12, 4 );

const char* common_help_string = "Widing machine control app" NL;

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
  STD_PROLOG_USBCDC;

  UVAR('t') = 100;
  UVAR('n') =  20;

  ledsx.initHW();
  ledsx.reset( 0xFF );

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

  // log_add( "Test0 " );
  uint8_t l_v = 0;
  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;

  uint32_t tmc_prev = tc0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcc = HAL_GetTick();
    uint32_t tmc = tcc;
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 )
            << "  ms_tick= " << ( tmc - tm0 ) << " dlt= " << ( tmc - tmc_prev ) << NL;
    if( UVAR('w') ) {
      std_out.flush();
    }
    ++l_v;
    l_v &= 0x0F;
    ledsx.write( l_v );
    leds.toggle( 1 );
    tmc_prev = tcc;

    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

