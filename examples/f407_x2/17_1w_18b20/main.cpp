#include <cstring>
#include <cstdlib>

#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
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

int cmd_1wire0( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG );
CmdInfo CMDINFO_WIRE0 { "wire0", 'w', cmd_1wire0, " - test 1-wire"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_WIRE0,
  &CMDINFO_TEST0,
  nullptr
};

#define d_mcs delay_bad_mcs

class IoPin {
  public:
   IoPin( GPIO_TypeDef *a_gpio, uint16_t a_pin )
     : gpio( a_gpio ), pin( a_pin ) {};
   void initHW();
   void sw1() { gpio->SET_BIT_REG = pin; };
   void sw0() {
     #if RESET_BIT_SHIFT == 0
     gpio->RESET_BIT_REG = pin;
     #else
     gpio->RESET_BIT_REG = pin << RESET_BIT_SHIFT;
     #endif
   };
   void set_sw0( bool s ) { if( s ) sw1(); else sw0(); }
   uint8_t rw() {
     delay_bad_mcs( 1 );
     return rw_raw();
   };
   uint8_t rw_raw() {
     return (gpio->IDR & pin) ? 1 : 0;
   };

  protected:
   GPIO_TypeDef *gpio;
   uint16_t pin;
};

void IoPin::initHW()
{
  GPIO_enableClk( gpio );
  GPIO_InitTypeDef gio;
  gio.Pin   = pin;
  gio.Mode  = GPIO_MODE_OUTPUT_OD;
  gio.Pull  = GPIO_NOPULL;
  gio.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init( gpio, &gio );
}


class OneWire {
  public:
   enum {
     T_W1_L =  6, T_W1_H = 64,
     T_W0_L = 60, T_W0_H = 10,
     T_R_L = 6, T_R_H1 = 9, T_R_H2 = 55,
     T_RST_L = 480, T_RST_H1 = 70, T_RST_H2 = 410
   };
   OneWire( IoPin  &a_p )
     : p( a_p ) {};
   void initHW() { p.sw1(); };
   void eot()    { p.sw1(); };

   bool reset() {
     p.sw0(); d_mcs( T_RST_L ); p.sw1(); d_mcs( T_RST_H1 );
     bool r = ! p.rw_raw(); d_mcs( T_RST_H2 );
     return r;
   };
   void w0() { p.sw0(); d_mcs( T_W0_L ); p.sw1(); d_mcs( T_W0_H ); };
   void w1() { p.sw0(); d_mcs( T_W1_L ); p.sw1(); d_mcs( T_W1_H ); };
   void write1bit( bool b ) { if( b ) w1(); else w0(); };
   uint8_t read1bit() {
     p.sw0(); d_mcs( T_R_L ); p.sw1(); d_mcs( T_R_H1 );
     uint8_t r = p.rw_raw(); d_mcs( T_R_H2 );
     return r;
   };
   void write1byte( uint8_t w ) { write_buf( &w, 1 ); };
   uint8_t read1byte(){ uint8_t r; read_buf( &r, 1 ); return r; };
   void write_buf( const uint8_t *b, uint16_t l );
   void read_buf( uint8_t *b, uint16_t l );
  protected:
   IoPin &p;
};

void OneWire::write_buf( const uint8_t *b, uint16_t l )
{
  if( !b || !l ) { return; }
  for( uint16_t i=0; i<l; ++i,++b ) {
    uint8_t c = *b;
    for( uint8_t j=0; j<8; ++j ) {
      write1bit( c & 1 );
      c >>= 1;
    }
    delay_ms( 1 );
    // d_mcs( T_R_L );
  };
}

void OneWire::read_buf( uint8_t *b, uint16_t l )
{
  if( !b || !l ) { return; }
  for( uint16_t i=0; i<l; ++i,++b ) {
    uint8_t c = 0;
    for( uint8_t j=0; j<8; ++j ) {
      if( read1bit() ) { c |= (1<<j); }
    }
    *b = c;
    delay_ms( 1 );
    // d_mcs( T_R_L );
  };
}


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

IoPin pin_wire1( GPIOE, GPIO_PIN_15 );
OneWire wire1( pin_wire1 );

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  pin_wire1.initHW();
  wire1.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
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
  wire1.initHW();
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
  int n = user_vars['n'-'a'];
  uint32_t t_step = user_vars['t'-'a'];
  if( argc > 1 ) {
    n = strtol( argv[1], 0, 0 );
  }
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  int prty = uxTaskPriorityGet( 0 );
  pr_sdx( prty );
  const char *nm = pcTaskGetTaskName( 0 );
  pr( "name: \"" ); pr( nm ); pr( "\"" NL );

  // log_add( "Test0 " );
  // TickType_t tc0 = xTaskGetTickCount();

  break_flag = 0;
  bool out_flag = user_vars['o'-'a'];
  for( int i=0; i<n && !break_flag; ++i ) {
    pin_wire1.set_sw0( i & 1 );
    if( out_flag ) {
      uint16_t iv = pin_wire1.rw();
      pr( "i= " ); pr_d( i );
      pr( "  iv= " ); pr_h( iv );
      pr( NL );
    }
    delay_bad_mcs( t_step );
    // vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;
  pin_wire1.sw1();

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_1wire0( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  uint8_t buf[12];
  pr( NL "1wire test start." NL );
  // wire1.sw1();
  // delay_bad_mcs( 500 );
  // wire1.sw0();
  // delay_bad_mcs( 500 );
  bool have_dev = wire1.reset();
  pr_sdx( have_dev );
  if( !have_dev ) {
    return 1;
  }

  delay_ms( 1 ); // for logic analizator
  wire1.reset();
  wire1.write1byte( 0x33 ); // 33 = read ROM (only one device)
  wire1.read_buf( buf, 8 );
  dump8( buf, 8 );

  wire1.reset();
  wire1.write1byte( 0xCC ); // skip rom
  wire1.write1byte( 0xBE ); // read buf
  wire1.read_buf( buf, 8 );
  dump8( buf, 8 );

  wire1.reset();
  wire1.write1byte( 0xCC ); // skip rom
  wire1.write1byte( 0x44 ); // convert
  delay_ms( 1000 );

  wire1.reset();
  wire1.write1byte( 0xCC ); // skip rom
  wire1.write1byte( 0xBE ); // read buf
  wire1.read_buf( buf, 9 );
  dump8( buf, 9 );
  int te = buf[0] + (buf[1] << 8);
  pr_sdx( te );
  te *= 1000; te /= 16;
  pr_sdx( te );

  wire1.eot();
  delay_ms( 100 );

  pr( NL "1wire test end." NL );
  return 0;
}

//  ----------------------------- configs ----------------
//

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

