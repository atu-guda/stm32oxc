#include <cstring>
#include <cstdlib>
#include <climits>
#include <iterator>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>

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

int menu4b_output( const char *s1, const char *s2 = nullptr );

volatile uint32_t menu4b_ev_global = 0;
void menu4b_ev_dispatch();

int init_menu4b_buttons(); // board dependent function: to separate file

struct Menu4bItem {
  const char *name;
  int *pv;
  int vstep = 1, vmin = INT_MIN, vmax = INT_MAX;
  int (*fun)(int) = nullptr;
};

enum MenuCmd : uint32_t {
  Esc = 1, Up = 2, Down = 3, Enter = 4, Run = 10
};


enum MenuLevel {
  ready = 0, select = 1, edit = 2
};



struct MenuState {
  const Menu4bItem* menu;
  const uint32_t menu_size;
  const char* run_cmd = "h\n";
  MenuLevel level = MenuLevel::ready;
  int index = 0; // selected element index
  int *pv   = nullptr; // ptr current var under edit (level2)
  int v = 0; // current value
  static const char *const menuLevelName[5];
};

const char *const MenuState::menuLevelName[] = { "Ready ", "Select ", "Edit ", "?3 ", "?4? " };

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


void menu4b_ev_dispatch()
{
  uint32_t ev = menu4b_ev_global;
  menu4b_ev_global = 0;
  if( ev == 0 ) {
    return;
  }

  if( ev == MenuCmd::Run ) { // special case
    // std_out << "#!!! run2" NL;
    ungets( 0, menu4b_state.run_cmd );
    return;
  }

  // TODO: more special cases

  char buf[32];
  buf[0] = 'M'; buf[1] = ' ';
  i2dec_n( ev, buf+2 );
  strcat( buf, "\n" );
  ungets( 0, buf );
}

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

  MenuCmd cmd = MenuCmd( cmd_i );

  const char *item_name = "";

  switch( menu4b_state.level ) {

    case MenuLevel::ready:
      switch( cmd ) {
        case MenuCmd::Esc:
          menu4b_state.level = MenuLevel::select; menu4b_state.index = 0;
          break;
        case MenuCmd::Up:
          break;
        case MenuCmd::Down:
          break;
        case MenuCmd::Enter:
          menu4b_ev_global = MenuCmd::Run;
          delay_ms( 10 );
          break;
        default: break;
      };
      break;

    case MenuLevel::select:
      switch( cmd ) {
        case MenuCmd::Esc:
          menu4b_state.level = MenuLevel::ready;
          break;
        case MenuCmd::Up:
          ++menu4b_state.index;
          break;
        case MenuCmd::Down:
          --menu4b_state.index;
          break;
        case MenuCmd::Enter:
          menu4b_state.level = MenuLevel::edit;
          if( menu4b_state.menu[menu4b_state.index].pv ) {
            menu4b_state.v = *menu4b_state.menu[menu4b_state.index].pv;
          } else {
            menu4b_state.v = menu4b_state.menu[menu4b_state.index].vmin;
          }
          break;
        default: break;
      };
      break;

    case MenuLevel::edit:
      switch( cmd ) {
        case MenuCmd::Esc:
          menu4b_state.level = MenuLevel::select;
          break;
        case MenuCmd::Up:
          menu4b_state.v += menu4b_state.menu[menu4b_state.index].vstep;
          break;
        case MenuCmd::Down:
          menu4b_state.v -= menu4b_state.menu[menu4b_state.index].vstep;
          break;
        case MenuCmd::Enter:
          if( menu4b_state.menu[menu4b_state.index].pv ) {
            *menu4b_state.menu[menu4b_state.index].pv = clamp( menu4b_state.v, menu4b_state.menu[menu4b_state.index].vmin, menu4b_state.menu[menu4b_state.index].vmax );
          }
          if( menu4b_state.menu[menu4b_state.index].fun ) {
            menu4b_state.menu[menu4b_state.index].fun( menu4b_state.v );
          }
          menu4b_state.level = MenuLevel::select;
          break;
        default: break;
      };
      menu4b_state.v = clamp( menu4b_state.v, menu4b_state.menu[menu4b_state.index].vmin, menu4b_state.menu[menu4b_state.index].vmax );
      break;

    default: // ???
      break;
  }

  menu4b_state.index = clamp( menu4b_state.index, 0, (int)(menu4b_state.menu_size-1) );
  item_name = menu4b_state.menu[menu4b_state.index].name;

  int show_v = 0;
  char pre_char = ' ';
  switch( menu4b_state.level ) {
    case MenuLevel::ready:
      break;
    case MenuLevel::select:
      if( menu4b_state.menu[menu4b_state.index].pv ) {
        show_v = *menu4b_state.menu[menu4b_state.index].pv;
      }
      pre_char = '>';
      break;
    case MenuLevel::edit:
      show_v = menu4b_state.v;
      pre_char = '*';
      break;
    default:
      break;
  };


  char buf[32], b0[24];
  buf[0] = pre_char; buf[1] = ' '; buf[2] = '\0';
  if( menu4b_state.level != MenuLevel::ready ) {
    strcat( buf, item_name );
    strcat( buf, "= " );
    i2dec_n( show_v, b0 );
    strcat( buf, b0 );
    strcat( buf, "   " );
  }


  menu4b_output( buf, MenuState::menuLevelName[(int)menu4b_state.level] );

  return 0;
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

void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
}

void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
}

void EXTI3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
}

void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  static uint32_t last_exti_tick = 0;

  uint32_t curr_tick = HAL_GetTick();
  if( curr_tick - last_exti_tick < 100 ) {
    return; // ignore too fast events
  }

  uint32_t cmd = 0;
  switch( pin ) {
    case GPIO_PIN_0: cmd = MenuCmd::Esc;   break;
    case GPIO_PIN_1: cmd = MenuCmd::Up;    break;
    case GPIO_PIN_2: cmd = MenuCmd::Down;  break;
    case GPIO_PIN_3: cmd = MenuCmd::Enter; break;
    default: break;
  }

  leds.toggle( BIT0 );
  if( ! on_cmd_handler ) {
    menu4b_ev_global = cmd;
  } else {
    break_flag = 1;
  }

  last_exti_tick = curr_tick;
}

int init_menu4b_buttons() // board dependent function: to separate file
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef gio = {
    .Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
    .Mode  = GPIO_MODE_IT_FALLING,
    .Pull  = GPIO_PULLUP,
    .Speed = GPIO_SPEED_FREQ_LOW
  };
  HAL_GPIO_Init( GPIOA, &gio );

  HAL_NVIC_SetPriority( EXTI0_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI0_IRQn );
  HAL_NVIC_SetPriority( EXTI1_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI1_IRQn );
  HAL_NVIC_SetPriority( EXTI2_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI2_IRQn );
  HAL_NVIC_SetPriority( EXTI3_IRQn, 14, 0 );
  HAL_NVIC_EnableIRQ(   EXTI3_IRQn );

  return 1;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

