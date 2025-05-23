// #include <ranges>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_stepmotor_gpio.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


const char* common_help_string = "App to test stepmotor" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [N]  - test step"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

//PinsOut motor { BOARD_MOTOR_DEFAULT_GPIO, BOARD_MOTOR_DEFAULT_PIN0, 4 };
//StepMotorDriverGPIO m_drv( motor );
StepMotorDriverGPIO_e m_drv( BOARD_MOTOR_DEFAULT_GPIO, BOARD_MOTOR_DEFAULT_PIN0, 4 );
StepMotor mot( m_drv, 0 );



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;
  UVAR('a') = 1; // autoOff


  mot.init();

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
  int n = arg2long_d( 1, argc, argv, UVAR('n')  );
  uint32_t t_step = UVAR('t');

  auto m = mot.getMode();
  if( (int)m != UVAR('m') ) {
    mot.setMode( UVAR('m') );
  }
  m = mot.getMode();
  int d  = ( n >= 0 ) ? 1 : -1;
  int nn = ( n >= 0 ) ? n : -n;

  std_out << NL "Test0: n= " << n << " t= " << t_step << " m= "  << m << " d= " << d << NL;


  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<nn && !break_flag; ++i ) {

    mot.step( d );
    if( t_step > 500 ) {
      std_out <<  i <<  ' '  <<  mot.getPhase()  <<  ' '  <<  mot.getV()
         <<  ' ' << ( HAL_GetTick() - tm0 )   <<  NL;
      std_out.flush();
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  if( UVAR('o') ) {
    mot.off();
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

