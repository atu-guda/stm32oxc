#include <cmath>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_as5600.h>
#include <oxc_stepmotor_gpio.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

StepMotorDriverGPIO_e m_drv( BOARD_MOTOR_DEFAULT_PIN0, 4 );
StepMotor motor( m_drv, 0 );


const char* common_help_string = "App to control stepmotor and measure angle by AS5600" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test something 0"  );
DCL_CMD_REG( go, 'G', " [n] - go n steps, mode = m;"  );
DCL_CMD_REG( me, 'M', " measure;"  );
DCL_CMD_REG( set_alp, 'S', " [v=0] - set current value"  );



I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 5;
  UVAR_n = 360;
  UVAR_x = 2048; // 147443; // 147312
  UVAR_z = 200;
  // config
  UVAR_c = AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off;

  UVAR_e = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

  motor.init();
  motor.off();

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
  float da   = arg2float_d( 1, argc, argv,   5.0f,   -360.0f,   360.0f );
  float amax = arg2float_d( 2, argc, argv, 360.0f, -36000.0f, 36000.0f );
  uint32_t t_step = UVAR_t;

  float k1 = (float)(UVAR_x) / 360;

  StatChannel sta;

  int m = UVAR_m;
  motor.setMode( m );

  std_out <<  NL "# Test0: da= "  <<  da << " amax= " << amax  <<  " t= "  <<  t_step
          << " cfg= " << HexInt16( UVAR_c ) << " k1= " << k1 << ' ' << m << NL;

  if( da * amax <= 0.0f ) {
    std_out << "## error: bad input params" << NL;
    return 1;
  }

  int d = 1;
  if( da < 0 ) {
    d = -1;
  }

  ang_sens.setCfg( UVAR_c );

  ang_sens.setStartPosCurr();
  delay_ms( 10 );
  (void)ang_sens.getAngleN();
  delay_ms( 10 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  int32_t a_ctic = 0;
  break_flag = 0;
  for( float a=0; fabs(a)<=fabs(amax) && !break_flag; a += da ) {

    int32_t a_i = (int32_t)( a * k1 + 0.5f );
    int32_t d_a_i = a_i - a_ctic;
    int32_t d_a_i_u = (d_a_i >= 0) ? d_a_i : -d_a_i;

    for( int i=0; i< d_a_i_u && !break_flag; ++i ) {
      motor.step( d );
      delay_ms( t_step );
    }
    a_ctic = a_i;

    leds[1].set();
    delay_ms_brk( UVAR_z );

    auto alp_real = ang_sens.getAngleN();
    float alp_real_deg = 1.0e-3f * AS5600::to_mDeg( alp_real );
    float alp_raw_deg  = 1.0e-3f * AS5600::to_mDeg( ang_sens.getAngleRaw() );
    float a_e = a - alp_real_deg;

    uint32_t tcc = HAL_GetTick();

    std_out <<  a << ' ' << alp_real_deg << ' ' << a_e
            <<  ' ' << (tcc - tm00) << ' ' << d_a_i << ' ' << alp_raw_deg << NL;

    std_out.flush();
    sta.add( a_e );
    leds[1].reset();
  }
  motor.off();

  std_out << "#==s "  << ang_sens.getAGCSetting()   << ' ' <<  ang_sens.getCORDICMagnitude()
          << ' '    << ang_sens.isMagnetDetected() << ' ' << HexInt8( ang_sens.getStatus() ) << NL;

  sta.calc();
  std_out << "#==a "  << sta.sd   << ' ' << max( fabsf(sta.min), fabsf(sta.max) )
          <<' '<<  sta.min << ' ' << sta.max << ' ' << sta.mean << ' ' << sta.n << NL;

  return 0;
}

int cmd_me( int argc, const char * const * argv )
{
  auto  alp_real = ang_sens.getAngleN();
  float alp_real_deg = 1.0e-3f * AS5600::to_mDeg( alp_real );
  auto  alp_raw = ang_sens.getAngleRaw();
  float alp_raw_deg  = 1.0e-3f * AS5600::to_mDeg( alp_raw );

  std_out << alp_real_deg << ' ' << alp_raw_deg
    << ' ' << alp_real << ' ' << alp_raw << ' ' << ang_sens.getN_turn() << NL;

  return 0;
}

int cmd_set_alp( int argc, const char * const * argv )
{
  float v = arg2float_d( 1, argc, argv, 0, -1000000, 10000000 );
  std_out <<  NL "# set_alp: v= " << v << NL;

  if( fabs(v) < 1e-5f ) {
    ang_sens.setStartPosCurr();
  } else {
    auto old_a = ang_sens.getAngleRaw();
    int32_t turns = (int32_t)( roundf( v / 360.0f - 0.49999f ) );
    v -= 360.0f * turns;
    int32_t via = old_a - AS5600::from_mDeg( int32_t( v * 1000.0f ) );
    uint16_t vi = (uint16_t)( via & 0x0FFF );

    // std_out << "## old_a= " << old_a << " via= " << via << " vi= " << vi << " turns= " << turns << NL;
    // std_out << "## turns= " << ang_sens.getN_turn() << ' ' << ang_sens.getAngleNoTurn() << ' ' << ang_sens.getOldVal() << NL;
    ang_sens.setStartPos( vi );
    (void)ang_sens.getAngle(); // to reset old val
    ang_sens.setN_turn( turns );
  }

  return 0;
}


int cmd_go( int argc, const char * const * argv )
{
  float nf = arg2float_d( 1, argc, argv, UVAR_n, -1000000, 10000000 );
  bool is_deg = ( argc > 2 ) && ( argv[2][0] == 'd' );
  uint32_t t_step = UVAR_t;

  float k1 = (float)(UVAR_x) / 360;
  int n = 0;
  if( is_deg ) {
    n = (int)( nf * k1 + 0.5f );
  } else {
    n = (int)( nf );
  }

  int m = UVAR_m;
  motor.setMode( m );


  std_out <<  "# Go: n= "  <<  n  <<  " t= "  <<  t_step
    << " m= " << m <<  NL;

  int d = 1;
  if( n < 0 ) {
    d = -1; n = -n;
  }

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    motor.step( d );
    delay_ms_until_brk( &tm0, t_step );
  }

  motor.off();

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

