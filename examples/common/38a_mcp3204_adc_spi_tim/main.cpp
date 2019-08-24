#include <cerrno>

#include <algorithm>
#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

using namespace std;
using namespace SMLRL;

using sreal = StatData::sreal;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MCP3204 ADC SPI device with timer" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_show_stats( int argc, const char * const * argv );
CmdInfo CMDINFO_SHOWSTATS { "show_stats", 'Y', cmd_show_stats, " [N [start]]- show statistics"  };


  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OUT,
  &CMDINFO_SHOWSTATS,
  nullptr
};



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc, uint32_t arr );
void tim2_deinit();
volatile unsigned tim_flag = 0;

const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in bytes for 16-bit samples
vector<uint16_t> ADC_buf;
unsigned n_lines = 0, last_n_ch = 1;
uint32_t last_dt_us = 1000;
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };
void adc_out_to( OutStream &os, uint32_t n, uint32_t st );
void adc_show_stat( OutStream &os, uint32_t n, uint32_t st );

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000; // in us here
  UVAR('n') = 10;
  UVAR('v') = 5100; // external power supply
  UVAR('c') = n_ADC_ch_max;

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_64 ) != HAL_OK ) { // 1.xx MBit/s < 2
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
  // spi_d.setTssDelay( 10 );
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  static const uint8_t mcp3204_cmds1ch[4][2] = {
    { 0x06, 0x00 },
    { 0x06, 0x40 },
    { 0x06, 0x80 },
    { 0x06, 0xC0 },
  };

  int t_step = UVAR('t');
  t_step /= 10; t_step *= 10; // round to 10 us
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  int min_t_step = 50 * n_ch;
  if( t_step < min_t_step ) {
    t_step = min_t_step;
  }

  // StatData sdat( n_ch );

  // sreal kv = 0.001f * UVAR('v') / 4096;

  std_out << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << " us " NL;
  // std_out << "# Coeffs: ";
  // for( decltype(n_ch) j=0; j<n_ch; ++j ) {
  //   std_out << ' ' << v_coeffs[j];
  // }
  // std_out << NL;

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  ADC_buf.resize( 0, 0 );
  ADC_buf.shrink_to_fit();
  ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  n_lines = 0; last_n_ch = n_ch; last_dt_us = t_step;

  uint32_t tm00 =  HAL_GetTick();
  int rc = 0;
  // bool do_out = ! UVAR('b');

  uint32_t psc = calc_TIM_psc_for_cnt_freq( tim2h.Instance, 100000 ); // 10 us each
  tim2_init( psc, ( t_step / 10 ) - 1 );
  tim_print_cfg( tim2h.Instance );

  if( HAL_TIM_Base_Start_IT( &tim2h ) != HAL_OK ) {
    tim2_deinit();
    std_out << "Fail to init timer" NL;
    return 5;
  }
  HAL_NVIC_EnableIRQ( TIM2_IRQn );

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    while( tim_flag == 0 ) { /* NOP */ };
    tim_flag = 0;

    // sreal v[n_ch];

    // if( UVAR('l') ) {  leds.set( BIT2 ); }

    uint8_t buf[3]; // first - fake
    for( decltype(+n_ch) j=0; j < n_ch; ++j ) {
      spi_d.duplex( mcp3204_cmds1ch[j], buf, 3 );
      uint16_t v = buf[2] + ( ( buf[1] & 0x0F ) << 8 );
      ADC_buf[ n_lines * n_ch + j ] = v;
      // delay_bad_mcs( 1 );
    }
    ++n_lines;

    // if( UVAR('l') ) {  leds.reset( BIT2 ); }

    // for( decltype(n_ch) j=0; j<n_ch; ++j ) {
    //   sreal cv = kv * ADC_buf[j] * v_coeffs[j];
    //   v[j] = cv;
    // }
    // sdat.add( v );

    // if( do_out ) {
    //   std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
    //   for( auto vc : v ) {
    //     std_out  << ' '  <<  vc;
    //   }
    //   std_out << NL;
    // }

  }

  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  HAL_TIM_Base_Stop_IT( &tim2h );


  std_out << "# n_lines= " << n_lines << " dt_appr= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;
  // sdat.calc();
  // std_out << sdat << NL;

  delay_ms( 10 );

  return rc;
}

void tim2_init( uint16_t presc, uint32_t arr )
{
  __TIM2_CLK_ENABLE();

  HAL_NVIC_SetPriority( TIM2_IRQn, 1, 0 );
  // HAL_NVIC_EnableIRQ( TIM2_IRQn );

  tim2h.Instance               = TIM2;
  tim2h.Init.Prescaler         = presc;
  tim2h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim2h.Init.Period            = arr;
  tim2h.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  tim2h.Init.RepetitionCounter = 0; // only for adv timers

  if( HAL_TIM_Base_Init( &tim2h ) != HAL_OK ) {
    Error_Handler( 4 );
    return;
  }

}

void tim2_deinit()
{
  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  __TIM2_CLK_DISABLE();
}

void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &tim2h );
  tim_flag = 1;
  leds.toggle( BIT1 );
}


int cmd_out( int argc, const char * const * argv )
{
  auto ns = n_lines;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adc_out_to( std_out, n, st );
  adc_show_stat( std_out, n, st );

  return 0;
}


int cmd_show_stats( int argc, const char * const * argv )
{
  auto ns = n_lines;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adc_show_stat( std_out, n, st );

  return 0;
}

void adc_show_stat( OutStream &os, uint32_t n, uint32_t st )
{
  if( n+st >= n_lines+1 ) {
    n = n_lines - st;
  }

  StatData sdat( last_n_ch );

  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    sreal vv[last_n_ch];
    for( decltype(+last_n_ch) j=0; j< last_n_ch; ++j ) {
      vv[j] = 0.001f * (float) ADC_buf[ii*last_n_ch+j] * UVAR('v') / 4096;
    }
    sdat.add( vv );
  }
  sdat.calc();
  sdat.out_parts( os );

}

// some different from common ADC
void adc_out_to( OutStream &os, uint32_t n, uint32_t st )
{
  if( n+st >= n_lines ) {
    n = n_lines - st;
  }

  os << "# n= " << n << " n_ch= " << last_n_ch << " st= " << st
     << " dt= " << ( 1e-6f * last_dt_us ) << NL;

  break_flag = 0;
  for( uint32_t i=0; i<n && !break_flag; ++i ) {
    uint32_t ii = i + st;
    float t = 1e-6f * last_dt_us * ii;
    os <<  FltFmt( t, cvtff_auto, 14, 6 ) << ' ';
    for( decltype(+last_n_ch) j=0; j< last_n_ch; ++j ) {
      float v = 0.001f * (float) ADC_buf[ii*last_n_ch+j] * UVAR('v') / 4096;
      os << v;
    }
    os << NL;
  }

}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

