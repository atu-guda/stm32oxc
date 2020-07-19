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
DMA_HandleTypeDef hdma_tim_ch1;

// debug
PinOut dbg_pin( GpioA, 9 );
char xlog_buf[2048];



void TIM_XferHalfCpltCallback( DMA_HandleTypeDef *hdma );
void TIM_XferCpltCallback( DMA_HandleTypeDef *hdma );
void TIM_ErrCallback( DMA_HandleTypeDef *hdma );

void MX_DMA_Init();
void tim_cfg();
uint16_t t_min, t_max; // timings got 0/1 on wire (in times ticks) min1-max0 = 0, max1-min0 = 1
const uint32_t max_leds = 128; // each LED require 3*8 = 24 bit, each require halfword
uint8_t lbuf[max_leds*3];

struct DS2812_info {
  static constexpr uint16_t size_1led = 3 * 8;
  static uint16_t t_min, t_max;     // CCR values for '0' and '1'
  uint16_t buf[ 2 * size_1led ]; // 2 chunks, each 24: 1 bit (CCR) for output via timer
  uint8_t *ibuf = nullptr;
  int size = 0;
  volatile int pos = -2;      // pos not unsigned - special values for start/stop
  int send( const uint8_t *d, int sz );
  static void rgb2tim( uint8_t r, uint8_t g, uint8_t b, uint16_t *d );
  static void color2tim( uint8_t c, uint16_t *d );
  void fill_zeros() { for( auto &v : buf ) { v = 0; } }
  void fill_zeros0() { for( unsigned i=0; i<size_1led; ++i ) { buf[i] = 0; } }
  void fill_zeros1() { for( unsigned i=0; i<size_1led; ++i ) { buf[i+size_1led] = 0; } }
  static void calc_minmax( uint16_t arr );
};
uint16_t DS2812_info::t_min, DS2812_info::t_max;

void DS2812_info::calc_minmax( uint16_t arr )
{
  t_min = (uint16_t)( arr * 8 / 25 ); // * 0.32
  t_max = (uint16_t)( arr - t_min );
}

void DS2812_info::color2tim( uint8_t c, uint16_t *d )
{
  int i=0;
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

int DS2812_info::send( const uint8_t *d, int sz )
{
  sz &= ~1; // TODO: may be better:
  if( size != 0 || !d || sz < 2 ) {
    return 0;
  }
  size = sz; pos = -2;

  size = 0;
  return 1;
}

DS2812_info dsi;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 5;
  UVAR('n') = 1;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  dbg_pin.initHW();
  dbg_pin.reset();
  MX_DMA_Init();
  tim_cfg();
  set_log_buf( xlog_buf, sizeof(xlog_buf) );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  // unsigned v = arg2long_d( 1, argc, argv, 0, 0, 100 );
  unsigned td = arg2long_d( 2, argc, argv, UVAR('t'), 0 );
  // unsigned n = UVAR('n');


  std_out << "# Test:  " <<  "  td= " << td <<  NL;
  std_out << "## cb: " << HexInt((void*)hdma_tim_ch1.XferCpltCallback) << NL;

  lbuf[0] = 0xFF; lbuf[1]  = 0x00; lbuf[2]  = 0x00;
  lbuf[3] = 0x00; lbuf[4]  = 0xFF; lbuf[5]  = 0x00;
  lbuf[6] = 0x00; lbuf[7]  = 0x00; lbuf[8]  = 0xFF;
  lbuf[9] = 0x20; lbuf[10] = 0x20; lbuf[11] = 0x20;
  dsi.ibuf = lbuf; dsi.size = 4; dsi.pos = 0;
  dsi.fill_zeros();

  log_reset();

  // if( v > 0 ) {
  //   dsi.fill_zeros();
  // } else {
  //   DS2812_info::rgb2tim( 0x55, 0x33, 0x11, dsi.buf );
  //   DS2812_info::rgb2tim( 0xAA, 0x55, 0x77, dsi.buf+24 );
  // }

  dbg_pin.set();
  // __HAL_TIM_ENABLE( &tim_h );
  //TIM_CCxChannelCmd( TIM_EXA, TIM_CHANNEL_1, 0 );
  delay_mcs( 20 );
  UVAR('i') = 0;
  UVAR('j') = 0;
  UVAR('k') = 0;
  UVAR('q') = 0;

  UVAR('r') = HAL_TIM_PWM_Start_DMA( &tim_h, TIM_CHANNEL_1, (uint32_t*)dsi.buf, 2*24 );

  // tim_print_cfg( TIM_EXA );
  // dump32( TIM_EXA, 0x100, 1 );

  delay_ms( td );
  HAL_TIM_PWM_Stop_DMA( &tim_h, TIM_CHANNEL_1 );
  TIM_EXA->CCR1 = 0;
  TIM_EXA->CNT = 0;

  // delay_ms( td );
  delay_mcs( 20 );
  // TIM_CCxChannelCmd( TIM_EXA, TIM_CHANNEL_1, 0 );
  // __HAL_TIM_DISABLE( &tim_h );
  dbg_pin.reset();


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

void tim_cfg()
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
    return;
  }
  UVAR('x') = 1;

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );
  UVAR('x') = 2;


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
    return;
  }

  TIM_EXA->CCR1 = 0;
  TIM_EXA->CNT  = 0;
  // TIM_CCxChannelCmd( TIM_EXA, TIM_CHANNEL_1, 0 );
  UVAR('x') = 11;

}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  UVAR('z') = 1;
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PIN1, TIM_EXA_GPIOAF );

  hdma_tim_ch1.Instance                 = DMA2_Stream1;
  hdma_tim_ch1.Init.Channel             = DMA_CHANNEL_6;
  hdma_tim_ch1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tim_ch1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tim_ch1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tim_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_tim_ch1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_tim_ch1.Init.Mode                = DMA_CIRCULAR;
  // hdma_tim_ch1.Init.Mode                = DMA_NORMAL;
  hdma_tim_ch1.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_tim_ch1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_tim_ch1 ) != HAL_OK ) {
    UVAR('e') = 14;
    return;
  }

  __HAL_LINKDMA( &tim_h, hdma[TIM_DMA_ID_CC1], hdma_tim_ch1 );

  UVAR('m') =
  HAL_DMA_RegisterCallback( &hdma_tim_ch1, HAL_DMA_XFER_CPLT_CB_ID,     TIM_XferCpltCallback     );
  HAL_DMA_RegisterCallback( &hdma_tim_ch1, HAL_DMA_XFER_HALFCPLT_CB_ID, TIM_XferHalfCpltCallback );
  HAL_DMA_RegisterCallback( &hdma_tim_ch1, HAL_DMA_XFER_ERROR_CB_ID,    TIM_ErrCallback );

  UVAR('z') = 2;
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PINS );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}

