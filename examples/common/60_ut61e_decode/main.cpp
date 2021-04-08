#define _GNU_SOURCE
#include <cstring>
#include <cmath>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <oxc_ut61e_decode.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to decode data from UT61e via UART" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test UT61E data"  };
int cmd_si_test( int argc, const char * const * argv );
CmdInfo CMDINFO_SI_TEST { "si_test", 'S', cmd_si_test, " - si units "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SI_TEST,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}


extern DMA_HandleTypeDef hdma_usart_ut61e_rx;
extern UART_HandleTypeDef huart_ut61e;
volatile uint32_t r_sz = 0;

char ubuf[16];
UT61E_package ut61e_pkg;


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

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
  uint8_t *dst = (uint8_t*)(&ut61e_pkg);
  if( sz == UT61E_PKT_SZ ) {
    for( unsigned i=0; i<UT61E_PKT_SZ; ++i ) {
      dst[i] = ubuf[i] & 0x7F; // parity bit ???
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
  // memset( ubuf1, 0x00, sizeof( ubuf1 ) );

  HAL_UARTEx_ReceiveToIdle_DMA( &huart_ut61e, (uint8_t*)&ubuf, UT61E_PKT_SZ );

  for( unsigned i=0; i<1000; ++i ) { // try to wait for initial transfer
    if( r_sz == UT61E_PKT_SZ ) {
      break;
    }
    delay_ms( 1 );
  }

  char flg_buf[80];
  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( decltype (+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tc = HAL_GetTick();

    if( ! ut61e_pkg.is_good() ) {
      std_out << "# err: bad package" << NL;
      dump8( &ut61e_pkg, 16 );
      continue;
    }

    if( UVAR('d') > 0 ) {
      dump8( &ut61e_pkg, 16 );
    }

    std_out << ( tc - tm00 ) << ' ';

    // TODO: correct mutex
    int32_t ival = ut61e_pkg.ival();
    int32_t p10  = ut61e_pkg.range_exp();
    if( ival >= UT61E_package::ol_ival ) {
      p10 = 30;
    }
    ut61e_pkg.flagsStr( flg_buf );
    const char *vname = ut61e_pkg.value_name();

    char si_chr;
    float v = ival * exp10f( p10 );
    float v1 = to_SI_prefix( v, &si_chr );

    std_out << FltFmt( v,  cvtff_auto, 11, 4 ) << ' ' << vname << ' '
            << FltFmt( v1, cvtff_auto, 11, 4 ) << ' ' << si_chr << vname << ' '
            << ival << 'e' << p10  << ' ' << vname << ' '
            << ut61e_pkg.func_name()  << ' ' << ut61e_pkg.func_idx() << ' ' << flg_buf << NL;

    r_sz = 0;
    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}

int cmd_si_test( int argc, const char * const * argv )
{
  xfloat v = 2.1345e-30;

  char c;
  for( ; v < 1e30f; v *= -10 ) {
    xfloat v1 = to_SI_prefix( v, &c );
    std_out << v << ' ' <<  v1 << ' ' <<  c << NL;
  }
  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

