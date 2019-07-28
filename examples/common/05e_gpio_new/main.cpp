#include <cstring>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const int led_delay_init = 1000000; // in mcs
volatile int led_delay = led_delay_init;
volatile uint32_t last_exti_tick = 0;
const int btn_deadtime = 200;


const char* common_help_string = "Appication to test new GPIO init approach" NL;

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
  static unsigned n = 0;
  GpioD.ODR ^= 1u << 11;
  ++n;
  if( ( n & 0x000F ) == 1 ) {
    GpioD.ODR ^= 1u << 12;
  }
  if( ( n & 0x00FF ) == 1 ) {
    GpioD.ODR ^= 1u << 13;
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


  GpioD.cfgOut_N( BIT11 | BIT12 | BIT13 );

  GpioD.cfgOut( 14, true );
  GpioD.cfgAF(  15, 6, true );
  GpioD.cfg_set_af(  0,  15 );
  GpioD.cfgIn( 1, GpioRegs::Pull::up );

  GpioA.cfgOut( 5, true );
  GpioA.cfgAF(  1, 1 );
  GpioB.cfgOut( 0, true );
  GpioB.cfgAF(  1, 2 );
  GpioC.cfgAF(  0, 7, true );
  // GpioC.cfgIn( 1, GpioRegs::Pull::up );

  uint8_t eln = 13;
  GpioC.cfgIn( eln, GpioRegs::Pull::down  );
  uint32_t tmp = SYSCFG->EXTICR[ eln >> 2 ];
  tmp &= ~( SYSCFG_EXTICR1_EXTI0_Msk << ( SYSCFG_EXTICR1_EXTI1_Pos * ( eln & 0x03 ) ) );
  tmp |=  ( GpioIdx( GpioC )         << ( SYSCFG_EXTICR1_EXTI1_Pos * ( eln & 0x03 ) ) );
  SYSCFG->EXTICR[ eln >> 2 ] = tmp;

  EXTI->IMR  = 1 << eln;
  EXTI->RTSR = 1 << eln;

  GpioA.cfgAnalog( 3 );

  oxc_add_aux_tick_fun( led_task_nortos );

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

  // board_def_btn_init( true );

  dump8( EXTI, 0x20 );
  dump8( SYSCFG, 0x60 );

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
  leds.toggle( BIT0 );
  if( curr_tick - last_exti_tick < btn_deadtime ) {
    return; // ignore too fast events
  }
  last_exti_tick = curr_tick;
  if( pin == BOARD_BTN0_BIT )  {
    leds.toggle( BIT1 );
  } else if( pin == BOARD_BTN1_BIT )  {
    leds.toggle( BIT2 );
  }
  // leds.toggle( BIT0 );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

