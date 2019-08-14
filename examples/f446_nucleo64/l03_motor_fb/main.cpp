#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to control motor with feedback" NL;

TIM_HandleTypeDef htim8;
int  MX_TIM8_Init(void);
void HAL_TIM_Base_MspInit(  TIM_HandleTypeDef* tim_baseHandle );
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle );


volatile uint16_t sensors_state;
void init_exti_pins();
PinsOut ctlEn( GpioA, 8, 1 );
PinsOut ctlAB( GpioA, 9, 2 );

uint32_t wait_with_cond( uint32_t ms, uint16_t n_hwticks, uint32_t *rc, uint32_t *dp, uint32_t ign_bits = 0 );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10;
  UVAR('g') = 10;
  UVAR('n') = 1;
  UVAR('s') = 10;

  ctlEn.initHW();  ctlEn.write( 0 );
  ctlAB.initHW();  ctlAB.write( 0 );
  sensors_state = 0;
  init_exti_pins();
  if( MX_TIM8_Init() != 0 ) {
    die4led( 0x0F );
  }

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// EXTI IRQ sensors B1, B2:
void init_exti_pins()
{
  GpioB.enableClk();
  GpioB.cfgIn_N( GPIO_PIN_1 | GPIO_PIN_2, GpioRegs::Pull::down );
  GpioB.setEXTI( 1, GpioRegs::ExtiEv::updown );
  GpioB.setEXTI( 2, GpioRegs::ExtiEv::updown );

  HAL_NVIC_SetPriority( EXTI1_IRQn, 12, 0 );
  HAL_NVIC_EnableIRQ( EXTI1_IRQn );
  HAL_NVIC_SetPriority( EXTI2_IRQn, 12, 0 );
  HAL_NVIC_EnableIRQ( EXTI2_IRQn );
}

void EXTI1_IRQHandler()
{
  if( GpioB.IDR & GPIO_PIN_1 ) {
    leds.set( 2 );
    sensors_state |= 1;
  } else {
    leds.reset( 2 );
    sensors_state &= ~1;
  }

  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_IRQHandler()
{
  if( GpioB.IDR & GPIO_PIN_2 ) {
    leds.set( 4 );
    sensors_state |= 2;
  } else {
    leds.reset( 4 );
    sensors_state &= ~2;
  }
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
}

uint32_t wait_with_cond( uint32_t ms, uint16_t n_hwticks, uint32_t *rc, uint32_t *dp, uint32_t ign_bits )
{
  uint32_t lrc = 0x10, w = 0, ldp = 0; // 0x10 bit = to many loops
  uint32_t tc0 = HAL_GetTick();
  ign_bits = ~ign_bits;

  TIM8->CNT = 0;
  TIM8->CR1 |= 1;

  uint32_t i;
  for( i=0; i<1000000000; ++i ) {
    if( ( ldp = TIM8->CNT ) >= n_hwticks ) {
      lrc = 0; break; // req number of ticks
    }
    if( sensors_state & ign_bits ) {
      lrc = sensors_state; break;
    }
    if( (i & 0xFF) == 0xFF ) {
      if( ( w = HAL_GetTick() - tc0 ) >= ms ) {
        lrc = 0x20; break; // overtime
      }
    }
  }

  TIM8->CR1 &= ~1;

  if( rc ) {
    *rc = lrc;
  }
  if( dp ) {
    *dp = ldp;
  }
  pr_sdx( i );
  return w;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int ts     = arg2long_d( 1, argc, argv, UVAR('s'), 0 );
  int tg     = arg2long_d( 2, argc, argv, UVAR('g'), 0 );
  int dir    = arg2long_d( 3, argc, argv, 0, 0, 1 );
  int n_step = arg2long_d( 4, argc, argv, UVAR('n'), 1, 10000 );
  uint32_t t_step = UVAR('t');
  std_out <<  NL "Test0: ts= " <<  ts << " tg= " <<  tg << " t= " <<  t_step
     <<  " n_step= " << n_step << NL;

  leds.reset( 1 );
  ctlEn.write( 0 );
  ctlAB.write( dir ? 1 : 2 );

  break_flag = 0;
  uint32_t src = 0, w = 0, dp = 0;
  for( int i=0; i<n_step && !break_flag; ++i ) {
    leds.set( 1 ); ctlEn.set( 1 );
    w = wait_with_cond( tg, ts, &src, &dp, dir ? 2 : 1 );
    leds.reset( 1 ); ctlEn.reset( 1 );
    if( src ) {
      break;
    }
    delay_ms( t_step );
  }

  ctlEn.write( 0 );  ctlAB.write( 0 );
  leds.reset( 1 );
  pr_sdx( sensors_state );  pr_sdx( w );  pr_sdx( dp ); pr_sdx( src );

  return 0;
}


int  MX_TIM8_Init(void)
{
  TIM_SlaveConfigTypeDef sSlaveConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 65535;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &htim8 ) != HAL_OK ) {
    return 1;
  }

  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if( HAL_TIM_SlaveConfigSynchronization(&htim8, &sSlaveConfig) != HAL_OK )
  {
    return 2;
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK )  {
    return 3;
  }
  return 0;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM8 )  {
    __HAL_RCC_TIM8_CLK_ENABLE();
    /** TIM8 GPIO Configuration  A0     ------> TIM8_ETR     */
    GpioA.cfgAF( 0, GPIO_AF3_TIM8 );

    /* TIM8 interrupt Init */
    // HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 3, 0 );
    // HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn );
    // HAL_NVIC_SetPriority(TIM8_CC_IRQn, 3, 0 );
    // HAL_NVIC_EnableIRQ( TIM8_CC_IRQn );
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance==TIM8 )
  {
    __HAL_RCC_TIM8_CLK_DISABLE();

    /* TIM8 interrupt Deinit */
    // HAL_NVIC_DisableIRQ( TIM8_UP_TIM13_IRQn );
    // HAL_NVIC_DisableIRQ( TIM8_CC_IRQn );
  }
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

