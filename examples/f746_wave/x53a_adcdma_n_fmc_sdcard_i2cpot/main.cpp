#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <vector>

#include <oxc_auto.h>

#include <board_sdram.h>

#include <ff.h>
#include <fatfs.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

extern "C" {
void task_pot( void *prm UNUSED_ARG );
}

SDRAM_HandleTypeDef hsdram;

// BOARD_DEFINE_LEDS_EXTRA; //  PinsOut ledsx( GPIOE, 1, 6 ); // E1-E6

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;
FIL out_file;
const int pbufsz = 128;
const int fspath_sz = 32;
extern char fspath[fspath_sz];
int  print_curr( const char *s );
int  out_to_curr( uint32_t n, uint32_t st ); // 0 = ok

// buffer to file output
const uint32_t fbuf_wr_k   = 8;
const uint32_t fbuf_size   = ( fbuf_wr_k + 1 ) * 1024;
const uint32_t fbuf_hwmark = ( fbuf_wr_k ) * 1024;
const uint32_t fbuf_maxlinesz = 512;
char  fbuf[fbuf_size];
char* fbuf_h = fbuf + fbuf_hwmark;
uint32_t fbuf_pos = 0;
void reset_filebuf();
int  add_to_file( const char *s );
int  flush_file();
void print_file_info( const FIL *f );
void print_fsinfo( const FATFS *fs );


extern "C" {
 void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc );
 void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
 void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim );
}
const uint32_t ADCDMA_chunk_size = 1024; // in bytes, for for now. may be up to 64k-small
HAL_StatusTypeDef ADC_Start_DMA_n( ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length, uint32_t chunkLength, uint8_t elSz );
int adc_init_exa_4ch_dma_n( uint32_t presc, uint32_t sampl_cycl, uint8_t n_ch );
uint32_t calc_ADC_clk( uint32_t presc, int *div_val );
uint32_t hint_ADC_presc();
void ADC_DMA_REINIT();
void pr_ADC_state();
int do_one_run( uint32_t n );
int do_potctl( int v1, int v2, int t );
int do_outsd( const char *afn, uint32_t n, uint32_t st );

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
uint32_t tim_freq_in; // timer input freq
uint32_t adc_clk = ADC_FREQ_MAX;     // depend in MCU, set in adc_init_exa_4ch_dma
// uint32_t t_step = 100000; // in us, recalculated before measurement
float t_step_f = 0.1; // in s, recalculated before measurement
int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX_FMC; // MCU dependent, in bytes for 16-bit samples

// vector<uint16_t> ADC_buf;
uint16_t *ADC_buf_x = (uint16_t*)(SDRAM_ADDR);

volatile int adc_end_dma = 0;
volatile int adc_dma_error = 0;
volatile uint32_t n_series = 0;
uint32_t n_series_todo = 0;
volatile int on_save_state = 0;
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


