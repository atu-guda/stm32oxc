#include <cstring>
#include <cstdlib>

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

int menu_lcd_output( const char *s );

int init_buttons(); // board dependent function: to separate file

struct MenuState {
  int level = 0; // 0: off, 1: select 2: change. TODO: more levels: hierarhy
  int index = 0; // selected element index
};

MenuState mstate;
int xx_m_val = 0; // fake manu var



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

  lcdt.init_4b();
  lcdt.puts_xy( 0, 1, "Start!" );

  init_buttons();

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

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
  int cmd = arg2long_d( 1, argc, argv, 0 );
  char buf[32], b0[24];

  switch( cmd ) {
    case 0: // empty cmd
      break;
    case 1: // Esc/ first enter
      if( mstate.level < 1 ) {
        mstate.level = 1;
      } else {
        --mstate.level;
      }
      break;
    case 2: // up
      if( mstate.level == 2 ) {
        ++xx_m_val;
      } else {
        ++mstate.index;
      }
      break;
    case 3: // down
      if( mstate.level == 2 ) {
        --xx_m_val;
      } else {
        --mstate.index;
      }
      break;
    case 4: // enter
      // TODO: store
      --mstate.level;
      break;
    default:
      break;
  }

  if( mstate.level < 0 ) {
    mstate.level = 0;
  }
  if( mstate.index < 0 ) { // TODO: clamp
    mstate.index = 0;
  }

  strcpy( buf, "> " );
  i2dec_n( cmd, buf+2 );
  strcat( buf, " " );
  i2dec_n( mstate.level, b0 );
  strcat( buf, b0 );
  strcat( buf, " " );
  i2dec_n( mstate.index, b0 );
  strcat( buf, b0 );


  menu_lcd_output( buf );

  return 0;
}

int menu_lcd_output( const char *s )
{
  lcdt.cls();
  lcdt.puts_xy( 0, 0, s );
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
    case GPIO_PIN_0: cmd = 1; break;
    case GPIO_PIN_1: cmd = 2; break;
    case GPIO_PIN_2: cmd = 3; break;
    case GPIO_PIN_3: cmd = 4; break;
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

