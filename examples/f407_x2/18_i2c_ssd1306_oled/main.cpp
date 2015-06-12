#include <cstring>
#include <cstdlib>

#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_debug_i2c.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

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

int cmd_putbyte( int argc, const char * const * argv );
CmdInfo CMDINFO_PUTBYTE { "putbyte", 'Y', cmd_putbyte, " ofs byte - pyt byte on screen"  };

int cmd_vline( int argc, const char * const * argv );
CmdInfo CMDINFO_VLINE { "vline", 0, cmd_vline, " [start [end]] - test vline"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_CLS,
  &CMDINFO_PUTBYTE,
  &CMDINFO_VLINE,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;

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
     MEM_SZ = ( X_SZ * Y_SZ / 8 ),
     PRE_BUF = 8 // special place before buffer
   };
   struct OfsData {
     uint32_t ofs1;
     uint8_t m1;
     uint32_t ofs2;
     uint8_t m2;
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
   void cls();
   void out_screen();
   uint8_t* fb() { return scr; }

   uint32_t xy2ofs( uint16_t x, uint16_t y ) {
     return x + (uint32_t)( y >> 3 ) * X_SZ;
   }
   uint8_t xy2midx( uint16_t x UNUSED_ARG, uint16_t y ) {
     return (uint8_t)( y & 0x07 );
   }
   void fillAll( uint32_t col );
   void pixx( uint16_t x, uint16_t y, uint32_t col ); // w/o control: private?
   void pix(  uint16_t x, uint16_t y, uint32_t col );  // with control
   void hline( uint16_t x1, uint16_t y,  uint16_t x2, uint32_t col );
   void vline( uint16_t x,  uint16_t y1, uint16_t y2, uint32_t col );
  private:
   void vline0( const OfsData &od );
   void vline1( const OfsData &od );
  private:
   I2C_HandleTypeDef &i2ch;
   uint8_t addr2;
   uint8_t xscr[PRE_BUF+MEM_SZ];
   uint8_t *scr = xscr + PRE_BUF;
   const uint8_t msk_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
   const uint8_t msk_uns[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };
   const uint8_t msk_l1[8]  = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 };
   const uint8_t msk_l2[8]  = { 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };
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
  memset( scr, 0, MEM_SZ );
  uint8_t on_cmd[] = { 0x00, 0x8D, 0x14, 0xAF };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  on_cmd, 4, I2C_TO );
}

void SSD1306::cls()
{
  memset( scr, 0, MEM_SZ );
  out_screen();
}

void SSD1306::out_screen()
{
  uint8_t go_00[] = { 0x00, 0xB0, 0x00, 0x10 };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  go_00, 4, I2C_TO );
  xscr[PRE_BUF-1] = DATA_N;
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xscr+PRE_BUF-1, MEM_SZ+1, I2C_TO );
}

void SSD1306::fillAll( uint32_t col )
{
  uint8_t v = col ? 0xFF : 0;
  memset( scr, v, MEM_SZ );
}

void SSD1306::pixx( uint16_t x, uint16_t y, uint32_t col )
{
  uint32_t ofs = xy2ofs( x, y);
  uint8_t midx = xy2midx( x, y );
  if( col ) {
    scr[ofs] |= msk_set[midx];
  } else {
    scr[ofs] &= msk_uns[midx];
  }
}

void SSD1306::pix(  uint16_t x, uint16_t y, uint32_t col )
{
  if( x >= X_SZ || y >= Y_SZ ) {
    return;
  }
  pixx( x, y, col );
}

void SSD1306::hline( uint16_t x1,uint16_t y, uint16_t x2, uint32_t col )
{
  if( x2 < x1 ) {
    uint16_t t = x2; x2 = x1; x1 = t;
  };
  if( x1 >= X_SZ || y >= Y_SZ ) {
    return;
  }
  if( x2 >= X_SZ ) {
    x2 = X_SZ-1;
  }
  uint32_t ofs = xy2ofs(  x1, y );
  uint8_t midx = xy2midx( x1, y );
  uint16_t n = x2 - x1;

  if( col ) {
    uint8_t m = msk_set[midx];
    for( uint16_t i=0; i<n; ++i, ++ofs ) {
      scr[ofs] |= m;
    }
  } else {
    uint8_t m = msk_uns[midx];
    for( uint16_t i=0; i<n; ++i, ++ofs ) {
      scr[ofs] &= m;
    }
  }
}

