#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_usartio.h> // TODO: auto
#include <oxc_namedints.h>

#include "main.h"
#include "oxc_tmc2209.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

PinsOut ledsx( GpioB, 12, 4 );


PinsIn pins_tower( TOWER_GPIO, TOWER_PIN0, TOWER_N );
PinsIn pins_swlim( SWLIM_GPIO, SWLIM_PIN0, SWLIM_N );
PinsIn pins_diag (  DIAG_GPIO,  DIAG_PIN0,  DIAG_N );
PinsIn pins_user_start( USER_START_GPIO,  USER_START_PIN0,  USER_START_N );
PinsIn pins_user_stop(   USER_STOP_GPIO,   USER_STOP_PIN0,   USER_STOP_N );

volatile uint32_t porta_sensors_bits {0};
volatile uint32_t portb_sensors_bits {0};
// no user pins for now
const uint32_t porta_sensor_mask = SWLIM_BITS_ALL;
const uint32_t portb_sensor_mask = TOWER_BITS_ALL | DIAG_BITS_ALL;
bool read_sensors(); // returns true at bad condition
uint32_t sensor_flags = SWLIM_BITS_ALL;

const char* common_help_string = "Winding machine control app" NL;

TIM_HandleTypeDef tim2_h;
TIM_HandleTypeDef tim5_h;
uint32_t volatile tim2_pulses {0}, tim2_need {0}, tim5_pulses {0}, tim5_need {0};
int tim_n_cfg( TIM_HandleTypeDef &t_h, TIM_TypeDef *tim, uint32_t ch );
int tim2_cfg();
void tim2_start();
void tim2_stop();
int tim5_cfg();
void tim5_start();
void tim5_stop();
uint32_t calc_TIM_arr_for_base_freq_flt( TIM_TypeDef *tim, float base_freq ); // like from oxc_tim.h buf for float

// UART_CONSOLE_DEFINES
UART_HandleTypeDef uah_motordrv;
UsartIO motordrv( &uah_motordrv, USART1 );

STD_USART1_IRQ( motordrv );
uint32_t TMC2209_read_reg( uint8_t dev, uint8_t reg );
uint32_t TMC2209_read_reg_n_try( uint8_t dev, uint8_t reg, int n_try );
int TMC2209_write_reg( uint8_t dev, uint8_t reg, uint32_t v );

bool prepare_drv( uint8_t drv ); // true = ok
int  drv_prepared = 0;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_start( int argc, const char * const * argv );
CmdInfo CMDINFO_START { "start", 'S', cmd_start, " - start motor"  };
int cmd_stop( int argc, const char * const * argv );
CmdInfo CMDINFO_STOP { "stop", 'P', cmd_stop, " - stop motor"  };
int cmd_speed( int argc, const char * const * argv );
CmdInfo CMDINFO_SPEED { "speed", 'F', cmd_speed, " speed [dev] - set speed for motor turn/s or mm/s"  };
int cmd_readreg( int argc, const char * const * argv );
CmdInfo CMDINFO_READREG { "readreg", 'R', cmd_readreg, " reg - read TMC2209 register"  };
int cmd_writereg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "writereg", 'W', cmd_writereg, " reg val - write TMC2209 register"  };
int cmd_rotate( int argc, const char * const * argv );
CmdInfo CMDINFO_ROTATE { "rot", '\0', cmd_rotate, " turns - rotate"  };
int cmd_move( int argc, const char * const * argv );
CmdInfo CMDINFO_MOVE { "move", 'M', cmd_move, " mm [no_opto] - move"  };
int cmd_prep( int argc, const char * const * argv );
CmdInfo CMDINFO_PREP { "prep", '\0', cmd_prep, " - prepare drivers"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_START,
  &CMDINFO_STOP,
  &CMDINFO_SPEED,
  &CMDINFO_READREG,
  &CMDINFO_WRITEREG,
  &CMDINFO_ROTATE,
  &CMDINFO_MOVE,
  &CMDINFO_PREP,
  nullptr
};

TaskData td;

constexpr NamedInt   ob_n_total  {  "n_total",      &td.n_total  };
constexpr NamedInt   ob_v_rot    {  "v_rot",        &td.v_rot  };
constexpr NamedInt   ob_v_mov_o  {  "v_mov_o",      &td.v_mov_o  };

constexpr const NamedObj *const objs_info[] = {
  & ob_v_rot,
  & ob_v_mov_o,
  & ob_n_total,
  nullptr
};

NamedObjs objs( objs_info );

// print/set hook functions

bool print_var_ex( const char *nm, int fmt )
{
  return objs.print( nm, fmt );
}

bool set_var_ex( const char *nm, const char *s )
{
  auto ok =  objs.set( nm, s );
  print_var_ex( nm, 0 );
  return ok;
}



