#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_ds3231.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr (see 'C')"  };

int cmd_set_time( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_TIME { "stime", 0, cmd_set_time, " hour min sec - set RTC time "  };

int cmd_set_date( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_DATE { "sdate", 0, cmd_set_date, " year month day - set RTC date "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_SET_TIME,
  &CMDINFO_SET_DATE,
  &CMDINFO_SETADDR,
  &CMDINFO_TEST0,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
DS3231 rtc( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
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
    delay_ms_until_brk( &tc0, t_step );
  }

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

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Need addr [1-127]" NL );
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  rtc.setAddr( addr );
  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