const int pot_steps = 256;
const int pot_r_0 = 47100, add_r_0 = 10000; // initial full scale and additional resistance
volatile int pot_v1 = 128, pot_v2 = 128, pot_t = 1000, pot_tick = 0;
int pot_r1 = add_r_0 + pot_v1 * pot_r_0 / pot_steps;
int pot_r2 = add_r_0 + pot_v2 * pot_r_0 / pot_steps;
char fn_auto[64];

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };
int cmd_potctl( int argc, const char * const * argv );
CmdInfo CMDINFO_POTCTL { "potctl", 'P', cmd_potctl, "v1 [ v2 T ] - set pot resistance params"  };
int cmd_iterrun( int argc, const char * const * argv );
CmdInfo CMDINFO_ITERRRUN { "iterrun", 'Z', cmd_iterrun, "v1 n step=1 - run series of measurement"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_POTCTL,
  &CMDINFO_OUT,
  &CMDINFO_OUTSD,
  &CMDINFO_ITERRRUN,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device
I2CClient digpot( i2cd, 0x2D );

void MX_FMC_Init(void);
void BSP_SDRAM_Initialization_sequence( uint32_t RefreshCount );

int SDIO_on();  // 0 = ok (hard+fatfs)
int SDIO_off(); // 0 = ok


int main(void)
{
  BOARD_PROLOG;

  MX_FMC_Init();
  BSP_SDRAM_Initialization_sequence( 0 ); // 0 is fake


  tim_freq_in = HAL_RCC_GetPCLK1Freq(); // to TIM2
  uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
  if( tim_freq_in < hclk_freq ) {
    tim_freq_in *= 2;
  }

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  // UVAR('p') = (tim_freq_in/1000000)-1; // timer PSC, for 1MHz
  UVAR('p') = 17;  // for high freq, form 2MS/s (a=1) to 100 S/s (a=39999)
  UVAR('a') = 39; // timer ARR, 100 kHz, *4= 400 kS/s
  UVAR('c') = n_ADC_ch_max;
  UVAR('n') = 500000; // number of series
  UVAR('s') = 3; // sampling time index
  UVAR('r') = pot_r_0; // POT full scale resistance
  UVAR('o') = add_r_0; // Initial R_b

  #ifdef PWR_CR1_ADCDC1
  PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  strcpy( fn_auto, "d_xxx.txt" );

  BOARD_POST_INIT_BLINK;

  MX_FATFS_Init();

  BOARD_CREATE_STD_TASKS;
  xTaskCreate( task_pot,  "pot", def_stksz, nullptr,   1, nullptr );

  SCHEDULER_START;
  return 0;
}

int SDIO_on()
{
  MX_SDIO_SD_Init();
  int rc_i =  HAL_SD_Init( &hsd );
  UVAR('e') = rc_i;
  if( rc_i != HAL_OK ) {
    pr( "Fail in HAL_SD_Init: rc= " ); pr_d( rc_i ); pr( NL );
    return 1;
  }
  delay_ms( 50 );
  // MX_FATFS_Init();
  UVAR('j') = HAL_SD_GetState( &hsd );
  UVAR('i') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';
  delay_ms( 20 );
  FRESULT fr = f_mount( &fs, "", 1 );
  UVAR('f') = fr;
  if( fr != FR_OK ) {
    pr( "Fail to mount fs: " ); pr_d( fr ); pr( NL );
  }
  pr( "FS mounted" NL );
  delay_ms( 20 );
  out_file.obj.fs = nullptr;
  return fr;
}

int SDIO_off()
{
  FRESULT fr = f_mount( nullptr, "", 1 );
  pr( "umount: fr= " ); pr_d( fr ); pr( NL ); delay_ms( 10 );
  int rc_d =  HAL_SD_DeInit( &hsd );
  UVAR('e') = rc_d;
  delay_ms( 20 );
  out_file.obj.fs = nullptr;
  fs.fs_type = 0; // none
  return rc_d;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}

void pr_ADC_state()
{
  if( UVAR('d') > 0 ) {
    pr( " ADC: SR= " ); pr_h( BOARD_ADC_DEFAULT_DEV->SR   );
    pr( "  CR1= "   );  pr_h( BOARD_ADC_DEFAULT_DEV->CR1  );
    pr( "  CR2= "   );  pr_h( BOARD_ADC_DEFAULT_DEV->CR2  );
    pr( "  SQR1= "  );  pr_h( BOARD_ADC_DEFAULT_DEV->SQR1 );
    pr( "  SQR3= "  );  pr_h( BOARD_ADC_DEFAULT_DEV->SQR3 );
    pr( NL );
  }
}

void pr_DMA_state()
{
  if( UVAR('d') > 0 ) {
    pr( "DMA: CR= " ); pr_h( hdma_adc1.Instance->CR );
    pr( " NDTR= " );   pr_d( hdma_adc1.Instance->NDTR );
    pr( " PAR= " );    pr_h( hdma_adc1.Instance->PAR );
    pr( " M0AR= " );   pr_h( hdma_adc1.Instance->M0AR );
    pr( " M1AR= " );   pr_h( hdma_adc1.Instance->M1AR );
    pr( " FCR= " );    pr_h( hdma_adc1.Instance->FCR );
    pr( NL );
  }
}

void pr_TIM_state( TIM_TypeDef *htim )
{
  if( UVAR('d') > 1 ) {
    pr_sdx( htim->CNT  );
    pr_sdx( htim->ARR  );
    pr_sdx( htim->PSC  );
    pr_shx( htim->CR1  );
    pr_shx( htim->CR2  );
    pr_shx( htim->SMCR );
    pr_shx( htim->DIER );
    pr_shx( htim->SR   );
  }
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 100000000 ); // number of series
  //                                  limited by n_ADC_series_max

  return do_one_run( n );
}

