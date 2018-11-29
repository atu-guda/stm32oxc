#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <vector>

#include <oxc_auto.h>

#include <fatfs_sd_st.h>
#include <ff.h>

#include <oxc_fs_cmd0.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure ADC data (4ch) and store to SD card" NL;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;
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
float t_step_f = 0.1; // in s, recalculated before measurement
int v_adc_ref = 3250; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in bytes for 16-bit samples

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
FIL out_file;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  FS_CMDS0,
  &CMDINFO_OUT,
  &CMDINFO_OUTSD,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  tim_freq_in = get_TIM_in_freq( TIM2 ); // TODO: define

  MX_SDIO_SD_Init(); // TODO: only during write or similar ops TODO: need generic commands rewrite
  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('x') = HAL_SD_GetState( &hsd );
  UVAR('y') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';
  UVAR('z') = f_mount( &fs, "", 1 );
  out_file.obj.fs = nullptr;

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('j') = tim_freq_in;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM2, 1000000 ); // timer PSC, for 1MHz
  UVAR('a') = 99999; // timer ARR, for 10Hz TODO: better time or freq based
  UVAR('c') = n_ADC_ch_max;
  UVAR('n') = 8; // number of series
  UVAR('s') = 0; // sampling time index
  UVAR('d') = 1; // debug

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
    os <<  "# ADC: SR= " << HexInt( BOARD_ADC_DEFAULT_DEV->SR  )
       <<  "  CR1= "     << HexInt( BOARD_ADC_DEFAULT_DEV->CR1 )
       <<  "  CR2= "     << HexInt( BOARD_ADC_DEFAULT_DEV->CR2 )
       <<  "  CR2= "     << HexInt( BOARD_ADC_DEFAULT_DEV->CR2 )
       <<  NL;
  }
}

void pr_DMA_state()
{
  if( UVAR('d') > 0 ) {
    STDOUT_os;
    os <<  "# DMA: CR= " << HexInt( hdma_adc1.Instance->CR )
       << " NDTR= "      << hdma_adc1.Instance->NDTR
       << " PAR= "       << HexInt( hdma_adc1.Instance->PAR )
       << " M0AR= "      << HexInt( hdma_adc1.Instance->M0AR )
       << " M1AR= "      << HexInt( hdma_adc1.Instance->M1AR )
       << " FCR= "       << HexInt( hdma_adc1.Instance->FCR )
       << NL;
  }
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };

  uint32_t tim_psc = UVAR('p');
  uint32_t tim_arr = UVAR('a');

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= n_sampl_times ) { sampl_t_idx = n_sampl_times-1; };
  uint32_t f_sampl_max = adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  float tim_f = (float)tim_freq_in / t_step_tick; // timer update freq, Hz
  t_step_f    = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * t_step_f * 1000 ); // in ms

  if( UVAR('d') > 0 ) {
    os << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
       << " t_step_f= " << t_step_f << " s  t_wait0= " << t_wait0 << " ms" NL;
  }

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


  os << "# Timer: tim_freq_in= " << tim_freq_in << " Hz / ((" << tim_psc
     << "+1)*(" << tim_arr << "+1)) =" << tim_f << " Hz; t_step = " << t_step_f << " s" NL;
  delay_ms( 1 );

  int div_val = -1;
  adc_clk = calc_ADC_clk( adc_presc, &div_val );
  os << "# ADC: n_ch= " << n_ch << " n= " << n << " adc_clk= " << adc_clk << " div_val= " << div_val
     << " sampl_t_idx= " << sampl_t_idx << " sampl= " << sampl_times_cycles[sampl_t_idx]
     <<  " f_sampl_max= " << f_sampl_max << " Hz; t_wait0= " << t_wait0 << " ms" NL;
  delay_ms( 10 );

  uint32_t n_ADC_sampl = n * n_ch;
  ADC_buf.resize( 0, 0 );
  ADC_buf.shrink_to_fit();
  ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  os << "# ADC_buf.size= " << ADC_buf.size() << " data= 0x" << HexInt( ADC_buf.data() ) << NL;

  adc_end_dma = 0; adc_dma_error = 0; n_series = 0; n_series_todo = n;
  UVAR('b') = 0; UVAR('g') = 0; UVAR('e') = 0;   UVAR('x') = 0; UVAR('y') = 0; UVAR('z') = 0;
  if( ADC_buf.data() == nullptr ) {
    os <<  "# Error: fail to allocate memory" NL;
    return 2;
  }

  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)ADC_buf.data(), n_ADC_sampl ) != HAL_OK )   {
    os <<  "# Error: ADC_Start_DMA error" NL;
    return 3;
  }
  tim2_init( UVAR('p'), UVAR('a') );

  delay_ms_brk( t_wait0 );
  for( uint32_t ti=0; adc_end_dma == 0 && ti<(uint32_t)UVAR('t') && !break_flag;  ++ti ) {
    delay_ms(1);
  }
  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

  tim2_deinit();
  HAL_ADC_Stop_DMA( &hadc1 ); // needed
  if( adc_end_dma == 0 ) {
    os <<  "# Error: Fail to wait DMA end " NL;
  }
  if( adc_dma_error != 0 ) {
    os <<  "# Error: Found DMA error " << HexInt( adc_dma_error ) <<  NL;
  }
  os <<  "#  tick: " <<  ( tcc - tm00 ) <<  " good= " << UVAR('g') << " err= " << UVAR('e') <<  NL;

  if( n_series_todo > 8 ) {
    out_to_curr( 6, 0 );
    os <<  "....." NL;
    out_to_curr( 4, n_series_todo-2 );
  } else {
    out_to_curr( n_series_todo+2, 0 );
  }

  os <<  NL;

  pr_ADC_state();
  os <<  NL;

  delay_ms( 10 );

  return 0;
}

void print_curr( const char *s )
{
  if( !s ) {
    return;
  }
  if( out_file.obj.fs == nullptr ) {
    STDOUT_os;
    os <<  s;
    delay_ms( 2 );
    return;
  }
  f_puts( s, &out_file );
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
  out_file.obj.fs = nullptr;
  uint32_t n = arg2long_d( 1, argc, argv, n_series_todo, 0, n_series_todo+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,             0, 0, n_series_todo-2 );

  out_to_curr( n, st );

  return 0;
}

int cmd_outsd( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc < 2 ) {
    os << "# Error: need filename [n [start]]" NL;
    return 1;
  }

  out_file.obj.fs = nullptr;
  uint32_t n = arg2long_d( 2, argc, argv, n_series_todo, 0, n_series_todo+1 ); // number output series
  uint32_t st= arg2long_d( 3, argc, argv,             0, 0, n_series_todo-2 );

  const char *fn = argv[1];
  FRESULT r = f_open( &out_file, fn, FA_WRITE | FA_OPEN_ALWAYS );
  if( r == FR_OK ) {
    out_to_curr( n, st );
    f_close( &out_file );
  } else {
    os << "Error: f_open error: " << r << NL;
  }
  out_file.obj.fs = nullptr;

  return r;
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