void idle_main_task()
{
  // leds.toggle( 1 );
}


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') =   100;
  UVAR('n') =     2;
  UVAR('a') = 49999; // ARR
  UVAR('l') =    20; // delay
  UVAR('d') =     0; // TMC2209 device addr

  ledsx.initHW();
  ledsx.reset( 0xFF );

  pins_tower.initHW();
  pins_swlim.initHW();
  pins_diag.initHW();
  pins_user_start.initHW();
  pins_user_stop.initHW();

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  if( ! init_uart( &uah_motordrv ) ) {
    die4led( 1 );
  }
  if( ! tim2_cfg() ) {
    die4led( 2 );
  }
  if( ! tim5_cfg() ) {
    die4led( 3 );
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

    delay_ms( 1 );
    memset( in_buf, '\x00', sizeof(in_buf) );
    ledsx.reset( 1 );
    auto r_n = motordrv.read( in_buf, 16, 100 );

    uint32_t  tcc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 ) << " dt = " << ( tcc - tcb )
            << " wr_ok=" << wr_ok << " r_n= " << r_n << " w_n= " << w_n << NL;
    dump8( in_buf, 16 );

    delay_ms_until_brk( &tc0, t_step );
  }

  uint32_t r1 = TMC2209_read_reg( 0, 2 );
  TMC2209_write_reg( 0, 1, 0x149 );
  uint32_t r2 = TMC2209_read_reg( 0, 2 );
  TMC2209_write_reg( 0, 1, 0x149 );
  TMC2209_write_reg( 0, 1, 0x141 );
  uint32_t r3 = TMC2209_read_reg( 0, 2 );
  std_out << "# R2: r1= " << HexInt( r1 ) << "  r2= " << HexInt( r2 ) << "  r3= " << HexInt( r3 ) <<NL;

  return 0;
}

// TODO: to class
int TMC2209_write_reg( uint8_t dev, uint8_t reg, uint32_t v )
{
  TMC2209_rwdata wd;
  wd.fill( dev, reg, v );
  auto w_n = motordrv.write_s( (const char*)wd.rawCData(), sizeof(wd) );
  if( w_n != sizeof(wd) ) {
    return 0;
  }
  delay_mcs( 200 );
  motordrv.wait_eot( 10 );
  return 1;
}

uint32_t TMC2209_read_reg( uint8_t dev, uint8_t reg )
{
  TMC2209_rreq  rqd;
  rqd.fill( dev, reg );

  motordrv.reset();

  ledsx.set( 1 );

  auto w_n = motordrv.write_s( (const char*)rqd.rawCData(), sizeof(rqd) );
  if( w_n != sizeof(rqd) ) {
    std_out << "# Err: w_n = " << w_n << NL;
    return TMC2209_bad_val;
  }
  motordrv.wait_eot( 10 );

  char in_buf[16]; // some more

  delay_ms( 1 ); // TODO: config

  memset( in_buf, '\x00', sizeof(in_buf) );
  ledsx.reset( 1 );

  auto r_n = motordrv.read( in_buf, 16, 100 );


  if( r_n != sizeof(TMC2209_rreq) + sizeof(TMC2209_rwdata) ) {
    std_out << "# Err: r_n = " << r_n << NL;
    return TMC2209_bad_val;
  }

  TMC2209_rwdata *rd = bit_cast<TMC2209_rwdata*>( in_buf + sizeof(TMC2209_rreq) );
  // TODO: check crc
  uint32_t v = __builtin_bswap32( rd->data );
  delay_mcs( 100 );
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

int tim_n_cfg( TIM_HandleTypeDef &t_h, TIM_TypeDef *tim, uint32_t ch )
{
  int pbase = 49999; // TODO: ???
  t_h.Instance               = tim;
  t_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( tim, 1000000  );
  t_h.Init.Period            = pbase;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &t_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &t_h, &sClockSourceConfig );

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig ) != HAL_OK ) {
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

  HAL_TIM_PWM_Stop_IT( &t_h, ch );
  tim_oc_cfg.Pulse = pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &t_h, &tim_oc_cfg, ch ) != HAL_OK ) {
    UVAR('e') = 3;
    return 0;
  }
  return 1;
}


int tim2_cfg()
{
  return tim_n_cfg( tim2_h, TIM2, TIM_CHANNEL_2 );
}

