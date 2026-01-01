#include <stdio.h>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_ticker.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

TIM_HandleTypeDef htim2;
int MX_TIM2_Init(void);
volatile int dt_irq = 0;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - "  );



int delay_led_step { 500 };

int on_delay_actions()
{
  // static OxcTicker delay_tick( &delay_led_step, 1 );
  // if( delay_tick.isTick() ) {
  //   leds[1].toggle();
  // }
  return 0;
}


void xxx_main_loop_nortos( SmallRL *sm, AuxTickFun f_idle )
{
  if( !sm ) {
    die4led( 0_mask );
  }

  UVAR_i = 0;

  // eat pre-input
  reset_in( 0 );
  for( unsigned i=0; i<256; ++i ) {
    auto v = tryGet( 0 );
    if( v.empty() ) {
      break;
    }
    ++UVAR_i;
  }

  srl.re_ps(); srl.reset();

  while( true ) {
    //leds[2].set();
    auto v = tryGet_irqdis( 0 );
    //leds.reset( 2 );

    if( v.good() ) {
      sm->addChar( v.c );
    } else {
      if( f_idle ) {
        f_idle();
      }

      if( UVAR_q > 0 ) {
        delay_ms( UVAR_q );
      } else {
        on_delay_actions();
      }

    }
  }
}

int main(void)
{
  STD_PROLOG_UART;

  UVAR_t = 100;
  UVAR_n =  10;
  UVAR_q =   0;
  UVAR_l =   0; // delay type

  MX_TIM2_Init();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME " #####################"  NL );


  oxc_add_aux_tick_fun( led_task_nortos );

  xxx_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR_n, 0 );
  uint32_t t_step = arg2long_d( 2, argc, argv, UVAR_t, 0, 100000 );
  int tp = arg2long_d( 3, argc, argv, UVAR_v, 0, 10 );
  std_out <<  "# test_delays : n= " << n << " t= " << t_step << " tp= " << tp << NL;

  TickType tc0 = GET_OS_TICK(), tc00 = tc0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    TickType tcc = GET_OS_TICK();
    float v1 = 3600 * 166.66f / (float)(dt_irq);
    std_out << i << ' ' << ( tcc - tc00 ) <<  ' ' << dt_irq << ' ' << v1
      << ' ' << TIM2->CNT << ' ' << TIM2->CCR1 << NL;

    delay_ms_until_brk( &tc0, t_step );

  }

  std_out << get_TIM_in_freq( TIM2 ) << ' ' << get_TIM_cnt_freq( TIM2 ) << NL;

  return 0;
}

int MX_TIM2_Init(void)
{
  htim2.Instance               = TIM2;
  htim2.Init.Prescaler         = 83;
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = 4294967295;
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim2, &sClockSourceConfig ) != HAL_OK )  {
    return 0;
  }

  if( HAL_TIM_IC_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim2, &sMasterConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_IC_InitTypeDef sConfigIC;
  //sConfigIC.ICPolarity  = TIM_ICPOLARITY_BOTHEDGE;
  sConfigIC.ICPolarity  = TIM_ICPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if( HAL_TIM_IC_ConfigChannel( &htim2, &sConfigIC, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }

  if( HAL_TIM_IC_Start_IT( &htim2, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR_e = 23;
    return 0;
  }


  return 1;

}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim_base )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if( htim_base->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority( TIM2_IRQn, 14, 0 );
    HAL_NVIC_EnableIRQ( TIM2_IRQn );
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if( htim_base->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
    HAL_NVIC_DisableIRQ( TIM2_IRQn );
  }
}

void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &htim2 );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  uint32_t cap2;
  static uint32_t c_old = 0xFFFFFFFF;
  leds[2].toggle();

  if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 )  {
    cap2 = HAL_TIM_ReadCapturedValue( htim, TIM_CHANNEL_1 );
    if( cap2 > c_old ) {
      dt_irq = cap2 - c_old ;
    }
    c_old = cap2;

    UVAR_m = cap2;
    UVAR_z = htim->Instance->CNT;
  }
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

