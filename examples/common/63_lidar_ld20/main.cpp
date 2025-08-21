#include <array>
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
DCL_CMD( test0,  'T',   " - test data" );
DCL_CMD( abort,  'A',   " - Abort tranfer" );
DCL_CMD( udump,  'U',   " - UART dump" );
DCL_CMD( uread,  '\0',  " - try to read w/o DMA" );
DCL_CMD( usend,  'Z',   " - try to send (debug)" );
DCL_CMD( pr_sz,  '\0',  " - print and reset sizes (debug)" );

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_test0,
  &CMDINFO_abort,
  &CMDINFO_udump,
  &CMDINFO_uread,
  &CMDINFO_usend,
  &CMDINFO_pr_sz,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}


extern DMA_HandleTypeDef hdma_usart_lidar_rx;
extern UART_HandleTypeDef huart_lidar;
volatile uint32_t r_sz = 0;

const unsigned ubsz { 64 };
char ubuf[ubsz]; // 47 + round
char xbuf[ubsz]; // debug copy
char ybuf[ubsz];
Lidar_LD20_Data lidar_pkg;

std::array<uint32_t,128> r_szs; // size of received
std::array<uint32_t,128> r_val; // values of received
unsigned n_szs {0};

int main(void)
{
  BOARD_PROLOG;

  UVAR('d') =  1;
  UVAR('n') =  4;
  UVAR('t') = 10;

  // SCB_DisableICache();  SCB_DisableDCache();

  MX_LIDAR_LD20_DMA_Init();
  UVAR('z') = MX_LIDAR_LD20_UART_Init();
  __HAL_USART_DISABLE( &huart_lidar );


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
  const auto ev_type  = HAL_UARTEx_GetRxEventType( huart );
  if( ev_type != HAL_UART_RXEVENT_TC ) {
    return;
  }
  // HAL_UART_RXEVENT_TC    0   RxEvent linked to Transfer Complete event
  // HAL_UART_RXEVENT_HT    1   RxEvent linked to Half Transfer event
  // HAL_UART_RXEVENT_IDLE  2   RxEvent linked to IDLE event

  auto u_isr = UART_LIDAR_LD20->USART_SR_REG;
  __HAL_USART_CLEAR_IDLEFLAG( huart );

  if( u_isr & USART_ISR_IDLE ) {
    ++UVAR('l');
  }
  if( u_isr & USART_ISR_ORE ) {
    ++UVAR('o');
  }
  if( n_szs < r_szs.size() ) {
    r_val[n_szs  ] = ev_type;
    r_szs[n_szs++] = sz;
  }

  // if( sz != Lidar_LD20_Data::pkgSize ) {
  //   return;
  // }
  r_sz = sz;

  // TODO: correct mutex/flag
  SCB_InvalidateDCache_by_Addr( ubuf, ubsz );
  uint8_t *dst = (uint8_t*)(&lidar_pkg);
  memset( dst,  '\0', Lidar_LD20_Data::pkgSize );
  memset( xbuf, '\0', ubsz );
  std::memcpy( dst, ubuf, std::min(sz,(uint16_t)Lidar_LD20_Data::pkgSize) );
  std::memcpy( xbuf, ubuf,std::min(sz,(uint16_t)ubsz) );
  ++UVAR('k');
  memset( ubuf,  '\0', sizeof( ubuf ) );

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

  __HAL_USART_ENABLE( &huart_lidar );
  if( abrt ) {
    HAL_UART_Abort( &huart_lidar ); // test
  }

  // TODO: check ongouing
  // TODO: try w/o ToIdle - idle state not detected (why?)
  auto h_rc = HAL_UARTEx_ReceiveToIdle_DMA( &huart_lidar, (uint8_t*)&ubuf, sizeof(ubuf) );
  // auto h_rc = HAL_UART_Receive_DMA( &huart_lidar, (uint8_t*)&ubuf, sizeof(ubuf) );
  if( h_rc != HAL_OK ) {
    std_out << "# Error: fail to RectToIdle: " << h_rc << NL;
    // return 5;
  }


  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( decltype (+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tc = HAL_GetTick();

    // if( UVAR('d') > 0 ) {
    //   dump8( &lidar_pkg, 0x30 );
    // }

    if( UVAR('d') > 0 ) {
      at_disabled_irq( []() {std::memcpy(ybuf,xbuf,ubsz);} );
      dump8( ybuf, ubsz );
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

// to check timings - remove after debug
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

int cmd_pr_sz( int argc, const char * const * argv )
{
  for( unsigned i=0; i< n_szs; ++i ) {
    std_out << "# " << i << ' ' << r_szs[i] << ' ' << HexInt8( r_val[i] ) << NL;
  }
  n_szs = 0;
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

