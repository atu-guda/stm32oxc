#include "stm32f3xx_hal.h"
// #include <oxc_base.h>

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void SystemClock_Config(void);

void MX_GPIO_Init(void);


int  delay_bad(void);
void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );
void _exit( int rc );

void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask )
{
  GPIOx->ODR = ( PortVal & mask ) | ( GPIOx->ODR & (~mask) );
}


int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  int i=0x04,  j = 0;
  // GPIOE->ODR = 0x0F;
  while (1)
  {
    j = i << 8;
    GPIO_WriteBits( GPIOE, j, 0xFF00 );
    ++i;
    i &= 0x0FF;
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
  __GPIOE_CLK_ENABLE();
  // __GPIOG_CLK_ENABLE();
  // __GPIOH_CLK_ENABLE();

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 */
  // GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Pin = 0xFF00; // 8-15
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

  /*Configure GPIO pins : PA0 PA1 */
  // GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  // GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  // GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,ox/inc

