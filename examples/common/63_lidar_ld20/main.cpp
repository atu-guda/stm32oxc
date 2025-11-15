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
DCL_CMD_REG( test0,  'T',   " - test data" );
DCL_CMD_REG( abort,  'A',   " - Abort tranfer" );
DCL_CMD_REG( udump,  'U',   " - UART dump" );
DCL_CMD_REG( uread,  '\0',  " - try to read w/o DMA" );
DCL_CMD_REG( usend,  'Z',   " - try to send (debug)" );
DCL_CMD_REG( pr_sz,  '\0',  " - print and reset sizes (debug)" );


void idle_main_task()
{
  // leds.toggle( 1 );
}


extern DMA_HandleTypeDef hdma_usart_lidar_rx;
extern UART_HandleTypeDef huart_lidar;

const unsigned ubsz { 64 };
char ubuf[ubsz]; // 47 + round
char xbuf[ubsz]; // debug copy
char ybuf[ubsz];
Lidar_LD20_Data    lidar_d;
Lidar_LD20_Handler lidar_h;

std::array<uint32_t,128> r_szs; // size of received
std::array<uint32_t,128> r_val; // values of received
unsigned n_szs {0};

int main(void)
{
  BOARD_PROLOG;

  UVAR('d') =  0;
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

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
  leds.set( 4 );
  if( !huart || huart->Instance != UART_LIDAR_LD20 ) {
    return;
  }
  ++UVAR('j');
  unsigned sz = huart->RxXferSize;
  if( sz > ubsz ) {
    sz = ubsz;
  }

  memset( xbuf, '\0', ubsz );
  // SCB_InvalidateDCache_by_Addr( ubuf, ubsz );
  // std::memcpy( dst, ubuf, std::min(sz,(uint16_t)Lidar_LD20_Data::pkgSize) );
  lidar_h.add_bytes( (const uint8_t*)ubuf, sz );
  std::memcpy( xbuf, ubuf, sz );
  memset( ubuf,  '\0', sizeof( ubuf ) ); // remove after debug

  leds.reset( 4 );
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef *huart )
{
  ++UVAR('e');
}


int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  // uint32_t abrt = arg2long_d( 2, argc, argv, 0, 0, 1 );
  std_out << "# T1: n= " << n << NL;

  memset( ubuf,  0x00, sizeof( ubuf ) );

  std_out << "# gstate: " << huart_lidar.gState << " RxState: " << huart_lidar.RxState << NL;
  if( huart_lidar.RxState == HAL_UART_STATE_READY ) {
    __HAL_USART_ENABLE( &huart_lidar );
    auto h_rc = HAL_UART_Receive_DMA( &huart_lidar, (uint8_t*)&ubuf, sizeof(ubuf) );
    if( h_rc != HAL_OK ) {
      std_out << "# Error: fail to RectToIdle: " << h_rc << NL;
      // return 5;
    }
  }


  unsigned old_nf { 0 };
  int ts { 0 }, old_ts { -1 };
  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( decltype (+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tc = HAL_GetTick();

    if( UVAR('d') > 1 ) {
      at_disabled_irq( []() { std::memcpy(ybuf, xbuf, ubsz); } );
      dump8( ybuf, ubsz );
    }
    auto nf = lidar_h.get_nf();

    if( UVAR('d') > 0 ) {
      std_out << "#  " << ( tc - tm00 ) << ' ' << UVAR('i') << ' '  << nf << NL;
    }

    if( nf != old_nf ) {
      at_disabled_irq( []() { lidar_d = *lidar_h.getData(); } );

      do {
        ts = lidar_d.ts; // find bad data. stage 1: bad d(ts)
        if( old_ts < 0 ) { old_ts = ts-1; }
        int d_ts = ts - old_ts;
        if( d_ts < 0 ) {
          d_ts += 30000;
        }
        if( d_ts > 300 ) { // TODO: param, TODO: drop only one
          break;
        }
        int d_alp = lidar_d.alp_en - lidar_d.alp_st;
        if( d_alp < 0 ) {
          d_alp += 36000;
        }
        if( d_alp > 1000 ) {
          break;
        }
        int step_alp = d_alp / Lidar_LD20_Data::n_samples;

        std_out << "#a " << nf << ' ' << lidar_d.alp_st  << ' ' << lidar_d.alp_en
          << ' ' << (lidar_d.alp_en - lidar_d.alp_st) << ' ' << d_ts << ' ' << ( tc - tm00 ) << NL;

        for( int i=0; i < Lidar_LD20_Data::n_samples; ++i ) {
          int alp = ( lidar_d.alp_st + i * step_alp ) % 36000;
          std_out << alp << ' ' << lidar_d.d[i].l << ' ' << lidar_d.d[i].v << NL;
        }

        if( UVAR('d') > 0 ) {
          dump8( lidar_d.cdata(), ubsz );
        }
      }  while( false );
      old_ts = ts;
    }
    old_nf = nf;

    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}

int cmd_abort( int argc, const char * const * argv )
{
  HAL_UART_Abort( &huart_lidar );
  __HAL_USART_DISABLE( &huart_lidar );
  lidar_h.reset();
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

