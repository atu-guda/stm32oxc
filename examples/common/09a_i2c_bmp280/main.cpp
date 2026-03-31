#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_bmp280.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use BMP280 barometer" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test BMP280"  );


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
BMP280 baro( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n =   10;
  UVAR_p = BMP280::def_i2c_addr;

  UVAR_e = i2c_default_init( i2ch /*, 400000 */ );

  i2c_dbg = &i2cd;
  i2c_client_def = &baro;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t n = arg2ulong_d( 1, argc, argv, UVAR_n, 0 );
  uint32_t t_step = UVAR_t;

  std_out << NL "Test0: n= " << n << " t= " << t_step << NL;
  std_out.flush();

  if( ! baro.check_id() ) {
    std_out << "# error: fail to detect BMP280, id= " << baro.getId() << NL;
    return 2;
  }

  if( ! baro.config( BMP280::cfg_filter_16 | BMP280::cfg_sb_1, BMP280::ctrl_mode_normal | BMP280::ctrl_osrs_t_x2 | BMP280::ctrl_osrs_p_x16) ) {
    std_out << "# error: fail to config BMP280 " NL;
    return 3;
  }

  baro.readCalibrData();
  // debug:
  // dump8( baro.getCalibr(), sizeof(BMP280::CalibrData) );
  // const auto &cd = *baro.getCalibr();
  // std_out << "# calibr: T:"
  //    << cd.dig_t1 << ' '
  //    << cd.dig_t2 << ' '
  //    << cd.dig_t3 << NL << "# P: "
  //    << cd.dig_p1 << ' '
  //    << cd.dig_p2 << ' '
  //    << cd.dig_p3 << ' '
  //    << cd.dig_p4 << ' '
  //    << cd.dig_p5 << ' '
  //    << cd.dig_p6 << ' '
  //    << cd.dig_p7 << ' '
  //    << cd.dig_p8 << ' '
  //    << cd.dig_p9 << NL;

  int p_old = 0, p_00 = 0;

  std_out << "#  t      T         P           dp        dp0     p_raw"  NL;

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  int rc = 0;
  for( uint32_t i=0; i<n && !break_flag; ++i ) {

    uint32_t tc = HAL_GetTick();
    if( ! baro.readData() ) {
      rc = 100;
      break;
    }

    baro.calc();
    int32_t p_raw = baro.get_P_raw();
    int32_t t100  = baro.get_T();
    int32_t p     = baro.get_P(); // 24674867 -> 96386.2016 Pa;
    float pf      = p / 256.0f; // 24x8
    if( i == 0 ) {
      p_old = p_00 = p;
    }
    int dp   = p - p_old;
    int dp0  = p - p_00;
    p_old   = p;
    std_out << FmtInt( (tc - tm00), 8 ) << ' ' <<  FloatMult( t100, 2, 3 ) << ' ' << pf << ' '
      << (dp/256.0f) << ' ' << (dp0/256.0f) << ' ' << p_raw << NL;

    // dump8( baro.getData(), BMP280::sz_data );

    delay_ms_until_brk( &tm0, t_step );
  }

  return rc | break_flag;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

