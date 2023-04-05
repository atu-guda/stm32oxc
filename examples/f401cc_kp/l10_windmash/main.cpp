#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_usartio.h> // TODO: auto
#include <oxc_namedints.h>
#include <oxc_atleave.h>

#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

int debug {0};

PinsOut ledsx( GpioB, 12, 4 );


PinsIn pins_tower( TOWER_GPIO, TOWER_PIN0, TOWER_N );
PinsIn pins_swlim( SWLIM_GPIO, SWLIM_PIN0, SWLIM_N );
PinsIn pins_diag (  DIAG_GPIO,  DIAG_PIN0,  DIAG_N );
PinsIn pins_user_start( USER_START_GPIO,  USER_START_PIN0,  USER_START_N );
PinsIn pins_user_stop(   USER_STOP_GPIO,   USER_STOP_PIN0,   USER_STOP_N );
void init_EXTI();

volatile uint32_t porta_sensors_bits {0};
volatile uint32_t portb_sensors_bits {0};
// no user pins for now
const uint32_t porta_sensor_mask = SWLIM_BITS_ALL;
const uint32_t portb_sensor_mask = TOWER_BITS_ALL | DIAG_BITS_ALL;
bool read_sensors(); // returns true at bad condition
uint32_t sensor_flags = SWLIM_BITS_ALL;
bool check_top { false }, check_bot { false }; // work copy from td, only in go

const char* common_help_string = "Winding machine control app " __DATE__ " " __TIME__ NL;

constexpr uint32_t tim_psc_freq   {  10000000 };
constexpr uint32_t tim_pbase_init {  tim_psc_freq / 200 };

constexpr float max_move_len { 200.0f };
constexpr float xlim_move_len { 0.1f };


TIM_HandleTypeDef tim_r_h;
TIM_HandleTypeDef tim_m_h;
int volatile tim_r_pulses {0}, tim_r_need {0}, tim_m_pulses {0}, tim_m_need {0};
int tim_n_cfg( TIM_HandleTypeDef &t_h, TIM_TypeDef *tim, uint32_t ch );
int tim_r_cfg();
void tim_r_start();
void tim_r_stop();
int tim_m_cfg();
void tim_m_start();
void tim_m_stop();
void tims_start( uint8_t devs );
void tims_stop( uint8_t devs );
void timn_start( uint8_t dev );
void timn_stop( uint8_t dev );
uint32_t calc_TIM_arr_for_base_freq_flt( TIM_TypeDef *tim, float base_freq ); // like from oxc_tim.h buf for float

// UART_CONSOLE_DEFINES
UART_HandleTypeDef uah_motordrv;
UsartIO motordrv( &uah_motordrv, USART1 );
TMC_UART_drv tmc_uart_drv( &motordrv );
TMC2209::TMC_devices tmc( &tmc_uart_drv, 4 );

int read_TMC_stat( uint8_t dev, TMC_stat &s );

STD_USART1_IRQ( motordrv );


bool prepare_drv( uint8_t drv ); // true = ok
int ensure_drv_prepared();
int  drv_prepared = 0;
int do_move( float mm, float vm, uint8_t dev );
int do_go( float nt );
const char*  break_flag2str();

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
CmdInfo CMDINFO_READREG { "readreg", '\0', cmd_readreg, " reg - read TMC2209 register"  };
int cmd_writereg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "writereg", '\0', cmd_writereg, " reg val - write TMC2209 register"  };
int cmd_rotate( int argc, const char * const * argv );
CmdInfo CMDINFO_ROTATE { "rot", 'R', cmd_rotate, " turns [v] - rotate"  };
int cmd_move( int argc, const char * const * argv );
CmdInfo CMDINFO_MOVE { "move", 'M', cmd_move, " mm [no_opto] [v] - move"  };
int cmd_prep( int argc, const char * const * argv );
CmdInfo CMDINFO_PREP { "prep", '\0', cmd_prep, " - prepare drivers"  };
int cmd_calc( int argc, const char * const * argv );
CmdInfo CMDINFO_CALC { "calc", '\0', cmd_calc, "n_tot w_d(um) w_l(um) - calculate task"  };
int cmd_repos( int argc, const char * const * argv );
CmdInfo CMDINFO_REPOS { "repos", '\0', cmd_repos, " mm - reposition to "  };
int cmd_meas_x( int argc, const char * const * argv );
CmdInfo CMDINFO_MEAS_X { "meas_x", '\0', cmd_meas_x, " - measure workspace "  };
int cmd_go( int argc, const char * const * argv );
CmdInfo CMDINFO_GO { "go", 'G', cmd_go, " [n] - go/continue next layer "  };
int cmd_off( int argc, const char * const * argv );
CmdInfo CMDINFO_OFF { "off", '\0', cmd_off, " - OFF drivers "  };

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
  &CMDINFO_REPOS,
  &CMDINFO_MEAS_X,
  &CMDINFO_GO,
  &CMDINFO_OFF,
  nullptr
};

