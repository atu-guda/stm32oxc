#include <oxc_auto.h>
#include <main.h>

TIM_HandleTypeDef tim_freq_meas_h;


int tim_freq_meas_cfg()
{
  auto &t_h { tim_freq_meas_h };
  t_h.Instance               = TIM_FREQ_MEAS;
  t_h.Init.Prescaler         = 0;
  t_h.Init.Period            = 0xFFFFFFFF;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &t_h ) != HAL_OK ) {
    UVAR_e = 1; // like error
    return 0;
  }

  static const TIM_SlaveConfigTypeDef sSlaveConfig = {
    .SlaveMode        = TIM_SLAVEMODE_EXTERNAL1,
    .InputTrigger     = TIM_TS_ETRF,
    .TriggerPolarity  = TIM_TRIGGERPOLARITY_NONINVERTED,
    .TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1,
    .TriggerFilter    = 0
  };
  if( HAL_TIM_SlaveConfigSynchro( &t_h, &sSlaveConfig ) != HAL_OK ) {
    UVAR_e = 2;
    return 0;
  }

  static const TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };
  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig ) != HAL_OK ) {
    UVAR_e = 4;
    return 0;
  }

  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_FREQ_MEAS ) {
    TIM_FREQ_MEAS_EN;
    MEAS_FREQ_PIN.cfgAF( TIM_FREQ_MEAS_GPIO_AF ); // PA0  --> T2.ETR
  } else if( tim_baseHandle->Instance == TIM_STEP ) {
    TIM_STEP_EN;
  } else if( tim_baseHandle->Instance == TIM_FLOW_PWM ) {
    TIM_FLOW_PWM_EN;
  }
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
  if( timHandle->Instance == TIM_STEP ) {
    STEP_PIN.cfgAF( TIM_STEP_GPIO_AF );
  } else if(timHandle->Instance == TIM_FLOW_PWM ) {
    FLOW_PWM_PIN.cfgAF( TIM_FLOW_PWM_GPIO_AF );
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 ) {
    TIM_FREQ_MEAS_DIS;
    MEAS_FREQ_PIN.cfgIn();
  } else if( tim_baseHandle->Instance == TIM_STEP ) {
    TIM_STEP_DIS;
    STEP_PIN.cfgIn();
  } else if( tim_baseHandle->Instance == TIM_FLOW_PWM ) {
    TIM_FLOW_PWM_DIS;
    FLOW_PWM_PIN.cfgIn();
  }
}



