#include <oxc_base.h>
#include <oxc_gpio.h>

// use tim6 as ADC start tick:
// APB1: V1: 2*50MHz /(1+99)   = 1MHz   (1-50000 ARR)=(1MHz-20Hz)  too fast start, generic - good, working range good
//       V2: 2*50MHz /(1+999)  = 100kHz (1-50000 ARR)=(100kHz-2Hz) normal start, show end
//       V3: 2*50MHz /(1+9999) =  10kHz (1-50000 ARR)=(10kHz-0.2Hz) med start start, debug end
//

extern TIM_HandleTypeDef tim6h;

void tim6_init( uint16_t presc, uint16_t arr )
{
  __TIM6_CLK_ENABLE();

  // just for debug
  HAL_NVIC_SetPriority( TIM6_DAC_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( TIM6_DAC_IRQn );

  tim6h.Instance = TIM6;
  // tim6h.Init.Prescaler = presc;
  tim6h.Init.Prescaler = 49999;
  // tim6h.Init.CounterMode = TIM_COUNTERMODE_UP;
  // tim6h.Init.Period    = arr; // 1 Hz
  tim6h.Init.Period    = 5000;

  // TIM6->PSC = 999;   // 100 kHz
  // TIM6->ARR = 1000;  // 100 Hz
  TIM6->PSC = 9999;   // 10 kHz
  TIM6->ARR = 1000;  // 10 Hz

  TIM6->CR2 = 0x20;  // UPDATE->TRGO

  // if( HAL_TIM_Base_Init( &tim6h ) != HAL_OK ) {
  //   Error_Handler( 4 );
  //   return;
  // }
  if( HAL_TIM_Base_Start_IT( &tim6h ) != HAL_OK ) {
    Error_Handler( 4 );
    return;
  }

  // TIM_MasterConfigTypeDef sMasterConfig;
  // sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
  // sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  // if( HAL_TIMEx_MasterConfigSynchronization( &tim6h, &sMasterConfig ) != HAL_OK )  {
  //   Error_Handler( 11 );
  // }


}

// void HAL_TIM_MspInit( TIM_HandleTypeDef* htim_pwm )
// {
//   if( htim_pwm->Instance !=TIM6 ) {
//     return;
//   }
//   __TIM6_CLK_ENABLE();
//   leds.toggle( BIT1 );
// }

