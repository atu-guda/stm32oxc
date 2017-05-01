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

// configs





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,../../../inc

