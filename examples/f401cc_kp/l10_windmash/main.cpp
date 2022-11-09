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
void tims_start( uint8_t devs );
void tims_stop( uint8_t devs );
void timn_start( uint8_t dev );
void timn_stop( uint8_t dev );
uint32_t calc_TIM_arr_for_base_freq_flt( TIM_TypeDef *tim, float base_freq ); // like from oxc_tim.h buf for float

// UART_CONSOLE_DEFINES
UART_HandleTypeDef uah_motordrv;
UsartIO motordrv( &uah_motordrv, USART1 );

STD_USART1_IRQ( motordrv );
uint32_t TMC2209_read_reg( uint8_t dev, uint8_t reg );
uint32_t TMC2209_read_reg_n_try( uint8_t dev, uint8_t reg, int n_try );
int TMC2209_write_reg( uint8_t dev, uint8_t reg, uint32_t v );

bool prepare_drv( uint8_t drv ); // true = ok
int ensure_drv_prepared();
int  drv_prepared = 0;
int do_move( float mm, float vm, uint8_t dev );

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
int cmd_calc( int argc, const char * const * argv );
CmdInfo CMDINFO_CALC { "calc", '\0', cmd_calc, "n_tot w_d(um) w_l(um) - calculate task"  };

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
  &CMDINFO_CALC,
  nullptr
};

TaskData td;

int TaskData::calc( int n_tot, int d_w, int w_l, bool even )
{
  n_done = n_ldone = c_lay =  n_lay = 0;
  if( n_tot < 1 || d_w < 20 || w_l < 2 * d_w ) {
    n_tot = 0;
    return 0;
  }
  n_total = n_tot; d_wire = d_w; w_len = w_l;
  unsigned n_lay_max = w_len / d_wire;
  n_lay = ( n_tot + n_lay_max - 1 ) / n_lay_max;
  if( even ) {
    ++n_lay;
    n_lay &= ~1u;
  }
  n_2lay = ( n_total + n_lay / 2 ) / n_lay;

  float d_w_e = 0.001f * w_len / n_2lay;
  v_mov = (int)(v_rot * d_w_e);

  return 1;
}

constexpr NamedInt   ob_n_total { "n_total", &td.n_total };
constexpr NamedInt   ob_d_wire  { "d_wire",  &td.d_wire };
constexpr NamedInt   ob_w_len   { "w_len",   &td.w_len };
constexpr NamedInt   ob_v_rot   { "v_rot",   &td.v_rot };
constexpr NamedInt   ob_v_mov_o { "v_mov_o", &td.v_mov_o };
constexpr NamedInt   ob_n_lay   { "n_lay",   &td.n_lay };
constexpr NamedInt   ob_n_2lay  { "n_2lay",  &td.n_2lay };
constexpr NamedInt   ob_v_mov   { "v_mov",   &td.v_mov };
constexpr NamedInt   ob_n_done  { "n_done",  &td.n_done, NamedObj::Flags::ro };
constexpr NamedInt   ob_n_ldone { "n_ldone", &td.n_ldone };
constexpr NamedInt   ob_c_lay   { "c_lay",   &td.c_lay };


