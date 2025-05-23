#include <oxc_auto.h>
#include <oxc_main.h>

#include <board_sdram.h>
#include <stm32f429i_discovery_ts.h>

#include <GUI.h>
#include <WM.h>


using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

extern LTDC_HandleTypeDef hltdc;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test STEMWin graph with SDRAM" NL;
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
void task_grout( void *prm UNUSED_ARG );
} // extern "C"



int main(void)
{
  BOARD_PROLOG;

  __HAL_RCC_CRC_CLK_ENABLE();

  bsp_init_sdram();

  std_out << "# point 0" NL; delay_ms( 10 );

  BSP_TS_Init( 240, 320 );

  std_out << "# point 1" NL; delay_ms( 10 ); // hang after here

  GUI_Init();
  GUI_Initialized = 1;

  std_out << "# point 2" NL; delay_ms( 10 );

  /* Activate the use of memory device feature */
  WM_SetCreateFlags( WM_CF_MEMDEV );
  GUI_Clear();
  GUI_SetFont( &GUI_Font20_1 );
  GUI_DispStringAt( "XXX!", (LCD_GetXSize()-100)/2, (LCD_GetYSize()-20)/2 );


  UVAR('t') = 1000;
  UVAR('n') = 10;

  BOARD_POST_INIT_BLINK;

  std_out <<  NL "##################### " PROJ_NAME NL;

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


void task_grout( void *prm UNUSED_ARG )
{
  delay_ms( 1000 );
  if( GUI_Initialized )  {
    GUI_DispStringAt( "Hello world!", (LCD_GetXSize()-100)/2, (LCD_GetYSize()-20)/2 );
  }

  while( 1 ) {
    if( GUI_Initialized )  {
      BSP_Pointer_Update();
    }
    delay_ms( 50 );
  }

  // vTaskDelete(NULL);
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

  return 0;
}

int cmd_cls( int argc, const char * const * argv )
{
  GUI_Clear();
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
  return 0;
}


int cmd_color( int argc, const char * const * argv )
{
  uint32_t c = arg2long_d( 1, argc, argv,   0x00FFFFFF, 0 );
  GUI_SetColor( c );
  return 0;
}



void LTDC_IRQHandler(void)
{
  HAL_LTDC_IRQHandler( &hltdc );
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

