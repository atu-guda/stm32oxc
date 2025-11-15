#include <climits>

#include <oxc_auto.h>
#include <oxc_main.h>

// local configs requires
#ifndef DEFINES_FOR_WS2812
#error "Defines for WS2812 are required, may be in local_hal_conf.h"
#endif

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test WS2812 LED controller with timer and DMA" NL;

TIM_HandleTypeDef tim_h;
DMA_HandleTypeDef hdma_tim_chx;

extern const int8_t sin_table_8x8[256];
// char xlog_buf[2048];


void MX_DMA_Init();
int tim_cfg();

struct RGB8_Arr {
  RGB8_Arr( uint8_t *data, uint32_t size ) : d( data ), sz( size ) {};
  uint8_t* data() { return d; };
  const uint8_t* cdata() const { return d; };
  uint32_t size() const { return sz; }
  void clear() { for( unsigned i=0; i<sz*3; ++i ) { d[i] = 0; } }
  void set(  uint8_t r, uint8_t g, uint8_t b, uint32_t pos );
  void fill( uint8_t r, uint8_t g, uint8_t b, uint32_t begin = 0, uint32_t end = UINT_MAX );

  uint8_t *d;  // data, not owning
  uint32_t sz; // size in elements
};

void RGB8_Arr::set( uint8_t r, uint8_t g, uint8_t b, uint32_t pos )
{
  if( pos >= sz ) {
    return;
  }
  uint32_t p3 = pos * 3;
  d[p3] = r; d[p3+1] = g; d[p3+2] = b;
}

void RGB8_Arr::fill( uint8_t r, uint8_t g, uint8_t b, uint32_t begin, uint32_t end )
{
  if( end > sz ) {
    end = sz;
  }
  if( begin >= end ) {
    return;
  }
  for( uint32_t p3 = begin*3; p3 < end*3; p3 += 3 ) {
    d[p3] = r; d[p3+1] = g; d[p3+2] = b;
  }
}

const uint32_t max_leds = 128; // each LED require 3*8 = 24 bit
uint8_t lbuf[max_leds*3];
RGB8_Arr pbuf( lbuf, max_leds );

// TODO: move to oxc lib
struct WS2812_info {
  using tim_ch_t = decltype(TIM_CHANNEL_1);
  WS2812_info( TIM_HandleTypeDef *_tim_h_p, tim_ch_t t_ch ) : tim_h_p(_tim_h_p), tim_ch( t_ch ) {};
  static constexpr uint16_t size_1led = 3 * 8;
  static uint16_t t_min, t_max;     // CCR values for '0' and '1'
  uint16_t buf[ 2 * size_1led ];    // 2 chunks, each 24: 1 bit (CCR) for output via timer
  const uint8_t *ibuf = nullptr;
  int size = 0;
  volatile int pos = 0;
  volatile bool busy = false;
  TIM_HandleTypeDef *tim_h_p;
  const tim_ch_t tim_ch;
  //
  int send( const uint8_t *d, int sz );
  static void rgb2tim( uint8_t r, uint8_t g, uint8_t b, uint16_t *d );
  static void color2tim( uint8_t c, uint16_t *d );
  void fill_zeros() { for( auto &v : buf ) { v = 0; } }
  void fill_zeros0() { for( unsigned i=0; i<size_1led; ++i ) { buf[i] = 0; } }
  void fill_zeros1() { for( unsigned i=0; i<size_1led; ++i ) { buf[i+size_1led] = 0; } }
  void callback_half();
  void callback_full();
  static void calc_minmax( uint16_t arr );
};
uint16_t WS2812_info::t_min, WS2812_info::t_max;

void WS2812_info::calc_minmax( uint16_t arr )
{
  t_min = (uint16_t)( arr * 7 / 25 ); // * 0.35/1.25  = 0.28
  t_max = (uint16_t)( arr - t_min );
}

void WS2812_info::color2tim( uint8_t c, uint16_t *d )
{
  unsigned i = 0;
  for( uint8_t m=0x80; m; m>>=1, ++i ) {
    d[i] = ( c & m ) ? t_max : t_min;
  }
}

