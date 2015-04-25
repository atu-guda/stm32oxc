#include <cstring>
#include <cstdlib>

#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usartio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

void MX_GPIO_Init(void);


// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

// SmallRL storage and config
int smallrl_print( const char *s, int l );
int smallrl_exec( const char *s, int l );
void smallrl_sigint(void);
QueueHandle_t smallrl_cmd_queue;


SmallRL srl( smallrl_print, smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int idle_flag = 0;
int break_flag = 0;


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {

void task_main( void *prm UNUSED_ARG );
void task_leds( void *prm UNUSED_ARG );
void task_gchar( void *prm UNUSED_ARG );


}

// void on_received_char( const char *s, int l );

UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();
  // MX_GPIO_Init();
  init_uart( &uah );

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );

  usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );
  leds.write( 0x00 );

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty  TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  while (1)
  {
    leds.toggle( BIT1 );
    delay_ms( 500 );
  }
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  delay_ms( 50 );
  user_vars['t'-'a'] = 1000;

  usartio.itEnable( UART_IT_RXNE );

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while (1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      srl.redraw();
    }
    idle_flag = 0;
    delay_ms( 60000 );
    // delay_ms( 1 );

  }
  vTaskDelete(NULL);
}

void task_gchar( void *prm UNUSED_ARG )
{
  char sc[2] = { 0, 0 };
  while (1) {
    int n = usartio.recvByte( sc, 10000 );
    if( n ) {
      // pr( NL "--- c='" ); pr( sc ); pr( "\"" NL );
      // leds.toggle( BIT0 );
      srl.addChar( sc[0] );
      idle_flag = 1;
    }
  }
  vTaskDelete(NULL);
}




void _exit( int rc )
{
  exit_rc = rc;
  die4led( rc );
}


int pr( const char *s )
{
  if( !s || !*s ) {
    return 0;
  }
  prl( s, strlen(s) );
  return 0;
}

int prl( const char *s, int l )
{
  // usartio.sendBlockSync( s, l );
  usartio.sendBlock( s, l );
  idle_flag = 1;
  return 0;
}

// ---------------------------- smallrl -----------------------


int smallrl_print( const char *s, int l )
{
  prl( s, l );
  return 1;
}

int smallrl_exec( const char *s, int l )
{
  exec_direct( s, l );
  return 1;
}



void smallrl_sigint(void)
{
  break_flag = 1;
  idle_flag = 1;
  leds.toggle( BIT3 );
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int a1 = 16;
  if( argc > 1 ) {
    a1 = strtol( argv[1], 0, 0 );
  }
  pr( NL "Test0: a1= " ); pr_d( a1 );
  pr( NL );

  int prty = uxTaskPriorityGet( 0 );
  pr_sdx( prty );
  const char *nm = pcTaskGetTaskName( 0 );
  pr( "name: \"" ); pr( nm ); pr( "\"" NL );

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  uint32_t t_step = user_vars['t'-'a'];
  uint32_t tm0 = HAL_GetTick();

  for( int i=0; i<a1 && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    uint32_t tmc = HAL_GetTick();
    pr( " Fake Action i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( "  ms_tick: "); pr_d( tmc - tm0 );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( 1000 );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------
void MX_GPIO_Init(void)
{
  // // putput init moved to PinsOut initHW
  //
  // __HAL_RCC_SYSCFG_CLK_ENABLE();
  // __GPIOA_CLK_ENABLE();
  // GPIO_InitTypeDef gpi;
  //
  // /* Configure  input GPIO pins : PA0 PA1 */
  // gpi.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  // // gpi.Mode = GPIO_MODE_EVT_RISING;
  // gpi.Mode = GPIO_MODE_IT_RISING;
  // gpi.Pull = GPIO_PULLDOWN;
  // HAL_GPIO_Init( GPIOA, &gpi );
  //
  // // HAL_NVIC_SetPriority( EXTI0_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  // // HAL_NVIC_SetPriority( EXTI0_IRQn, 4, 0 );
  // // HAL_NVIC_EnableIRQ( EXTI0_IRQn );
}

void init_uart( UART_HandleTypeDef *uahp, int baud )
{
  uahp->Instance = USART2;
  uahp->Init.BaudRate     = baud;
  uahp->Init.WordLength   = UART_WORDLENGTH_8B;
  uahp->Init.StopBits     = UART_STOPBITS_1;
  uahp->Init.Parity       = UART_PARITY_NONE;
  uahp->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uahp->Init.Mode         = UART_MODE_TX_RX;
  uahp->Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( uahp ) != HAL_OK )  {
    die4led( 0x08 );
  }
}


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