TaskData td;

int TaskData::calc( int n_tot, int d_w, int w_l, bool even )
{
  n_done = p_ldone = p_ltask = c_lay =  n_lay = 0;
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
  p_ltask = n_2lay * motor_step2turn * motor_mstep;

  float d_w_e = 0.001f * w_len / n_2lay;
  v_mov = (int)( v_rot * d_w_e + 0.4999f );

  // debug:
  uint32_t d_wire_eff = ( ( w_len * 100  / n_2lay ) + 49 ) / 100;
  std_out << "# debug: n_lay_max= " << n_lay_max << " n_lay= " << n_lay
    << " d_w_e= " << d_w_e << " v_mov= " << v_mov << " d_wire_eff= " << d_wire_eff << NL;

  return 1;
}

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_IOBJ_TD(x) constexpr NamedInt   ob_##x { #x, &td.x }

ADD_IOBJ_TD( n_total );
ADD_IOBJ_TD( d_wire  );
ADD_IOBJ_TD( w_len   );
ADD_IOBJ_TD( v_rot   );
ADD_IOBJ_TD( v_mov_o );
ADD_IOBJ_TD( w_len_m );
ADD_IOBJ_TD( s_rot_m );
ADD_IOBJ_TD( s_mov_m );
ADD_IOBJ_TD( dt );
ADD_IOBJ_TD( check_top );
ADD_IOBJ_TD( check_bot );
ADD_IOBJ_TD( n_lay   );
ADD_IOBJ_TD( n_2lay  );
ADD_IOBJ_TD( v_mov   );
ADD_IOBJ_TD( n_done  );
ADD_IOBJ_TD( p_ldone );
ADD_IOBJ_TD( p_ltask );
ADD_IOBJ_TD( p_move  );
ADD_IOBJ_TD( c_lay   );
ADD_IOBJ   ( debug   );

#undef ADD_IOBJ
#undef ADD_IOBJ_TD


