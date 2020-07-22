#include <cstring>
#include <iterator>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test DS2812 LED controller with timer and DMA" NL;

TIM_HandleTypeDef tim_h;
DMA_HandleTypeDef hdma_tim_chx;

// char xlog_buf[2048];


void MX_DMA_Init();
int tim_cfg();
const uint32_t max_leds = 128; // each LED require 3*8 = 24 bit, each require halfword
uint8_t lbuf[max_leds*3];

// TODO: move to oxc lib
struct DS2812_info {
  using tim_ch_t = decltype(TIM_CHANNEL_1);
  DS2812_info( TIM_HandleTypeDef *_tim_h_p, tim_ch_t t_ch ) : tim_h_p(_tim_h_p), tim_ch( t_ch ) {};
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
uint16_t DS2812_info::t_min, DS2812_info::t_max;

void DS2812_info::calc_minmax( uint16_t arr )
{
  t_min = (uint16_t)( arr * 7 / 25 ); // * 0.35/1.25  = 0.28
  t_max = (uint16_t)( arr - t_min );
}

void DS2812_info::color2tim( uint8_t c, uint16_t *d )
{
  unsigned i = 0;
  for( uint8_t m=0x80; m; m>>=1, ++i ) {
    d[i] = ( c & m ) ? t_max : t_min;
  }
}

void DS2812_info::rgb2tim( uint8_t r, uint8_t g, uint8_t b, uint16_t *d )
{
  if( !d ) {
    return;
  }
  color2tim( g, d );
  color2tim( r, d+8 );
  color2tim( b, d+16 );
}

int DS2812_info::send( const uint8_t *d, int sz ) // size in leds: 3*sz bytes in d[] required
{
  sz &= 0xFFFE; // limit size ond oddness
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

void DS2812_info::callback_half()
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

void DS2812_info::callback_full()
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

DS2812_info dsi( &tim_h, TIM_CHANNEL_1 );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test LED output"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 5;
  UVAR('n') = 4;

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
  unsigned td = arg2long_d( 2, argc, argv, UVAR('t'), 0 );
  unsigned n = UVAR('n');
  uint8_t v0 = UVAR('b'); //background


  std_out << "# Test:  " <<  "  td= " << td <<  NL;

  for( unsigned i=0; i<n*3; ++i ) {
    lbuf[i] = v0;
  }

  lbuf[0] =    v; lbuf[1]  = 0x00; lbuf[2]  = 0x00;
  lbuf[3] = 0x00; lbuf[4]  =    v; lbuf[5]  = 0x00;
  lbuf[6] = 0x00; lbuf[7]  = 0x00; lbuf[8]  =    v;
  lbuf[9] =    v; lbuf[10] =    v; lbuf[11] =    v;
  dsi.ibuf = lbuf; dsi.size = n; dsi.pos = 0;
  dsi.fill_zeros();

  // log_reset();


  UVAR('i') = 0;
  UVAR('j') = 0;
  UVAR('k') = 0;
  UVAR('q') = 0;

  UVAR('r') = dsi.send( lbuf, n );



  // uint32_t tm0 = HAL_GetTick();
  // break_flag = 0;
  // for( unsigned i=0; i<n && !break_flag; ++i ) {
  //   ch.ccr = ch.v * pbase / 256;
  //   if( UVAR('d') > 0 ) {
  //     std_out << NL;
  //   }
  //
  //   delay_ms_until_brk( &tm0, t_step );
  // }

  return 0;
}

//  ----------------------------- configs ----------------

int tim_cfg()
{
  auto pbase = calc_TIM_arr_for_base_psc( TIM_EXA, 0, 800000 );
  UVAR('a') = pbase;
  DS2812_info::calc_minmax( pbase );

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

  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = 0; // pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
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

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PIN1, TIM_EXA_GPIOAF );

  hdma_tim_chx.Instance                 = DMA2_Stream1; // XXX
  hdma_tim_chx.Init.Channel             = DMA_CHANNEL_6;// XXX
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

  __HAL_LINKDMA( &tim_h, hdma[TIM_DMA_ID_CC1], hdma_tim_chx ); // XXX

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PIN1 );
}

void MX_DMA_Init()
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  HAL_NVIC_SetPriority( DMA2_Stream1_IRQn, 8, 0 ); // XXX
  HAL_NVIC_EnableIRQ( DMA2_Stream1_IRQn );

}


void DMA2_Stream1_IRQHandler() // XXX
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

