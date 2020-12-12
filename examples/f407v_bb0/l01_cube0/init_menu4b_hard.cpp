#include <oxc_auto.h>
#include <oxc_gpio.h>
#include <oxc_menu4b.h>

// arch-dependentf functions for Menu4b
// this file: f407v_bb0: b0,b1,b3,b4

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

void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
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
    case GPIO_PIN_0: cmd = MenuCmd::Enter;   break;
    case GPIO_PIN_1: cmd = MenuCmd::Down;    break;
    case GPIO_PIN_2: cmd = MenuCmd::Esc;     break;
    case GPIO_PIN_4: cmd = MenuCmd::Up;      break;
    default: break;
  }

  leds.toggle( BIT1 );
  UVAR('m') = cmd;
  if( ! on_cmd_handler ) {
    menu4b_ev_global = cmd;
  } else {
    // leds.toggle( BIT0 );
    on_btn_while_run( cmd );
  }

  last_exti_tick = curr_tick;
}

int init_menu4b_buttons() // board dependent function
{
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GpioB.cfgIn_N( GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4, GpioRegs::Pull::down );
  GpioB.setEXTI( 0, GpioRegs::ExtiEv::up );
  GpioB.setEXTI( 1, GpioRegs::ExtiEv::up );
  GpioB.setEXTI( 2, GpioRegs::ExtiEv::up );
  GpioB.setEXTI( 4, GpioRegs::ExtiEv::up );

  HAL_NVIC_SetPriority( EXTI0_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI0_IRQn );
  HAL_NVIC_SetPriority( EXTI1_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI1_IRQn );
  HAL_NVIC_SetPriority( EXTI2_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI2_IRQn );
  HAL_NVIC_SetPriority( EXTI4_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI4_IRQn );

  return 1;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