constexpr const NamedObj *const objs_info[] = {
  & ob_n_total,
  & ob_d_wire,
  & ob_w_len,
  & ob_v_rot,
  & ob_v_mov_o,
  & ob_w_len_m,
  & ob_s_rot_m,
  & ob_s_mov_m,
  & ob_dt,
  & ob_check_top,
  & ob_check_bot,
  & ob_n_lay,
  & ob_n_2lay,
  & ob_v_mov,
  & ob_n_done,
  & ob_p_ldone,
  & ob_p_ltask,
  & ob_p_move,
  & ob_c_lay,
  & ob_debug,
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
  UVAR('d') =     0; // TMC2209 device addr for manual
  UVAR('o') =     0; // more measure and output during move/go

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
  if( ! tim_r_cfg() ) {
    die4led( 2 );
  }
  if( ! tim_m_cfg() ) {
    die4led( 3 );
  }

  motordrv.setHandleCbreak( false );
  devio_fds[5] = &motordrv;
  devio_fds[6] = &motordrv;
  motordrv.itEnable( UART_IT_RXNE );

  init_EXTI();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

void init_EXTI()
{
  // TODO: data driven
  TOWER_GPIO.setEXTI( TOWER_PIN_UP, GpioRegs::ExtiEv::updown );
  TOWER_GPIO.setEXTI( TOWER_PIN_CE, GpioRegs::ExtiEv::updown );
  TOWER_GPIO.setEXTI( TOWER_PIN_DW, GpioRegs::ExtiEv::updown );
  DIAG_GPIO.setEXTI(  DIAG_PIN_ROT, GpioRegs::ExtiEv::up );
  DIAG_GPIO.setEXTI(  DIAG_PIN_MOV, GpioRegs::ExtiEv::up );
  HAL_NVIC_SetPriority( EXTI0_IRQn, 15, 0 );
  HAL_NVIC_EnableIRQ(   EXTI0_IRQn );
  HAL_NVIC_SetPriority( EXTI1_IRQn, 15, 0 );
  HAL_NVIC_EnableIRQ(   EXTI1_IRQn );
  HAL_NVIC_SetPriority( EXTI2_IRQn, 15, 0 );
  HAL_NVIC_EnableIRQ(   EXTI2_IRQn );
  HAL_NVIC_SetPriority( EXTI4_IRQn, 15, 0 );
  HAL_NVIC_EnableIRQ(   EXTI4_IRQn );
  HAL_NVIC_SetPriority( EXTI9_5_IRQn, 15, 0 );
  HAL_NVIC_EnableIRQ(   EXTI9_5_IRQn );
}

int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  TMC2209::rreq  rqd;
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
    std_out << "## Write: " NL;
    dump8( (const char*)rqd.rawCData(), sizeof(rqd) );
    auto w_n = motordrv.write_s( (const char*)rqd.rawCData(), sizeof(rqd) );
    auto wr_ok = motordrv.wait_eot( 100 );
    // ledsx.reset( 1 );
    // auto wr_ok = 1;

    delay_ms( 1 );
    memset( in_buf, '\x00', sizeof(in_buf) );
    ledsx.reset( 1 );
    auto r_n = motordrv.read( in_buf, 16, 200 );

    uint32_t  tcc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 ) << " dt = " << ( tcc - tcb )
            << " wr_ok=" << wr_ok << " r_n= " << r_n << " w_n= " << w_n << NL;
    std_out << "## read: " NL;
    dump8( in_buf, r_n );

    delay_ms_until_brk( &tc0, t_step );
  }

  std_out << "### Regs: " NL;
  uint32_t r1 = tmc.read_reg( 0, 2 );
  tmc.write_reg( 0, 1, 0x149 );
  uint32_t r2 = tmc.read_reg( 0, 2 );
  tmc.write_reg( 0, 1, 0x149 );
  tmc.write_reg( 0, 1, 0x141 );
  uint32_t r3 = tmc.read_reg( 0, 2 );
  std_out << "# R2: r1= " << HexInt( r1 ) << "  r2= " << HexInt( r2 ) << "  r3= " << HexInt( r3 ) <<NL;

  return 0;
}


int cmd_readreg( int argc, const char * const * argv )
{
  uint8_t reg = (uint8_t) arg2long_d( 1, argc, argv, 0, 0, 127 );
  uint32_t v = tmc.read_reg( UVAR('d'), reg );
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
  int rc = tmc.write_reg( UVAR('d'), reg, v );
  std_out << "# rc= " << rc << NL;
  return 0;
}

