#include <oxc_base.h>
#include <oxc_gpio.h>

// use tim6 as ADC start tick:
// APB1: V1: 50MHz /(1+49)   = 1MHz   (1-50000 ARR)=(1MHz-20Hz)  too fast start, generic - good, working range good
//       V2: 50MHz /(1+499)  = 100kHz (1-50000 ARR)=(100kHz-2Hz) normal start, show end
//       V3: 50MHz /(1+4999) =  10kHz (1-50000 ARR)=(10kHz-0.2Hz) med start start, debug end
//

extern TIM_HandleTypeDef tim6h;

void tim6_init( uint16_t presc, uint16_t arr )
{
  __TIM6_CLK_ENABLE();

  // just for debug
  HAL_NVIC_SetPriority( TIM6_DAC_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( TIM6_DAC_IRQn );

  tim6h.Instance = TIM6;
  tim6h.Init.Prescaler = presc;
  tim6h.Init.Period    = arr; // 1 Hz
  tim6h.Init.ClockDivision = 0;
  tim6h.Init.CounterMode = TIM_COUNTERMODE_UP;
  if( HAL_TIM_Base_Start_IT( &tim6h ) != HAL_OK ) {
    Error_Handler( 4 );
    return;
  }


}

// void HAL_TIM_MspInit( TIM_HandleTypeDef* htim_pwm )
// {
//   if( htim_pwm->Instance !=TIM6 ) {
//     return;
//   }
//   __TIM6_CLK_ENABLE();
//   leds.toggle( BIT1 );
// }

