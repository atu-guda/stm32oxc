#include <oxc_auto.h>
#include <oxc_main.h>

#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test console with buttons IRQ" NL;


void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 500;
  UVAR_n =  20;

  init_btns();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