int tim_n_cfg( TIM_HandleTypeDef &t_h, TIM_TypeDef *tim, uint32_t ch )
{
  int pbase = tim_pbase_init;
  t_h.Instance               = tim;
  t_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( tim, tim_psc_freq );
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


int tim_r_cfg()
{
  return tim_n_cfg( tim_r_h, TIM_ROT, TIM_ROT_CH );
}

int tim_m_cfg()
{
  return tim_n_cfg( tim_m_h, TIM_MOV, TIM_MOV_CH );
}

void tim_r_start()
{
  HAL_TIM_PWM_Start_IT( &tim_r_h, TIM_ROT_CH );
  __HAL_TIM_DISABLE_IT( &tim_r_h, TIM_IT_CC2 ); // we need PWM, but IRQ on update event
  __HAL_TIM_ENABLE_IT(  &tim_r_h, TIM_IT_UPDATE );
}

void tim_r_stop()
{
  HAL_TIM_PWM_Stop_IT(  &tim_r_h, TIM_ROT_CH );
  __HAL_TIM_DISABLE_IT( &tim_r_h, TIM_IT_UPDATE );
}

void tim_m_start()
{
  HAL_TIM_PWM_Start_IT( &tim_m_h, TIM_MOV_CH );
  __HAL_TIM_DISABLE_IT( &tim_m_h, TIM_IT_CC3 ); // we need PWM, but IRQ on update event
  __HAL_TIM_ENABLE_IT(  &tim_m_h, TIM_IT_UPDATE );
}

void tim_m_stop()
{
  HAL_TIM_PWM_Stop_IT(  &tim_m_h, TIM_MOV_CH );
  __HAL_TIM_DISABLE_IT( &tim_m_h, TIM_IT_UPDATE );
}

void tims_start( uint8_t devs )
{
  if( devs & 1 ) {
    tim_r_start();
  }
  if( devs & 2 ) {
    tim_m_start();
  }
}

void tims_stop( uint8_t devs )
{
  if( devs & 1 ) {
    tim_r_stop();
  }
  if( devs & 2 ) {
    tim_m_stop();
  }
}

void timn_start( uint8_t dev )
{
  if( dev ) {
    tim_m_start();
  } else {
    tim_r_start();
  }
}

void timn_stop( uint8_t dev )
{
  if( dev ) {
    tim_m_stop();
  } else {
    tim_r_stop();
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
  auto tim = ( dev == 0 ) ? TIM_ROT : TIM_MOV;
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

  // debug
  std_out << "# debug: dev= " << dev << " ARR= " << arr << " freq= " << freq << NL;

  return 0;
}

int cmd_speed( int argc, const char * const * argv )
{
  float speed = arg2float_d( 1, argc, argv, 1.0f, 0.0f, 100.0f );
  int dev = arg2long_d( 2, argc, argv, 0, 0, 1 );

  set_drv_speed( dev, speed );

  tim_print_cfg( TIM_ROT );
  tim_print_cfg( TIM_MOV );

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

  tmc.write_reg( dev, 0, rev ? reg00_def_rev : reg00_def_forv ); // direction


  int pulses = mm * motor_step2turn * motor_mstep;
  auto c_pulses    = dev ? ( &tim_m_pulses ) : ( &tim_r_pulses );
  auto need_pulses = dev ? ( &tim_m_need )   : ( &tim_r_need );
  *c_pulses = 0;
  *need_pulses = pulses;

  check_top  = false;  check_bot  = false;

  uint32_t s_max = dev ? td.s_mov_m : td.s_rot_m;
  tmc.write_reg( dev, 0x40, s_max ); // TODO: dep(speed)

  std_out << "# move: dev= " << (int)dev << " x= " << mm << " rev= " << rev
          << " pulses= " << pulses << " v= " << vm << " s_max= " <<  s_max << NL;

  ledsx.reset( 0x0F );
  TMC_stat st_dev {0,0};

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0;

  break_flag = 0;
  timn_start( dev );
  for( int i=0; i<10000000 && !break_flag; ++i ) { // TODO: calc time

    if( *c_pulses >= pulses ) {
      break;
    }

    if( UVAR('o' ) ) {
      read_TMC_stat( dev, st_dev );
      if( st_dev.status & TMC2209_R6F_badflags ) {
        break_flag = dev ? (int)(BreakNum::drv_flags_mov) : (int)(BreakNum::drv_flags_rot);
        timn_stop( dev );
      }
    }

    read_sensors();
    if( ( porta_sensors_bits & sensor_flags ) != sensor_flags ) { // TODO: more checks
      break_flag = (int)(BreakNum::limits);
      timn_stop( dev );
    }

    uint32_t tc = HAL_GetTick();

    std_out << FmtInt( tc - tm0, 10 ) << ' ';
    if( UVAR('o') ) {
        std_out << FmtInt( st_dev.sg_val, 6 ) << ' ';
    }
    std_out << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits ) << ' '
            << FltFmt( (float) (*c_pulses) / (motor_step2turn * motor_mstep), cvtff_fix, 8, 2 )
            << NL;

    delay_ms_until_brk( &tc0, td.dt );
  }

  if( break_flag ) {
    ledsx.set( 1 );
  }

  timn_stop( dev );
  auto d_pulses = pulses - *c_pulses;
  float d_x = (float) *c_pulses / (motor_step2turn * motor_mstep);
  *need_pulses = 0;

  std_out << "# move result pulses: task= " << pulses << " done=" << *c_pulses
          << " delta= " << d_pulses << " d_x= " << d_x << " break= " << break_flag << ' '
          << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits ) << NL;
  std_out << "# " << break_flag2str() << NL;

  return break_flag;
}

