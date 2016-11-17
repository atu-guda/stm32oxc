#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;
extern "C" {
 void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc );
 void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
}
void MX_ADC1_Init(void);
void ADC_DMA_REINIT();
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
int v_adc_ref = 3250; // in mV, measured before test, adjust as UVAR('v')
const int n_ADC_ch = 4;
const int n_ADC_sampl  = 8;
const int n_ADC_data = n_ADC_ch * n_ADC_sampl;
const int n_ADC_data_guard = n_ADC_data + n_ADC_ch * 2;
uint16_t adc_v0[ n_ADC_data_guard ];
volatile int adc_end_dma = 0;
volatile int adc_dma_error = 0;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {

void task_main( void *prm UNUSED_ARG );


}


UART_HandleTypeDef huart1;
UsartIO usartio( &huart1, USART1 );
void MX_USART1_UART_Init(void);

STD_USART1_SEND_TASK( usartio );
// STD_USART1_RECV_TASK( usartio );
STD_USART1_IRQ( usartio );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();

  MX_USART1_UART_Init();
  leds.write( 0x0F );  delay_bad_ms( 200 );
  MX_ADC1_Init();
  leds.write( 0x0A );  delay_bad_ms( 200 );

  // usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );

  UVAR('t') = 100;
  UVAR('n') = 4;
  UVAR('v') = v_adc_ref;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart1_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
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
  SET_UART_AS_STDIO(usartio);

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  char buf[32];
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_wait = UVAR('t');
  // MX_ADC1_Init(); // more then need?
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t_wait= " ); pr_d( t_wait );  pr( NL );
  pr( "ADCx_SR= " ); pr_h( hadc1.Instance->SR );  pr( NL );
  // uint16_t v = 0;
  hadc1.Instance->SR = 0;
  ADC_DMA_REINIT();

  // log_add( "Test0 " );

  for( int i=0; i< n_ADC_data_guard; ++i ) {
    adc_v0[i] = 0;
  }
  break_flag = 0;
  adc_end_dma = 0; adc_dma_error = 0;
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)adc_v0, n_ADC_data ) != HAL_OK )   {
    pr( "ADC_Start_DMA error" NL );
  }

  for( uint32_t ti=0; adc_end_dma == 0 && ti<t_wait; ++ti ) {
    delay_ms(1);
  }
  TickType_t tcc = xTaskGetTickCount();
  // HAL_ADC_Stop( &hadc1 );
  if( adc_end_dma == 0 ) {
    pr( "Fail to wait DMA end " NL );
  }
  if( adc_dma_error != 0 ) {
    pr( "Found DMA error " NL );
  }
  pr( "  tick: "); pr_d( tcc - tc00 ); pr( NL );

  for( int i=0; i< n_ADC_sampl+2; ++i ) { // +2 = show guard
    for( int j=0; j< n_ADC_ch; ++j ) {
      // pr_d( adc_v0[i*n_ADC_ch+j] ) ; pr( "\t" );
      int vv = adc_v0[i*n_ADC_ch+j] * 10 * UVAR('v') / 4096;
      ifcvt( vv, 10000, buf, 4 );
      pr( buf ); pr( "\t" );
    }
    pr( NL );
  }

  pr( "ADCx_SR= " ); pr_h( hadc1.Instance->SR );  pr( NL );
  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  UVAR('x') = hadc1.Instance->SR;
  HAL_ADC_Stop_DMA( hadc );
  hadc1.Instance->SR = 0;
  adc_end_dma = 1;
  leds.toggle( BIT2 );
  ++UVAR('a');
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  UVAR('z') = HAL_ADC_GetError( hadc );
  UVAR('d') = hadc->DMA_Handle->ErrorCode;
  UVAR('y') = hadc1.Instance->SR;
  HAL_ADC_Stop_DMA( hadc );
  hadc1.Instance->SR = 0;
  hadc->DMA_Handle->ErrorCode = 0;
  adc_end_dma = 2; adc_dma_error = 1;
  leds.toggle( BIT0 );
  ++UVAR('e');
}

void _exit( int rc )
{
  die4led( rc );
}

// // configs
FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

