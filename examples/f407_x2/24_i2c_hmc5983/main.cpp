#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_hmc5983.h>

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

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
HMC5983 mag( &i2ch );
void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();
  leds.write( 0x0F );  delay_bad_ms( 200 );

  MX_I2C1_Init( i2ch );
  i2c_dbg = &mag;


  UVAR('t') = 500;
  UVAR('n') = 50;

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
  constexpr auto n_scales = HMC5983::getNScales();
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int scale = arg2long_d( 2, argc, argv, 1, 0, n_scales-1 );
  uint32_t t_step = UVAR('t');

  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( " scale= " ); pr_d( scale );
  pr( NL );

  int scale_min = -1000000; // INT16_MIN
  int scale_max =  1000000; // INT16_MAX

  BarHText bar_x( 1, 1, 100, scale_min, scale_max );
  BarHText bar_y( 1, 2, 100, scale_min, scale_max );
  BarHText bar_z( 1, 3, 100, scale_min, scale_max );

  // mag.resetDev();

  if( ! mag.init(  HMC5983::cra_odr_75_Hz, HMC5983::Scales( scale ) ) ) {
    pr( "Fail to init HMC5983, Error= "  );
    pr_d( mag.getErr() ); pr( NL );
    return 1;
  }

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  int16_t temp;
  const int32_t *xyz;

  term_clear();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    mag.read1( 10 );
    xyz = mag.getMagAllmcGa();
    temp = mag.getTemp();
    TickType_t tcc = xTaskGetTickCount();
    bar_x.draw( xyz[0] );
    bar_y.draw( xyz[1] );
    bar_z.draw( xyz[2] );
    term_set_xy( 10, 5 );
    pr( "i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( " [ " ); pr_d( xyz[0] );
    pr( " ; " ); pr_d( xyz[1] );
    pr( " ; " ); pr_d( xyz[2] );
    pr( " ] T= " /* NL  */ ); pr_d( temp ); pr( " " NL );
    vTaskDelayUntil( &tc0, t_step );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

