#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

BOARD_DEFINE_LEDS_EX;


void MX_ADC1_Init(void);
ADC_HandleTypeDef hadc1;
int v_adc_ref = 3250; // in mV, measured before test


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


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
int init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL_EX );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL_EX );
    return 0;
  }

  HAL_Delay( 200 ); // delay_bad_ms( 200 );
  leds.write( 0x00 ); delay_ms( 200 );
  leds.write( BOARD_LEDS_ALL_EX );  HAL_Delay( 200 );

  if( ! init_uart( &uah ) ) {
      die4led( 1 );
  }
  leds.write( 0x0A );  delay_bad_ms( 200 );


  // 

  leds.write( BOARD_LEDS_ALL );  HAL_Delay( 200 );

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('v') = v_adc_ref;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
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
  SET_UART_AS_STDIO( usartio );

  default_main_loop();
  vTaskDelete(NULL);
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  char buf[32];
  MX_ADC1_Init();
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );
  uint16_t v = 0;

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  for( int i=0; i<n && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    pr( "ADC start  i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    if( HAL_ADC_Start( &hadc1 ) != HAL_OK )  {
      pr( "  !! ADC Start error" NL );
      break;
    }
    HAL_ADC_PollForConversion( &hadc1, 10 );
    pr( " ADC1.SR= " ); pr_h( ADC1->SR );
    v = 0;
    if( HAL_IS_BIT_SET( HAL_ADC_GetState( &hadc1 ), HAL_ADC_STATE_REG_EOC ) )  {
      v = HAL_ADC_GetValue( &hadc1 );
      int vv = v * 10 * UVAR('v') / 4096;
      ifcvt( vv, 10000, buf, 4 );
      pr( " v= " ); pr_d( v ); pr( " vv= " ); pr( buf );
    }

    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }

  pr( NL );
  __HAL_RCC_ADC1_CLK_DISABLE();

  return 0;
}


//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

