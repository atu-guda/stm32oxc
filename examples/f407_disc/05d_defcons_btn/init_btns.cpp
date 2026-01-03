#include <oxc_auto.h>

#include "main.h"

void init_btns()
{
  const PortPin in_pins[] = { PC5, PC6, PC8, PC9 }; // TODO: config

  for( auto pin : in_pins ) {
    pin.enableClk();
    pin.cfgIn(   GpioPull::down );
    pin.setEXTI( ExtiEv::up );
  }

  HAL_NVIC_SetPriority( EXTI9_5_IRQn, 15, 0 ); // TODO: use auto assign or func?
  HAL_NVIC_EnableIRQ(   EXTI9_5_IRQn );
}

void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  static uint32_t last_exti_tick = 0;

  uint32_t curr_tick = HAL_GetTick();
  if( curr_tick - last_exti_tick < 100 ) {
    return; // ignore too fast events
  }

  uint32_t cmd = 0;
  switch( pin ) {
    case GPIO_PIN_5:
      cmd = 1;
      break;
    case GPIO_PIN_6:
      cmd = 2;
      break;
    case GPIO_PIN_8:
      cmd = 3;
      break;
    case GPIO_PIN_9:
      cmd = 4;
      break;
    default: break;
  }

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


void EXTI9_5_IRQHandler(void)
{
  ++UVAR_j;
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
}

