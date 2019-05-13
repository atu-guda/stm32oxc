#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_mpu6050.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MPU6050 accel + gyro" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_data0( int argc, const char * const * argv );
CmdInfo CMDINFO_DATA0 { "data0", 'D', cmd_data0, " - data transmission 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_DATA0,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
MPU6050 accel( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &accel;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  accel.init();
  accel.setAccScale( MPU6050::ACC_scale::accs_8g );
  accel.setDLP( MPU6050::DLP_BW::bw_44 );
  delay_ms( 50 );

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

  std_out << NL "Test0: n= " << n << " t= " << t_step << NL;
  std_out.flush();

  int16_t adata[MPU6050::mpu6050_alldata_sz];
  // accel.sleep();
  i2cd.resetDev();
  accel.setDLP( MPU6050::DLP_BW::bw_10 );
  accel.init();

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    for( auto &d : adata ) { d = 0; };
    accel.getAll( adata );
    for( int j=0; j<MPU6050::mpu6050_alldata_sz; ++j ) {
      std_out << (int)(adata[j]) << ' ';
    }

    std_out << NL;
    std_out.flush();
    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}



int cmd_data0( int argc UNUSED_ARG , const char * const * argv  UNUSED_ARG )
{
  int16_t adata[MPU6050::mpu6050_alldata_sz];
  int tick_c = HAL_GetTick();

  pr( "@0 " ); pr_d( tick_c ); pr( " " );

  for( auto &d : adata ) { d = 0; };
  accel.getAll( adata );
  for( int j=0; j<MPU6050::mpu6050_alldata_sz; ++j ) {
    pr_d( (int)(adata[j]) ); pr( " " );
  }
  pr( NL );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

