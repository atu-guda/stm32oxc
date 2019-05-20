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

int menu_lcd_output( const char *s1, const char *s2 = nullptr );

volatile uint32_t menu_ev_global = 0;
void menu_ev_dispatch();

int init_buttons(); // board dependent function: to separate file

struct MenuItem {
  const char *name;
  int *pv;
  int vstep = 1, vmin = INT_MIN, vmax = INT_MAX;
  int (*fun)(int) = nullptr;
};

enum class MenuCmd {
  Esc = 0, Up = 1, Down = 2, Enter = 3, N
};


enum class MenuLevel {
  ready = 0, select = 1, edit = 2, N
};

const char *const menuLevelName[] = { "Ready", "Select", "Edit", "?3", "?4?" };


struct MenuState {
  const MenuItem* menu;
  const uint32_t menu_size;
  MenuLevel level = MenuLevel::ready;
  int index = 0; // selected element index
  int *pv   = nullptr; // ptr current var under edit (level2)
  int v = 0; // currenr value
};


int T_off = 100, T_hyst = 10, t_dt = 1000;
int fun_x1( int n );

int fun_x1( int n )
{
  std_out << "#-- fun_x1: n= " << n << NL;
  return n;
}


const MenuItem menu_main[] = {
  { "T_off",   &T_off,    1, -100,   5000, nullptr },
  { "T_hyst" , &T_hyst,   1,    0,   1000, nullptr },
  { "t_dt",      &t_dt, 100,  100, 100000, nullptr },
  { "fun_x1",  nullptr,   1,    7, 100000,  fun_x1 }
};

MenuState mstate { menu_main, size( menu_main) };


void menu_ev_dispatch()
{
  uint32_t ev = menu_ev_global;
  menu_ev_global = 0;
  switch( ev ) {
    case 10:
      std_out << "#!!! run2" NL;
      ungets( 0, "T\n" );
      break;
    default:
      break;
  }
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
  oxc_add_aux_tick_fun( menu_ev_dispatch );

  lcdt.init_4b();
  lcdt.puts_xy( 0, 1, "Ready!" );

  init_buttons();

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

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    i2dec_n( i, buf0 );
    lcdt.puts_xy( 0, 1, buf0 );

    std_out << i << NL;
    delay_ms_until_brk( &tm0, t_step );
  }
  lcdt.puts_xy( 0, 1, "Stop!  " );

  return 0;
}


int cmd_menu( int argc, const char * const * argv )
{
  int cmd_i = arg2long_d( 1, argc, argv, 0 );

  if( cmd_i < 0 || cmd_i >= (int)MenuCmd::N ) {
    return 1;
  }
  MenuCmd cmd = MenuCmd( cmd_i );

  const char *item_name = "";

  switch( mstate.level ) {

    case MenuLevel::ready:
      switch( cmd ) {
        case MenuCmd::Esc:
          mstate.level = MenuLevel::select; mstate.index = 0;
          break;
        case MenuCmd::Up:
          break;
        case MenuCmd::Down:
          break;
        case MenuCmd::Enter:
          std_out << "# ***Run!" NL;
          // ungets( 0, "T\n" );
          menu_ev_global = 10; // RUN!
          delay_ms( 10 );
          break;
        default: break;
      };
      break;

    case MenuLevel::select:
      switch( cmd ) {
        case MenuCmd::Esc:
          mstate.level = MenuLevel::ready;
          break;
        case MenuCmd::Up:
          ++mstate.index;
          break;
        case MenuCmd::Down:
          --mstate.index;
          break;
        case MenuCmd::Enter:
          mstate.level = MenuLevel::edit;
          if( mstate.menu[mstate.index].pv ) {
            mstate.v = *mstate.menu[mstate.index].pv;
          } else {
            mstate.v = mstate.menu[mstate.index].vmin;
          }
          // copy + exec?
          break;
        default: break;
      };
      break;

    case MenuLevel::edit:
      switch( cmd ) {
        case MenuCmd::Esc:
          mstate.level = MenuLevel::select;
          break;
        case MenuCmd::Up:
          mstate.v += mstate.menu[mstate.index].vstep;
          break;
        case MenuCmd::Down:
          mstate.v -= mstate.menu[mstate.index].vstep;
          break;
        case MenuCmd::Enter:
          if( mstate.menu[mstate.index].pv ) {
            *mstate.menu[mstate.index].pv = clamp( mstate.v, mstate.menu[mstate.index].vmin, mstate.menu[mstate.index].vmax );
          }
          if( mstate.menu[mstate.index].fun ) {
            mstate.menu[mstate.index].fun( mstate.v );
          }
          mstate.level = MenuLevel::select;
          break;
        default: break;
      };
      mstate.v = clamp( mstate.v, mstate.menu[mstate.index].vmin, mstate.menu[mstate.index].vmax );
      break;

    default: // ???
      break;
  }

  mstate.index = clamp( mstate.index, 0, (int)(mstate.menu_size-1) );
  item_name = mstate.menu[mstate.index].name;

  int show_v = 0;
  switch( mstate.level ) {
    case MenuLevel::ready:
      break;
    case MenuLevel::select:
      if( mstate.menu[mstate.index].pv ) {
        show_v = *mstate.menu[mstate.index].pv;
      }
      break;
    case MenuLevel::edit:
      show_v = mstate.v;
      break;
    default:
      break;
  };


  char buf[32], b0[24];
  strcpy( buf, "> " );
  if( mstate.level != MenuLevel::ready ) {
    strcat( buf, item_name );
    strcat( buf, "= " );
    i2dec_n( show_v, b0 );
    strcat( buf, b0 );
  }


  menu_lcd_output( buf, menuLevelName[(int)mstate.level] );

  return 0;
}

int menu_lcd_output( const char *s1, const char *s2 )
{
  lcdt.cls();
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
  if( curr_tick - last_exti_tick < 200 ) {
    return; // ignore too fast events
  }
  uint8_t cmd = 0;
  switch( pin ) {
    case GPIO_PIN_0: cmd = (uint8_t)(MenuCmd::Esc);   break;
    case GPIO_PIN_1: cmd = (uint8_t)(MenuCmd::Up);    break;
    case GPIO_PIN_2: cmd = (uint8_t)(MenuCmd::Down);  break;
    case GPIO_PIN_3: cmd = (uint8_t)(MenuCmd::Enter); break;
    default: break;
  }

  char buf[6];
  buf[0] = 'M'; buf[1] = ' '; buf[2] = '0' + cmd; buf[3] = '\n'; buf[4] = '\0';
  leds.toggle( BIT0 );
  if( ! on_cmd_handler ) {
    ungets( 0, buf );
  } else {
    break_flag = 1;
  }

  last_exti_tick = curr_tick;
}

int init_buttons() // board dependent function: to separate file
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

