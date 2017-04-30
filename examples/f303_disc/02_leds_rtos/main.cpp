#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;

void MX_GPIO_Init(void);


BOARD_DEFINE_LEDS;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {

void task_leds( void *prm UNUSED_ARG );

}

volatile int led_delay = 1000;

int main(void)
{
  STD_PROLOG_START;

  MX_GPIO_Init();

  leds.write( 0x0A );
  delay_bad_ms( 500 );
  // HAL_Delay( 500 );
  // delay_ms( 500 );
  leds.write( 0x0F );
  delay_bad_ms( 500 );

  xTaskCreate( task_leds, "leds", 2*def_stksz, 0, 1, 0 );

  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  int i=8;
  while (1)
  {
    leds.write( i );
    ++i;
    i &= BOARD_LEDS_ALL;
    delay_ms( led_delay );
    // HAL_Delay( 1000 );
  }
}

// configs
void MX_GPIO_Init(void)
{
  // __HAL_RCC_SYSCFG_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef gpi;

  /* Configure  input GPIO pin : PA0 */
  gpi.Pin = GPIO_PIN_0;
  gpi.Mode = GPIO_MODE_IT_RISING;
  gpi.Pull = GPIO_PULLDOWN;
  gpi.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init( GPIOA, &gpi );

  HAL_NVIC_SetPriority( EXTI0_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( EXTI0_IRQn );
}

void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( BIT0 );
}


void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  if( pin == BIT0 )  {
    leds.reset( 0xFF );
    led_delay >>= 1;
    if( led_delay < 1 ) {
      led_delay = 1000;
    }
  }
  if( pin == BIT1 )  {
    leds.set( 0xAA );
    led_delay = 1000;
  }
  leds.toggle( 0x01 );
}

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

