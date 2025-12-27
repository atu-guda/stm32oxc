#include <oxc_auto.h>
#include <oxc_menu4b.h>

// arch-dependentf functions for Menu4b
// this file: BluePill, A0:A3

void on_btn_while_run( int cmd );

void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
}

void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
}

void EXTI3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
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
    case GPIO_PIN_0: cmd = MenuCmd::Esc;   break;
    case GPIO_PIN_1: cmd = MenuCmd::Up;    break;
    case GPIO_PIN_2: cmd = MenuCmd::Down;  break;
    case GPIO_PIN_3: cmd = MenuCmd::Enter; break;
    default: break;
  }

  leds[0].toggle();
  if( ! on_cmd_handler ) {
    menu4b_ev_global = cmd;
  } else {
    on_btn_while_run( cmd );
  }

  last_exti_tick = curr_tick;
}

int init_menu4b_buttons() // board dependent function TODO: config
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  const PinNum pins[] { 0_pin, 1_pin, 2_pin, 3_pin };

  for( auto p : pins ) {
    GpioA.cfgIn( p, GpioPull::up );
    GpioA.setEXTI( p, ExtiEv::down );
  }

  // TODO: macro/ const fun
  const decltype(EXTI0_IRQn) irqs[] { EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn };
  for( auto irq : irqs ) {
    HAL_NVIC_SetPriority( irq, 14, 0 );
    HAL_NVIC_EnableIRQ(   irq );
  }

  return 1;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

