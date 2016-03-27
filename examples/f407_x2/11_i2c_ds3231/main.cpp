#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_debug_i2c.h>
#include <oxc_ds3231.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_set_time( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_TIME { "stime", 0, cmd_set_time, " hour min sec - set RTC time "  };

int cmd_set_date( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_DATE { "sdate", 0, cmd_set_date, " year month day - set RTC date "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_SET_TIME,
  &CMDINFO_SET_DATE,
  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
DS3231 rtc( &i2ch );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();
  leds.write( 0x0F );  delay_bad_ms( 200 );

  MX_I2C1_Init( i2ch );
  i2c_dbg = &rtc;


  UVAR('t') = 1000;
  UVAR('n') = 10;

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


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  rtc.resetDev();
  char time_buf[10], date_buf[14];
  uint8_t t_hour, t_min, t_sec;

  TickType_t tc0 = xTaskGetTickCount();

  break_flag = 0;
  rtc.setCtl( 0 ); // enable only clock on bat

  for( int i=0; i<n && !break_flag ; ++i ) {

    pr( "[" ); pr_d( i ); pr( "]  " );
    rtc.getTime( &t_hour, &t_min, &t_sec );
    rtc.getTimeStr( time_buf );
    pr( time_buf );
    pr( "   =   " ); pr_d( t_hour ); pr( ":" ); pr_d( t_min ); pr( ":" ); pr_d( t_sec );
    rtc.getDateStr( date_buf );
    pr( "  / " ); pr( date_buf );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_set_time( int argc, const char * const * argv )
{
  uint8_t t_hour, t_min, t_sec;
  if( argc < 4 ) {
    pr( "3 args required" NL );
    return 1;
  }
  t_hour = atoi( argv[1] );
  t_min  = atoi( argv[2] );
  t_sec  = atoi( argv[3] );
  return rtc.setTime( t_hour, t_min, t_sec );
}


int cmd_set_date( int argc, const char * const * argv )
{
  uint16_t year;
  uint8_t mon, day;
  if( argc < 4 ) {
    pr( "3 args required" NL );
    return 1;
  }
  year = atoi( argv[1] );
  mon  = atoi( argv[2] );
  day  = atoi( argv[3] );
  return rtc.setDate( year, mon, day );
}


//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