int cmd_rotate( int argc, const char * const * argv )
{
  float turns = arg2float_d( 1, argc, argv, 1.0f, -50000.0f, 50000.0f );
  float vm = arg2float_d( 2, argc, argv, td.v_rot * speed_scale, 0.0f, 20.0f );

  RestoreAtLeave rst_st( sensor_flags );
  sensor_flags = 0; // FAKE flags

  auto rc =  do_move( turns, vm, 0 );

  return rc;
}

int cmd_move( int argc, const char * const * argv )
{
  float mm = arg2float_d( 1, argc, argv, 1.0f, -max_move_len, max_move_len );
  int ignore_opto = arg2long_d( 2, argc, argv, 0, 0, 1 );
  float vm = arg2float_d( 3, argc, argv, td.v_mov_o * speed_scale, 0.0f, 20.0f );

  RestoreAtLeave rst_st( sensor_flags );
  sensor_flags = ignore_opto ? SWLIM_BITS_SW : SWLIM_BITS_ALL;

  auto rc =  do_move( mm, vm, 1 );

  return rc;
}

int cmd_repos( int argc, const char * const * argv )
{
  float mm = arg2float_d( 1, argc, argv, 1.0f, -max_move_len, max_move_len );
  float vm = td.v_mov_o * speed_scale;

  float xmm, shi, emm;
  if( mm > 0 ) {
    xmm = -max_move_len; shi =  xlim_move_len; emm = mm - shi;
  } else {
    xmm =  max_move_len; shi = -xlim_move_len; emm = mm - shi;
  }

  RestoreAtLeave rst_st( sensor_flags );

  sensor_flags = SWLIM_BITS_ALL;
  auto rc = do_move( xmm, vm, 1 );
  if( break_flag != 0 && break_flag != (int)BreakNum::limits ) {
    return rc;
  }

  sensor_flags = SWLIM_BITS_SW;
  do_move( shi, vm, 1 );
  if( break_flag ) {
    return rc;
  }

  sensor_flags = SWLIM_BITS_ALL;
  rc = do_move( emm, vm, 1 );

  return rc;
}

int cmd_meas_x( int argc, const char * const * argv )
{
  float vm = td.v_mov_o * speed_scale;
  RestoreAtLeave rst_st( sensor_flags );

  sensor_flags = SWLIM_BITS_ALL;
  auto rc = do_move( max_move_len, vm, 1 ); // find right limit
  if( break_flag != 0 && break_flag != (int)BreakNum::limits ) {
    return rc;
  }

  sensor_flags = SWLIM_BITS_SW;
  rc = do_move( -xlim_move_len, vm, 1 );  // substep to left
  if( break_flag ) {
    return rc;
  }

  sensor_flags = SWLIM_BITS_ALL;           // find right limit
  rc = do_move( -max_move_len, vm, 1 );
  if( break_flag != 0 && break_flag != (int)BreakNum::limits ) {
    return rc;
  }

  auto d_xt = tim_m_pulses;
  float d_x = xlim_move_len + (float)(d_xt) / ( motor_step2turn * motor_mstep );

  sensor_flags = SWLIM_BITS_SW;
  rc = do_move( xlim_move_len, vm, 1 ); // substep to right
  if( break_flag ) {
    return rc;
  }

  td.w_len_m  = (int)( d_x * 1000 );
  std_out << "# d_x= " << d_x << ' ' << d_xt << NL;

  return rc;
}

int read_TMC_stat( uint8_t dev, TMC_stat &s )
{
  s.status = tmc.read_reg( dev, 0x6F );
  s.sg_val = tmc.read_reg( dev, 0x41 );
  if( s.status == TMC2209::bad_val ) {
    return 1;
  }
  if( s.sg_val == TMC2209::bad_val ) {
    return 2;
  }
  return 0;
}

