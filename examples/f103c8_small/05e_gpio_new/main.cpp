#include <cstring>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

volatile uint32_t last_exti_tick = 0;
const int btn_deadtime = 50;


const char* common_help_string = "Appication to test new GPIO init approach" NL;

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
  static unsigned n = 0;
  GpioB.toggle( 1u << 12 );
  ++n;
  if( ( n & 0x0007 ) == 1 ) {
    GpioB.toggle( 1u << 13 );
  }
  if( ( n & 0x000F ) == 1 ) {
    GpioB.toggle( 1u << 14 );
  }
}


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =  20;

  GpioA.enableClk();
  GpioB.enableClk();
  GpioC.enableClk();
  GpioD.enableClk();
  GpioE.enableClk();

  BOARD_POST_INIT_BLINK;

  GpioB.cfgOut_N( BIT12 | BIT13 | BIT14 | BIT15 );
  // GpioB.cfgOut( 12 );
  GpioB.cfgOut( 8, true );

  GpioB.cfgAnalog( 0 );
  GpioB.cfgIn( 1 );
  GpioB.cfgIn( 2, GpioRegs::Pull::up );

  GpioB.cfgAF( 6, 1 );
  GpioB.cfgAF( 7, 1, true );

  //
  // GpioA.cfgAF(  9, 1 ); // check cfg for UART TX
  // GpioA.cfgIn( 10 );    // check cfg for UART RX
  //
  // // board_def_btn_init( true );
  GpioA.cfgIn( BOARD_BTN0_N, GpioRegs::Pull::down  );
  GpioA.setEXTI( BOARD_BTN0_N, GpioRegs::ExtiEv::up );
  //
  //
  bool needIRQ = true;
  #ifdef BOARD_BTN0_EXIST
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN0_IRQ, BOARD_BTN0_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN0_IRQ );
  }
  #endif

  #ifdef BOARD_BTN1_EXIST
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN1_IRQ, BOARD_BTN1_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN1_IRQ );
  }
  #endif

  oxc_add_aux_tick_fun( led_task_nortos );

  // dump8( EXTI, 0x20 );
  // dump8( SYSCFG, 0x60 );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


#ifdef BOARD_BTN0_EXIST
#define EXTI_BIT0 BOARD_BTN0_BIT
void BOARD_BTN0_IRQHANDLER(void)
{
  HAL_GPIO_EXTI_IRQHandler( BOARD_BTN0_BIT );
}
#else
#define EXTI_BIT0 0
#endif

#ifdef BOARD_BTN1_EXIST
#define EXTI_BIT1 BOARD_BTN1_BIT
void BOARD_BTN1_IRQHANDLER(void)
{
  HAL_GPIO_EXTI_IRQHandler( BOARD_BTN1_BIT );
}
#else
#define EXTI_BIT1 0
#endif

void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  uint32_t curr_tick = HAL_GetTick();
  if( curr_tick - last_exti_tick < btn_deadtime ) {
    return; // ignore too fast events
  }
  last_exti_tick = curr_tick;

  GpioB.toggle( BIT15 );

  // if( pin == BOARD_BTN0_BIT )  {
  //   leds.toggle( BIT1 );
  // } else if( pin == BOARD_BTN1_BIT )  {
  //   leds.toggle( BIT2 );
  // }
  // leds.toggle( BIT0 );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

