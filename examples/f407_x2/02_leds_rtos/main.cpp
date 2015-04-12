#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

// void MX_GPIO_Init(void);


// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {

void task_leds( void *prm UNUSED_ARG );

}

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  xTaskCreate( task_leds, "leds", 2*def_stksz, 0, 1, 0 );

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
    i &= 0x0F;
    delay_ms( 1000 );
    // HAL_Delay( 1000 );
  }
}

void _exit( int rc )
{
  exit_rc = rc;
  for( ;; );
}

// // configs
// void MX_GPIO_Init(void)
// {
//
//   // GPIO_InitTypeDef GPIO_InitStruct;
//   // moved to PinsOut initHW
//
//   /*Configure GPIO pins : PA0 PA1 */
//   // GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
//   // GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
//   // GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//   // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
// }

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

