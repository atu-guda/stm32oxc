#include <cstring>
#include <cstdlib>
#include <algorithm>

#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_debug_i2c.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <oxc_pixbuf.h>
#include <oxc_pixbuf1v.h>

#include "usbd_desc.h"
#include <usbd_cdc.h>
#include <usbd_cdc_interface.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_print, smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_cls( int argc, const char * const * argv );
CmdInfo CMDINFO_CLS { "cls", 'X', cmd_cls, " - clear screen"  };

int cmd_vline( int argc, const char * const * argv );
CmdInfo CMDINFO_VLINE { "vline", 0, cmd_vline, " [start [end]] - test vline"  };

int cmd_line( int argc, const char * const * argv );
CmdInfo CMDINFO_LINE { "line", 'L', cmd_line, " - test line"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_CLS,
  &CMDINFO_VLINE,
  &CMDINFO_LINE,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;




// -----------------------------------------------------------------------------

class SSD1306 {
  public:
   enum {
     BASE_ADDR = 0x3C,
     X_SZ = 128,
     Y_SZ = 64,
     I2C_TO  = 100,
     CMD_1 = 0x80,
     CMD_N = 0x00,
     DATA_1 = 0xC0,
     DATA_N = 0x40,
     CMD_MODE = 0x20,
     CMD_CONTRAST = 0x81,
     CMD_RAM = 0xA4, // default, output follows RAM
     CMD_FULLON = 0xA5, // output in ON, independent of RAM
     CMD_NOINVERSE = 0xA6,
     CMD_INVERSE = 0xA7,
     CMD_OFF = 0xAE,
     CMD_ON = 0xAF,
     MEM_SZ = ( X_SZ * Y_SZ / 8 )
   };
   SSD1306( I2C_HandleTypeDef &a_i2ch, uint8_t a_addr = BASE_ADDR )
     : i2ch( a_i2ch ), addr2( a_addr<<1 ) {};
   void init();
   void cmd1( uint8_t cmd );
   void cmd2( uint8_t cmd, uint8_t val );
   void data1( uint8_t d );

   void switch_on() { cmd1( CMD_ON ); };
   void switch_off() { cmd1( CMD_OFF ); };
   void contrast( uint8_t v ) { cmd2( CMD_CONTRAST, v ); };
   void full_on() { cmd1( CMD_FULLON ); };
   void on_ram() { cmd1( CMD_RAM ); };
   void no_inverse() { cmd1( CMD_NOINVERSE ); };
   void inverse() { cmd1( CMD_INVERSE ); };
   void mode_horisontal() { cmd2( CMD_MODE, 0x00 ); };
   void mode_vertical()   { cmd2( CMD_MODE, 0x01 ); };
   void mode_paged()      { cmd2( CMD_MODE, 0x02 ); };
   void out( PixBuf1V &pb );

  private:
   I2C_HandleTypeDef &i2ch;
   uint8_t addr2;
};

void SSD1306::cmd1( uint8_t cmd )
{
  uint8_t xcmd[] = { CMD_1, cmd };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xcmd, 2, I2C_TO );
}

void SSD1306::cmd2( uint8_t cmd, uint8_t val )
{
  uint8_t xcmd[] = { CMD_N, cmd, val };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xcmd, 3, I2C_TO );
}

void SSD1306::data1( uint8_t d )
{
  uint8_t xcmd[] = { DATA_1, d };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xcmd, 2, I2C_TO );
}


void SSD1306::init()
{
  uint8_t on_cmd[] = { 0x00, 0x8D, 0x14, 0xAF };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  on_cmd, 4, I2C_TO );
}



void SSD1306::out( PixBuf1V &pb )
{
  uint8_t go_00[] = { 0x00, 0xB0, 0x00, 0x10 };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  go_00, 4, I2C_TO );
  uint8_t *buf = pb.fb();
  --buf; // one byte for cmd
  *buf = DATA_N;
  HAL_I2C_Master_Transmit( &i2ch, addr2,  buf, MEM_SZ+1, I2C_TO );
}

// ----------------------------------------------------------------

PixBuf1V pb0( 128, 64 );
SSD1306 screen( i2ch );


