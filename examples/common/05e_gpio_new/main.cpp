#include <cstring>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

#if defined(USE_OXC_CONSOLE_UART)
  const char* common_help_string = "Appication to test common console with auto I/O selection: UART" NL;
#elif defined(USE_OXC_CONSOLE_USB_CDC)
  const char* common_help_string = "Appication to test common console with auto I/O selection: USB_CDC" NL;
#else
  const char* common_help_string = "Appication to test common console with auto I/O selection: Unknown" NL;
#endif

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_test_rate( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST_RATE { "test_rate", 0, cmd_test_rate, "[ n [len] ] - test output rate"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TEST_RATE,
  nullptr
};

void idle_main_task()
{
   GpioD.ODR ^= 7u << 11;
   UVAR('b') = GpioD.ODR;
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =  20;

  BOARD_POST_INIT_BLINK;

  UVAR('a')  = GpioD.MODER;

  GpioD.cfgOutN( BIT11 | BIT12 | BIT13 );
  // GpioD.cfgOutPP( 11 );
  // GpioD.cfgOutPP( 12 );
  // GpioD.cfgOutPP( 13 );

  GpioD.cfgOut( 14, true );

  // uint8_t pin_num = 11; // D11
  // uint32_t t = GpioD.MODER;
  // t &= ~( 3u << ( 2 * pin_num ) );
  // t |=  ( 1u << ( 2 * pin_num ) );
  // GpioD.MODER = t;
  // GpioD.OTYPER  &= ~( 1u << pin_num );
  // GpioD.OSPEEDR |=  ( 3u << ( pin_num  * 2 ) );
  // GpioD.PUPDR   &= ~( 3u << ( 2 * pin_num ) );
  // uint8_t idx = pin_num >> 4;
  //
  // GpioD.AFR[idx] &= ~( 0x0F << ( pin_num * 4 ) );



  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

