#include <oxc_auto.h>

void MX_GPIO_Init(void);
int  delay_bad(void);

USE_DIE_ERROR_HANDLER;



int main(void)
{
  HAL_Init();

  int rc = SystemClockCfg();
  if( rc ) {
    die( 1 );
    return 0;
  }

  MX_GPIO_Init();


  int i=0x04;
  BOARD_LEDS_GPIO->ODR = BOARD_LEDS_MASK;
  HAL_Delay( 1500 );
  while( 1 ) {
    GPIO_WriteBits( BOARD_LEDS_GPIO, i<<BOARD_LEDS_OFS, BOARD_LEDS_MASK );
    ++i;
    i &= BOARD_LEDS_ALL;
    HAL_Delay( 200 );
    // delay_bad();
  }
  return 0;
}

int delay_bad()
{
  volatile int k = 0, j;
  for( j=0; j<800000; ++j ) {
    k += j * j;
  }
  return k;
}

// configs


void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  //__GPIOA_CLK_ENABLE();
  //__GPIOB_CLK_ENABLE();
  // __GPIOC_CLK_ENABLE();
  // __GPIOD_CLK_ENABLE();
  // __GPIOE_CLK_ENABLE();
  // __GPIOG_CLK_ENABLE();
  // __GPIOH_CLK_ENABLE();
  BOARD_LEDS_GPIO_ON;

  GPIO_InitStruct.Pin   = BOARD_LEDS_MASK;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_MAX;
  HAL_GPIO_Init( BOARD_LEDS_GPIO, &GPIO_InitStruct );

  /*Configure GPIO pins : PA0 PA1 */
  // GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  // GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  // GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,ox/inc

