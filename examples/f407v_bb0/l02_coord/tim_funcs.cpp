#include <oxc_auto.h>

#include "main.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

TIM_HandleTypeDef* pwm_tims[n_tim_pwm] { &htim3, &htim10, &htim11 };


// to common PWM timers init
const TIM_ClockConfigTypeDef def_pwm_CSC {
    .ClockSource    = TIM_CLOCKSOURCE_INTERNAL,
    .ClockPolarity  = TIM_ICPOLARITY_RISING,
    .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
    .ClockFilter    = 0
};
const TIM_MasterConfigTypeDef def_pwm_MasterConfig {
  .MasterOutputTrigger = TIM_TRGO_RESET,
  .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
};
const TIM_OC_InitTypeDef def_pwm_ConfigOC {
  .OCMode       = TIM_OCMODE_PWM1, // TIM_OCMODE_FORCED_INACTIVE,
  .Pulse        = 0,
  .OCPolarity   = TIM_OCPOLARITY_HIGH,
  .OCNPolarity  = TIM_OCNPOLARITY_HIGH,
  .OCFastMode   = TIM_OCFAST_DISABLE,
  .OCIdleState  = TIM_OCIDLESTATE_RESET,
  .OCNIdleState = TIM_OCNIDLESTATE_RESET
};

// see: ~/proj/stm32/cube/f407_coord/Core/Src/tim.c

// except TIM6_callback

int MX_PWM_common_Init( unsigned idx, unsigned channel )
{
  if( idx >= n_tim_pwm ) {
    return 0;
  }
  auto ti = pwm_tims[idx];
  if( ti->Instance == nullptr ) {
    return 0;
  }
  auto t = ti->Instance;
  auto psc   = calc_TIM_psc_for_cnt_freq( t, TIM_PWM_base_freq );
  auto arr   = calc_TIM_arr_for_base_psc( t, psc, TIM_PWM_count_freq );

  ti->Init.Prescaler         = psc;
  ti->Init.CounterMode       = TIM_COUNTERMODE_UP;
  ti->Init.Period            = arr;
  ti->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  ti->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( ti ) != HAL_OK ) {
    UVAR('e') = 31;
    return 0;
  }

  if( HAL_TIM_ConfigClockSource( ti,
        const_cast<TIM_ClockConfigTypeDef*>(&def_pwm_CSC) ) != HAL_OK ) {
    UVAR('e') = 32;
    return 0;
  }

  if( HAL_TIM_PWM_Init( ti ) != HAL_OK ) {
    UVAR('e') = 33;
    return 0;
  }

  if( HAL_TIMEx_MasterConfigSynchronization( ti,
        const_cast<TIM_MasterConfigTypeDef*>(&def_pwm_MasterConfig) ) != HAL_OK ) {
    UVAR('e') = 34;
    return 0;
  }

  if( HAL_TIM_PWM_ConfigChannel( ti,
        const_cast<TIM_OC_InitTypeDef*>(&def_pwm_ConfigOC), channel ) != HAL_OK ) {
    UVAR('e') = 35;
    return 0;
  }

  // start here
  HAL_TIM_PWM_Start( ti, channel );

  HAL_TIM_MspPostInit( ti );
  return 1;
}

int MX_TIM3_Init()
{
  htim3.Instance = TIM3;
  return MX_PWM_common_Init( 0, TIM_CHANNEL_1 );
}

int MX_TIM10_Init()
{
  htim10.Instance = TIM11;
  return MX_PWM_common_Init( 1, TIM_CHANNEL_1  );
}

int MX_TIM11_Init()
{
  htim11.Instance = TIM11;
  return MX_PWM_common_Init( 2, TIM_CHANNEL_1  );
}

int MX_TIM6_Init()
{
  auto psc   = calc_TIM_psc_for_cnt_freq( TIM6, TIM6_base_freq );       // 83
  auto arr   = calc_TIM_arr_for_base_psc( TIM6, psc, TIM6_count_freq );
  htim6.Instance               = TIM6;
  htim6.Init.Prescaler         = psc;
  htim6.Init.Period            = arr;
  htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim6 ) != HAL_OK ) {
    UVAR('e') = 61;
    return 0;
  }

  if( HAL_TIMEx_MasterConfigSynchronization( &htim6,
        const_cast<TIM_MasterConfigTypeDef*>(&def_pwm_MasterConfig) ) != HAL_OK ) {
    UVAR('e') = 62;
    return 0;
  }
  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if(      tim_baseHandle->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  }
  else if( tim_baseHandle->Instance == TIM3 ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
  else if( tim_baseHandle->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    GpioD.cfgAF( 13, GPIO_AF2_TIM4 );  // TIM4.2 D13 se3
    GpioD.cfgAF( 14, GPIO_AF2_TIM4 );  // TIM4.3 D13 se2
    GpioD.cfgAF( 15, GPIO_AF2_TIM4 );  // TIM4.4 D13 se1
    // HAL_NVIC_SetPriority( TIM4_IRQn, 5, 0 );
    // HAL_NVIC_EnableIRQ( TIM4_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM6 ) {
    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority( TIM6_DAC_IRQn, 5, 0 );
    HAL_NVIC_EnableIRQ(   TIM6_DAC_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM10 ) {
    __HAL_RCC_TIM10_CLK_ENABLE();
  }
  else if( tim_baseHandle->Instance == TIM11 ) {
    __HAL_RCC_TIM11_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
  if(      timHandle->Instance == TIM2 ) {
    GpioB.cfgAF( 10, GPIO_AF1_TIM2 );  // TIM2.3: B10 PWM3? aux1.6
    GpioB.cfgAF( 11, GPIO_AF1_TIM2 );  // TIM2.4: B11 PWM4? aux1.8
  }
  else if( timHandle->Instance == TIM3 ) {
    GpioC.cfgAF( 6, GPIO_AF2_TIM3 );  // TIM3.1:  C6 PWM0
  }
  else if( timHandle->Instance == TIM10 ) {
    GpioB.cfgAF( 8, GPIO_AF3_TIM10 ); // TIM10.1: B8 PWM1
  }
  else if( timHandle->Instance == TIM11 ) {
    GpioB.cfgAF( 9, GPIO_AF3_TIM11 ); // TIM11.1: B9 PWM2
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if(      tim_baseHandle->Instance == TIM2  ) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  }
  else if( tim_baseHandle->Instance == TIM3  ) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  }
  else if( tim_baseHandle->Instance == TIM4  ) {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM4_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM6  ) {
    __HAL_RCC_TIM6_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM6_DAC_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM10 ) {
    __HAL_RCC_TIM10_CLK_DISABLE();
  }
  else if( tim_baseHandle->Instance == TIM11 ) {
    __HAL_RCC_TIM11_CLK_DISABLE();
  }
}

void TIM4_IRQHandler()
{
  HAL_TIM_IRQHandler( &htim4 );
}

void TIM6_DAC_IRQHandler()
{
  HAL_TIM_IRQHandler( &htim6 );
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  if( htim->Instance == TIM6 ) {
    TIM6_callback();
    return;
  }
}
