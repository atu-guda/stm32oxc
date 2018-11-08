#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <vector>
// #include <string>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

void print_curr( const char *s );
void out_to_curr( uint32_t n, uint32_t st );

OutStream& operator<<( OutStream &os, float rhs );

OutStream& operator<<( OutStream &os, float rhs ) // TODO: to library
{
  char buf[32];

  snprintf( buf, sizeof(buf), "%#g", (double)rhs );
  // snprintf( buf, sizeof(buf), "%16.6e", rhs );
  os << buf;
  return os;
}


extern "C" {
 void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc );
 void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
 void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim );
}
int adc_init_exa_4ch_dma( uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );
uint32_t calc_ADC_clk( uint32_t adc_presc, int *div_val );
uint32_t hint_ADC_presc();
void ADC_DMA_REINIT();
void pr_ADC_state();

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
uint32_t tim_freq_in; // timer input freq
uint32_t adc_clk = ADC_FREQ_MAX;     // depend in MCU, set in adc_init_exa_4ch_dma
// uint32_t t_step = 100000; // in us, recalculated before measurement
float t_step_f = 0.1; // in s, recalculated before measurement
int v_adc_ref = 3250; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in byter for 16-bit samples

vector<uint16_t> ADC_buf;

volatile int adc_end_dma = 0;
volatile int adc_dma_error = 0;
volatile uint32_t n_series = 0;
uint32_t n_series_todo = 0;
const uint32_t n_sampl_times = 7; // current number - in UVAR('s')
const uint32_t sampl_times_codes[n_sampl_times] = { // all for 36 MHz ADC clock
  ADC_SAMPLETIME_3CYCLES   , //  15  tick: 2.40 MSa,  0.42 us
  ADC_SAMPLETIME_15CYCLES  , //  27  tick: 1.33 MSa,  0.75 us
  ADC_SAMPLETIME_28CYCLES  , //  40  tick:  900 kSa,  1.11 us
  ADC_SAMPLETIME_56CYCLES  , //  68  tick:  529 kSa,  1.89 us
  ADC_SAMPLETIME_84CYCLES  , //  96  tick:  375 kSa,  2.67 us
  ADC_SAMPLETIME_144CYCLES , // 156  tick:  231 kSa,  4.33 us
  ADC_SAMPLETIME_480CYCLES   // 492  tick:   73 kSa, 13.67 us
};
const uint32_t sampl_times_cycles[n_sampl_times] = { // sample+conv(12)
    15,  // ADC_SAMPLETIME_3CYCLES
    27,  // ADC_SAMPLETIME_15CYCLES
    40,  // ADC_SAMPLETIME_28CYCLES
    68,  // ADC_SAMPLETIME_56CYCLES
    96,  // ADC_SAMPLETIME_84CYCLES
   156,  // ADC_SAMPLETIME_144CYCLES
   492,  // ADC_SAMPLETIME_480CYCLES
};



TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc = 36, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();

const int pbufsz = 128;
// FIL out_file;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_to( int argc, const char * const * argv );
CmdInfo CMDINFO_TO { "to", 0, cmd_to, " - test float output "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OUT,
  &CMDINFO_TO,
  nullptr
};




