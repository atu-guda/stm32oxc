#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

volatile uint16_t sensors_state;
void init_exti_pins();
PinsOut ctlEn( GPIOA, 8, 1 );
PinsOut ctlAB( GPIOA, 9, 2 );

uint32_t wait_with_cond( uint32_t ms, uint16_t n_hwticks, uint32_t *rc, uint32_t ign_bits = 0 );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 10;
  UVAR('g') = 10;
  UVAR('n') = 1;

  ctlEn.initHW();  ctlEn.write( 0 );
  ctlAB.initHW();  ctlAB.write( 0 );
  sensors_state = 0;
  init_exti_pins();

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS_UART;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}

// EXTI IRQ sensors B1, B2:
void init_exti_pins()
{
  GPIO_enableClk( GPIOB );
  GPIO_InitTypeDef gpi;
  gpi.Mode  = GPIO_MODE_IT_RISING_FALLING;
  gpi.Pull  = GPIO_PULLDOWN;
  gpi.Speed = GPIO_SPEED_MAX;

  gpi.Pin = GPIO_PIN_1 | GPIO_PIN_2;
  HAL_GPIO_Init( GPIOB, &gpi );
  HAL_NVIC_SetPriority( EXTI1_IRQn, /* configKERNEL_INTERRUPT_PRIORITY + */ 1, 0 );
  HAL_NVIC_EnableIRQ( EXTI1_IRQn );
  HAL_NVIC_SetPriority( EXTI2_IRQn, /* configKERNEL_INTERRUPT_PRIORITY + */ 1, 0 );
  HAL_NVIC_EnableIRQ( EXTI2_IRQn );
}

void EXTI1_IRQHandler()
{
  if( GPIOB->IDR & GPIO_PIN_1 ) {
    leds.set( 2 );
    sensors_state |= 1;
  } else {
    leds.reset( 2 );
    sensors_state &= ~1;
  }

  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_IRQHandler()
{
  if( GPIOB->IDR & GPIO_PIN_2 ) {
    leds.set( 4 );
    sensors_state |= 2;
  } else {
    leds.reset( 4 );
    sensors_state &= ~2;
  }
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
}

uint32_t wait_with_cond( uint32_t ms, uint16_t /* n_hwticks */, uint32_t *rc, uint32_t ign_bits )
{
  uint32_t lrc = 0x10, w = 0; // 0x10 bit = to many loops
  TickType_t tc0 = xTaskGetTickCount();
  ign_bits = ~ign_bits;

  for( uint32_t i=0; i<1000000000; ++i ) {
    if( sensors_state & ign_bits ) {
      lrc = sensors_state; break;
    }
    if( (i & 0xFF) == 0xFF ) {
      if( ( w = xTaskGetTickCount() - tc0 ) >= ms ) {
        lrc = 0; break;
      }
    }
  }

  if( rc ) {
    *rc = lrc;
  }
  return w;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int tg     = arg2long_d( 1, argc, argv, UVAR('g'), 0 );
  int dir    = arg2long_d( 2, argc, argv, 0, 0, 1 );
  int n_step = arg2long_d( 3, argc, argv, UVAR('n'), 1, 10000 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: tg= " ); pr_d( tg ); pr( " t= " ); pr_d( t_step ); pr( " n_step= " ); pr_d( n_step );
  pr( NL );

  leds.reset( 1 );
  ctlEn.write( 0 );
  ctlAB.write( dir ? 1 : 2 );

  TickType_t tc0 = xTaskGetTickCount(); // , tc00 = tc0;

  break_flag = 0;
  uint32_t src = 0, w = 0;
  for( int i=0; i<n_step && !break_flag; ++i ) {
    // TickType_t tcc = xTaskGetTickCount();
    // pr( " step i= " ); pr_d( i );
    // pr( "  tick: "); pr_d( tcc - tc00 );
    // pr( NL );
    leds.set( 1 ); ctlEn.set( 1 );
    // delay_ms( tg );
    w = wait_with_cond( tg, 1000, &src, dir ? 2 : 1 );
    leds.reset( 1 ); ctlEn.reset( 1 );
    if( src ) {
      break;
    }
    vTaskDelayUntil( &tc0, t_step );
  }

  ctlEn.write( 0 );  ctlAB.write( 0 );
  leds.reset( 1 );
  pr_sdx( sensors_state );  pr_sdx( w );   pr_sdx( src );

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