int do_one_run( uint32_t n )
{
  char pbuf[pbufsz];
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= n_sampl_times ) { sampl_t_idx = n_sampl_times-1; };
  uint32_t f_sampl_max = adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t t_step_tick =  (UVAR('a')+1) * (UVAR('p')+1); // in timer input ticks
  float tim_f = tim_freq_in / t_step_tick; // timer update freq, Hz
  t_step_f = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * t_step_f * 1000 ); // in ms

  // make n a multimple of ADCDMA_chunk_size
  uint32_t lines_per_chunk = ADCDMA_chunk_size / ( n_ch * 2 );
  n = ( ( n - 1 ) / lines_per_chunk + 1 ) * lines_per_chunk;
  uint32_t n_ADC_bytes = n * n_ch * 2;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  tim2_deinit();

  leds.reset( BIT0 | BIT1 | BIT2 );
  delay_ms( 100 );

  uint32_t presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_dma_n( presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    pr( "ADC init failed, errno= " ); pr_d( errno ); pr( NL );
    return 1;
  }
  pr_ADC_state();

  snprintf( pbuf, pbufsz-1, "Timer: tim_freq_in= %lu Hz / ( (%u+1)*(%u+1)) = %#.7g Hz; t_step = %#.7g s " NL,
                                    tim_freq_in,       UVAR('p'), UVAR('a'), tim_f,    t_step_f );
  pr( pbuf ); delay_ms( 1 );

  int div_val = -1;
  adc_clk = calc_ADC_clk( presc, &div_val );
  snprintf( pbuf, pbufsz-1, "ADC: n_ch= %d n=%lu adc_clk= %lu div_val= %d s_idx= %lu sampl= %lu; f_sampl_max= %lu Hz; t_wait0= %lu ms" NL,
                                  n_ch,    n,    adc_clk,     div_val,  sampl_t_idx, sampl_times_cycles[sampl_t_idx],
                                  f_sampl_max, t_wait0 );
  pr( pbuf ); delay_ms( 10 );

  memset( ADC_buf_x, n_ADC_bytes + ADCDMA_chunk_size, 0 );

  // pr( "ADC_buf.size= " ); pr_d( ADC_buf.size() );  pr( " data= " ); pr_h( (uint32_t)(ADC_buf.data()) );
  pr( "ADC data= " ); pr_h( (uint32_t)(ADC_buf_x) );
  pr( " n_ADC_bytes= " ); pr_d( n_ADC_bytes ); pr( NL );
  adc_end_dma = 0; adc_dma_error = 0; n_series = 0; n_series_todo = n;
  UVAR('b') = 0; UVAR('g') = 0; UVAR('e') = 0;   UVAR('x') = 0; UVAR('y') = 0; UVAR('z') = 0;

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  // log_add( "start" NL );
  if( ADC_Start_DMA_n( &hadc1, (uint32_t*)ADC_buf_x, n_ADC_bytes, ADCDMA_chunk_size, 2 ) != HAL_OK )   {
    pr( "ADC_Start_DMA_n error = "  ); pr_h( hdma_adc1.ErrorCode );  pr( NL );
    return 1;
  }
  pr_DMA_state();
  // log_add( "TI_0" NL );
  pr( "Waiting for pot state change 0->1" NL );
  while(  pot_tick ) { delay_ms( 1 ); }
  while( !pot_tick ) { delay_ms( 1 ); }
  tim2_init( UVAR('p'), UVAR('a') );
  // log_add( "TI_1" NL );

  delay_ms( t_wait0 );
  for( uint32_t ti=0; adc_end_dma == 0 && ti<(uint32_t)UVAR('t'); ++ti ) {
    delay_ms(1);
  }
  TickType_t tcc = xTaskGetTickCount();
  delay_ms( 10 ); // to settle all

  tim2_deinit();
  // log_add( "TD" NL );
  pr_DMA_state();
  HAL_ADC_Stop_DMA( &hadc1 ); // needed
  if( adc_end_dma == 0 ) {
    pr( "Fail to wait DMA end " NL );
  }
  if( adc_dma_error != 0 ) {
    pr( "Found DMA error "  ); pr_h( adc_dma_error ); pr( NL );
  }
  pr( "  tick: "); pr_d( tcc - tc00 );
  pr( " good= " ); pr_d( UVAR('g') );   pr( " err= " ); pr_d( UVAR('e') );
  pr( NL );

  out_to_curr( 2, 0 );
  if( n_series_todo > 2 ) {
    pr( "....." NL );
    out_to_curr( 4, n_series_todo-2 );
  }

  pr( NL );

  pr_ADC_state();
  pr( NL );

  delay_ms( 10 );

  return 0;
}