int do_go( float nt )
{
  if( td.c_lay >= td.n_lay ) {
    std_out << "# All done!" << NL;
    return 0;
  }
  if( ! ensure_drv_prepared() ) {
    std_out << "# Error: drivers not prepared" << NL;
    return  1;
  }

  uint32_t err_bits { 0 };
  ledsx.reset( 0x0F );

  int pulses = td.p_ltask - td.p_ldone;
  int max_pulses = nt * motor_step2turn * motor_mstep;
  if( pulses > max_pulses ) {
    pulses = max_pulses;
  }

  std_out << "# go: c_lay= " << td.c_lay <<  " pulses: " << pulses << NL;

  tim_r_pulses = 0; tim_m_pulses = 0;
  tim_r_need = pulses; // rotaion is a main movement

  bool rev = false;
  if( td.c_lay & 1 ) {
    rev = true;
  }
  float v_rot = td.v_rot * speed_scale;
  set_drv_speed( 0, v_rot );
  float v_mov = td.v_mov * speed_scale;
  set_drv_speed( 1, v_mov );
  td.p_move = 0;

  tmc.write_reg( 0, 0x40, td.s_rot_m ); // TODO: dep(speed)
  tmc.write_reg( 1, 0x40, td.s_mov_m ); // TODO: dep(speed)

  std_out << "# pulses= " << pulses << " rev= " << rev << " v_rot= " << v_rot << " v_mov= " << v_mov << NL;

  if( pulses < 1 || (int)pulses > td.p_ltask ) {
    std_out << "# Error: bad pulses" << NL;
    return  1;
  }

  tims_stop( 3 );

  TMC_stat st_rot {0,0}, st_mov {0,0};

  tmc.write_reg( 0, 0, reg00_def_forv ); // rot direction
  tmc.write_reg( 1, 0, rev ? reg00_def_rev : reg00_def_forv ); // move direction

  sensor_flags = SWLIM_BITS_ALL;
  check_top  = td.check_top;  check_bot  = td.check_bot;

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0;

  break_flag = 0;
  tims_start( 3 );
  for( int i=0; i<10000000 && !break_flag; ++i ) { // TODO: calc time

    const uint32_t t_sl = HAL_GetTick();

    if( tim_r_pulses >= pulses ) {
      break;
    }

    if( UVAR('o' ) ) {
      read_TMC_stat( 0, st_rot );
      read_TMC_stat( 1, st_mov );
      if( st_rot.status & TMC2209_R6F_badflags ) {
        break_flag = (int)(BreakNum::drv_flags_rot);
        err_bits |= 1;
        tims_stop( 3 );
      }
      if( st_mov.status & TMC2209_R6F_badflags ) {
        break_flag = (int)(BreakNum::drv_flags_mov);
        err_bits |= 4;
        tims_stop( 3 );
      }
    }


    if( ( porta_sensors_bits & sensor_flags ) != sensor_flags ) { // TODO: more checks
      break_flag = (int)(BreakNum::drv_flags_rot );
      err_bits |= 16;
      tims_stop( 3 );
    }

    const uint32_t tc = HAL_GetTick();

    const float d_r_c = (float) ( td.p_ldone + tim_r_pulses ) / (motor_step2turn * motor_mstep);

    std_out << FmtInt( tc - tm0, 10 ) << ' ';
    if( UVAR('o') ) {
      std_out << FmtInt( tc - t_sl, 5 ) << ' '
              << FmtInt( st_rot.sg_val, 6 ) << ' ' << FmtInt( st_mov.sg_val, 6 ) << ' ';
    }
    std_out << HexInt16( porta_sensors_bits ) << ' '
            << HexInt16( portb_sensors_bits ) << ' '
            << FltFmt( d_r_c, cvtff_fix, 8, 2 ) << NL;

    delay_ms_until_brk( &tc0, td.dt );
  }

  tims_stop( 3 );
  read_TMC_stat( 0, st_rot );
  read_TMC_stat( 1, st_mov );

  if( break_flag ) {
    ledsx.set( 1 );
  }
  std_out << "# " << HexInt( st_rot.status ) << ' ' << HexInt( st_mov.status )
          << ' ' << FmtInt( st_rot.sg_val, 6 ) << ' ' << FmtInt( st_mov.sg_val, 6 ) << NL;

  UVAR('c') = tim_m_pulses;

  // TODO: separate function + fake call
  auto d_pulses = tim_r_pulses;
  td.p_ldone += d_pulses;
  float d_r = (float) d_pulses / (motor_step2turn * motor_mstep);
  tim_r_need = tim_m_need = 0;
  int add_turns = int( d_r + 0.499f );
  td.n_done  += add_turns;

  if( td.p_ldone >= td.p_ltask ) {
    td.p_ldone = 0;
    std_out << "###################################################### END layer " << td.c_lay << NL;
    ++td.c_lay;
  }

  check_top  = false;  check_bot  = false;

  std_out << "# go: pulses: task= " << pulses << " done=" << d_pulses
          << " d_r= " << d_r << " break= " << break_flag << ' '
          << HexInt16( porta_sensors_bits ) << ' ' << HexInt16( portb_sensors_bits ) << NL;
  std_out << "#  add_turns= " << add_turns
          << " n_done= " << td.n_done << " c_lay= " << td.c_lay << NL;
  std_out << "# " << break_flag2str() << ' ' << HexInt( err_bits ) << NL;

  return break_flag;
}


