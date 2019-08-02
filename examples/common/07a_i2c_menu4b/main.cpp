#include <cstring>
#include <cstdlib>
#include <climits>
#include <iterator>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>
#include <oxc_menu4b.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test menu with hd4480 LCD screen" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_menu( int argc, const char * const * argv );
CmdInfo CMDINFO_MENU { "menu", 'M', cmd_menu, " N - menu action"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_MENU,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );

int init_menu4b_buttons(); // board dependent function: in separate file


int T_off = 100, T_hyst = 10, t_dt = 1000;
int fun_x1( int n );

int fun_x1( int n )
{
  std_out << "#-- fun_x1: n= " << n << NL;
  return n;
}


const Menu4bItem menu_main[] = {
  { "T_off",   &T_off,    1, -100,   5000, nullptr },
  { "T_hyst" , &T_hyst,   1,    0,   1000, nullptr },
  { "t_dt",      &t_dt, 100,  100, 100000, nullptr },
  { "fun_x1",  nullptr,   1,    0, 100000,  fun_x1 }
};

MenuState menu4b_state { menu_main, size( menu_main), "T\n" };



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10000000;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( menu4b_ev_dispatch );

  lcdt.init_4b();
  lcdt.puts_xy( 0, 1, "Ready!" );

  init_menu4b_buttons();

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = t_dt;

  char buf0[32];

  std_out << NL "# go: n= " << n << " t= " << t_step << NL;
  std_out.flush();
  lcdt.cls();

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    // i2dec_n( i, buf0 );
    uint32_t tc = HAL_GetTick();
    ifcvt( tc - tm00, 1000, buf0, 3, 4 );
    lcdt.puts_xy( 0, 0, buf0 );

    std_out << i << ' '  << buf0 << ' ' << (tc - tm00 ) << NL;
    delay_ms_until_brk( &tm0, t_step );
  }
  lcdt.puts_xy( 0, 1, "Stop!  " );

  return 0;
}


int cmd_menu( int argc, const char * const * argv )
{
  int cmd_i = arg2long_d( 1, argc, argv, 0 );
  return menu4b_cmd( cmd_i );
}

int menu4b_output( const char *s1, const char *s2 )
{
  // lcdt.cls();
  if( s1 ) {
    lcdt.puts_xy( 0, 0, s1 );
  }
  if( s2 ) {
    lcdt.puts_xy( 0, 1, s2 );
  }
  return 1;
}

void on_btn_while_run( int cmd )
{
  leds.toggle( BIT1 );
  switch( cmd ) {
    case  MenuCmd::Esc:
      break_flag = 1; errno = 10000;
      break;
    case  MenuCmd::Up:
      break;
    case  MenuCmd::Down:
      break;
    case  MenuCmd::Enter:
      // btn_run = !btn_run;
      break;
    default: break;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

