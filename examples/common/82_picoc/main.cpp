#include <oxc_auto.h>
#include <picoc.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test picoc interpratator" NL;

#define PICOC_STACK_SIZE (32*1024)
int picoc_cmdline_handler( char *s );
Picoc pc;

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
  leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =  20;

  cmdline_handlers[0] = picoc_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  PicocInitialise( &pc, PICOC_STACK_SIZE );
  PicocIncludeAllSystemHeaders( &pc );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int a = arg2long_d( 1, argc, argv,   2,    0, 127 );
  std_out << "Test0: a= " << a <<  NL;

  return 0;
}

int picoc_cmdline_handler( char *s )
{
  // static int nnn = 0;

  if( !s  ||  s[0] != ';' ) { // not my
    return -1;
  }

  const char *cmd = s + 1;
  std_out << NL "# C: cmd= \"" << cmd << '"' << NL;
  delay_ms( 10 );

  int rc = 0;

  return rc;

}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