int cmd_go( int argc, const char * const * argv )
{
  float nt = arg2float_d( 1, argc, argv, 100000.0f, 0.001f, 1000000.0f );
  int rc = do_go( nt );
  return rc;
}


const RegsCmd TMC_init_seq[] = {
  { 0x00, reg00_def_forv },
  { 0x10, reg10_def      },
  { 0x6C, reg6C_def      },
  { 0x14, 0x0FFFFF       },
  { 0x40, 0x0FF          },
};

bool prepare_drv( uint8_t drv )
{
  uint32_t r = tmc.read_reg( drv, 0x06 );
  if( ( r & 0xFF000000 ) != 0x21000000 ) {
    std_out << "# Error init drv " << drv << " bad signature " << HexInt( r ) << NL;
    return false;
  }

  for( auto [reg,val] : TMC_init_seq ) {
    int rc = tmc.write_reg( drv, reg, val );
    if( !rc ) {
      std_out << "# Error init drv " << drv << " reg_0" << NL;
      return false;
    }
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

int cmd_off( int argc, const char * const * argv )
{
  drv_prepared = 0;
  tmc.write_reg( 0, 0x6C, reg6C_off );
  tmc.write_reg( 1, 0x6C, reg6C_off );
  return 0;
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

  if( td.w_len > td.w_len_m ) {
    std_out << "# WARNING: w_len > w_len_m" << NL;
  }

  return 0;
}

const char*  break_flag2str()
{
  static const char* strs[] = {
    "none",
    "cbreak",
    "limits",    // 2
    "tower_top", // 3
    "tower_bot", // 4
    "drv_flags_rot",
    "drv_smin_rot",
    "drv_flags_mov",
    "drv_smin_mov",
    "drv_diag_rot",
    "drv_diag_mov",
    "?max",
  };
  static_assert( std::size(strs) == (unsigned)(BreakNum::max)+1, "Bad break flag strings number" );
  unsigned bfi = (unsigned)break_flag >= (unsigned)(BreakNum::max) ? (unsigned)(BreakNum::max) : (unsigned)break_flag;
  return strs[bfi];
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_ROT ) {
    TIM_ROT_EN;
    GpioA.cfgAF_N( TIM_ROT_GPIO_PIN, TIM_ROT_GPIO_AF );
    HAL_NVIC_SetPriority( TIM_ROT_IRQn, 8, 0 );
    HAL_NVIC_EnableIRQ( TIM_ROT_IRQn );
    UVAR('z') = 7;
    return;
  }

  if( htim->Instance == TIM_MOV ) {
    TIM_MOV_EN;
    GpioA.cfgAF_N( TIM_MOV_GPIO_PIN, TIM_MOV_GPIO_AF );
    HAL_NVIC_SetPriority( TIM_MOV_IRQn, 9, 0 );
    HAL_NVIC_EnableIRQ( TIM_MOV_IRQn );
    UVAR('z') += 8;
    return;
  }

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_ROT ) {
    TIM_ROT_DIS;
    GpioA.cfgIn_N( TIM_ROT_GPIO_PIN );
    HAL_NVIC_DisableIRQ( TIM_ROT_IRQn );
    return;
  }

  if( htim->Instance == TIM_MOV ) {
    TIM_MOV_DIS;
    GpioA.cfgIn_N( TIM_MOV_GPIO_PIN );
    HAL_NVIC_DisableIRQ( TIM_MOV_IRQn );
    return;
  }
}