constexpr const NamedObj *const objs_info[] = {
  & ob_n_total,
  & ob_d_wire,
  & ob_w_len,
  & ob_v_rot,
  & ob_v_mov_o,
  & ob_n_lay,
  & ob_n_2lay,
  & ob_v_mov,
  & ob_n_done,
  & ob_n_ldone,
  & ob_c_lay,
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
    // std_out << "# Err: r_n = " << r_n << NL;
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
    delay_ms( 20 );
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
  int pbase = 3124; // TODO: ???
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

void tims_start( uint8_t devs )
{
  if( devs & 1 ) {
    tim2_start();
  }
  if( devs & 2 ) {
    tim5_start();
  }
}

void tims_stop( uint8_t devs )
{
  if( devs & 1 ) {
    tim2_stop();
  }
  if( devs & 2 ) {
    tim5_stop();
  }
}

void timn_start( uint8_t dev )
{
  if( dev ) {
    tim5_start();
  } else {
    tim2_start();
  }
}

void timn_stop( uint8_t dev )
{
  if( dev ) {
    tim5_stop();
  } else {
    tim2_stop();
  }
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
  tims_start( devs );
  return 0;
}

int cmd_stop( int argc, const char * const * argv )
{
  int devs = arg2long_d( 1, argc, argv, 3, 0, 3 );
  tims_stop( devs );
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

  // stop timer during update
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

int do_move( float mm, float vm, uint8_t dev )
{
  if( ! ensure_drv_prepared() ) {
    std_out << "# Error: drivers not prepared" << NL;
    return  1;
  }
  timn_stop( dev );

  bool rev = false;
  if( mm < 0 ) {
    mm = -mm;
    rev = true;
  }
  set_drv_speed( dev, vm );

  TMC2209_write_reg( dev, 0, rev ? reg00_def_rev : reg00_def_forv ); // direction

  uint32_t pulses = (uint32_t)( mm * motor_step2turn * motor_mstep );
  auto c_pulses    = dev ? ( &tim5_pulses ) : ( &tim2_pulses );
  auto need_pulses = dev ? ( &tim5_need )   : ( &tim2_need );
  *c_pulses = 0;
  *need_pulses = pulses;
  auto dt = UVAR('l'); // todo: common param

  std_out << "# move: dev= " << (int)dev << " x= " << mm << " rev= " << rev
          << " pulses= " << pulses << " v= " << vm << NL;

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0;

  break_flag = 0;
  timn_start( dev );
  for( int i=0; i<10000000 && !break_flag; ++i ) { // TODO: calc time

    if( *c_pulses >= pulses ) {
      break;
    }
    delay_bad_mcs( 10 );
    uint32_t r6F_m = TMC2209_read_reg_n_try( dev, 0x6F, 4 );
    uint32_t r41_m = TMC2209_read_reg_n_try( dev, 0x41, 4 );
    read_sensors();
    if( ( porta_sensors_bits & sensor_flags ) != sensor_flags ) { // TODO: more checks
       break_flag = 1;
    }

    uint32_t tc = HAL_GetTick();
    std_out << HexInt( r6F_m ) << ' ' << HexInt( r41_m ) << ' '
            << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits )
            << ' ' << (int)( tc - tm0 ) << NL;
    delay_ms_until_brk( &tc0, dt );
  }

  timn_stop( dev );
  UVAR('b') = *c_pulses;
  auto d_pulses = pulses - *c_pulses;
  float d_x = (float) d_pulses / (motor_step2turn * motor_mstep);
  *need_pulses = 0;

  std_out << "# move result pulses: task= " << pulses << " done=" << *c_pulses
          << " delta= " << d_pulses << " d_x= " << d_x << " break= " << break_flag << ' '
          << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits ) << NL;

  return break_flag;
}

int cmd_rotate( int argc, const char * const * argv )
{
  float turns = arg2float_d( 1, argc, argv, 1.0f, -50000.0f, 50000.0f );
  float vm = arg2float_d( 2, argc, argv, td.v_rot * 0.001f, 0.0f, 20.0f );

  auto old_sf = sensor_flags;
  sensor_flags = 0; // FAKE flags

  auto rc =  do_move( turns, vm, 0 );

  sensor_flags = old_sf;

  return rc;
}

int cmd_move( int argc, const char * const * argv )
{
  float mm = arg2float_d( 1, argc, argv, 1.0f, -200.0f, 200.0f );
  int ignore_opto = arg2long_d( 2, argc, argv, 0, 0, 1 );
  float vm = arg2float_d( 3, argc, argv, td.v_mov_o * 0.001f, 0.0f, 20.0f );

  auto old_sf = sensor_flags;
  sensor_flags = ignore_opto ? SWLIM_BITS_SW : SWLIM_BITS_ALL;

  auto rc =  do_move( mm, vm, 1 );

  sensor_flags = old_sf;

  return rc;
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

int ensure_drv_prepared()
{
  if( drv_prepared ) {
    return 1;
  }
  if( ! prepare_drv( 0 ) ) {
    std_out << " Fail to prepare drv 0" << NL;
    return 0;
  }
  if( ! prepare_drv( 1 )  ) {
    std_out << " Fail to prepare drv 1" << NL;
    return 0;
  }
  drv_prepared = 1;
  return 1;
}

int cmd_prep( int argc, const char * const * argv )
{
  drv_prepared = 0;
  int rc = ensure_drv_prepared();

  return ( rc != 0 ) ? 1 : 0;
}

int cmd_calc( int argc, const char * const * argv )
{
  int n_t = arg2long_d( 1, argc, argv,     0,  0, 1000000 );
  int d_w = arg2long_d( 2, argc, argv,   210, 20,    5000 );
  int w_l = arg2long_d( 3, argc, argv, 20000, 50,  100000 );
  int eve = arg2long_d( 4, argc, argv,     0,  0,       1 );

  if( ! td.calc( n_t, d_w, w_l, eve ) ) {
    std_out << "# error: bad input data" << NL;
    return 1;
  }

  cmd_pvar( 1, nullptr );


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

