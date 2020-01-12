#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;

TIM_HandleTypeDef htim2;
void MX_TIM2_Init();


uint32_t xxn = 0;

mu_t mu0 = 0;


BOARD_DEFINE_LEDS;

int main(void)
{
  STD_PROLOG_START;

  leds.write( 0 );
  MX_TIM2_Init();
  HAL_TIM_Base_Start_IT( &htim2 );

  while(1) {
    leds.toggle( BIT1 );
    if( xxn > 1 ) {
      // leds.toggle( BIT0 );
      leds.set( BIT0 );
    }

    leds.set( BIT2 );
    mu_lock( &mu0 );
    if( xxn == 0 ) {
      delay_ms( 1 );
      ++xxn;
    } else if( xxn == 1 ) {
      delay_ms( 1 );
      --xxn;
    }
    mu_unlock( &mu0 );
    leds.reset( BIT2 );

    delay_ms( 100 );
  }
  return 0;
}

void TIM2_IRQHandler(void)
{
  leds.toggle( BIT3 );

  //  mu_lock( &mu0 ) - bad in IRQ!
  if( mu_trylock( &mu0 ) == 0 ) {
    if( xxn == 0 ) {
      ++xxn;
    } else if( xxn == 1 ) {
      --xxn;
    }
    mu_unlock( &mu0 );
  }

  HAL_TIM_IRQHandler( &htim2 );
  leds.reset( BIT3 );
}

void MX_TIM2_Init()
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  // htim2.Init.Prescaler = 71;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  #if defined (STM32F1) || defined (STM32F2) || defined (STM32F3) || defined (STM32F7) || defined (STM32H7)
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  #endif
  if( HAL_TIM_Base_Init( &htim2 ) != HAL_OK ) {
    die4led( 0 );
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim2, &sClockSourceConfig ) != HAL_OK ) {
    die4led( 1 );
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim2, &sMasterConfig) != HAL_OK )   {
    die4led( 2 );
  }

}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    HAL_NVIC_SetPriority( TIM2_IRQn, 1, 0 );
    HAL_NVIC_EnableIRQ( TIM2_IRQn );
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM2_IRQn );
  }
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

