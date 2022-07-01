#include <cmath>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <oxc_as5600.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

PinsOut motor { BOARD_MOTOR_DEFAULT_GPIO, BOARD_MOTOR_DEFAULT_PIN0, 4 };

const uint8_t half_steps4[] = { 1, 3, 2, 6, 4, 0xC, 8, 9 };
const uint8_t full_steps4[] = { 1, 2, 4, 8 };

struct MotorMode {
  int n_steps;
  const uint8_t *steps;
};

MotorMode m_modes[] = {
  { 4, full_steps4 },
  { 8, half_steps4 }
};
const int n_modes = sizeof(m_modes)/sizeof(MotorMode);

const char* common_help_string = "App to control stepmotor and measure angle by AS5600" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_go( int argc, const char * const * argv );
CmdInfo CMDINFO_GO { "go", 'G', cmd_go, " [n] - go n steps, mode = m;"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GO,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 5;
  UVAR('n') = 360;
  UVAR('x') = 147312;
  UVAR('z') = 200;
  // config
  UVAR('c') = AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

  motor.initHW();
  motor.set( 0 );

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
  uint32_t t_step = UVAR('t');

  float k1 = (float)(UVAR('x')) / 360;

  const uint8_t *steps = m_modes[0].steps; // only full-step here
  int ns = m_modes[0].n_steps;

  std_out <<  NL "Test0: da= "  <<  da << " amax= " << amax  <<  " t= "  <<  t_step
          << " cfg= " << HexInt16( UVAR('c') ) << " k1= " << k1 << NL;

  if( da * amax <= 0.0f ) {
    std_out << "## error: bad input params" << NL;
    return 1;
  }

  int d = 1;
  if( da < 0 ) {
    d = ns - 1; // todo: nmax
  }

  ang_sens.setCfg( UVAR('c') );

  ang_sens.setStartPosCurr();
  delay_ms( 10 );
  (void)ang_sens.getAngleN();
  delay_ms( 10 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  int32_t a_ctic = 0;
  int ph = 0; // no keep phase across call
  break_flag = 0;
  for( float a=0; fabs(a)<=fabs(amax) && !break_flag; a += da ) {

    int32_t a_i = (int32_t)( a * k1 + 0.5f );
    int32_t d_a_i = a_i - a_ctic;
    int32_t d_a_i_u = (d_a_i >= 0) ? d_a_i : -d_a_i;

    for( int i=0; i< d_a_i_u && !break_flag; ++i ) {
      motor.write( steps[ph] );
      delay_ms( 5 );
      ph += d;
      ph %= ns;
    }
    a_ctic = a_i;
    delay_ms( 20 );

    auto alp_r = ang_sens.getAngleN();
    float alp_a = 1.0e-3f * AS5600::to_mDeg( ang_sens.getAngleRaw() );
    auto alp_mDeg = AS5600::to_mDeg( alp_r );
    float a_r = 1.0e-3f * alp_mDeg;
    float a_e = a - a_r;

    uint32_t tcc = HAL_GetTick();
    // auto sta = ang_sens.getStatus();

    std_out <<  a << ' ' << a_r << ' ' << a_e // << ' ' <<  alp_r
            <<  ' ' << (tcc - tm00) << ' ' << d_a_i << ' ' << alp_a << NL;

    std_out.flush();
    leds.set( 2 );
    delay_ms_brk( UVAR('z') );
    leds.reset( 2 );
  }
  motor.write( 0 );

  std_out << "#== "  << ang_sens.getAGCSetting()   << ' ' <<  ang_sens.getCORDICMagnitude()
          << ' '    << ang_sens.isMagnetDetected() << ' ' << HexInt8( ang_sens.getStatus() ) << NL;

  return 0;
}

int cmd_go( int argc, const char * const * argv )
{
  static int ph = 0; // to keep phase across call

  float nf = arg2float_d( 1, argc, argv, UVAR('n'), -1000000, 10000000 );
  bool is_deg = ( argc > 2 ) && ( argv[2][0] == 'd' );
  uint32_t t_step = UVAR('t');

  float k1 = (float)(UVAR('x')) / 360;
  int n = 0;
  if( is_deg ) {
    n = (int)( nf * k1 + 0.5f );
  } else {
    n = (int)( nf );
  }

  int m = UVAR('m');
  if( m >= n_modes ) {
    m = 0;
  }

  const uint8_t *steps = m_modes[m].steps;
  int ns = m_modes[m].n_steps;
  if( ph >= ns ||  ph < 0 ) { ph = 0; }


  std_out <<  "# Go: n= "  <<  n  <<  " t= "  <<  t_step
    << " m= " << m <<  NL;

  int d = 1;
  if( n < 0 ) {
    d = ns - 1; n = -n;
  }

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    if( t_step > 500 ) {
      std_out <<  " Step  i= " <<  i <<  " ph: "  <<  ph  <<  " v: "  <<  steps[ph]
         <<  "  tick: " << ( HAL_GetTick() - tm00 )   <<  NL;
      std_out.flush();
    }
    motor.write( steps[ph] );
    ph += d;
    ph %= ns;

    delay_ms_until_brk( &tm0, t_step );
  }

  motor.write( 0 );

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

