#include <cstdarg>
#include <cerrno>
#include <array>

#include <oxc_auto.h>
//#include <oxc_outstr.h>
//#include <oxc_hd44780_i2c.h>
//#include <oxc_menu4b.h>
//#include <oxc_statdata.h>
// #include <oxc_ds3231.h>


#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

const char* common_help_string = "Appication coord device control" NL;

PinsOut stepdir_e1( GpioE,  0, 2 );
PinsOut stepdir_e0( GpioE, 14, 2 );
PinsOut stepdir_x(  GpioE,  8, 2 );
PinsOut stepdir_y(  GpioE, 10, 2 );
PinsOut stepdir_z(  GpioE, 12, 2 );

array stepdirs { &stepdir_x, &stepdir_y, &stepdir_z, &stepdir_e0, &stepdir_e1 };

PinOut en_motors( GpioC, 11 );

PinsOut aux3(  GpioD, 7, 4 );

// just init test
// PinsIn in_tst( GpioC, 4, 2, GpioRegs::Pull::down );

// B8  = T10.1 = PWM0
// B9  = T11.1 = PWM1
// C6  = T3.1  = PWM2
// B10 = T2.3  = PWM3?
// B11 = T2.4  = PWM4?
//
// A0:A4, ?A7 - ADC

// I2C_HandleTypeDef i2ch;
// DevI2C i2cd( &i2ch, 0 );
// HD44780_i2c lcdt( i2cd, 0x3F );
// HD44780_i2c *p_lcdt = &lcdt;
// DS3231 rtc( i2cd );


// extern TIM_HandleTypeDef htim1;
// int MX_TIM1_Init();


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " axis N [dt] - test"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  // DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


void idle_main_task()
{
}

inline void motors_off() {  en_motors = 1; }
inline void motors_on()  {  en_motors = 0; }

// ---------------------------------------- main -----------------------------------------------

int main()
{
  STD_PROLOG_UART;

  UVAR('a') =         2; // Z axis
  UVAR('t') =        10;
  UVAR('n') =      1000;
  UVAR('u') =       100;

  for_each( stepdirs.begin(), stepdirs.end(), []( auto sd) { sd->initHW();  *sd = 0; } );
  aux3.initHW(); aux3 = 0;
  en_motors.initHW();
  motors_off();
  // in_tst.initHW();

  // UVAR('e') = i2c_default_init( i2ch );
  // i2c_dbg = &i2cd;
  // i2c_client_def = &lcdt;
  // lcdt.init_4b();
  // lcdt.cls();
  // lcdt.puts("I ");

  BOARD_POST_INIT_BLINK;

  leds.reset( 0xFF );

  // test
  // delay_ms( 200 );
  // leds  = 0x0F;      delay_ms( 2000 );
  // leds  = 0x00;      delay_ms( 2000 );
  // leds[0] =  1;      delay_ms( 2000 );
  // leds[3] =  1;      delay_ms( 2000 );
  // leds[2].toggle();  delay_ms( 2000 );
  // leds[3].reset();   delay_ms( 2000 );
  // leds ^= 0x0F;      delay_ms( 2000 );
  // leds %= 0x0F;      delay_ms( 2000 );
  // leds |= 0x0F;      delay_ms( 2000 );


  pr( NL "##################### " PROJ_NAME NL );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}



int cmd_test0( int argc, const char * const * argv )
{
  int a = arg2long_d( 1, argc, argv, UVAR('a'), 0, 100000000 ); // motor index
  int n = arg2long_d( 2, argc, argv, UVAR('n'), -10000000, 100000000 ); // number of pulses with sign
  uint32_t dt = arg2long_d( 3, argc, argv, UVAR('t'), 0, 1000 ); // ticks in ms

  bool rev = false;
  if( n < 0 ) {
    n = -n; rev = true;
  }

  if( (size_t)a > size(stepdirs) ) {
    std_out << "# Error: bad motor index " << a << NL;
    return 2;
  }

  stepdirs[a]->sr( 0x02, rev );
  motors_on();
  uint32_t tm0 = HAL_GetTick(), tc0 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    // uint32_t tmc = HAL_GetTick();
    // std_out << i << ' ' << ( tmc - tm0 )  << NL;

    leds[0].toggle();
    (*stepdirs[a])[0].toggle();

    if( dt > 0 ) {
      delay_ms_until_brk( &tc0, dt );
    } else {
      delay_mcs( UVAR('u') );
    }

  }
  motors_off();

  std_out << "# Test: " << NL;
  int rc = break_flag;

  return rc + rev;
}



// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