int main(void)
{
  BOARD_PROLOG;

  tim_freq_in = get_TIM_in_freq( TIM2 );

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('j') = tim_freq_in;
  UVAR('p') = ( tim_freq_in / 1000000 ) - 1; // timer PSC, for 1MHz
  UVAR('a') = 99999; // timer ARR, for 10Hz
  UVAR('c') = n_ADC_ch_max;
  UVAR('n') = 8; // number of series
  UVAR('s') = 0; // sampling time index

  #ifdef PWR_CR1_ADCDC1
  PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


void pr_ADC_state()
{
  if( UVAR('d') > 0 ) {
    STDOUT_os;
    os <<  " ADC: SR= " << HexInt( BOARD_ADC_DEFAULT_DEV->SR  )
       <<  "  CR1= "    << HexInt( BOARD_ADC_DEFAULT_DEV->CR1 )
       <<  "  CR2= "    << HexInt( BOARD_ADC_DEFAULT_DEV->CR2 )
       <<  "  CR2= "    << HexInt( BOARD_ADC_DEFAULT_DEV->CR2 )
       <<  NL;
  }
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  char pbuf[pbufsz];
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };

  uint32_t tim_psc = UVAR('p');
  uint32_t tim_arr = UVAR('a');

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= n_sampl_times ) { sampl_t_idx = n_sampl_times-1; };
  uint32_t f_sampl_max = adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  float tim_f = (float)tim_freq_in / t_step_tick; // timer update freq, Hz
  uint32_t tim_fi = (uint32_t)( tim_f * 1000000 );
  t_step_f    = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * t_step_f * 1000 ); // in ms

  os << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
     << " tim_fi = " << tim_fi << " uHz"
     << " t_step_f= " << t_step_f << " Hz  t_wait0= " << t_wait0 << " ms" NL;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  tim2_deinit();

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_dma( adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    os <<  "ADC init failed, errno= " << errno << NL;
    return 1;
  }
  pr_ADC_state();


  os << "# Timer: tim_freq_in= " << tim_freq_in << "  Hz / (( " << tim_psc
     << "+1)*( " << tim_arr << "+1)) =" << tim_f << " Hz; t_step = " << t_step_f << " s" NL;
  delay_ms( 1 );

  int div_val = -1;
  adc_clk = calc_ADC_clk( adc_presc, &div_val );
  snprintf( pbuf, pbufsz-1, "# ADC: n_ch= %d n=%lu adc_clk= %lu div_val= %d s_idx= %lu sampl= %lu; f_sampl_max= %lu Hz; t_wait0= %lu ms" NL,
                                    n_ch,    n,    adc_clk,     div_val,  sampl_t_idx, sampl_times_cycles[sampl_t_idx],
                                    f_sampl_max, t_wait0 );
  os << pbuf;
  delay_ms( 10 );

  uint32_t n_ADC_bytes = n * n_ch;
  ADC_buf.resize( 0, 0 );
  ADC_buf.shrink_to_fit();
  ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  os << "# ADC_buf.size= " << ADC_buf.size() << " data= " << HexInt( ADC_buf.data() ) << NL;

  adc_end_dma = 0; adc_dma_error = 0; n_series = 0; n_series_todo = n;
  UVAR('b') = 0; UVAR('g') = 0; UVAR('e') = 0;   UVAR('x') = 0; UVAR('y') = 0; UVAR('z') = 0;
  if( ADC_buf.data() == nullptr ) {
    os <<  "Error: fail to allocate memory" NL;
    return 2;
  }

  leds.reset( BIT0 | BIT1 | BIT2 );
  // TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;


  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)ADC_buf.data(), n_ADC_bytes ) != HAL_OK )   {
    os <<  "ADC_Start_DMA error" NL;
  }
  tim2_init( UVAR('p'), UVAR('a') );

  delay_ms( t_wait0 );
  for( uint32_t ti=0; adc_end_dma == 0 && ti<(uint32_t)UVAR('t'); ++ti ) {
    delay_ms(1);
  }
  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

  tim2_deinit();
  HAL_ADC_Stop_DMA( &hadc1 ); // needed
  if( adc_end_dma == 0 ) {
    os <<  "Fail to wait DMA end " NL;
  }
  if( adc_dma_error != 0 ) {
    os <<  "Found DMA error " << HexInt( adc_dma_error ) <<  NL;
  }
  os <<  "#  tick: " <<  ( tcc - tm00 ) <<  " good= " << UVAR('g') << " err= " << UVAR('e') <<  NL;

  out_to_curr( 2, 0 );
  if( n_series_todo > 2 ) {
    os <<  "....." NL;
    out_to_curr( 4, n_series_todo-2 );
  }

  os <<  NL;

  pr_ADC_state();
  os <<  NL;

  delay_ms( 10 );

  return 0;
}

int cmd_to( int /*argc*/, const char * const * /*argv*/ )
{
  STDOUT_os;
  char b[128];
  for( float v = 1.987654e-6; v < 1e7; v *= -10 ) {
    snprintf( b, sizeof(b), "f= %f e= %e g= %g i= %d ", v, v, v, (int)( v*1000000) );
    // to_chars( b, end(b), v );
    os << b << NL;
    // string s = to_string( v );
    // os << s.c_str() << NL;
  }
  return 0;
}

void print_curr( const char *s )
{
  if( !s ) {
    return;
  }
  STDOUT_os;
  // if( out_file.fs == nullptr ) {
    os <<  s;
    delay_ms( 2 );
  //  return;
  //}
  // f_puts( s, &out_file );
}

void out_to_curr( uint32_t n, uint32_t st )
{
  char buf[32];
  char pbuf[pbufsz];
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };

  if( n+st >= n_series_todo+1 ) {
    n = 1 + n_series_todo - st;
  }

  float t = st * t_step_f;
  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    t = t_step_f * ii;
    snprintf( pbuf, pbufsz-1, "%#12.7g  ", t );
    for( int j=0; j< n_ch; ++j ) {
      int vv = ADC_buf[ii*n_ch+j] * 10 * UVAR('v') / 4096;
      ifcvt( vv, 10000, buf, 4 );
      strcat( pbuf, buf ); strcat( pbuf, "  " );
    }
    strcat( pbuf, NL );
    print_curr( pbuf );
  }
}

int cmd_out( int argc, const char * const * argv )
{
  // out_file.fs = nullptr;
  uint32_t n = arg2long_d( 1, argc, argv, n_series_todo, 0, n_series_todo+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,             0, 0, n_series_todo-2 );

  out_to_curr( n, st );

  return 0;
}



void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  adc_end_dma |= 1;
  UVAR('x') = hadc1.Instance->SR;
  if( UVAR('b') == 0 ) {
    UVAR('b') = 1;
  }
  ++UVAR('g'); // 'g' means good
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  UVAR('y') = hadc1.Instance->SR;
  adc_end_dma |= 2;
  // tim2_deinit();
  if( UVAR('b') == 0 ) {
    UVAR('b') = 2;
  }
  UVAR('z') = HAL_ADC_GetError( hadc );
  adc_dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  UVAR('y') = hadc1.Instance->SR;
  ++UVAR('e');
}


void HAL_ADCEx_InjectedConvCpltCallback( ADC_HandleTypeDef * /*hadc*/ )
{
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

