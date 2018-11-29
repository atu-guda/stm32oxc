#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <vector>

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


int adc_init_exa_4ch_dma( uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );
void ADC_DMA_REINIT();

ADC_Info adc;

uint32_t tim_freq_in; // timer input freq
float t_step_f = 0.1; // in s, recalculated before measurement
int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in byter for 16-bit samples

vector<uint16_t> ADC_buf;


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
  UVAR('p') =  calc_TIM_psc_for_cnt_freq( TIM2, 1000000 ); // timer PSC, for 1MHz
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
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= adc_n_sampl_times ) { sampl_t_idx = adc_n_sampl_times-1; };
  uint32_t f_sampl_max = adc.adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  float tim_f = (float)tim_freq_in / t_step_tick; // timer update freq, Hz
  t_step_f    = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * t_step_f * 1000 ); // in ms

  os << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
     << " t_step_f= " << t_step_f << " s  t_wait0= " << t_wait0 << " ms" NL;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  tim2_deinit();

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_dma( adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    os <<  "ADC init failed, errno= " << errno << NL;
    return 1;
  }
  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }


  os << "# Timer: tim_freq_in= " << tim_freq_in << "  Hz / (( " << tim_psc
     << "+1)*( " << tim_arr << "+1)) =" << tim_f << " Hz; t_step = " << t_step_f << " s" NL;
  delay_ms( 1 );

  int div_val = -1;
  adc.adc_clk = calc_ADC_clk( adc_presc, &div_val );
  snprintf( pbuf, pbufsz-1, "# ADC: n_ch= %d n=%lu adc_clk= %lu  div_val= %d s_idx= %lu sampl= %lu; f_sampl_max= %lu Hz; t_wait0= %lu ms" NL,
                                    n_ch,    n,    adc.adc_clk,  div_val,   sampl_t_idx, sampl_times_cycles[sampl_t_idx],
                                    f_sampl_max, t_wait0 );
  os << pbuf;
  delay_ms( 10 );

  uint32_t n_ADC_bytes = n * n_ch;
  ADC_buf.resize( 0, 0 );
  ADC_buf.shrink_to_fit();
  ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  os << "# ADC_buf.size= " << ADC_buf.size() << " data= " << HexInt( ADC_buf.data() ) << NL;

  adc.reset_cnt();
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
  for( uint32_t ti=0; adc.end_dma == 0 && ti<(uint32_t)UVAR('t'); ++ti ) {
    delay_ms(1);
  }
  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

  tim2_deinit();
  HAL_ADC_Stop_DMA( &hadc1 ); // needed
  if( adc.end_dma == 0 ) {
    os <<  "Fail to wait DMA end " NL;
  }
  if( adc.dma_error != 0 ) {
    os <<  "Found DMA error " << HexInt( adc.dma_error ) <<  NL;
  } else {
    adc.n_series = n;
  }
  os <<  "#  tick: " <<  ( tcc - tm00 )  <<  NL;

  out_to_curr( 2, 0 );
  if( adc.n_series > 2 ) {
    os <<  "....." NL;
    out_to_curr( 4, adc.n_series-2 );
  }

  os <<  NL;

  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }
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
    // delay_ms( 2 );
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

  if( n+st >= adc.n_series+1 ) {
    n = 1 + adc.n_series - st;
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
  auto ns = adc.n_series;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  out_to_curr( n, st );

  return 0;
}



void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 1;
  adc.good_SR =  adc.last_SR = hadc1.Instance->SR;
  adc.last_end = 1;
  adc.last_error = 0;
  ++adc.n_good;
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 2;
  adc.bad_SR = adc.last_SR = hadc1.Instance->SR;
  // tim2_deinit();
  adc.last_end  = 2;
  adc.last_error = HAL_ADC_GetError( hadc );
  adc.dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  ++adc.n_bad;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

