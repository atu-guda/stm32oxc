#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

// handle H5 2 different callbacks
#ifdef EXTI_RPR1_RPIF0_Msk
  #if BOARD_BTN0_ACTIVE_DOWN == 0
    #define EXTI_CALLBACK_FUN HAL_GPIO_EXTI_Rising_Callback
  #else
    #define EXTI_CALLBACK_FUN HAL_GPIO_EXTI_Falling_Callback
  #endif
#else
  #define EXTI_CALLBACK_FUN HAL_GPIO_EXTI_Callback
#endif

void MX_GPIO_Init(void);


extern "C" {
void task_leds( void *prm UNUSED_ARG );
} // extern "C"

const int led_delay_init = 1000000; // in mcs
volatile int led_delay = led_delay_init;
volatile uint32_t last_exti_tick = 0;
const int btn_deadtime = 200;

int main(void)
{
  STD_PROLOG_START;

  MX_GPIO_Init();

  BOARD_POST_INIT_BLINK;

  xTaskCreate( task_leds, "leds", 2*def_stksz, 0, 1, 0 );

  SCHEDULER_START;
  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  // int i=1;
  while(1) {
    leds[0].toggle();
    // leds.write( i );
    // ++i;
    // i &= BOARD_LEDS_ALL;
    delay_mcs( led_delay );
  }
}

// configs
void MX_GPIO_Init(void)
{
  board_def_btn_init( true );
}


#if defined(BOARD_BTN0)
void BOARD_BTN0_IRQHANDLER(void)
{
  leds[3].toggle();
  HAL_GPIO_EXTI_IRQHandler( BOARD_BTN0.bitmask() );
  #ifdef BOARD_BTN0_1_SAME_IRQ
    HAL_GPIO_EXTI_IRQHandler( BOARD_BTN1.bitmask() );
  #endif
}
#endif

#if defined(BOARD_BTN1)
#ifndef BOARD_BTN0_1_SAME_IRQ
void BOARD_BTN1_IRQHANDLER(void)
{
  HAL_GPIO_EXTI_IRQHandler( BOARD_BTN1.bitmask() );
}
#endif
#endif

void EXTI_CALLBACK_FUN( uint16_t mask )
{
  uint32_t curr_tick = HAL_GetTick();
  // leds.toggle( BIT3M );
  if( curr_tick - last_exti_tick < btn_deadtime ) {
    return; // ignore too fast events
  }

  if( mask == BOARD_BTN0.bitmask() )  {
    leds[1].toggle();
    led_delay >>= 1;
    if( led_delay < 1 ) {
      led_delay = led_delay_init;
    }
  }

  if( mask == BOARD_BTN1.bitmask() )  {
    leds[3].toggle();
    led_delay = led_delay_init;
  }
  // leds.toggle( BIT0M );
  last_exti_tick = curr_tick;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

