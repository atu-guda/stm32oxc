#include <oxc_auto.h>

#include <oxc_modbus_rtu_client.h>

#include <cstring>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

UART_HandleTypeDef huart1;

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

void idle_main_task()
{
  // leds.toggle( 1 );
}

// MODBUS_RTU_client m_cli;
char ibuf[256];
uint16_t ibuf_pos = 0;





int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 200;
  UVAR('n') =  20;
  UVAR('a') =  '@';

  MX_MODBUS_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  // UVAR('x') = UART_MODBUS->CR1;
  // UART_MODBUS->CR1 |=  UsartIO::CR1_UE  | UsartIO::CR1_RE | UsartIO::CR1_TE | UsartIO::CR1_RXNEIE;
  // UVAR('y') = UART_MODBUS->CR1;

  // log_add( "Test0 " );
  uint32_t tm0 = HAL_GetTick();

  uint32_t tc0 = tm0, tc00 = tm0;


  uint32_t tmc_prev = tc0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcc = HAL_GetTick();
    uint32_t tmc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 )
            << "  ms_tick= " << ( tmc - tm0 ) << " dlt= " << ( tmc - tmc_prev ) << ' '
            << HexInt16(UART_MODBUS->USART_SR_REG) << ' '
            << HexInt16(UART_MODBUS->USART_RX_REG) << NL;
    // leds.toggle( 1 );
    tmc_prev = tcc;

    UART_MODBUS->USART_TX_REG = (uint8_t)UVAR('a');


    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

int cmd_out( int argc, const char * const * argv )
{
  std_out <<  "# ibuf_pos:  " << ibuf_pos <<  NL;
  dump8( ibuf, (ibuf_pos+15) & 0x0FFF0 );
  ibuf_pos = 0;
  memset( ibuf, '\x00', size(ibuf) );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

