#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test stepmotor" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [N] [01] - test step 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

PinsOut motor { BOARD_MOTOR_DEFAULT_GPIO, BOARD_MOTOR_DEFAULT_PIN0, 4 };

const uint8_t half_steps4[] = { 1, 3, 2, 6, 4, 12, 8, 9 };
const uint8_t full_steps4[] = { 1, 2, 4, 8 };
const uint8_t half_steps3[] = { 1, 3, 2, 6, 4, 5 };

struct MotorMode {
  int n_steps;
  const uint8_t *steps;
};

MotorMode m_modes[] = {
  { 8, half_steps4 },
  { 4, full_steps4 },
  { 6, half_steps3 }, // 3-phase modes
  { 3, full_steps4 }  // part of 4-phase
};
const int n_modes = sizeof(m_modes)/sizeof(MotorMode);


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;

  motor.initHW();

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
  static int ph = 0;
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

  int m = UVAR('m');
  if( m >= n_modes ) {
    m = 0;
  }

  const uint8_t *steps = m_modes[m].steps;
  int ns = m_modes[m].n_steps;
  if( ph >= ns ||  ph < 0 ) { ph = 0; }

  int d = 1;
  if( argc > 2  && argv[2][0] == '-' ) {
    d = ns - 1; // % no zero
  }
  STDOUT_os;
  os << NL "Test0: n= " << n << " t= " << t_step << " m= "  << m << " d= "  << d <<  NL;

  if( n < 1 ) {
    motor.write( 0 );
    ph = 0;
    os <<  NL "Motor off." NL;
    return 0;
  }


  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    if( t_step > 500 ) {
      os <<  " Step  i= " <<  i <<  " ph: "  <<  ph  <<  " v: "  <<  steps[ph]
         <<  "  tick: " << ( HAL_GetTick() - tm0 )   <<  NL;
      os.flush();
    }
    motor.write( steps[ph] );
    ph += d;
    ph %= ns;

    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