void WS2812_info::rgb2tim( uint8_t r, uint8_t g, uint8_t b, uint16_t *d )
{
  if( !d ) {
    return;
  }
  color2tim( g, d );
  color2tim( r, d+8 );
  color2tim( b, d+16 );
}

int WS2812_info::send( const uint8_t *d, int sz ) // size in leds: 3*sz bytes in d[] required
{
  sz &= 0xFFFE; // limit size and oddness
  UVAR('y') = sz;
  if( busy || !d || sz < 2 ) {
    return 0;
  }

  ibuf = d;
  size = sz; pos = 0;
  fill_zeros();
  busy = true;

  unsigned tw_ms = 2 + ( 1250 * sz + 120000 ) / 1000000;
  UVAR('f') = tw_ms;

  int rc_s = HAL_TIM_PWM_Start_DMA( tim_h_p, tim_ch, (uint32_t*)buf, 2 * size_1led );
  if( rc_s != HAL_OK ) {
    size = 0;  busy = false;
    return 0;
  }

  for( unsigned i=0; i<tw_ms; ++i ) {
    if( !busy ) {
      break;
    }
    delay_ms( 1 );
  }
  rc_s = !busy;

  size = 0; ibuf = nullptr;
  busy = false;
  return rc_s;
}

void WS2812_info::callback_half()
{
  uint16_t xpos = pos * 3;
  if( pos > size+1 ) {
    HAL_TIM_PWM_Stop_DMA( tim_h_p, tim_ch );
    // __HAL_TIM_DISABLE( tim_h_p );
  }

  if( pos < size ) {
    rgb2tim( ibuf[xpos], ibuf[xpos+1], ibuf[xpos+2], buf );
  } else {
    fill_zeros0();
  }
  ++pos;
}

void WS2812_info::callback_full()
{
  uint16_t xpos = pos * 3;
  if( pos > size+2 ) {
    busy = false;
  }

  if( pos < size ) {
    rgb2tim( ibuf[xpos], ibuf[xpos+1], ibuf[xpos+2], buf+24 );
  } else {
    fill_zeros1();
  }
  ++pos;
}

WS2812_info dsi( &tim_h, WS2812_TIM_CH );

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test LED output"  );
DCL_CMD_REG( pix, 'P', " r g b pos - set pixel"  );
DCL_CMD_REG( wave, 'W', " type t_scale l_scale - test wave"  );



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 20;
  UVAR('n') = 8;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  MX_DMA_Init();
  tim_cfg();
  // set_log_buf( xlog_buf, sizeof(xlog_buf) );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  unsigned v = arg2long_d( 1, argc, argv, 32, 0, 255 );
  unsigned n = UVAR('n');
  uint8_t v0 = UVAR('b'); //background


  std_out << "# Test: v= " << v <<  NL;

  pbuf.fill( v0, v0, v0, 0, n );

  pbuf.set( v, 0, 0, 0 );
  pbuf.set( 0, v, 0, 1 );
  pbuf.set( 0, 0, v, 2 );
  pbuf.set( v, v, v, 3 );


  UVAR('r') = dsi.send( pbuf.cdata(), n );

  return 0;
}

int cmd_pix( int argc, const char * const * argv )
{
  uint8_t r = (uint8_t)(arg2long_d( 1, argc, argv, 1, 0, 255 ));
  uint8_t g = (uint8_t)(arg2long_d( 2, argc, argv, 1, 0, 255 ));
  uint8_t b = (uint8_t)(arg2long_d( 3, argc, argv, 1, 0, 255 ));
  uint16_t p = (uint16_t)(arg2long_d( 4, argc, argv, 0, 0, max_leds ));

  std_out << "# pix: (" << r << ',' << g << ',' << b << ") " << p << NL;
  pbuf.set( r, g, b, p );
  UVAR('r') = dsi.send( pbuf.cdata(), UVAR('n') );
  return 0;
}

