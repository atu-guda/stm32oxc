#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

const char* common_help_string = "Appication to test common console via UART" NL;

// --- local commands;


void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  STD_PROLOG_UART;

  UVAR_t = 100;
  UVAR_n =  20;

  BOARD_POST_INIT_BLINK;


  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

