#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

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
  int i=1;
  while(1) {
    leds.write( i );
    ++i;
    i &= BOARD_LEDS_ALL;
    delay_mcs( led_delay );
  }
}

// configs
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef gpi;
  gpi.Mode  = GPIO_MODE_IT_RISING;
  gpi.Pull  = GPIO_PULLDOWN;
  gpi.Speed = GPIO_SPEED_MAX;

#ifdef BOARD_BTN0_EXIST
  GPIO_enableClk( BOARD_BTN0_GPIO );
  gpi.Pin = BOARD_BTN0_BIT;
  HAL_GPIO_Init( BOARD_BTN0_GPIO, &gpi );
  HAL_NVIC_SetPriority( BOARD_BTN0_IRQ, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( BOARD_BTN0_IRQ );
#endif

#ifdef BOARD_BTN1_EXIST
  GPIO_enableClk( BOARD_BTN1_GPIO );
  gpi.Pin  = BOARD_BTN1_BIT;
  HAL_GPIO_Init( BOARD_BTN1_GPIO, &gpi );
  HAL_NVIC_SetPriority( BOARD_BTN1_IRQ, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( BOARD_BTN1_IRQ );
#endif

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
  if( pin == EXTI_BIT0 )  {
    leds.reset( BOARD_LEDS_ALL );
    led_delay >>= 1;
    if( led_delay < 1 ) {
      led_delay = led_delay_init;
    }
  }
  if( pin == EXTI_BIT1 )  {
    leds.set( 0xAA );
    led_delay = led_delay_init;
  }
  leds.toggle( BIT0 );
  last_exti_tick = curr_tick;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

