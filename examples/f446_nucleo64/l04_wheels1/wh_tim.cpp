#include <oxc_auto.h>
#include <oxc_tim.h>

#include "wheels_pins.h"

void tim1_cfg()
{
  tim1_h.Instance               = TIM1;
  // US defines tick: 5.8 mks approx 1mm 170000 = v_c/2 in mm/s, 998 or 846
  tim1_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM1, 170000 ); // 987 on test
  tim1_h.Init.Period            = calc_TIM_arr_for_base_psc( TIM1, tim1_h.Init.Prescaler, 20 ); // F approx 20Hz: for  motor PWM
  tim1_h.Init.ClockDivision     = 0;
  tim1_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim1_h ) != HAL_OK ) {
    UVAR('e') = 111; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim1_h, &sClockSourceConfig );

  HAL_TIM_PWM_Init( &tim1_h );

  HAL_TIM_PWM_Stop( &tim1_h, TIM_CHANNEL_1 );
  HAL_TIM_PWM_Stop( &tim1_h, TIM_CHANNEL_2 );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = 0;
  HAL_TIM_PWM_ConfigChannel( &tim1_h, &tim_oc_cfg, TIM_CHANNEL_1 );
  HAL_TIM_PWM_ConfigChannel( &tim1_h, &tim_oc_cfg, TIM_CHANNEL_2 );
  HAL_TIM_PWM_Start( &tim1_h, TIM_CHANNEL_1 );
  HAL_TIM_PWM_Start( &tim1_h, TIM_CHANNEL_2 );

  HAL_TIM_PWM_Stop( &tim1_h, TIM_CHANNEL_3 );

  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = 3; // 16 us
  HAL_TIM_PWM_ConfigChannel( &tim1_h, &tim_oc_cfg, TIM_CHANNEL_3 );

  TIM_IC_InitTypeDef  tim_ic_cfg;
  // tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_RISING;
  tim_ic_cfg.ICPolarity  = TIM_ICPOLARITY_BOTHEDGE; // rising - start, falling - stop
  tim_ic_cfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_ic_cfg.ICPrescaler = TIM_ICPSC_DIV1;
  tim_ic_cfg.ICFilter    = 0; // 0 - 0x0F
  if( HAL_TIM_IC_ConfigChannel( &tim1_h, &tim_ic_cfg, TIM_CHANNEL_4 ) != HAL_OK ) {
    UVAR('e') = 21;
    return;
  }
  HAL_NVIC_SetPriority( TIM1_CC_IRQn, 5, 0 );
  HAL_NVIC_EnableIRQ( TIM1_CC_IRQn );
  if( HAL_TIM_IC_Start_IT( &tim1_h, TIM_CHANNEL_4 ) != HAL_OK ) {
    UVAR('e') = 23;
  }

  HAL_TIM_PWM_Start( &tim1_h, TIM_CHANNEL_3 );
}

void TIM1_CC_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &tim1_h );
}

void tim3_cfg()
{
  tim3_h.Instance               = TIM_N_R;
  tim3_h.Init.Prescaler         = 0;
  tim3_h.Init.Period            = 0xFFFF; // max: unused ? or 0?
  tim3_h.Init.ClockDivision     = 0;
  tim3_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim3_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim3_h ) != HAL_OK ) {
    UVAR('e') = 113; // like error
    return;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if( HAL_TIM_SlaveConfigSynchronization( &tim3_h, &sSlaveConfig ) != HAL_OK ) {
    UVAR('e') = 1113;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &tim3_h, &sMasterConfig ) != HAL_OK )  {
    UVAR('e') = 1123;
  }

  TIM_IC_InitTypeDef sConfigIC;
  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_TRC;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter    = 0;
  HAL_TIM_IC_ConfigChannel( &tim3_h, &sConfigIC, TIM_CHANNEL_1 );
  HAL_TIM_IC_Init( &tim3_h );

  TIM_N_R->CR1 |= 1; // test
}

void tim4_cfg()
{
  tim4_h.Instance               = TIM_N_L;
  tim4_h.Init.Prescaler         = 0;
  tim4_h.Init.Period            = 0xFFFF;
  tim4_h.Init.ClockDivision     = 0;
  tim4_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim4_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim4_h ) != HAL_OK ) {
    UVAR('e') = 114; // like error
    return;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if( HAL_TIM_SlaveConfigSynchronization( &tim4_h, &sSlaveConfig ) != HAL_OK ) {
    UVAR('e') = 1114;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &tim4_h, &sMasterConfig ) != HAL_OK )  {
    UVAR('e') = 1123;
  }

  TIM_N_L->CR1 |= 1; // test
}


void tim14_cfg()
{
  tim14_h.Instance               = TIM_SERVO;
  // cnt_freq: 1MHz,
  tim14_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM_SERVO, 1000000 );
  tim14_h.Init.Period            = 9999; // 100 Hz, pwm in us 500:2500
  tim14_h.Init.ClockDivision     = 0;
  tim14_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim14_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim14_h ) != HAL_OK ) {
    UVAR('e') = 119; // like error
    return;
  }

  HAL_TIM_PWM_Init( &tim14_h );

  HAL_TIM_PWM_Stop( &tim14_h, TIM_CHANNEL_1 );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.Pulse = us_dir_zero;
  HAL_TIM_PWM_ConfigChannel( &tim14_h, &tim_oc_cfg, TIM_CHANNEL_1 );

  HAL_TIM_PWM_Start( &tim14_h, TIM_CHANNEL_1 );
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM1 ) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    /** TIM1:  A8 ---> TIM1_CH1,  A9  ---> TIM1_CH2,  A10  ---> TIM1_CH3,  PA11 ---> TIM1_CH4   */
    T1_ALL_GPIO_Port.cfgAF_N( T1_1_M_Right_Pin | T1_2_M_Left_Pin | T1_3_US_Pulse_Pin | T1_4_US_Echo_Pin, GPIO_AF1_TIM1 );
  }
  else if( tim_baseHandle->Instance == TIM_N_R ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    /** TIM_N_L:  A6     ------> TIM3_CH1  */
    GpioA.cfgAF_N( T3_1_M_count_R_Pin, GPIO_AF2_TIM3 );
  }
  else if( tim_baseHandle->Instance == TIM_N_L ) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    /** TIM4:  B6     ------> TIM4_CH1  */
    T4_1_M_count_l_GPIO_Port.cfgAF_N( T4_1_M_count_l_Pin, GPIO_AF2_TIM4 );
  }
  else if( tim_baseHandle->Instance == TIM_SERVO ) {
    __HAL_RCC_TIM14_CLK_ENABLE();
    /** TIM14 GPIO Configuration   A7     ------> TIM14_CH1     */
    T14_1_servo_GPIO_Port.cfgAF_N( T14_1_servo_Pin, GPIO_AF9_TIM14 );
  }
}


void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM1 )   {
    HAL_NVIC_DisableIRQ( TIM1_CC_IRQn );
    __HAL_RCC_TIM1_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM_N_R  ) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM_N_L  ) {
    __HAL_RCC_TIM4_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM_SERVO ) {
    __HAL_RCC_TIM14_CLK_DISABLE();
  }
}