int cmd_wave( int argc, const char * const * argv )
{
  unsigned tp      = arg2long_d( 1, argc, argv,   0, 0,      0 ); // just synglee type for now
  unsigned t_scale = arg2long_d( 2, argc, argv, 100, 1, 100000 ); // time step scale, def = 100
  unsigned l_scale = arg2long_d( 3, argc, argv, 100, 1, 100000 ); // length step scale, def = 100
  unsigned n = UVAR('n');
  uint32_t ph[3];    // phases for first LED (R,G,B) // 32 bit, used for output only 8 MSB
  // openssl prime -generate -bits 24 -hex // 23, 22
  uint32_t d_ph[3] = { 0xEB7FB9, 0x671A9D, 0x33F443 } ;  // phases deltas

  for( auto &p : ph ) { p = 0; }


  std_out << "# Wave:  " <<  "  tp= " << tp <<  NL;

  unsigned n_t = 1000;
  unsigned t_step = 20; // ms
  uint32_t tm0 = HAL_GetTick();
  break_flag = 0;
  for( unsigned i=0; i<n_t && !break_flag; ++i ) {
    for( unsigned i=0; i<3; ++i ) {
      ph[i] += d_ph[i] * t_scale / 10;
    }
    if( UVAR('d') > 0 ) {
      std_out << "# " << i << NL;
    }

    for( unsigned j=0; j<n; ++j ) {
      unsigned ph_l = j * l_scale * 5000000;
      uint8_t r = 127 + sin_table_8x8[ ( ph_l + ph[0] ) >> 24 ];
      uint8_t g = 127 + sin_table_8x8[ ( ph_l + ph[1] ) >> 24 ];
      uint8_t b = 127 + sin_table_8x8[ ( ph_l + ph[2] ) >> 24 ];
      pbuf.set( r, g, b, j );
    }

    dsi.send( pbuf.cdata(), n );

    delay_ms_until_brk( &tm0, t_step );
  }

  pbuf.fill( 0, 0, 0, 0, n );
  dsi.send( pbuf.cdata(), n );

  return 0;
}

//  ----------------------------- configs ----------------

int tim_cfg()
{
  auto pbase = calc_TIM_arr_for_base_psc( TIM_EXA, 0, 800000 );
  UVAR('a') = pbase;
  WS2812_info::calc_minmax( pbase );

  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = 0;
  tim_h.Init.Period            = pbase;
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop( &tim_h, WS2812_TIM_CH );
  tim_oc_cfg.Pulse = 0; // pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, WS2812_TIM_CH ) != HAL_OK ) {
    UVAR('e') = 11;
    return 0;
  }
  return 1;
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( WS2812_TIM_PIN, TIM_EXA_GPIOAF );

  hdma_tim_chx.Instance                 = WS2812_DMA_INSTANCE;
  hdma_tim_chx.Init.Channel             = WS2812_DMA_CHANNEL;
  hdma_tim_chx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tim_chx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tim_chx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tim_chx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_tim_chx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_tim_chx.Init.Mode                = DMA_CIRCULAR;
  hdma_tim_chx.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_tim_chx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_tim_chx ) != HAL_OK ) {
    UVAR('e') = 14;
    return;
  }

  __HAL_LINKDMA( &tim_h, hdma[WS2812_TIM_DMA_ID], hdma_tim_chx );

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( WS2812_TIM_PIN );
}

void MX_DMA_Init()
{
  WS2812_DMA_ENABLE;

  HAL_NVIC_SetPriority( WS2812_DMA_IRQN, 8, 0 );
  HAL_NVIC_EnableIRQ(   WS2812_DMA_IRQN );

}


void WS2812_DMA_IRQHANDLER()
{
  // leds.toggle( BIT0 );
  HAL_DMA_IRQHandler( &hdma_tim_chx );
}


void HAL_TIM_PWM_PulseFinishedHalfCpltCallback( TIM_HandleTypeDef *htim )
{
  dsi.callback_half();
}

void  HAL_TIM_PWM_PulseFinishedCallback( TIM_HandleTypeDef *htim )
{
  dsi.callback_full();
}

void  HAL_TIM_ErrorCallback(  TIM_HandleTypeDef *htim )
{
  dsi.busy = false;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

