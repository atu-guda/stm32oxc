#include <oxc_auto.h>
#include <oxc_usartio.h> // TODO: auto

#include "uart_wm.h"
#include "oxc_tmc2209.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

PinsOut ledsx( GpioB, 12, 4 );

const char* common_help_string = "Widing machine control app" NL;

TIM_HandleTypeDef tim2_h;
int tim2_cfg();

// UART_CONSOLE_DEFINES
UART_HandleTypeDef uah_motordrv;
UsartIO motordrv( &uah_motordrv, USART1 );

STD_USART1_IRQ( motordrv );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') =  1000;
  UVAR('n') =     4;
  UVAR('a') = 49999; // ARR
  UVAR('z') =     0; // auto stop after test

  ledsx.initHW();
  ledsx.reset( 0xFF );

  if( ! init_uart( &uah_motordrv ) ) {
    die4led( 1 );
  }
  if( ! tim2_cfg() ) {
    die4led( 2 );
  }
  HAL_TIM_PWM_Start( &tim2_h, TIM_CHANNEL_2 );

  motordrv.setHandleCbreak( false );
  devio_fds[5] = &motordrv;
  devio_fds[6] = &motordrv;
  motordrv.itEnable( UART_IT_RXNE );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  // TMC2209_rwdata rd;
  TMC2209_rreq  rqd;
  char in_buf[80];

  tim2_cfg();
  HAL_TIM_PWM_Start( &tim2_h, TIM_CHANNEL_2 );

  // motordrv.enable();
  motordrv.reset();

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    motordrv.reset();
    uint32_t  tcb = HAL_GetTick();
    rqd.fill( UVAR('d'), i );
    // rqd.crc = (uint8_t)i;
    ledsx.set( 1 );
    auto w_n = motordrv.write( (const char*)rqd.rawCData(), sizeof(rqd) );
    auto wr_ok = motordrv.wait_eot( 100 );
    // ledsx.reset( 1 );
    // auto wr_ok = 1;

    delay_ms( 10 );
    memset( in_buf, '\x00', sizeof(in_buf) );
    ledsx.reset( 1 );
    auto r_n = motordrv.read( in_buf, 16, 100 );

    uint32_t  tcc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 ) << " dt = " << ( tcc - tcb )
            << " wr_ok=" << wr_ok << " r_n= " << r_n << " w_n= " << w_n
            << ' ' << HexInt( motordrv.getSR() ) << NL;
    dump8( in_buf, 16 );

    leds.toggle( 1 );

    delay_ms_until_brk( &tc0, t_step );
  }
  tim_print_cfg( TIM2 );
  if( UVAR('z') ) {
    HAL_TIM_PWM_Stop( &tim2_h, TIM_CHANNEL_2 );
  }

  return 0;
}

int tim2_cfg()
{
  int pbase = UVAR('a');
  tim2_h.Instance               = TIM2;
  tim2_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM_EXA, 1000000  );
  tim2_h.Init.Period            = pbase;
  tim2_h.Init.ClockDivision     = 0;
  tim2_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim2_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim2_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim2_h, &sClockSourceConfig );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop( &tim2_h, TIM_CHANNEL_2 );
  tim_oc_cfg.Pulse = pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &tim2_h, &tim_oc_cfg, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }
  // HAL_TIM_PWM_Start( &tim2_h, TIM_CHANNEL_2 );
  return 1;
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM2 ) {
    __GPIOA_CLK_ENABLE(); __TIM2_CLK_ENABLE();
    GpioA.cfgAF_N( GPIO_PIN_1, 1 );
    // TIM2_IRQn
    return;
  }

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM2 ) {
    __TIM2_CLK_DISABLE();
    GpioA.cfgIn_N( GPIO_PIN_1 );
    HAL_NVIC_DisableIRQ( TIM2_IRQn );
    return;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

