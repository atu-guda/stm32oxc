#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

uint8_t *sdram_mem = (uint8_t *)(0xD0000000);
extern "C" {
 void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim );
}
void MX_ADC1_Init( uint8_t n_ch, uint32_t sampl_time );
void pr_ADC_state();
ADC_HandleTypeDef hadc1;
const uint32_t tim_freq_in = 100000000; // depend in MCU, freq TODO: calculate
uint32_t t_step = 100000; // in us, recalculated before measurement
int v_adc_ref = 3250; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = 1024*4096; // SDRAM: 4MB
const uint32_t n_ADC_mem_guard  = n_ADC_mem + 2 * n_ADC_ch_max; // 2 lines for guard
// uint16_t adc_v0[ n_ADC_mem_guard ];
uint16_t *adc_v0 = (uint16_t*)(sdram_mem);
volatile uint32_t n_cvt = 0;
volatile uint32_t n_series = 0;
uint32_t n_cvt_todo = 0;
uint32_t n_series_todo = 0;

BOARD_CONSOLE_DEFINES;

TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc = 49, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_OUT,
  nullptr
};



void MX_FMC_Init(void);
void BSP_SDRAM_Initialization_sequence( uint32_t RefreshCount );


int main(void)
{
  BOARD_PROLOG;


  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('p') = 99; // timer PSC, for 1MHz
  UVAR('a') = 99999; // timer ARR, for 10Hz
  UVAR('c') = n_ADC_ch_max;
  UVAR('n') = 8; // number of series
  UVAR('s') = 1; // sampling time index

  leds.write( 0x0F );  delay_bad_ms( 200 );
  MX_FMC_Init();
  leds.write( 0x01 );  delay_bad_ms( 200 );
  BSP_SDRAM_Initialization_sequence( 0 ); // 0 if fake
  leds.write( 0x0A );  delay_bad_ms( 500 );


  // MX_ADC1_Init( 4, ADC_SAMPLETIME_28CYCLES );
  delay_bad_ms( 10 );
  // tim2_init( UVAR('p'), UVAR('a') );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}

void pr_ADC_state()
{
  if( UVAR('d') > 0 ) {
    pr_shx( ADC1->SR );
    pr_shx( ADC1->CR1 );
    pr_shx( ADC1->CR2 );
    pr_shx( ADC1->SMPR2 );
    pr_shx( ADC1->SQR1 );
    pr_shx( ADC1->SQR3 );
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
  char buf[32];
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  pr_sdx( n_ADC_series_max );
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= adc_n_sampl_times ) { sampl_t_idx = adc_n_sampl_times-1; };
  uint32_t f_sampl_ser = 25000000 / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  t_step =  (UVAR('a')+1) * (UVAR('p')+1); // in timer input ticks
  uint32_t tim_f = tim_freq_in / t_step; // timer update freq
  t_step /= 100; // * 1e6 / 1e8
  uint32_t t_wait0 = n  * t_step / 1000;
  if( t_wait0 < 1 ) { t_wait0 = 1; }

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  pr( NL "Test0: n= " ); pr_d( n ); pr( " n_ch= " ); pr_d( n_ch );
  pr( " tim_f= " ); pr_d( tim_f );
  pr( " t_step= " ); pr_d( t_step );
  pr( " us;  f_sampl_ser= " ); pr_d( f_sampl_ser );
  pr( " t_wait0= " ); pr_d( t_wait0 );  pr( NL );
  ifcvt( t_step, 1000000, buf, 6 );
  pr( " t_step= " ); pr( buf );
  pr( NL );
  // uint16_t v = 0;
  tim2_deinit();

  pr_ADC_state();
  hadc1.Instance->SR = 0;
  HAL_ADC_MspDeInit( &hadc1 );
  delay_ms( 10 );
  HAL_ADC_MspInit( &hadc1 );
  MX_ADC1_Init( n_ch, sampl_t_idx );
  delay_ms( 10 );

  // log_add( "Test0 " );

  uint32_t n_ADC_bytes = n * n_ch;
  uint32_t n_ADC_bytes_guard = n_ADC_bytes + n_ch * 2;
  for( uint32_t i=0; i<n_ADC_bytes_guard;  ++i ) { // TODO: memset
    adc_v0[i] = 0;
  }
  n_series = n_cvt = 0; n_cvt_todo = n * n_ch; n_series_todo = n;
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  if( HAL_ADC_Start_IT( &hadc1 ) != HAL_OK )   {
    pr( "ADC_Start_IT error" NL );
  }
  delay_ms( 1 ); // TODO: check
  tim2_init( UVAR('p'), UVAR('a') );

  idle_flag = 1;
  delay_ms( t_wait0 ); // main wait
  for( uint32_t ti=0;  n_cvt < n_cvt_todo && ti<(uint32_t)UVAR('t'); ++ti ) { // additiona wait
    delay_ms(10);
  }
  TickType_t tcc = xTaskGetTickCount();
  delay_ms( 10 ); // to settle all

  // HAL_ADC_Stop( &hadc1 );
  if( n_cvt < n_cvt_todo ) {
    pr( "Not all series receved " NL );
  }
  pr( "  tick: "); pr_d( tcc - tc00 );
  pr( NL );

  bool was_hole = false;
  for( uint32_t i=0; i< (n_series_todo+2); ++i ) { // +2 = show guard
    if( i > 2 && i < n_series_todo - 2 ) {
      if( ! was_hole ) {
        was_hole = true;
        pr( "....." NL );
      }
      continue;
    }
    for( int j=0; j< n_ch; ++j ) {
      // pr_d( adc_v0[i*n_ch+j] ) ; pr( "\t" );
      int vv = adc_v0[i*n_ch+j] * 10 * UVAR('v') / 4096;
      ifcvt( vv, 10000, buf, 4 );
      pr( buf ); pr( "\t" );
    }
    pr( NL );
  }
  pr( NL );

  pr_ADC_state();
  pr( NL );

  delay_ms( 10 );
  pr_TIM_state( TIM2 );

  return 0;
}

int cmd_out( int argc, const char * const * argv )
{
  char buf[32];
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };
  uint32_t n = arg2long_d( 1, argc, argv, n_series_todo, 0, n_series_todo+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,             0, 0, n_series_todo-2 );

  if( n+st >= n_series_todo+1 ) {
    n = 1 + n_series_todo - st;
  }

  uint32_t t = st * t_step;
  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    ifcvt( t, 1000000, buf, 6 );
    pr( buf ); pr( "   " );
    for( int j=0; j< n_ch; ++j ) {
      int vv = adc_v0[ii*n_ch+j] * 10 * UVAR('v') / 4096;
      ifcvt( vv, 10000, buf, 4 );
      pr( buf ); pr( "  " );
    }
    t += t_step;
    pr( NL );
    idle_flag = 1;
    delay_ms( 1 );
  }
  pr( NL );

  return 0;
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  uint16_t v = hadc->Instance->DR;
  adc_v0[n_cvt++] = v;
  // ++n_cvt;
  if( n_cvt >= n_cvt_todo ) {
    HAL_ADC_Stop_IT( hadc ); n_series = n_cvt / UVAR('c');
    ++UVAR('g'); // 'g' means good
    leds.toggle( BIT2 );
  }
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // tim2_deinit();
  UVAR('z') = HAL_ADC_GetError( hadc );
  UVAR('y') = hadc1.Instance->SR;
  hadc1.Instance->SR = 0;
  // leds.toggle( BIT0 );
  ++UVAR('e');
}


void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &tim2h );
}

// not used for now: only TRGO
void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  ++UVAR('i');
  UVAR('j') = htim->Instance->CNT;
  ++n_series;
  if( n_series < n_series_todo ) {
    // ADC1->CR2 |= 0x40000000; // SWSTART???
  } else {
    htim->Instance->CR1 &= ~1u;
    // STOP?
  }
  leds.toggle( BIT1 );
}

void HAL_ADCEx_InjectedConvCpltCallback( ADC_HandleTypeDef * /*hadc*/ )
{
}

//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

