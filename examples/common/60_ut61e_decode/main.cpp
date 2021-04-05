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
  UVAR('n') = 20;

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

  // TODO: correct mutex
  if( sz == UT61E_PKT_SZ ) {
    for( unsigned i=0; i<UT61E_PKT_SZ; ++i ) {
      ubuf1[i] = ubuf[i] & 0x7F; // parity bit ???
    }
    memset( ubuf,  0, sizeof( ubuf ) );
  }

  // leds.toggle( 1 );
}

int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  std_out << "# T1: n= " << n << NL;

  r_sz = 0;
  memset( ubuf,  0x00, sizeof( ubuf ) );
  memset( ubuf1, 0x00, sizeof( ubuf1 ) );

  HAL_UARTEx_ReceiveToIdle_DMA( &huart_ut61e, (uint8_t*)&ubuf, UT61E_PKT_SZ );

  for( unsigned i=0; i<1000; ++i ) { // try to wait for initial transfer
    if( r_sz == UT61E_PKT_SZ ) {
      break;
    }
    delay_ms( 1 );
  }

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( decltype (+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tc = HAL_GetTick();

    // TODO: correct mutex
    std_out << ( tc - tm00 ) << NL;
    dump8( ubuf1, 16 );

    r_sz = 0;
    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

