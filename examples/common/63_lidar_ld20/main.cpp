#include <cstring>
#include <cmath>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>

#include <oxc_lidar_ld20.h>

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to decode LD20 lidar data via UART" NL;



// --- local commands;
DCL_CMD( test0, 'T',   " - test data" );
DCL_CMD( abort, 'A',   " - Abort tranfer" );
DCL_CMD( udump, 'U',   " - UART dump" );
DCL_CMD( uread, '\0',  " - try to read w/o DMA" );
DCL_CMD( usend, 'Z',   " - try to send" );

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_test0,
  &CMDINFO_abort,
  &CMDINFO_udump,
  &CMDINFO_uread,
  &CMDINFO_usend,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}


extern DMA_HandleTypeDef hdma_usart_lidar_rx;
extern UART_HandleTypeDef huart_lidar;
volatile uint32_t r_sz = 0;

char ubuf[64]; // 47 + round
Lidar_LD20_Data lidar_pkg;


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10;
  UVAR('n') = 10;

  MX_LIDAR_LD20_DMA_Init();
  UVAR('z') = MX_LIDAR_LD20_UART_Init();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef *huart, uint16_t sz )
{
  if( huart->Instance != UART_LIDAR_LD20 ) {
    return;
  }
  ++UVAR('j');
  // if( sz != Lidar_LD20_Data::pkgSize ) {
  //   return;
  // }
  r_sz = sz;

  // TODO: correct mutex
  uint8_t *dst = (uint8_t*)(&lidar_pkg);
  std::memcpy( dst, ubuf, Lidar_LD20_Data::pkgSize );
  ++UVAR('k');
  // memset( ubuf,  0, sizeof( ubuf ) );

  leds.toggle( 4 );
}

int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t abrt = arg2long_d( 2, argc, argv, 0, 0, 1 );
  std_out << "# T1: n= " << n << NL;

  r_sz = 0;
  memset( ubuf,  0x00, sizeof( ubuf ) );

  if( abrt ) {
    HAL_UART_Abort( &huart_lidar ); // test
  }
  // auto h_rc = HAL_UARTEx_ReceiveToIdle_DMA( &huart_lidar, (uint8_t*)&ubuf, Lidar_LD20_Data::pkgSize );
  auto h_rc = HAL_UARTEx_ReceiveToIdle_DMA( &huart_lidar, (uint8_t*)&ubuf, sizeof(ubuf) );
  if( h_rc != HAL_OK ) {
    std_out << "# Error: fail to RectToIdle: " << h_rc << NL;
    // return 5;
  }

  // for( unsigned i=0; i<1000; ++i ) { // try to wait for initial transfer
  //   if( r_sz == Lidar_LD20_Data::pkgSize ) {
  //     break;
  //   }
  //   delay_ms( 1 );
  // }

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( decltype (+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tc = HAL_GetTick();

    if( UVAR('d') > 0 ) {
      dump8( &lidar_pkg, 0x30 );
    }

    std_out << ( tc - tm00 ) << ' ' << UVAR('i') << ' '  << r_sz;


    std_out << NL;

    r_sz = 0;
    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}

int cmd_abort( int argc, const char * const * argv )
{
  HAL_UART_Abort( &huart_lidar );
  return 0;
}

int cmd_udump( int argc, const char * const * argv )
{
  dump32( UART_LIDAR_LD20, 0x30 );
  return 0;
}

int cmd_uread( int argc, const char * const * argv )
{
  memset( ubuf, '\0', sizeof( ubuf ));
  auto rc = HAL_UART_Receive( &huart_lidar, (uint8_t*)ubuf, Lidar_LD20_Data::pkgSize, 1000 );
  dump32( UART_LIDAR_LD20, 0x30 );
  dump8( ubuf, 48 );
  std_out << "# rc= " << rc << ' ' << Lidar_LD20_Data::pkgSize << NL;
  return 0;
}

// to check timings
int cmd_usend( int argc, const char * const * argv )
{
  memset( ubuf, '\x5a', sizeof( ubuf ));
  const uint8_t b1 { '\xA5' };
            HAL_UART_Transmit( &huart_lidar, (const uint8_t*) &b1,                        1, 1000 ); // start pulse
  delay_bad_mcs( 100 );
  auto rc = HAL_UART_Transmit( &huart_lidar, (const uint8_t*)ubuf, Lidar_LD20_Data::pkgSize, 1000 );
  dump32( UART_LIDAR_LD20, 0x30 );
  std_out << "# rc= " << rc << NL;
  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

