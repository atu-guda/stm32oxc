#include <oxc_base.h>
#include <oxc_gpio.h>

// use tim2 as ADC start tick:
// APB1: V1: 2*50MHz /(1+99)   = 1MHz   (1-2^32-1 ARR)=(1MHz-2.3E-4Hz)  32-bit timer is great!
//  ARR:freq  1:1MHz 10:100kHz 100:10kHz 1000:1kHz 10000:100Hz 100000:10Hz 1000000:1Hz 1e7:0.1Hz

extern TIM_HandleTypeDef tim2h;

void tim2_init( uint16_t presc, uint32_t arr )
{
  __TIM2_CLK_ENABLE();

  // just for debug ???
  // HAL_NVIC_SetPriority( TIM2_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_SetPriority( TIM2_IRQn, 1, 0 );
  HAL_NVIC_EnableIRQ( TIM2_IRQn );

  tim2h.Instance               = TIM2;
  tim2h.Init.Prescaler         = presc;
  tim2h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim2h.Init.Period            = arr;
  tim2h.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  tim2h.Init.RepetitionCounter = 0; // only for adv timers

  //TIM2->CR2 = 0x20;  // UPDATE->TRGO

  if( HAL_TIM_Base_Init( &tim2h ) != HAL_OK ) {
    // TODO: Error_Handler( 4 );
    return;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &tim2h, &sMasterConfig ) != HAL_OK )  {
    // TODO: Error_Handler( 11 );
    return;
  }

  // if( HAL_TIM_Base_Start_IT( &tim2h ) != HAL_OK ) {
  //   TODO: Error_Handler( 4 );
  //   return;
  // }
  if( HAL_TIM_Base_Start( &tim2h ) != HAL_OK ) {
    // TODO: Error_Handler( 4 );
    return;
  }

}

void tim2_deinit()
{
  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  __TIM2_CLK_DISABLE();
}

// void HAL_TIM_MspInit( TIM_HandleTypeDef* htim_pwm )
// {
//   if( htim_pwm->Instance !=TIM2 ) {
//     return;
//   }
//   __TIM2_CLK_ENABLE();
//   leds.toggle( BIT1 );
// }

