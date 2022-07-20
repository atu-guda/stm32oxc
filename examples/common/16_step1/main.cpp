// #include <ranges>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

class StepMotorDriverBase {
  public:
   virtual void set( uint16_t outs ) noexcept = 0;
   virtual void init() noexcept = 0;
};

class StepMotorDriverGPIO : public StepMotorDriverBase {
  public:
   explicit StepMotorDriverGPIO( PinsOut &a_pins ) noexcept : pins( a_pins ) {}
   virtual void set( uint16_t outs ) noexcept override { pins.write( outs ); };
   virtual void init() noexcept override { pins.initHW(); };
  protected:
   PinsOut &pins;
};

class StepMotorDriverGPIO_e : public StepMotorDriverBase {
  public:
   explicit StepMotorDriverGPIO_e( GpioRegs &gi, uint8_t a_start, uint8_t a_n ) noexcept : pins( gi, a_start, a_n ) {}
   virtual void set( uint16_t outs ) noexcept override { pins.write( outs ); };
   virtual void init() noexcept override { pins.initHW(); };
  protected:
   PinsOut pins;
};

class StepMotor
{
  public:
   struct MotorMode {
     std::size_t n_steps;
     const uint16_t *steps;
   };
   StepMotor( StepMotorDriverBase &a_drv, std::size_t mode ) noexcept
     : drv( a_drv )
     { setMode( mode ); };
   // void set( uint16_t v ) noexcept { drv.set( v ); }
   void off() noexcept { drv.set( 0 ); }
   void init() noexcept { drv.init(); ph = 0; }
   void setMode( size_t a_mode ) noexcept;
   size_t getMode() const noexcept { return mode; }
   void setExternMode( const uint16_t *st, size_t ns ) noexcept;
   size_t getPhase() const noexcept { return ph; }
   uint16_t getV() const noexcept { return steps[ph]; }
   void setPhase( size_t phase ) noexcept { ph = phase % n_steps; }
   void stepF() { return step(  1 ); }
   void stepB() { return step( -1 ); };
   void step( int v );
  protected:
   StepMotorDriverBase &drv;
   const uint16_t *steps { nullptr };
   size_t n_steps { 0 };
   size_t ph { 0 };
   size_t mode { 0 };
   static constexpr uint16_t half_steps4[] { 1, 3, 2, 6, 4, 12, 8, 9 };
   static constexpr uint16_t full_steps4[] { 1, 2, 4, 8 };
   static constexpr uint16_t half_steps3[] { 1, 3, 2, 6, 4, 5 };
   static constexpr uint16_t full_steps3[] { 1, 2, 4 };
   static constexpr MotorMode m_modes[] = {
     { size(full_steps4), full_steps4 },
     { size(half_steps4), half_steps4 },
     { size(half_steps3), half_steps3 },
     { size(half_steps3), full_steps3 }
   };
   static constexpr size_t n_modes = size(m_modes);
};

void StepMotor::setMode( size_t a_mode ) noexcept
{
  mode    = (mode<n_modes)? a_mode : 0;
  steps   = m_modes[mode].steps;
  n_steps = m_modes[mode].n_steps;
  ph = 0;
}

void StepMotor::setExternMode( const uint16_t *st, size_t ns ) noexcept
{
  steps   = st;
  n_steps = ns;
  ph = 0;
  mode = 0xFF; // fake
}

void StepMotor::step( int v )
{
  ph += (size_t)v;
  ph %= n_steps;
  drv.set( steps[ph] );
}

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