void SSD1306::vline( uint16_t x, uint16_t y1, uint16_t y2, uint32_t col )
{
  if( y2 < y1 ) {
    uint16_t t = y2; y2 = y1; y1 = t;
  };
  if( x >= X_SZ || y1 >= Y_SZ ) {
    return;
  }
  if( y2 >= Y_SZ ) {
    y2 = Y_SZ-1;
  }
  OfsData od;
  od.ofs1  = xy2ofs( x, y1 );
  uint8_t midx1 = xy2midx( x, y1 );
  od.m1    = msk_l1[midx1];
  od.ofs2  = xy2ofs( x, y2 );
  uint8_t midx2 = xy2midx( x, y2 );
  od.m2    = msk_l2[midx2];

  if( od.ofs1 == od.ofs2 ) { // single segment
    od.m1 &= od.m2;
  }
  if( col ) {
    vline1( od );
  } else {
    vline0( od );
  }
}

void SSD1306::vline0( const OfsData &od )
{
  scr[od.ofs1] &= ~od.m1;
  if( od.ofs1 == od.ofs2 ) { // single segment
    return;
  }

  scr[od.ofs2] &= ~od.m2;
  for( uint32_t o = od.ofs1+X_SZ; o < od.ofs2; o+=X_SZ ) {
    scr[o] = 0;
  }

}

void SSD1306::vline1( const OfsData &od )
{
  scr[od.ofs1] |= od.m1;
  if( od.ofs1 == od.ofs2 ) { // single segment
    return;
  }

  scr[od.ofs2] |= od.m2;
  for( uint32_t o = od.ofs1+X_SZ; o < od.ofs2; o+=X_SZ ) {
    scr[o] = 0xFF;
  }
}



// ----------------------------------------------------------------

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
  // delay_ms( 500 );
  // screen.inverse();
  // delay_ms( 500 );
  // screen.no_inverse();
  //
  //
  // delay_ms( 500 );
  // screen.switch_off();
  // delay_ms( 1000 );
  screen.switch_on();

  screen.mode_horisontal();

  // screen.cmd1( 0xB0 );
  // screen.cmd1( 0x00 );
  // screen.cmd1( 0x10 );


  screen.cls();

  // uint8_t *fb = screen.fb();

  // for( uint16_t i = 0; i<1024; ++i ) {
  //   // fb[i] = (uint8_t)((i>>2) & 0xFF);
  //   // fb[i] = 0x41;
  //   fb[i] = ( i & 1 ) ? 1 : 2;
  // }
  // fb[0] = 0x7E; fb[125] = 0xFF; fb[127] = 0x80;

  // screen.fillAll( 1 );
  for( uint16_t co = 0; co < 64; ++ co ) {
    screen.pix( co,   co, 1 );
    screen.pix( 2*co, co, 1 );
  }
  screen.pix( 10,  10, 0 );
  screen.pix( 40,  20, 0 );

  screen.hline(  0,  0, 128, 1 );
  screen.hline( 11, 10, 100, 1 );
  screen.hline( 80, 10,  90, 0 );
  screen.hline(  0, 60, 200, 1 );
  screen.hline(  0, 30, 200, 0 );

  for( uint16_t ro=0; ro<64; ++ro ) {
    screen.vline( ro, 0, ro, (ro&1) );
  }

  screen.out_screen();

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_cls( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  screen.cls();
  pr( NL "CLS end." NL );
  return 0;
}

int cmd_putbyte( int argc, const char * const * argv )
{
  uint16_t ofs = 0;
  if( argc > 1 ) {
    ofs = strtol( argv[1], 0, 0 );
  }
  if( ofs > SSD1306::MEM_SZ ) {
    ofs= 0;
  }
  uint8_t v = 0;
  if( argc > 2 ) {
    v = (uint8_t)strtol( argv[2], 0, 0 );
  }
  pr( NL "putbyte: ofs= " ); pr_d( ofs ); pr( " v= " ); pr_h( v );

  uint8_t *fb = screen.fb();
  fb[ofs] = v;
  screen.out_screen();

  pr( NL "putbyte end." NL );
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
    screen.vline( y, y/2, y, (y&1) );
  }

  screen.out_screen();

  pr( NL "putbyte end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

