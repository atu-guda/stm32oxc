#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>


using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as counter" NL;

TIM_HandleTypeDef tim_cnt_h;
int MX_TIM_IN_Init();

// --- local commands;
DCL_CMD_REG( test0, 'T', "[t] [n] - test counter "  );
DCL_CMD_REG( tinit, 'I', " - reinit timer. f = filter"  );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 20;

  BOARD_POST_INIT_BLINK;

  std_out << NL "##################### " PROJ_NAME NL;

  MX_TIM_IN_Init();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int t = arg2long_d( 1, argc, argv, UVAR_t, 10, 100000 );
  int n = arg2long_d( 1, argc, argv, UVAR_n,  1,  10000 );

  for( int i=0; i<n && !break_flag; ++i ) {

    delay_ms( 10 ); // a-la actions

    TIM_IN_EXA->CNT = 0;
    delay_ms_brk( t );
    uint32_t cnt = TIM_IN_EXA->CNT;
    std_out << i << ' ' << cnt << NL;
  }


  // tim_print_cfg( TIM_EXA );

  return 0;
}

int cmd_tinit( int argc, const char * const * argv )
{
  // tim_cfg();
  // tim_print_cfg( TIM_EXA );
  MX_TIM_IN_Init();
  tim_print_cfg( TIM_IN_EXA );

  return 0;
}

int MX_TIM_IN_Init()
{
  tim_cnt_h.Instance           = TIM_IN_EXA;
  tim_cnt_h.Init.Prescaler     = 0;
  tim_cnt_h.Init.CounterMode   = TIM_COUNTERMODE_UP;
  tim_cnt_h.Init.Period        = 0xFFFFFFFF;
  tim_cnt_h.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_Base_Init( &tim_cnt_h ) != HAL_OK ) {
    errno = 1000;
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode       = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger    = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter   = UVAR_f; // 7
  if( HAL_TIM_SlaveConfigSynchro( &tim_cnt_h, &sSlaveConfig ) != HAL_OK ) {
    errno = 1001;
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &tim_cnt_h, &sMasterConfig ) != HAL_OK ) {
    errno = 1002;
    return 0;
  }
  TIM_IN_EXA->CR1 |= TIM_CR1_CEN;
  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_IN_EXA ) {
    TIM_IN_EXA_CLKEN;
    TIM_IN_EXA_PIN1.cfgAF( TIM_IN_EXA_GPIOAF );
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_IN_EXA ) {
    TIM_IN_EXA_CLKDIS;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