// ----------------------------------------------------------------
STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );


  i2ch.Instance             = I2C1;
  i2ch.State                = HAL_I2C_STATE_RESET;
  i2ch.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  i2ch.Init.ClockSpeed      = 400000;
  i2ch.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  i2ch.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  i2ch.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  i2ch.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  i2ch.Init.OwnAddress1     = 0;
  i2ch.Init.OwnAddress2     = 0;
  HAL_I2C_Init( &i2ch );
  i2ch_dbg = &i2ch;


  leds.write( 0x00 );

  user_vars['t'-'a'] = 1000;
  user_vars['n'-'a'] = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  usbcdc.init();
  usbcdc.setOnSigInt( sigint );
  devio_fds[0] = &usbcdc; // stdin
  devio_fds[1] = &usbcdc; // stdout
  devio_fds[2] = &usbcdc; // stderr
  delay_ms( 50 );

  delay_ms( 10 );
  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while(1) {
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


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int st_a = 2;
  if( argc > 1 ) {
    st_a = strtol( argv[1], 0, 0 );
    pr( NL "Test0: argv[1]= \"" ); pr( argv[1] ); pr( "\"" NL );
  }
  int en_a = 127;
  if( argc > 2 ) {
    pr( NL "Test0: argv[2]= \"" ); pr( argv[2] ); pr( "\"" NL );
    en_a = strtol( argv[2], 0, 0 );
  }
  pr( NL "Test0: st_a= " ); pr_h( st_a ); pr( " en_a= " ); pr_h( en_a );

  screen.init();

  delay_ms( 500 );
  screen.contrast( 0x20 );
  delay_ms( 500 );
  screen.contrast( 0xF0 );
  delay_ms( 500 );
  // screen.full_on();
  // delay_ms( 500 );
  // screen.on_ram();
  //
  delay_ms( 500 );
  screen.inverse();
  delay_ms( 500 );
  screen.no_inverse();
  //
  //
  // delay_ms( 500 );
  // screen.switch_off();
  // delay_ms( 1000 );
  screen.switch_on();

  screen.mode_horisontal();


  pb0.fillAll( 0 );

  // uint8_t *fb = screen.fb();

  // pb0.fillAll( 1 );
  for( uint16_t co = 0; co < 64; ++ co ) {
    pb0.pix( co,   co, 1 );
    pb0.pix( 2*co, co, 1 );
  }
  pb0.pix( 10,  10, 0 );
  pb0.pix( 40,  20, 0 );

  pb0.hline(  0,  0, 128, 1 );
  pb0.hline( 11, 10, 100, 1 );
  pb0.hline( 80, 10,  90, 0 );
  pb0.hline(  0, 60, 200, 1 );
  pb0.hline(  0, 30, 200, 0 );

  for( uint16_t ro=0; ro<64; ++ro ) {
    pb0.vline( ro, 0, ro, (ro&1) );
  }

  pb0.rect( 10, 10, 117, 53, 1 );
  pb0.rect( 11, 11, 116, 52, 0 );
  pb0.box(  12, 12, 64,  51, 1 );
  pb0.box(  65, 11, 115, 51, 0 );

  pb0.fillCircle( 64, 32, 10, 1 );
  pb0.fillCircle( 64, 32, 6,  0 );
  pb0.circle( 64, 32, 15, 0 );
  pb0.circle( 64, 32, 20, 1 );

  pb0.outChar( 20, 20, 'A', &Font12,  0 );
  pb0.outStrBox( 90, 20, "String", &Font8, 1, 0, 1, PixBuf::STRBOX_ALL );
  pb0.outStrBox( 90, 40, "Str2", &Font8, 0, 1, 0, PixBuf::STRBOX_BG );

  screen.out( pb0 );

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_cls( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  pb0.fillAll( 0 );
  screen.out( pb0 );
  pr( NL "CLS end." NL );
  return 0;
}


int cmd_vline( int argc, const char * const * argv )
{
  uint16_t y0 = 0;
  if( argc > 1 ) {
    y0 = strtol( argv[1], 0, 0 );
  }
  uint16_t y1 = 64;
  if( argc > 2 ) {
    y1 = strtol( argv[2], 0, 0 );
  }
  pr( NL "vline: y0= " ); pr_d( y0 ); pr( " y1= " ); pr_h( y1 );

  for( uint16_t y = y0; y<= y1; ++y ) {
    pb0.vline( y, y/2, y, (y&1) );
  }

  screen.out( pb0 );

  pr( NL "vline end." NL );
  return 0;
}

int cmd_line( int argc, const char * const * argv )
{
  uint16_t nl = 36;
  if( argc > 1 ) {
    nl = strtol( argv[1], 0, 0 );
  }
  uint16_t dn = 360 / nl;
  if( argc > 2 ) {
    dn = strtol( argv[2], 0, 0 );
  }
  pr( NL "lines: = nl" ); pr_d( nl ); pr( " dn= " ); pr_d( dn );

  uint16_t x0 = 64, y0 = 32;

  for( uint16_t an = 0;  an <= nl; ++an ) {
    float alp = an * dn * 3.1415926 / 180;
    uint16_t x1 = x0 + 30 * cosf(alp);
    uint16_t y1 = y0 + 30 * sinf(alp);
    // pr( "x1= " ); pr_d( x1 );
    // pr( " y1= " ); pr_d( y1 ); pr( NL );
    pb0.line( x0, y0, x1, y1, (an&1) );
  }

  screen.out( pb0 );

  pr( NL "lines end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

