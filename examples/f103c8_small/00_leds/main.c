#include "stm32f1xx_hal.h"
#include <oxc_base.h>

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

USE_DIE_ERROR_HANDLER;


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

  // HAL_RCC_MCOConfig( RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1 );
  // HAL_RCC_MCOConfig( RCC_MCO, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1 );

  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 ); // test?

  MX_GPIO_Init();

  int i=0x04,  j = 0;
  // GPIOE->ODR = 0x0F;
  while(1) {
    j = i << 8;
    GPIO_WriteBits( GPIOB, j, 0xF000 );
    ++i;
    i &= 0x0FF;
    HAL_Delay( 1000 );
    // delay_bad();
  }
  return 0;
}

int delay_bad()
{
  volatile int k = 0, j;
  for( j=0; j<100000; ++j ) {
    k += j * j;
  }
  return k;
}


/* void HAL_SYSTICK_Callback(void) */
/* { */
/*   static uint16_t v = 0; */
/*   GPIO_WriteBits( GPIOB, v, 0xF000 ); */
/*   if( v == 0 ) { */
/*     v = 0xF000; */
/*   } else { */
/*     v = 0; */
/*   } */
/* } */

// configs



// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,ox/inc

