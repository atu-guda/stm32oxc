#include <oxc_auto.h>
#include <main.h>

TIM_HandleTypeDef tim_freq_meas_h;


int tim_freq_meas_cfg()
{
  auto &t_h { tim_freq_meas_h };
  t_h.Instance               = TIM_FREQ_MEAS;
  t_h.Init.Prescaler         = tim_freq_meas_psc; // 0
  t_h.Init.Period            = 0xFFFFFFFF;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &t_h ) != HAL_OK ) {
    UVAR_e = 1; // like error
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &t_h, &sClockSourceConfig );

  if( HAL_TIM_IC_Init( &t_h ) != HAL_OK )  {
    UVAR_e = 3; // like error
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig) != HAL_OK ) {
    UVAR_e = 4;
    return 0;
  }

  TIM_IC_InitTypeDef sConfigIC;
  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel( &t_h, &sConfigIC, TIM_FREQ_MEAS_CHANNEL ) != HAL_OK ) {
    UVAR_e = 5;
    return 0;
  }

  return 1;
}


// void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
// {
//   if( htim->Instance == TIM_LWM ) {
//     TIM_LWM_EN;
//     GpioA.cfgAF_N( TIM_LWM_GPIO_PINS, TIM_LWM_GPIO_AF );
//     // HAL_NVIC_SetPriority( TIM_LWM_IRQn, 8, 0 );
//     // HAL_NVIC_EnableIRQ( TIM_LWM_IRQn );
//     return;
//   }
//
//
// }
//
// void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
// {
//   if( htim->Instance == TIM_LWM ) {
//     TIM_LWM_DIS;
//     GpioA.cfgIn_N( TIM_LWM_GPIO_PINS );
//     // HAL_NVIC_DisableIRQ( TIM_LWM_IRQn );
//     return;
//   }
// }
//
// // void TIM_LWM_IRQ_HANDLER()
// // {
// //   HAL_TIM_IRQHandler( &tim_lwm_h );
// // }
//
//
// void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
// {
//
// }


