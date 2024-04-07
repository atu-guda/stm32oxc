#include <errno.h>

#include <oxc_auto.h>


TIM_HandleTypeDef htim2;

static TIM_MasterConfigTypeDef sMasterConfig_def =
{ .MasterOutputTrigger = TIM_TRGO_RESET, .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE };


static int tim_base_config( TIM_HandleTypeDef *th );

int tim_base_config( TIM_HandleTypeDef *th )
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( th, &sClockSourceConfig ) != HAL_OK ) {
    return 0;
  }
  if( HAL_TIMEx_MasterConfigSynchronization( th, &sMasterConfig_def ) != HAL_OK ) {
    return 0;
  }
  return 1;
}


int MX_TIM2_Init()
{
  htim2.Instance           = TIM2;
  htim2.Init.Prescaler     = 0;
  htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim2.Init.Period        = 2000000; // TODO: param
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_PWM_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  if( ! tim_base_config( &htim2 ) ) {
    return 0;
  }


  TIM_OC_InitTypeDef sConfigOC;
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.Pulse      = 500; // TODO: set to 0 in production

  HAL_TIM_PWM_ConfigChannel( &htim2, &sConfigOC, TIM_CHANNEL_1 );
  HAL_TIM_PWM_Start( &htim2, TIM_CHANNEL_1 );

  return 1;
}


void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  //
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 )  { // PWM input
    __HAL_RCC_TIM2_CLK_ENABLE();
    //* A0 --> TIM2_CH1
    GpioA.cfgAF_N( GPIO_PIN_0, GPIO_AF1_TIM2 );
  }
}



void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 )  {
    __HAL_RCC_TIM2_CLK_DISABLE();
  }
}

