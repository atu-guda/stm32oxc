#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <bsp/board_stm32f429discovery_sdram.h>
#include <stm32f429i_discovery_ts.h>

#include <GUI.h>
#include <WM.h>


using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

BOARD_DEFINE_LEDS;

SDRAM_HandleTypeDef hsdram;
extern LTDC_HandleTypeDef hltdc;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_cls( int argc, const char * const * argv );
CmdInfo CMDINFO_CLS { "cls", 0, cmd_cls, " - clear screen"  };
int cmd_puts( int argc, const char * const * argv );
CmdInfo CMDINFO_PUTS { "puts", 0, cmd_puts, " str [x [y]] - put string at coord"  };
int cmd_line( int argc, const char * const * argv );
CmdInfo CMDINFO_LINE { "line", 0, cmd_line, " x1 y1 x2 y2 - draw a a line"  };
int cmd_color( int argc, const char * const * argv );
CmdInfo CMDINFO_COLOR { "color", 0, cmd_color, " color_val - set a default color value"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_CLS,
  &CMDINFO_PUTS,
  &CMDINFO_LINE,
  &CMDINFO_COLOR,
  nullptr
};

uint8_t GUI_Initialized = 0;
void BSP_Pointer_Update();

extern "C" {

void task_main( void *prm UNUSED_ARG );
void task_grout( void *prm UNUSED_ARG );


}


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART1 );
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART1_SEND_TASK( usartio );
// STD_USART1_RECV_TASK( usartio );
STD_USART1_IRQ( usartio );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();

  leds.write( 0x03 );  delay_bad_ms( 200 );
  // leds.write( 0x00 );  HAL_Delay( 200 );
  leds.write( 0x00 );  delay_ms( 200 );
  bsp_init_sdram( &hsdram );
  leds.write( 0x03 );  delay_bad_ms( 200 );
  init_uart( &uah );

  BSP_TS_Init( 240, 320 );
  __HAL_RCC_CRC_CLK_ENABLE();

  GUI_Init();
  GUI_Initialized = 1;

  /* Activate the use of memory device feature */
  WM_SetCreateFlags( WM_CF_MEMDEV );
  GUI_Clear();
  GUI_SetFont( &GUI_Font20_1 );
  GUI_DispStringAt( "XXX!", (LCD_GetXSize()-100)/2, (LCD_GetYSize()-20)/2 );

  leds.write( 0x01 );  delay_bad_ms( 1000 );
  // MainTask();

  // HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );

  // usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart1_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_grout,      "grout", 2*def_stksz, nullptr,   1, nullptr );
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

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}

void task_grout( void *prm UNUSED_ARG )
{
  delay_ms( 100 );
  if( GUI_Initialized )  {
    GUI_DispStringAt( "Hello world!", (LCD_GetXSize()-100)/2, (LCD_GetYSize()-20)/2 );
  }

  while( 1 ) {
    if( GUI_Initialized )  {
      BSP_Pointer_Update();
    }
    delay_ms( 50 );
  }

  vTaskDelete(NULL);
}

void BSP_Pointer_Update()
{
  GUI_PID_STATE TS_State;
  static TS_StateTypeDef prev_state;
  TS_StateTypeDef  ts;
  uint16_t xDiff, yDiff;

  BSP_TS_GetState( &ts );

  TS_State.Pressed = ts.TouchDetected;

  xDiff = (prev_state.X > ts.X) ? (prev_state.X - ts.X) : (ts.X - prev_state.X);
  yDiff = (prev_state.Y > ts.Y) ? (prev_state.Y - ts.Y) : (ts.Y - prev_state.Y);

  if( ts.TouchDetected ) {
    leds.toggle( 0x02 );
    pr( "ts.X= " ); pr_d( ts.X );  pr( " ts.Y= " ); pr_d( ts.Y ); pr( NL );
    GUI_DispCharAt( '*', ts.X, ts.Y );
    if( (prev_state.TouchDetected != ts.TouchDetected ) ||  (xDiff > 3 ) || (yDiff > 3))  {
      prev_state = ts;
      TS_State.Layer = 0;
      TS_State.x = ts.X;
      TS_State.y = ts.Y;
      GUI_TOUCH_StoreStateEx( &TS_State );
    }
  }
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  pr( NL "Test0: n= " ); pr_d( n ); pr( " argc= " ); pr_d( argc );
  pr( NL );

  int xsz = LCD_GetXSize(), ysz = LCD_GetYSize();
  pr( "xsz= "); pr_d( xsz );   pr( " ysz= "); pr_d( ysz ); pr( NL );

  pr( NL );
  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_cls( int argc, const char * const * argv )
{
  GUI_Clear();
  break_flag = 0;  idle_flag = 1;
  pr( NL "cls end." NL );
  return 0;
}

int cmd_puts( int argc, const char * const * argv )
{
  const char *s = "?";
  if( argc > 1 ) {
    s = argv[1];
  }
  int x = arg2long_d( 2, argc, argv, 10, 0 );
  int y = arg2long_d( 3, argc, argv, 10, 0 );
  GUI_DispStringAt( s, x, y );
  break_flag = 0;  idle_flag = 1;
  pr( NL "puts end." NL );
  return 0;
}

int cmd_line( int argc, const char * const * argv )
{
  int xsz = LCD_GetXSize(), ysz = LCD_GetYSize();
  int x1 = arg2long_d( 1, argc, argv,   0, 0, xsz );
  int y1 = arg2long_d( 2, argc, argv,   0, 0, ysz );
  int x2 = arg2long_d( 3, argc, argv, xsz, 0, xsz );
  int y2 = arg2long_d( 4, argc, argv, ysz, 0, ysz );
  GUI_DrawLine( x1, y1, x2, y2 );
  break_flag = 0;  idle_flag = 1;
  pr( NL "puts end." NL );
  return 0;
}


int cmd_color( int argc, const char * const * argv )
{
  uint32_t c = arg2long_d( 1, argc, argv,   0x00FFFFFF, 0 );
  GUI_SetColor( c );
  break_flag = 0;  idle_flag = 1;
  pr( NL "puts end." NL );
  return 0;
}



void LTDC_IRQHandler(void)
{
  HAL_LTDC_IRQHandler( &hltdc );
}


//  ----------------------------- configs ----------------



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