int tim5_cfg()
{
  return tim_n_cfg( tim5_h, TIM5, TIM_CHANNEL_3 );
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

void tim5_start()
{
  HAL_TIM_PWM_Start_IT( &tim5_h, TIM_CHANNEL_3 );
  __HAL_TIM_DISABLE_IT( &tim5_h, TIM_IT_CC3 ); // we need PWM, but IRQ on update event
  __HAL_TIM_ENABLE_IT( &tim5_h, TIM_IT_UPDATE );
}

void tim5_stop()
{
  HAL_TIM_PWM_Stop_IT( &tim5_h, TIM_CHANNEL_3 );
  __HAL_TIM_DISABLE_IT( &tim5_h, TIM_IT_UPDATE );
}


bool read_sensors()
{
  porta_sensors_bits = GPIOA->IDR & porta_sensor_mask;
  portb_sensors_bits = GPIOB->IDR & portb_sensor_mask;
  return porta_sensors_bits != porta_sensor_mask; //
}

int cmd_start( int argc, const char * const * argv )
{
  int devs = arg2long_d( 1, argc, argv, 3, 0, 3 );
  if( devs & 1 ) {
    tim2_need = 0;  tim2_pulses = 0;
    tim2_start();
    std_out << "# TIM2 start " << NL;
  }
  if( devs & 2 ) {
    tim5_pulses = 0;
    tim5_start();
    std_out << "# TIM5 start " << NL;
  }
  return 0;
}

int cmd_stop( int argc, const char * const * argv )
{
  tim2_need = 0;  tim2_pulses = 0;  tim5_pulses = 0;
  tim2_stop();
  tim5_stop();
  return 0;
}

uint32_t calc_TIM_arr_for_base_freq_flt( TIM_TypeDef *tim, float base_freq )
{
  uint32_t freq = get_TIM_cnt_freq( tim ); // cnf_freq
  uint32_t arr = freq / base_freq - 1;
  return arr;
}

int set_drv_speed( int dev, float speed )
{
  auto tim = ( dev == 0 ) ? TIM2 : TIM5;
  auto ccr = ( dev == 0 ) ? &(tim->CCR2) : &(tim->CCR3);
  float freq = motor_step2turn * motor_mstep * speed;
  uint32_t arr = calc_TIM_arr_for_base_freq_flt( tim, freq );

  // stop timer during update ??
  uint32_t old_cr1 = tim->CR1;
  tim->CR1 &= ~1u; // EN;
  tim->ARR = arr;
  *ccr = arr / 2;
  tim->CNT = 0;
  tim->CR1 = old_cr1;

  return 0;
}

int cmd_speed( int argc, const char * const * argv )
{
  float speed = arg2float_d( 1, argc, argv, 1.0f, 0.0f, 100.0f );
  int dev = arg2long_d( 2, argc, argv, 0, 0, 1 );

  set_drv_speed( dev, speed );

  tim_print_cfg( TIM2 );
  tim_print_cfg( TIM5 );

  return 0;
}

int cmd_rotate( int argc, const char * const * argv )
{
  if( ! drv_prepared ) {
    std_out << "# Error: drivers not prepared" << NL;
    return  1;
  }

  tim2_stop(); tim5_stop();

  bool rev = false;
  float turns = arg2float_d( 1, argc, argv, 1.0f, -50000.0f, 50000.0f );
  float vm = arg2float_d( 2, argc, argv, td.v_rot * 0.001f, 0.0f, 20.0f );
  if( turns < 0 ) {
    turns = -turns;
    rev = true;
  }
  set_drv_speed( 0, vm );

  TMC2209_write_reg( 0, 0, rev ? reg00_def_rev : reg00_def_forv );

  uint32_t pulses = (uint32_t)( turns * motor_step2turn * motor_mstep );
  tim2_pulses = 0;
  tim2_need = pulses;
  tim5_pulses = 0;
  auto dt = UVAR('l');

  std_out << "# rotate: turns= " << turns << " rev= " << rev << " pulses= " << pulses << NL;

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0;

  break_flag = 0;
  tim2_start();
  for( int i=0; i<10000000 && !break_flag; ++i ) { // TODO: calc time
    // TODO: check conditions
    if( tim2_pulses >= pulses ) {
      break;
    }
    read_sensors();
    delay_bad_mcs( 10 );
    uint32_t r6F_m = TMC2209_read_reg( 0, 0x6F );
    uint32_t r41_m = TMC2209_read_reg( 0, 0x41 );
    // if( ( porta_sensors_bits & sensor_flags ) != sensor_flags ) {
    //    break_flag = 1;
    // }

    uint32_t tc = HAL_GetTick();
    std_out << HexInt( r6F_m ) << ' ' << HexInt( r41_m ) << ' '
            << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits )
            << ' ' << (int)( tc - tm0 ) << NL;
    delay_ms_until_brk( &tc0, dt );
  }

  tim2_stop();
  tim2_need = 0;

  return 0;
}

