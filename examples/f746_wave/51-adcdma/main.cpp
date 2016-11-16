#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

void MX_ADC1_Init(void);
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
int v_adc_ref = 3250; // in mV, measured before test, adjust as UVAR('v')
const int n_ADC_ch = 4;
const int n_ADC_sampl  = 8;
const int n_ADC_data = n_ADC_ch * n_ADC_sampl;
const int n_ADC_data_guard = n_ADC_data + n_ADC_ch * 2;
uint16_t adc_v0[ n_ADC_data_guard ];


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
  // char buf[32];
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );  pr( NL );
  pr( "ADCx_SR= " ); pr_h( hadc1.Instance->SR );  pr( NL );
  // uint16_t v = 0;

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  for( int i=0; i< n_ADC_data_guard; ++i ) {
    adc_v0[i] = 0;
  }
  break_flag = 0;
  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)adc_v0, n_ADC_data ) != HAL_OK )   {
    pr( "ADC_Start_DMA error" NL );
  }

  delay_ms( 500 );
  // for( int i=0; i<n && !break_flag; ++i ) {
  //   TickType_t tcc = xTaskGetTickCount();
  //   pr( "ADC start  i= " ); pr_d( i );
  //   pr( "  tick: "); pr_d( tcc - tc00 );
  //   if( HAL_ADC_Start( &hadc1 ) != HAL_OK )  {
  //     pr( "  !! ADC Start error" NL );
  //     break;
  //   }
  //   for( int ch=0; ch<n_ADC_ch; ++ch ) {
  //     HAL_ADC_PollForConversion( &hadc1, 10 );
  //     v = 0;
  //     if( HAL_IS_BIT_SET( HAL_ADC_GetState( &hadc1 ), HAL_ADC_STATE_REG_EOC ) )  {
  //       v = HAL_ADC_GetValue( &hadc1 );
  //       int vv = v * 10 * UVAR('v') / 4096;
  //       ifcvt( vv, 10000, buf, 4 );
  //       pr( " v= " ); pr_d( v ); pr( " vv= " ); pr( buf );
  //     }
  //   }

  //  pr( NL );
  //  vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  //}
  for( int i=0; i< n_ADC_sampl+2; ++i ) { // +2 = show guard
    for( int j=0; j< n_ADC_ch; ++j ) {
      pr_d( adc_v0[i*n_ADC_ch+j] ) ; pr( "\t" );
    }
    pr( NL );
  }

  pr( NL );
  pr( "ADCx_SR= " ); pr_h( hadc1.Instance->SR );  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}



void _exit( int rc )
{
  die4led( rc );
}

// // configs
FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