void TIM_ROT_IRQ_HANDLER()
{
  HAL_TIM_IRQHandler( &tim_r_h );
}

void TIM_MOV_IRQ_HANDLER()
{
  HAL_TIM_IRQHandler( &tim_m_h );
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  read_sensors();
  uint32_t pa = porta_sensors_bits & sensor_flags;

  if( htim->Instance == TIM_ROT ) {
    ++UVAR('y');
    // ledsx.toggle( 2 );
    ++tim_r_pulses;
    if( tim_r_need > 0 && tim_r_pulses >= tim_r_need ) {
      tim_r_stop();
    }
    return;
  }

  if( htim->Instance == TIM_MOV ) {
    ++UVAR('x');
    ++tim_m_pulses;
    ++td.p_move;
    // ledsx.toggle( 4 );
    if( pa != sensor_flags ) {
      tims_stop( 3 );
      break_flag = (int)(BreakNum::limits);
    }

    if( check_top && ( portb_sensors_bits & TOWER_BIT_UP ) ) { // set = bad
      tims_stop( 3 );
      break_flag = (int)(BreakNum::tower_top);
    }

    if( check_bot && ( ( portb_sensors_bits & TOWER_BIT_DW ) == 0 ) ) { // reset = bad
      tims_stop( 3 );
      break_flag = (int)(BreakNum::tower_bot);
    }

    if( tim_m_need > 0 && tim_m_pulses >= tim_m_need ) {
      tim_m_stop();
    }
    return;
  }
}



void HAL_GPIO_EXTI_Callback( uint16_t pin_bit )
{
  ++UVAR('i');
  read_sensors();

  switch( pin_bit ) {
    case TOWER_BIT_UP:
      ledsx.toggle( 2 );
      if( check_top && ( portb_sensors_bits & TOWER_BIT_UP ) ) { // set = bad
        tims_stop( 3 );
        break_flag = (int)(BreakNum::tower_top);
        UVAR('z') = 100;
      }
      break;
    case TOWER_BIT_CE:
      // ledsx.toggle( 4 ); // NOP for now
      break;
    case TOWER_BIT_DW:
      ledsx.toggle( 8 );
      if( check_bot && ( ( portb_sensors_bits & TOWER_BIT_DW ) == 0 ) ) { // reset = bad
        tims_stop( 3 );
        break_flag = (int)(BreakNum::tower_bot);
        UVAR('z') = 102;
      }
      break;
    case DIAG_BIT_ROT:
      ledsx.toggle( 2 );
      tims_stop( 3 );
      break_flag = (int)(BreakNum::drv_diag_rot);
      UVAR('z') = 103;
      break;
    case DIAG_BIT_MOV:
      ledsx.toggle( 4 );
      tims_stop( 3 );
      break_flag = (int)(BreakNum::drv_diag_mov);
      UVAR('z') = 104;
      break;
    default:
      ledsx.toggle( 1 );
      break;
  }
}


void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler( TOWER_BIT_UP );
}

void EXTI1_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( TOWER_BIT_CE );
}

void EXTI2_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( TOWER_BIT_DW );
}

void EXTI3_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler( SWLIM_BIT_NO? );
}

void EXTI4_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( SWLIM_BIT_SR );
}

void EXTI9_5_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( SWLIM_BIT_SL );
  HAL_GPIO_EXTI_IRQHandler( SWLIM_BIT_OR );
  HAL_GPIO_EXTI_IRQHandler( SWLIM_BIT_OL );
  HAL_GPIO_EXTI_IRQHandler( DIAG_BIT_ROT );
  HAL_GPIO_EXTI_IRQHandler( DIAG_BIT_MOV );
}


void EXTI15_10_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler(SW_LEV_3_Pin);
}

// --------------------------------------------------------------------
// TMC_UART_drv:
void TMC_UART_drv::reset()
{
  drv->reset();
}

int  TMC_UART_drv::write( const uint8_t *data, int sz )
{
  int w_n = drv->write( (const char*)data, sz );
  drv->wait_eot( wait_ms );
  return w_n;
}

int  TMC_UART_drv::read( uint8_t *data, int sz )
{
  int r_n = drv->read( (char*)data, sz, wait_ms );
  return r_n;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

