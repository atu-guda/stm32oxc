#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_lsm303dlhc_mag.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;



const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

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
LSM303DHLC_Mag mag( &i2ch );
void MX_I2C1_Init( I2C_HandleTypeDef &i2c );

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
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  delay_bad_ms( 200 );  leds.write( 0 );

  if( !init_uart( &uah ) ) {
    die4led( 0x08 );
  }
  leds.write( 0x0A );  delay_bad_ms( 200 );

  MX_I2C1_Init( i2ch );
  i2c_dbg = &mag;


  UVAR('t') = 500;
  UVAR('n') = 50;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 1*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_UART_AS_STDIO(usartio);

  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int scale = arg2long_d( 2, argc, argv, 0, 0, 6 );
  uint32_t t_step = UVAR('t');
  const uint8_t mag_scales[] = {
    LSM303DHLC_Mag::crb_sens_1_3,
    LSM303DHLC_Mag::crb_sens_1_9,
    LSM303DHLC_Mag::crb_sens_2_5,
    LSM303DHLC_Mag::crb_sens_4_0,
    LSM303DHLC_Mag::crb_sens_4_7,
    LSM303DHLC_Mag::crb_sens_5_6,
    LSM303DHLC_Mag::crb_sens_8_1,
    0
  };
  uint8_t sens = mag_scales[scale];

  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( " scale= " ); pr_d( scale ); pr( " sens= " ); pr_h( sens );
  pr( NL );

  int scale_min = -1000; // INT16_MIN
  int scale_max =  1000; // INT16_MAX

  BarHText bar_x( 1, 1, 100, scale_min, scale_max );
  BarHText bar_y( 1, 2, 100, scale_min, scale_max );
  BarHText bar_z( 1, 3, 100, scale_min, scale_max );

  mag.resetDev();

  if( ! mag.init(  LSM303DHLC_Mag::cra_odr_15_Hz | LSM303DHLC_Mag::cra_temp_en, sens ) ) {
    pr( "Fail to init LSM303DHLC_Mag" NL );
    return 1;
  }

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  int16_t xyz[3], temp;

  term_clear();

  for( int i=0; i<n && !break_flag; ++i ) {
    mag.getMagAll( xyz );
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
    pr( " ] " /* NL  */ ); pr_d( temp );
    vTaskDelayUntil( &tc0, t_step );
  }

  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