void MX_DMA_Init()
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  HAL_NVIC_SetPriority( DMA2_Stream1_IRQn, 8, 0 );
  HAL_NVIC_EnableIRQ( DMA2_Stream1_IRQn );

}

// tmp copy
typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

void DMA2_Stream1_IRQHandler()
{
  leds.toggle( BIT0 );
  log_add("i");
  char cn2[2];
  cn2[0] = char('0' + dsi.pos); cn2[1] = '\0';
  log_add( cn2 );

  uint16_t p = dsi.pos;
  uint16_t xpos = p * 3;
  bool hc = false, tc = false;

  DMA_Base_Registers *regs = (DMA_Base_Registers *)hdma_tim_ch1.StreamBaseAddress;
  uint32_t tmpisr = regs->ISR;

  // half-complete
  if( ( tmpisr & ( DMA_FLAG_HTIF0_4 << hdma_tim_ch1.StreamIndex ) ) != RESET ) {
    if( __HAL_DMA_GET_IT_SOURCE( &hdma_tim_ch1, DMA_IT_HT ) != RESET ) {
      hc = true;
      ++UVAR('j');
      log_add("h");
    }
  }

  // fill complete
  if( ( tmpisr & ( DMA_FLAG_TCIF0_4 << hdma_tim_ch1.StreamIndex ) ) != RESET ) {
    if( __HAL_DMA_GET_IT_SOURCE( &hdma_tim_ch1, DMA_IT_TC ) != RESET ) {
      tc = true;
      ++UVAR('k');
      log_add("c");
    }
  }

  HAL_DMA_IRQHandler( &hdma_tim_ch1 );


  if( p > dsi.size+2 ) {
    log_add( "-STOP-" );
    HAL_TIM_PWM_Stop_DMA( &tim_h, TIM_CHANNEL_1 );
    // __HAL_TIM_DISABLE( &tim_h );
  }

  if( hc ) {
    if( p < dsi.size ) {
      DS2812_info::rgb2tim( dsi.ibuf[xpos], dsi.ibuf[xpos+1], dsi.ibuf[xpos+2], dsi.buf );
    } else {
      dsi.fill_zeros0();
    }
  }

  if( tc ) {
    if( p < dsi.size ) {
      DS2812_info::rgb2tim( dsi.ibuf[xpos], dsi.ibuf[xpos+1], dsi.ibuf[xpos+2], dsi.buf+24 );
    } else {
      dsi.fill_zeros1();
    }
  }

  ++dsi.pos;
  UVAR('i') = dsi.pos;

  log_add( NL );
}

void TIM_XferHalfCpltCallback( DMA_HandleTypeDef *hdma )
{
  ++UVAR('j');
  log_add("H");
}

void TIM_XferCpltCallback( DMA_HandleTypeDef *hdma )
{
  ++UVAR('k');
  log_add("C");
}

void TIM_ErrCallback( DMA_HandleTypeDef *hdma )
{
  ++UVAR('q');
  log_add("E");
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

