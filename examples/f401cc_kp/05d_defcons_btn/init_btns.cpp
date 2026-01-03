#include <oxc_auto.h>

#include "main.h"

void init_btns()
{
  GpioA.enableClk();
  GpioB.enableClk();
  static const PortPin in_pins[] = { PA4, PA5, PA6, PA7, PB0, PB1, PB2 };

  for( auto pin : in_pins ) {
    pin.cfgIn( GpioPull::down );
    pin.setEXTI(  ExtiEv::up );
  }

  // TODO: func?
  decltype( EXTI9_5_IRQn ) irqs[] = {
    EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn,
    EXTI4_IRQn, EXTI9_5_IRQn
  };

  for( auto irq: irqs ) {
    HAL_NVIC_SetPriority( irq, 15, 0 );
    HAL_NVIC_EnableIRQ( irq );
  }
}

void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  static uint32_t last_exti_tick = 0;

  uint32_t curr_tick = HAL_GetTick();
  if( curr_tick - last_exti_tick < 100 ) {
    return; // ignore too fast events
  }

  uint32_t cmd = pin;

  leds[0].toggle();
  UVAR_c = cmd;
  ++UVAR_i;

  // if( ! on_cmd_handler ) {
  //   menu4b_ev_global = cmd;
  // } else {
  //   on_btn_while_run( cmd );
  // }

  last_exti_tick = curr_tick;
}

void EXTI0_IRQHandler(void)
{
  ++UVAR_j;
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
}

void EXTI1_IRQHandler(void)
{
  ++UVAR_j;
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_IRQHandler(void)
{
  ++UVAR_j;
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
}

void EXTI4_IRQHandler(void)
{
  ++UVAR_j;
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
}

void EXTI9_5_IRQHandler(void)
{
  ++UVAR_k;

  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
}

