#include <cstring> // memset

#include <algorithm>
#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure ADC data (4ch) to SDRAM" NL;

uint8_t *sdram_mem = SDRAM_ADDR;



ADC_Info adc;

void adc_out_to( OutStream &os, uint32_t n, uint32_t st );
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
uint16_t *adc_v0 = (uint16_t*)(sdram_mem);

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

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OUT,
  &CMDINFO_SHOWSTATS,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  tim_freq_in = get_TIM_in_freq( TIM2 ); // TODO: define

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('j') = tim_freq_in;
  UVAR('p') = 17;  // for high freq, form 2MS/s (a=1) to 100 S/s (a=39999)
  UVAR('a') = 19; // timer ARR, 200 kHz, *4= 800 kS/s
  UVAR('c') = 4; // n_ADC_ch_max;
  // UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM2, base_freq ); // timer PSC, for 1MHz
  UVAR('n') = 8; // number of series
  UVAR('s') = 0; // sampling time index

  // TODO: test on F42x, F7xx
  #ifdef PWR_CR1_ADCDC1
  PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  bsp_init_sdram();

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
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)adc.n_ch_max );

  uint32_t tim_psc = UVAR('p');
  uint32_t tim_arr = UVAR('a');

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = clamp( UVAR('s'), 0, (int)adc_n_sampl_times-1 );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  float tim_f = (float)tim_freq_in / t_step_tick; // timer update freq, Hz
  adc.t_step_f = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * adc.t_step_f * 1000 ); // in ms

  // make n a multiple of ADCDMA_chunk_size
  uint32_t lines_per_chunk = ADCDMA_chunk_size / ( n_ch * 2 );
  n = ( ( n - 1 ) / lines_per_chunk + 1 ) * lines_per_chunk;
  uint32_t n_ADC_bytes = n * n_ch * 2;

  std_out << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
          << " t_step_f= " << adc.t_step_f << " s  t_wait0= " << t_wait0 << " ms" NL;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  tim2_deinit();

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_dma_n( adc, adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    std_out << "ADC init failed, errno= " << errno << NL;
    return 1;
  }
  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }


  std_out << "# Timer: tim_freq_in= " << tim_freq_in << "  Hz / ((" << tim_psc
     << "+1)*(" << tim_arr << "+1)) =" << tim_f << " Hz; t_step = " << adc.t_step_f << " s" NL;
  delay_ms( 1 );

  int div_val = -1;
  adc.adc_clk = calc_ADC_clk( adc_presc, &div_val );
  uint32_t f_sampl_max = adc.adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );
  std_out << "# ADC: n_ch= " << n_ch << " n= " << n << " adc_clk= " << adc.adc_clk << " div_val= " << div_val
     << " s_idx= " << sampl_t_idx << " sampl= " << sampl_times_cycles[sampl_t_idx]
     << " f_sampl_max= " << f_sampl_max << " Hz" NL;
  delay_ms( 10 );

  memset( ADC_buf_x, n_ADC_bytes + ADCDMA_chunk_size, 0 );
  adc.data = ADC_buf_x;
  adc.reset_cnt();

  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  if( ADC_Start_DMA_n( &adc.hadc, (uint32_t*)ADC_buf_x, n_ADC_bytes, ADCDMA_chunk_size, 2 ) != HAL_OK )   {
    std_out <<  "# ADC_Start_DMA_n error = "   << HexInt(  adc.hdma_adc.ErrorCode )   <<  NL;
    return 10;
  }
  tim2_init( UVAR('p'), UVAR('a') );

  delay_ms_brk( t_wait0 );
  for( uint32_t ti=0; adc.end_dma == 0 && ti<(uint32_t)UVAR('t') && !break_flag;  ++ti ) {
    delay_ms(1);
  }
  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

  tim2_deinit();
  HAL_ADC_Stop_DMA( &adc.hadc ); // needed
  if( adc.end_dma == 0 ) {
    std_out <<  "Fail to wait DMA end " NL;
  } else {
    if( adc.dma_error != 0 ) {
      std_out <<  "Found DMA error " << HexInt( adc.dma_error ) <<  NL;
    } else {
      adc.n_series = n;
    }
  }
  std_out <<  "#  tick: " <<  ( tcc - tm00 )  <<  NL;

  if( adc.n_series < 20 ) {
    adc_out_to( std_out, adc.n_series, 0 );
  } else {
    adc_out_to( std_out, 4, 0 );
    std_out <<  "....." NL;
    adc_out_to( std_out, 4, adc.n_series-4 );
  }

  std_out <<  NL;

  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }

  return 0;
}

void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler( &adc.hadc );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