int  print_curr( const char *s )
{
  if( !s  || !*s ) {
    return 0;
  }
  UINT l = strlen( s );
  if( out_file.obj.fs == nullptr ) {
    pr( s );
    delay_ms( 2 );
    return l;
  }

  // TMP: test old unbuffered
  // int rc = f_puts( s, &out_file );
  // return rc;

  int rc = add_to_file( s );
  if( rc < 1 ) {
    return 0;
  }
  return rc;
}

int out_to_curr( uint32_t n, uint32_t st )
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
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0, tc1 = tc0;
  int rc = 0;
  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    t = t_step_f * ii;
    snprintf( pbuf, pbufsz-1, "%#12.7g  ", t );
    for( int j=0; j< n_ch; ++j ) {
      int vv = ADC_buf_x[ii*n_ch+j] * 10 * UVAR('v') / 4096;
      ifcvt( vv, 10000, buf, 4 );
      strcat( pbuf, buf ); strcat( pbuf, "  " );
    }
    strcat( pbuf, i2dec( ii, buf ) );
    strcat( pbuf, NL );
    int l_w = print_curr( pbuf );
    if( l_w < 1 ) {
      pr( NL "Write error: errno = " ); pr_d( errno ); pr( " i= " ); pr_d( i ); pr( NL );
      print_file_info( &out_file );
      print_fsinfo( &fs );
      rc = 1;
      break;
    }
    idle_flag = 1;
    if( ( i % 1000 ) == 0 && i > 0 && out_file.obj.fs != nullptr ) {
      tc0 = xTaskGetTickCount();
      pr( "written " ); pr_d( i ); pr( " lines, "  ); pr_d( tc0 - tc00 ); pr( " ms, dlt = " ); pr_d( tc0 - tc1 ); pr( NL );
      tc1 = tc0;
      delay_ms( 10 );
    }
  }
  return rc;
}

int cmd_out( int argc, const char * const * argv )
{
  out_file.obj.fs = nullptr;
  uint32_t n = arg2long_d( 1, argc, argv, n_series_todo, 0, n_series_todo+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,             0, 0, n_series_todo-2 );

  out_to_curr( n, st );

  return 0;
}

void reset_filebuf()
{
  fbuf_pos = 0;
}

int add_to_file( const char *s )
{
  if( !s || !*s ) {
    return 0;
  }
  uint32_t l = strlen( s );
  if( l >= fbuf_maxlinesz ) {
    return 0;
  }
  memcpy( fbuf + fbuf_pos, s, l );
  fbuf_pos += l;
  if( fbuf_pos < fbuf_hwmark ) {
    return l;
  }
  UINT l_w;
  FRESULT rc = f_write( &out_file, fbuf, fbuf_hwmark, &l_w );
  fbuf_pos -= fbuf_hwmark;
  memmove( fbuf, fbuf + fbuf_hwmark, fbuf_pos );

  if( rc == FR_OK ) {
    return l;
  }
  errno = 5000 + rc;
  return 0;
}


int flush_file()
{
  if( fbuf_pos < 1 ) {
    return 0;
    // on_save_state = 0;
  }
  UINT l_w;
  FRESULT rc = f_write( &out_file, fbuf, fbuf_pos, &l_w );
  // on_save_state = 0;
  if( rc == FR_OK ) {
    return l_w;
  }
  errno = 5000 + rc;
  fbuf_pos = 0;
  return 0;
}

int do_outsd( const char *afn, uint32_t n, uint32_t st )
{
  const char *fn = afn ? afn : fn_auto;

  // out_file.obj.fs = nullptr;

  pr( "Output to \"" ); pr( fn ); pr( "\" n= " ); pr_d( n ); pr( " st= ") ; pr_d( st ); pr( NL );
  on_save_state = 1;
  delay_ms( 10 );
  if( SDIO_on() != 0 ) {
    SDIO_off();
    on_save_state = 0;
    return 1;
  }
  FRESULT r = f_open( &out_file, fn, FA_WRITE | FA_OPEN_ALWAYS );
  int rc = r;
  if( r == FR_OK ) {
    print_file_info( &out_file );
    print_fsinfo( &fs );
    reset_filebuf();
    rc = out_to_curr( n, st );
    flush_file();
    f_close( &out_file );
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }
  SDIO_off();
  on_save_state = 0;
  out_file.obj.fs = nullptr;

  return rc;
}

