#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;

void MX_GPIO_Init(void);


BOARD_DEFINE_LEDS_EX;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {

void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void HAL_GPIO_EXTI_Callback( uint16_t pin );

void task_leds( void *prm UNUSED_ARG );

}

volatile int led_delay = 1000;

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  delay_bad_ms( 200 );  leds.write( 0 );

  MX_GPIO_Init();

  leds.write( 0x0F );
  // delay_bad_ms( 500 );
  HAL_Delay( 500 );
  // delay_ms( 500 );
  leds.write( 0x0A );
  delay_bad_ms( 200 );

  xTaskCreate( task_leds, "leds", 2*def_stksz, 0, 1, 0 );

  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  int i=8;
  while(1) {
    leds.write( i );
    ++i;
    i &= 0x0F;
    delay_ms( led_delay );
    // HAL_Delay( 1000 );
  }
}

void _exit( int rc )
{
  exit_rc = rc;
  for( ;; );
}

// // configs
void MX_GPIO_Init(void)
{
  // putput init moved to PinsOut initHW

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef gpi;

  /* Configure  input GPIO pins : C13 */
  gpi.Pin = GPIO_PIN_13;
  // gpi.Mode = GPIO_MODE_EVT_RISING;
  gpi.Mode = GPIO_MODE_IT_RISING;
  gpi.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init( GPIOC, &gpi );

  // HAL_NVIC_SetPriority(  EXTI0_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_SetPriority( EXTI15_10_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  // HAL_NVIC_SetPriority( EXTI0_IRQn, 4, 0 );
  // HAL_NVIC_SetPriority( EXTI1_IRQn, 4, 0 );
  // HAL_NVIC_EnableIRQ(  EXTI0_IRQn );
  HAL_NVIC_EnableIRQ( EXTI15_10_IRQn );
}

// void EXTI0_IRQHandler(void)
// {
//   HAL_GPIO_EXTI_IRQHandler( BIT0 );
// }

void EXTI15_10_IRQHandler(void)
{
  if( EXTI->PR &  (1<<13) ) {
    HAL_GPIO_EXTI_IRQHandler( BIT13 );
  }
}

void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  // if( pin == BIT0 )  {
  //   leds.set( 0x0F );
  //   led_delay = 1000;
  // }
  if( pin == BIT13 )  {
    leds.reset( 0x0F );
    led_delay >>= 1;
    if( led_delay < 1 ) {
      led_delay = 1000;
    }
  }
  leds.toggle( 0x01 );
}

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

