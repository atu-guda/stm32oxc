#include <oxc_auto.h>

// now local?
constexpr uint16_t BOARD_LEDS_OFS { BOARD_LEDS_START.pinNum().Num() };
constexpr PinMask BOARD_LEDS_MASK { BOARD_LEDS_ALL << BOARD_LEDS_OFS };

auto& BOARD_LEDS_GPIO = BOARD_LEDS_START.port();

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
  GPIO_TypeDef* gp = reinterpret_cast<GPIO_TypeDef*>(&BOARD_LEDS_GPIO);

  uint16_t i { 0x04 };
  BOARD_LEDS_GPIO.ODR = BOARD_LEDS_MASK.bitmask();
  HAL_Delay( 1500 );

  while( 1 ) {
    GPIO_WriteBits( gp, i<<BOARD_LEDS_OFS, BOARD_LEDS_MASK.bitmask() );
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
  BOARD_LEDS_GPIO.enableClk();
  BOARD_LEDS_GPIO.cfgOut_N( BOARD_LEDS_MASK );
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

