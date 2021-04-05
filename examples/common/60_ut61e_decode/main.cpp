#include <cstring>

#include <oxc_auto.h>

#include <oxc_ut61e_decode.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to decode data from UT61e via UART" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}


extern DMA_HandleTypeDef hdma_usart_ut61e_rx;
extern UART_HandleTypeDef huart_ut61e;
volatile uint32_t r_sz = 0;
char ubuf[16], ubuf1[16];


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') = 2000;

  MX_UT61E_DMA_Init();
  UVAR('z') =  MX_UT61E_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef *huart, uint16_t sz )
{
  r_sz = sz;
  if( r_sz == 0 ) {
    r_sz = 50;
  }

  if( sz == 14 ) {
    memmove( ubuf1, ubuf, 14 );
    // memset( ubuf,  0, sizeof( ubuf ) );
  }
  leds.toggle( 1 );
}

int cmd_test0( int argc, const char * const * argv )
{

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  std_out << "# T1: n= " << n << NL;

  uint16_t nr = 0;
  r_sz = 0;
  memset( ubuf,  0x00, sizeof( ubuf ) );
  memset( ubuf1, 0x00, sizeof( ubuf1 ) );

  // HAL_UARTEx_ReceiveToIdle_DMA( &huart_ut61e, (uint8_t*)&ubuf, 14 );

  bool restart = true;
  break_flag = 0;
  for( decltype (+n) i=0; i<n && !break_flag; ++i ) {
    // leds.toggle( 4 );
    // memset( ubuf, 0, sizeof( ubuf ) );
    // HAL_StatusTypeDef  rc = HAL_UART_Receive( &huart_ut61e, (uint8_t*)&ubuf, 14, 1000 );
    HAL_UARTEx_ReceiveToIdle( &huart_ut61e, (uint8_t*)&ubuf, 14, &nr, 1000 );
    // if( restart ) {
    //   restart = false;
    //   HAL_UARTEx_ReceiveToIdle_DMA( &huart_ut61e, (uint8_t*)&ubuf, 14 );
    // }
    // if( rc != HAL_OK ) {
    //   // std_out << "# warn: rc= " << rc <<  NL;
    //   continue;
    // }
    dump8( ubuf, 16 );
    // if( r_sz != 0 ) {
    //   std_out << "#  r_sz= " << r_sz <<  NL;
    //
    // }
    // if( r_sz == 14 ) {
    //   restart = true;
    //   dump8( ubuf, 16 );
    //
    // }
    r_sz = 0;
    delay_ms( 2 );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

