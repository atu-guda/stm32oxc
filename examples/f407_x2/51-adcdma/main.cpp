#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;

extern "C" {
 void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc );
 void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
 void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim );
}
void MX_ADC1_Init( uint8_t n_ch, uint32_t sampl_time );
void ADC_DMA_REINIT();
void pr_ADC_state();
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
const uint32_t tim_freq_in = 84000000; // depend in MCU, freq TODO: calculate
uint32_t t_step = 100000; // in us, recalculated before measurement
int v_adc_ref = 3250; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = 1024*32; // MCU dependent
const uint32_t n_ADC_mem_guard  = n_ADC_mem + 2 * n_ADC_ch_max; // 2 lines for guard
uint16_t adc_v0[ n_ADC_mem_guard ];
volatile int adc_end_dma = 0;
volatile int adc_dma_error = 0;
volatile uint32_t n_series = 0;
uint32_t n_series_todo = 0;
const uint32_t n_sampl_times = 7; // current number - in UVAR('s')
const uint32_t sampl_times_codes[n_sampl_times] = { // all for 25 MHz ADC clock
  ADC_SAMPLETIME_3CYCLES   , //  15  tick: 1.4 MSa,  0.72 us
  ADC_SAMPLETIME_15CYCLES  , //  27  tick: 778 kSa,  1.29 us
  ADC_SAMPLETIME_28CYCLES  , //  40  tick: 525 kSa,  1.91 us
  ADC_SAMPLETIME_56CYCLES  , //  68  tick: 309 kSa,  3.24 us
  ADC_SAMPLETIME_84CYCLES  , //  96  tick: 218 kSa,  4.57 us
  ADC_SAMPLETIME_144CYCLES , // 156  tick: 134 kSa,  7.43 us
  ADC_SAMPLETIME_480CYCLES   // 492  tick:  42 kSa, 23.43 us
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
void tim2_init( uint16_t presc = 49, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

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


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  delay_bad_ms( 200 );  leds.write( 0 );

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('p') = 84-1; // timer PSC, for 1MHz
  UVAR('a') = 99999; // timer ARR, for 10Hz
  UVAR('c') = n_ADC_ch_max;
  UVAR('n') = 8; // number of series
  UVAR('s') = 1; // sampling time index

  leds.write( 0x0F );  delay_bad_ms( 200 );


  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_USBCDC_AS_STDIO(usbcdc);

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
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 0, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= n_sampl_times ) { sampl_t_idx = n_sampl_times-1; };
  uint32_t f_sampl_ser = 21000000 / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  t_step =  (UVAR('a')+1) * (UVAR('p')+1); // in timer input ticks
  uint32_t tim_f = tim_freq_in / t_step; // timer update freq
  t_step /= 84; // * 1e6 / 84e6
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

  break_flag = 0;
  uint32_t n_ADC_bytes = n * n_ch;
  uint32_t n_ADC_bytes_guard = n_ADC_bytes + n_ch * 2;
  for( uint32_t i=0; i<n_ADC_bytes_guard;  ++i ) { // TODO: memset
    adc_v0[i] = 0;
  }
  adc_end_dma = 0; adc_dma_error = 0; n_series = 0; n_series_todo = n;
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)adc_v0, n_ADC_bytes ) != HAL_OK )   {
    pr( "ADC_Start_DMA error" NL );
  }
  tim2_init( UVAR('p'), UVAR('a') );

  delay_ms( t_wait0 );
  for( uint32_t ti=0; adc_end_dma == 0 && ti<(uint32_t)UVAR('t'); ++ti ) {
    delay_ms(1);
  }
  TickType_t tcc = xTaskGetTickCount();
  delay_ms( 10 ); // to settle all

  // HAL_ADC_Stop( &hadc1 );
  if( adc_end_dma == 0 ) {
    pr( "Fail to wait DMA end " NL );
  }
  if( adc_dma_error != 0 ) {
    pr( "Found DMA error "  ); pr_d( adc_dma_error ); pr( NL );
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
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
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
    delay_ms( 5 );
  }
  pr( NL );

  return 0;
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // tim2_deinit();
  UVAR('x') = hadc1.Instance->SR;
  hadc1.Instance->SR = 0;
  adc_end_dma |= 1;
  leds.toggle( BIT2 );
  ++UVAR('g'); // 'g' means good
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // tim2_deinit();
  UVAR('z') = HAL_ADC_GetError( hadc );
  adc_dma_error = hadc->DMA_Handle->ErrorCode;
  UVAR('y') = hadc1.Instance->SR;
  HAL_ADC_Stop_DMA( hadc );
  hadc1.Instance->SR = 0;
  hadc->DMA_Handle->ErrorCode = 0;
  adc_end_dma |= 2;
  // leds.toggle( BIT0 );
  ++UVAR('e');
}

void _exit( int rc )
{
  die4led( rc );
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

// // configs
FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

