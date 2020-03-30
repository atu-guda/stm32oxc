#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cerrno>

#include <algorithm>
#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <board_sdram.h>

#include <oxc_fs_cmd0.h>
#include <fatfs_sd_st.h>
#include <oxc_io_fatfs.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure ADC data (4ch) to SDRAM and store to SD card = POT ctl" NL;

extern "C" {
void task_pot( void *prm UNUSED_ARG );
}

SDRAM_HandleTypeDef hsdram;


extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;

ADC_Info adc;

// void adc_out_to( OutStream &os, uint32_t n, uint32_t st );
void adc_show_stat( OutStream &os, uint32_t n = 0xFFFFFFFF, uint32_t st = 0 );
void pr_ADCDMA_state();

const uint32_t ADCDMA_chunk_size = 1024; // in bytes, for now. may be up to 64k-small
HAL_StatusTypeDef ADC_Start_DMA_n( ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length, uint32_t chunkLength, uint8_t elSz );
int adc_init_exa_4ch_dma_n( ADC_Info &adc, uint32_t presc, uint32_t sampl_cycl, uint8_t n_ch );
void ADC_DMA_REINIT();

uint32_t tim_freq_in; // timer input freq
int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX_FMC; // MCU dependent, in bytes for 16-bit samples

// vector<uint16_t> ADC_buf;
uint16_t *ADC_buf_x = (uint16_t*)(SDRAM_ADDR);

volatile uint32_t n_series = 0;
uint32_t n_series_todo = 0;



TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc = 36, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
extern CmdInfo CMDINFO_OUT;
int cmd_show_stats( int argc, const char * const * argv );
extern CmdInfo CMDINFO_SHOWSTATS;
int cmd_outsd( int argc, const char * const * argv );
extern CmdInfo CMDINFO_OUTSD;

int cmd_potctl( int argc, const char * const * argv );
CmdInfo CMDINFO_POTCTL { "potctl", 'P', cmd_potctl, "v1 [ v2 T ] - set pot resistance params"  };
int cmd_iterrun( int argc, const char * const * argv );
CmdInfo CMDINFO_ITERRRUN { "iterrun", 'Z', cmd_iterrun, "v1 n step=1 - run series of measurement"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_POTCTL,
  FS_CMDS0,
  &CMDINFO_OUT,
  &CMDINFO_SHOWSTATS,
  &CMDINFO_OUTSD,
  &CMDINFO_ITERRRUN,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device
I2CClient digpot( i2cd, 0x2C );


int main(void)
{
  BOARD_PROLOG;

  bsp_init_sdram( &hsdram );

  tim_freq_in = get_TIM_in_freq( TIM2 ); // TODO: define

  MX_SDIO_SD_Init();
  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('x') = HAL_SD_GetState( &hsd ); // 0 = HAL_OK, 1 = HAL_ERROR, 2 = HAL_BUSY, 3 = HAL_TIMEOUT
  UVAR('y') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';
  UVAR('z') = f_mount( &fs, "", 1 );

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('j') = tim_freq_in;
  UVAR('p') = 17;  // for high freq, form 2MS/s (a=1) to 100 S/s (a=39999)
  UVAR('a') = 19; // timer ARR, 200 kHz, *4= 800 kS/s
  UVAR('c') = 4; // n_ADC_ch_max;
  // UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM2, base_freq ); // timer PSC, for 1MHz
  UVAR('n') = 8; // number of series
  UVAR('s') = 0; // sampling time index

  UVAR('r') = pot_r_0; // POT full scale resistance
  UVAR('o') = add_r_0; // Initial R_b

  // TODO: test on F42x, F7xx
  #ifdef PWR_CR1_ADCDC1
  PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  BOARD_POST_INIT_BLINK;

  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}



// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 100000000 ); // number of series
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)adc.n_ch_max );
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
  if( sampl_t_idx >= adc_n_sampl_times ) { sampl_t_idx = adc_n_sampl_times-1; };
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
    pr( " XferCpltCallback= "   ); pr_a( hdma_adc1.XferCpltCallback   );
    pr( " XferM1CpltCallback= " ); pr_a( hdma_adc1.XferM1CpltCallback );
    pr( " XferErrorCallback= "  ); pr_a( hdma_adc1.XferErrorCallback  );
    pr( NL );
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
  // f_puts( s, &out_file );
  int rc = add_to_file( s );
  if( rc < 1 ) {
    return 0;
  }
  return rc;
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
      leds.reset( BIT1 );
      pot_tick = 0;
    } else {
      digpot.send_reg1_8bit( 0, pot_v1 );
      leds.set( BIT1 );
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
  pr( "iterrun: st=" ); pr_d( st ); pr( " n= " ); pr_d( n ); pr( " step= " ); pr_d( step ); pr( NL );

  int v = st, rc;
  for( int i=0; i < n; ++i ) {
    do_potctl( v, v, 1000 );
    rc = do_one_run( UVAR('n') );
    if( rc != 0 ) {
      return rc;
    }
    rc = do_outsd( nullptr, n_series_todo, 0 );
    if( rc != 0 ) {
      return rc;
    }
    v += step;
  }
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

