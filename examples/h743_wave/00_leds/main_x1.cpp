#include <oxc_auto.h>

void MX_GPIO_Init(void);
int  delay_bad(void);

USE_DIE_ERROR_HANDLER;


int main(void)
{
  HAL_Init();

  MX_GPIO_Init();

  int rc = SystemClockCfg();
  if( rc ) {
    die( rc );
    return 0;
  }

  GPIO_TypeDef* gp = reinterpret_cast<GPIO_TypeDef*>( &BOARD_LEDS_GPIO );



  int i=0x04;
  BOARD_LEDS_GPIO.ODR = BOARD_LEDS_MASK;
  HAL_Delay( 1500 );
  while( 1 ) {
    GPIO_WriteBits( gp, i<<BOARD_LEDS_OFS, BOARD_LEDS_MASK );
    ++i;
    i &= BOARD_LEDS_ALL;
    // GPIOH->ODR = 0xFFFF;
    GPIOI->ODR ^= BIT8;
    // HAL_Delay( 200 );
    // GPIOH->ODR = 0x0000;
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
  __HAL_RCC_GPIOI_CLK_ENABLE();

  // __HAL_RCC_GPIOH_CLK_ENABLE();
  BOARD_LEDS_GPIO.enableClk();

  BOARD_LEDS_GPIO.cfgOut_N( BOARD_LEDS_MASK );

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = BIT8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

