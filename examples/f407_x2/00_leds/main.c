#include <stm32f4xx.h>
#include <ox_base.h>


GPIO_InitTypeDef gp;

int delay_bad(void);
void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );
void _exit( int rc );

void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask )
{
  GPIOx->ODR = ( PortVal & mask ) | ( GPIOx->ODR & (~mask) );
}

void _exit( int rc UNUSED )
{
}

#define TPORT                GPIOD
#define TRCC  RCC_AHB1Periph_GPIOD

int main(void)
{

  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE );
  RCC_AHB1PeriphClockCmd(                 TRCC, ENABLE );

  // main bits
  gp.GPIO_Pin = 0x000F; // 4 lesser bits
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_Speed = GPIO_Speed_100MHz;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init( GPIOC, &gp );

  uint16_t shift = 12, mask =  0x000F << shift; // 0x0F00; //
  // test2 bits
  gp.GPIO_Pin = mask;
  GPIO_Init( TPORT, &gp );

  int i=0, j = 0;
  while (1)
  {
    j = ( (~i) & 0x000F ) << shift;
    GPIO_WriteBits( GPIOC, i, 0x000F );
    GPIO_WriteBits( TPORT, j, mask );
    ++i;
    i &= 0x0F;
    delay_bad();
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

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,ox/inc

