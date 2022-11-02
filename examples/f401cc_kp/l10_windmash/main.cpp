#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_tim.h>
#include <oxc_usartio.h> // TODO: auto

#include "uart_wm.h"
#include "oxc_tmc2209.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

PinsOut ledsx( GpioB, 12, 4 );

const char* common_help_string = "Winding machine control app" NL;

TIM_HandleTypeDef tim2_h;
TIM_HandleTypeDef tim5_h;
uint32_t volatile tim2_pulses {0}, tim2_need {0};
int tim2_cfg();
void tim2_start();
void tim2_stop();
uint32_t calc_TIM_arr_for_base_freq_flt( TIM_TypeDef *tim, float base_freq ); // like from oxc_tim.h buf for float

// UART_CONSOLE_DEFINES
UART_HandleTypeDef uah_motordrv;
UsartIO motordrv( &uah_motordrv, USART1 );

STD_USART1_IRQ( motordrv );
uint32_t TMC2209_read_reg( uint8_t dev, uint8_t reg );
uint32_t TMC2209_read_reg_n_try( uint8_t dev, uint8_t reg, int n_try );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_start( int argc, const char * const * argv );
CmdInfo CMDINFO_START { "start", 'S', cmd_start, " - start motor"  };
int cmd_stop( int argc, const char * const * argv );
CmdInfo CMDINFO_STOP { "stop", 'P', cmd_stop, " - stop motor"  };
int cmd_freq( int argc, const char * const * argv );
CmdInfo CMDINFO_FREQ { "freq", 'F', cmd_freq, " freq value - set freq for motor"  };
int cmd_readreg( int argc, const char * const * argv );
CmdInfo CMDINFO_READREG { "readreg", 'R', cmd_readreg, " reg - read TMC2209 register"  };
int cmd_writereg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "writereg", 'W', cmd_writereg, " reg val - write TMC2209 register"  };
int cmd_go( int argc, const char * const * argv );
CmdInfo CMDINFO_GO { "go", 'G', cmd_go, " turns - go motor some steps"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_START,
  &CMDINFO_STOP,
  &CMDINFO_FREQ,
  &CMDINFO_READREG,
  &CMDINFO_WRITEREG,
  &CMDINFO_GO,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') =   100;
  UVAR('n') =     8;
  UVAR('a') = 49999; // ARR
  UVAR('l') =     1; // delay after write
  UVAR('d') =     0; // TMC2209 device addr

  ledsx.initHW();
  ledsx.reset( 0xFF );

  if( ! init_uart( &uah_motordrv ) ) {
    die4led( 1 );
  }
  if( ! tim2_cfg() ) {
    die4led( 2 );
  }

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

  TMC2209_rreq  rqd;
  char in_buf[80];

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

    delay_ms( UVAR('l') );
    memset( in_buf, '\x00', sizeof(in_buf) );
    ledsx.reset( 1 );
    auto r_n = motordrv.read( in_buf, 16, 100 );

    uint32_t  tcc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 ) << " dt = " << ( tcc - tcb )
            << " wr_ok=" << wr_ok << " r_n= " << r_n << " w_n= " << w_n << NL;
    dump8( in_buf, 16 );

    leds.toggle( 1 );

    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

// TODO: to class
int TMC2209_write_reg( uint8_t dev, uint8_t reg, uint32_t v )
{
  TMC2209_rwdata wd;
  wd.fill( dev, reg, v );
  auto w_n = motordrv.write( (const char*)wd.rawCData(), sizeof(wd) );
  if( w_n != sizeof(wd) ) {
    return 0;
  }
  motordrv.wait_eot( 10 );
  return 1;
}

uint32_t TMC2209_read_reg( uint8_t dev, uint8_t reg )
{
  TMC2209_rreq  rqd;
  rqd.fill( dev, reg );

  motordrv.reset();

  ledsx.set( 1 );

  auto w_n = motordrv.write( (const char*)rqd.rawCData(), sizeof(rqd) );
  if( w_n != sizeof(rqd) ) {
    std_out << "# Err: w_n = " << w_n << NL;
    return TMC2209_bad_val;
  }
  motordrv.wait_eot( 10 );

  char in_buf[16]; // some more

  delay_ms( UVAR('l') );

  memset( in_buf, '\x00', sizeof(in_buf) );
  auto r_n = motordrv.read( in_buf, 16, 100 );

  ledsx.reset( 1 );

  if( r_n != sizeof(TMC2209_rreq) + sizeof(TMC2209_rwdata) ) {
    std_out << "# Err: r_n = " << r_n << NL;
    return TMC2209_bad_val;
  }

  TMC2209_rwdata *rd = bit_cast<TMC2209_rwdata*>( in_buf + sizeof(TMC2209_rreq) );
  // TODO: check crc
  uint32_t v = __builtin_bswap32( rd->data );
  return v;
}

uint32_t TMC2209_read_reg_n_try( uint8_t dev, uint8_t reg, int n_try )
{
  for( int i=0; i < n_try; ++i ) {
    uint32_t v = TMC2209_read_reg( dev, reg );
    if( v != TMC2209_bad_val ) {
      return v;
    }
    delay_ms( 50 );
  }
  return TMC2209_bad_val;
}

int cmd_readreg( int argc, const char * const * argv )
{
  uint8_t reg = (uint8_t) arg2long_d( 1, argc, argv, 0, 0, 127 );
  uint32_t v = TMC2209_read_reg_n_try( UVAR('d'), reg, 50 );
  std_out << "Reg " << (int)(reg) << " val: " << HexInt( v ) << ' ' << v << NL;
  return 0;
}