int cmd_outsd( int argc, const char * const * argv )
{
  const char *fn = fn_auto;
  if( argc > 1 ) {
    fn = argv[1];
  }

  uint32_t n = arg2long_d( 2, argc, argv, n_series_todo, 0, n_series_todo+1 ); // number output series
  uint32_t st= arg2long_d( 3, argc, argv,             0, 0, n_series_todo-2 );

  return do_outsd( fn, n, st );
}

int cmd_potctl( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Error: need  V1 [ V2 T ]" NL );
    return 1;
  }
  int v1 = arg2long_d( 1, argc, argv, 128,    0, pot_steps-1 );
  int v2 = arg2long_d( 2, argc, argv, pot_v1, 0, pot_steps-1 );
  int t  = arg2long_d( 3, argc, argv, 1000,   1, 100000 );
  return do_potctl( v1, v2, t );
}

int do_potctl( int v1, int v2, int t )
{
  pot_v1 = v1;
  pot_v2 = v2;
  pot_t  = t;
  pot_r1 = UVAR('o') + pot_v1 * UVAR('r') / pot_steps;
  pot_r2 = UVAR('o') + pot_v2 * UVAR('r') / pot_steps;

  fn_auto[0] = ( pot_v1 == pot_v2 ) ? 'r' : 'm';
  fn_auto[1] = '_'; fn_auto[2] = '\0';
  i2dec( pot_r1, fn_auto + 2, 6, '0' );
  strcat( fn_auto, ".txt" );

  pr( "v1 = " ); pr_d( pot_v1 ); pr( " v2 = " ); pr_d( pot_v2 ); pr( " pot_t = " ); pr_d( pot_t );  pr( NL );
  pr( "R1 = " ); pr_d( pot_r1 ); pr( " R2 = " ); pr_d( pot_r2 );
  pr( " fn=\"" ); pr( fn_auto ); pr( "\"" NL );
  return 0;
}

void print_file_info( const FIL *f )
{
  pr( "FIL info: " );
  if( !f ) {
    pr( "zero ptr!" NL );
    return;
  }
  pr( " fs= "     )     ; pr_a( f->obj.fs )   ;
  pr( " id= "     )     ; pr_h( f->obj.id )   ;
  pr( " flag= "   )     ; pr_h( f->flag )     ;
  pr( " err= "    )     ; pr_h( f->err )      ;
  pr( " fptr= "   )     ; pr_d( f->fptr )     ; pr( NL );
  pr( " fsize= "  )     ; pr_d( f->obj.objsize ) ;
  pr( " sclust= " )     ; pr_d( f->obj.sclust )  ;
  pr( " clust= "  )     ; pr_d( f->clust )   ; pr( NL );

  #if !_FS_READONLY
    pr( " dir_sect= " ) ; pr_d( f->dir_sect ) ;
    pr( " dir_ptr= " )  ; pr_a( f->dir_ptr )  ;
  #endif
  #if _USE_FASTSEEK
    pr( " cltbl= " )    ; pr_a( f->cltbl )    ;
  #endif
  #if _FS_LOCK
    pr( " lockid= " )   ; pr_d( f->obj.lockid )   ;
  #endif

  pr( NL );
}