int cmd_move( int argc, const char * const * argv )
{
  if( ! drv_prepared ) {
    std_out << "# Error: drivers not prepared" << NL;
    return  1;
  }

  tim2_stop(); tim5_stop();

  bool rev = false;
  float mm = arg2float_d( 1, argc, argv, 1.0f, -200.0f, 200.0f );
  int ignore_opto = arg2long_d( 2, argc, argv, 0, 0, 1 );
  float vm = arg2float_d( 3, argc, argv, td.v_mov_o * 0.001f, 0.0f, 20.0f );
  if( mm < 0 ) {
    mm = -mm;
    rev = true;
  }
  auto old_sf = sensor_flags;
  sensor_flags = ignore_opto ? SWLIM_BITS_SW : SWLIM_BITS_ALL;
  set_drv_speed( 1, vm );


  TMC2209_write_reg( 1, 0, rev ? reg00_def_rev : reg00_def_forv );

  uint32_t pulses = (uint32_t)( mm * motor_step2turn * motor_mstep );
  tim5_pulses = 0;
  tim5_need = pulses;
  auto dt = UVAR('l');

  std_out << "# move: mm= " << mm << " rev= " << rev << " pulses= " << pulses << " v= " << vm << NL;

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0;

  break_flag = 0;
  tim5_start();
  for( int i=0; i<10000000 && !break_flag; ++i ) { // TODO: calc time
    // TODO: check conditions
    if( tim5_pulses >= pulses ) {
      break;
    }
    delay_bad_mcs( 10 );
    uint32_t r6F_m = TMC2209_read_reg( 1, 0x6F );
    uint32_t r41_m = TMC2209_read_reg( 1, 0x41 );
    read_sensors();
    if( ( porta_sensors_bits & sensor_flags ) != sensor_flags ) {
       break_flag = 1;
    }

    uint32_t tc = HAL_GetTick();
    std_out << HexInt( r6F_m ) << ' ' << HexInt( r41_m ) << ' '
            << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits )
            << ' ' << (int)( tc - tm0 ) << NL;
    delay_ms_until_brk( &tc0, dt );
  }

  tim5_stop();
  tim5_need = 0;
  UVAR('b') = tim5_pulses;
  sensor_flags = old_sf;

  return 0;
}

bool prepare_drv( uint8_t drv )
{
  uint32_t r = TMC2209_read_reg_n_try( drv, 0x06, 10 );
  if( ( r & 0xFF000000 ) != 0x21000000 ) {
    std_out << "# Error init drv " << drv << " bad signature " << HexInt( r ) << NL;
    return false;
  }
  uint32_t n0 = TMC2209_read_reg_n_try( drv, 0x02, 10 ); // initial counter
  TMC2209_write_reg( drv, 0x00, reg00_def_forv ); // general config
  TMC2209_write_reg( drv, 0x10, reg10_def );      // IHOLD, IRUN, IHOLDDELAY
  TMC2209_write_reg( drv, 0x6C, reg6C_def );      // many bits
                                                  // 14, 41
  uint32_t n1 = TMC2209_read_reg_n_try( drv, 0x02, 10 );
  if( ( ( n1 - n0 ) & 0xFF ) != 3 ) {
    std_out << "# Error init drv " << drv << " bad counter " << ( n1 - n0 ) << NL;
    return false;
  }
  return true;
}

int cmd_prep( int argc, const char * const * argv )
{
  drv_prepared = 0;
  if( ! prepare_drv( 0 ) ) {
    return 1;
  }
  if( ! prepare_drv( 1 )  ) {
    return 2;
  }
  drv_prepared = 1;

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

  if( htim->Instance == TIM5 ) {
    __GPIOA_CLK_ENABLE(); __TIM5_CLK_ENABLE();
    GpioA.cfgAF_N( GPIO_PIN_2, GPIO_AF2_TIM5 );
    HAL_NVIC_SetPriority( TIM5_IRQn, 9, 0 );
    HAL_NVIC_EnableIRQ( TIM5_IRQn );
    UVAR('z') += 8;
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

  if( htim->Instance == TIM5 ) {
    __TIM5_CLK_DISABLE();
    GpioA.cfgIn_N( GPIO_PIN_2 );
    HAL_NVIC_DisableIRQ( TIM5_IRQn );
    return;
  }
}

void TIM2_IRQHandler()
{
  HAL_TIM_IRQHandler( &tim2_h );
}

void TIM5_IRQHandler()
{
  HAL_TIM_IRQHandler( &tim5_h );
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  if( htim->Instance == TIM2 ) {
    ++UVAR('y');
    ledsx.toggle( 2 );
    ++tim2_pulses;
    if( tim2_need > 0 && tim2_pulses >= tim2_need ) {
      tim2_stop();
    }
    return;
  }

  if( htim->Instance == TIM5 ) {
    ++UVAR('x');
    ++tim5_pulses;
    ledsx.toggle( 4 );
    uint32_t pa = SWLIM_GPIO.IDR & sensor_flags;
    if( pa != sensor_flags ) {
      tim5_stop();
      tim2_stop();
    }
    if( tim5_need > 0 && tim5_pulses >= tim5_need ) {
      tim5_stop();
    }
    return;
  }
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