int cmd_writereg( int argc, const char * const * argv )
{
  uint8_t reg = (uint8_t) arg2long_d( 1, argc, argv, 2, 0, 127 ); // 2 - read-only reg - to detect empty param
  uint32_t  v =           arg2long_d( 2, argc, argv, 0x7FFFFFFF, 0, 0x7FFFFFFF ); // bad value - the same
  std_out << "# Reg " << (int)(reg) << " val: " << HexInt( v ) << ' ' << v <<  NL;
  if( reg == 2 || reg == 4 || v == 0x7FFFFFFF ) {
    std_out << "Error: need 2 correct arguments " << NL;
    return 1;
  }
  int rc = TMC2209_write_reg( UVAR('d'), reg, v );
  std_out << "# rc= " << rc << NL;
  return 0;
}

int tim2_cfg()
{
  int pbase = UVAR('a'); // TODO: ???
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

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &tim2_h, &sMasterConfig ) != HAL_OK ) {
    UVAR('e') = 2;
    return 0;
  }

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop_IT( &tim2_h, TIM_CHANNEL_2 );
  tim_oc_cfg.Pulse = pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &tim2_h, &tim_oc_cfg, TIM_CHANNEL_2 ) != HAL_OK ) {
    UVAR('e') = 3;
    return 0;
  }
  return 1;
}

int cmd_start( int argc, const char * const * argv )
{
  tim2_need = 0;  tim2_pulses = 0;
  tim2_start();
  return 0;
}

void tim2_start()
{
  HAL_TIM_PWM_Start_IT( &tim2_h, TIM_CHANNEL_2 );
  __HAL_TIM_DISABLE_IT( &tim2_h, TIM_IT_CC2 ); // we need PWM, but IRQ on update event
  __HAL_TIM_ENABLE_IT( &tim2_h, TIM_IT_UPDATE );
}

void tim2_stop()
{
  HAL_TIM_PWM_Stop_IT( &tim2_h, TIM_CHANNEL_2 );
  __HAL_TIM_DISABLE_IT( &tim2_h, TIM_IT_UPDATE );
}

int cmd_stop( int argc, const char * const * argv )
{
  tim2_need = 0;  tim2_pulses = 0;
  tim2_stop();
  return 0;
}

uint32_t calc_TIM_arr_for_base_freq_flt( TIM_TypeDef *tim, float base_freq )
{
  uint32_t freq = get_TIM_cnt_freq( tim ); // cnf_freq
  uint32_t arr = freq / base_freq - 1;
  return arr;
}

int cmd_freq( int argc, const char * const * argv )
{
  float freq = arg2float_d( 1, argc, argv, 100.0f, 0.0f, 50000.0f );
  uint32_t arr = calc_TIM_arr_for_base_freq_flt( TIM2, freq );
  TIM2->ARR = arr;
  TIM2->CCR2 = arr / 2;
  TIM2->CNT = 0;

  tim_print_cfg( TIM2 );

  return 0;
}

int cmd_go( int argc, const char * const * argv )
{
  // force stop
  HAL_TIM_PWM_Stop_IT( &tim2_h, TIM_CHANNEL_2 );
  __HAL_TIM_DISABLE_IT( &tim2_h, TIM_IT_UPDATE );

  bool rev = false;
  float turns = arg2float_d( 1, argc, argv, 1.0f, -50000.0f, 50000.0f );
  if( turns < 0 ) {
    turns = -turns;
    rev = true;
  }

  // TODO: common prepare
  TMC2209_write_reg( 0, 0, rev ? 0x149 : 0x141 );

  uint32_t pulses = (uint32_t)( turns * 200 * 8 ); // TODO: 200-motor param, 8-drv param
  tim2_pulses = 0;
  tim2_need = pulses;

  break_flag = 0;
  tim2_start();
  for( int i=0; i<10000000 && !break_flag; ++i ) { // TODO: calc time
    // TODO: check conditions
    if( tim2_pulses >= pulses ) {
      break;
    }
    delay_ms( 10 );
  }

  tim2_stop();
  tim2_need = 0;  tim2_pulses = 0;

  return 0;
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM2 ) {
    __GPIOA_CLK_ENABLE(); __TIM2_CLK_ENABLE();
    GpioA.cfgAF_N( GPIO_PIN_1, 1 );
    HAL_NVIC_SetPriority( TIM2_IRQn, 8, 0 );
    HAL_NVIC_EnableIRQ( TIM2_IRQn );
    UVAR('z') = 7;
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

void TIM2_IRQHandler()
{
  HAL_TIM_IRQHandler( &tim2_h );
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  ++UVAR('y');
  ledsx.toggle( 2 );
  ++tim2_pulses;
  if( tim2_need > 0 && tim2_pulses >= tim2_need ) {
    tim2_stop();
  }
}

void TIM5_IRQHandler()
{
  HAL_TIM_IRQHandler( &tim5_h );
}


void EXTI0_IRQHandler(void)
{
  // HAL_GPIO_EXTI_IRQHandler(SW_ALARM_Pin);
}

void EXTI1_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(SW_LEV_1_Pin);
}

void EXTI2_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(SW_LEV_2_Pin);
}

void EXTI3_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(SW_LIM_L_Pin);
}

void EXTI4_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(SW_LIM_R_Pin);
}

void EXTI9_5_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(OP_LIM_L_Pin);
  // HAL_GPIO_EXTI_IRQHandler(OP_LIM_R_Pin);
  // HAL_GPIO_EXTI_IRQHandler(HALL_ROT_Pin);
}


void EXTI15_10_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(SW_LEV_3_Pin);
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