void print_fsinfo( const FATFS *fs )
{
  pr( "FATFS info: " );
  if( !fs ) {
    pr( "zero ptr!" NL );
    return;
  }
  pr( " fs_type= " )    ; pr_d( fs->fs_type )    ;
  pr( " drv= " )        ; pr_d( fs->drv )        ;
  pr( " csize= " )      ; pr_d( fs->csize )      ;
  pr( " n_fats= " )     ; pr_d( fs->n_fats )     ; pr( NL ) ;
  pr( " wflag= " )      ; pr_d( fs->wflag )      ;
  pr( " fsi_flag= " )   ; pr_d( fs->fsi_flag )   ;
  pr( " id= " )         ; pr_d( fs->id )         ; pr( NL ) ;

  #if _FS_REENTRANT
  pr( "  sobj= " )      ; pr_a( fs->sobj )       ;
  #endif
  #if !_FS_READONLY
  pr( " last_clust= " ) ; pr_d( fs->last_clst ) ;
  pr( " free_clust= " ) ; pr_d( fs->free_clst ) ;
  #endif
  #if _FS_RPATH
  pr( " cdir= " )       ; pr_d( fs->cdir )       ; pr( NL ) ;
  #endif

  pr( " n_fatent= " )   ; pr_d( fs->n_fatent )   ;
  pr( " fsize= " )      ; pr_d( fs->fsize )      ;
  pr( " volbase= " )    ; pr_d( fs->volbase )    ;
  pr( " fatbase= " )    ; pr_d( fs->fatbase )    ;
  pr( " dirbase= " )    ; pr_d( fs->dirbase )    ; pr( NL ) ;
  pr( " database= " )   ; pr_d( fs->database )   ;
  pr( " winsect= " )    ; pr_d( fs->winsect )    ; pr( NL ) ;

  // char vol_buf[32]; vol_buf[0] = '\0';
  // uint32_t vsn = 0, n_f_clust = 0;
  // FATFS *lfs;
  // if( fs.fs_type > 0 ) {
  //   f_getlabel( fsn, vol_buf, &vsn );
  //   pr( "volume_label: \"" ); pr( vol_buf ); pr( "\" vsn=" ); pr_d( vsn ); pr( NL );
  //   f_getfree( fsn, &n_f_clust, &lfs );
  //   pr( "free_clust: " ); pr_d( n_f_clust ); pr( " kB:" ); pr_d( n_f_clust * fs.csize / 2 ) ;pr( NL );
  // }

}

void task_pot( void *prm UNUSED_ARG )
{
  TickType_t tc0 = xTaskGetTickCount();
  while( 1 ) {
    if( on_save_state ) {
      delay_ms( 1000 );
      continue;
    }
    if( pot_tick ) {
      digpot.send_reg1_8bit( 0, pot_v2 );
      leds.reset( BIT2 );
      pot_tick = 0;
    } else {
      digpot.send_reg1_8bit( 0, pot_v1 );
      leds.set( BIT2 );
      pot_tick = 1;
    }
    vTaskDelayUntil( &tc0, pot_t );
  }
  vTaskDelete(NULL);
}

int cmd_iterrun( int argc, const char * const * argv )
{
  int st    = arg2long_d( 1, argc, argv, 0, 0, pot_steps  );
  int n     = arg2long_d( 2, argc, argv, 5, 1, pot_steps  );
  int step  = arg2long_d( 3, argc, argv, 1, 0, pot_steps-1 );

  if( st + n * step >= pot_steps ) {
    n = ( pot_steps - 1 - st ) / step;
  }
  pr( NL "iterrun: st=" ); pr_d( st ); pr( " n= " ); pr_d( n ); pr( " step= " ); pr_d( step ); pr( NL );

  int v = st, rc;
  for( int i=0; i < n; ++i ) {
    do_potctl( v, v, 1000 );
    delay_ms( 50 );
    rc = do_one_run( UVAR('n') );
    if( rc != 0 ) {
      return rc;
    }
    pr( "Measurement done..." NL );
    delay_ms( 50 );
    rc = do_outsd( nullptr, n_series_todo, 0 );
    if( rc != 0 ) {
      return rc;
    }
    v += step;
  }
  return 0;
}

void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  adc_end_dma |= 1;
  // tim2_deinit();
  UVAR('x') = hadc1.Instance->SR;
  // hadc1.Instance->SR = 0;
  if( UVAR('b') == 0 ) {
    UVAR('b') = 1;
  }
  // HAL_ADC_Stop_DMA( hadc );
  // leds.set( BIT2 );
  ++UVAR('g'); // 'g' means good
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  UVAR('y') = hadc1.Instance->SR;
  // ledsx.toggle( BIT0 );
  adc_end_dma |= 2;
  // log_add( "AEC" NL );
  // tim2_deinit();
  if( UVAR('b') == 0 ) {
    UVAR('b') = 2;
  }
  UVAR('z') = HAL_ADC_GetError( hadc );
  adc_dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  // hadc1.Instance->SR = 0;
  HAL_ADC_Stop_DMA( hadc );
  // leds.set( BIT0 );
  ++UVAR('e');
}


void HAL_ADCEx_InjectedConvCpltCallback( ADC_HandleTypeDef * /*hadc*/ )
{
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

