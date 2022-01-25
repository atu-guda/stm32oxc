#include <oxc_auto.h>

#include <oxc_modbus_rtu_client.h>

#include <cstring>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

UART_HandleTypeDef huart1;

int MX_TIM11_Init();
TIM_HandleTypeDef htim11;

const char* common_help_string = "Appication to test MODBUS RTU client" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " - print and clear ibuf"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OUT,
  nullptr
};

// USART_TypeDef*, volatile uint32_t*
MODBUS_RTU_client m_cli( UART_MODBUS, &(TIM11->CNT) );
void tick_for_modbus();

void idle_main_task()
{
  // leds.toggle( 1 );
}


void tick_for_modbus()
{
  m_cli.handle_tick();
}


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =   4;
  UVAR('a') =  '@';

  MX_TIM11_Init();
  MX_MODBUS_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( tick_for_modbus );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  uint32_t tm0 = HAL_GetTick();

  uint32_t tc0 = tm0, tc00 = tm0;
  uint16_t  txx0 = TIM11->CNT;


  uint32_t tmc_prev = tc0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcc = HAL_GetTick();
    uint16_t  txx1 = TIM11->CNT;
    uint16_t  txx2 = TIM11->CNT;
    if( i == 0 ) {
      txx0 = txx1;
    }
    uint16_t txx_d = txx1 - txx0;
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 )
            << " dlt= " << ( tcc - tmc_prev ) << ' '
            << txx1 << ' ' << txx2 << ' ' << txx_d << NL;
    tmc_prev = tcc;
    txx0 = txx1;

    UART_MODBUS->USART_TX_REG = (uint8_t)UVAR('a');

    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

int cmd_out( int argc, const char * const * argv )
{
  auto pos = m_cli.get_ibuf_pos();
  std_out <<  "# ibuf_pos:  " << pos <<  NL;
  auto ibuf = m_cli.get_ibuf();

  dump8( ibuf, (pos+15) & 0x0FFF0 );
  m_cli.reset();
  return 0;
}


// Timer parts

int MX_TIM11_Init()
{
  htim11.Instance               = TIM11;
  htim11.Init.Prescaler         = 1679; // TODO: calc
  htim11.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim11.Init.Period            = 65535;
  htim11.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim11 ) != HAL_OK ) {
    return 0;
  }
  HAL_TIM_Base_Start( &htim11 );
  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim_base )
{
  if( htim_base->Instance == TIM11 ) {
    __HAL_RCC_TIM11_CLK_ENABLE();
    // HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 1, 0);
    // HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
  }
}

void UART_MODBUS_IRQHANDLER(void)
{
  // leds.set( 2 );
  m_cli.handle_UART_IRQ();
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

