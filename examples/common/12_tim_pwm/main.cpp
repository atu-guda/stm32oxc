#include <iterator>

#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as PWM source" NL;

TIM_HandleTypeDef tim_h;


PwmCh pwmc[] = {
  { 0, TIM_CHANNEL_1, (TIM_EXA->CCR1), 25 },
  { 1, TIM_CHANNEL_2, (TIM_EXA->CCR2), 50 },
  { 2, TIM_CHANNEL_3, (TIM_EXA->CCR3), 75 },
  { 3, TIM_CHANNEL_4, (TIM_EXA->CCR4), 90 }
};
// const auto n_pwm_ch = size(pwmc);
void tim_cfg();
void pwm_recalc();
void pwm_update();

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test PWM vals"  );
DCL_CMD_REG( tinit, 'I', " - reinit timer"  );
DCL_CMD_REG( servo, 'S', " - prepare to servo control"  );
DCL_CMD_REG( go_servo, 'G', " v0 v1 v2 v3 - set servo 0-1000"  );


const uint32_t countmodes[] = {
  TIM_COUNTERMODE_UP,
  TIM_COUNTERMODE_DOWN,
  TIM_COUNTERMODE_CENTERALIGNED1,
  TIM_COUNTERMODE_CENTERALIGNED2,
  TIM_COUNTERMODE_CENTERALIGNED3
};
const auto n_countmodes = size(countmodes);

bool on_servo { false };


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM_EXA, 10000  ); // ->10kHz
  UVAR('a') = 9999; // ARR, 10kHz->1Hz
  UVAR('r') = 0;    // flag: raw values
  UVAR('m') = 0;    // mode: 0: up, 1: down, 2: updown
  UVAR('o') = 0;    // pOlarity 0: high 1: low
  UVAR('x') =  500;    // servo start value (us)
  UVAR('y') = 2500;    // servo end value (us)

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  tim_cfg();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  for( auto &ch : pwmc ) {
    if( argc <= (int)(ch.idx+1) ) {
      break;
    }
    ch.v = strtol( argv[ch.idx+1], 0, 0 );
  }

  std_out << NL "# Test0: pwm_vals[]= ";
  for( auto ch : pwmc ) {
    std_out << ch.v <<  ' ';
  }
  std_out <<  NL ;

  // pwm_recalc();
  pwm_update();
  tim_print_cfg( TIM_EXA );

  return 0;
}

int cmd_go_servo( int argc, const char * const * argv )
{
  if( !on_servo ) {
    cmd_servo( argc, argv );
  }
  if( !on_servo ) {
    return 1;
  }

  uint32_t scale = UVAR('y') - UVAR('x');
  for( auto &ch : pwmc ) {
    if( argc <= (int)(ch.idx+1) ) {
      break;
    }
    ch.v = UVAR('x')  + scale * strtol( argv[ch.idx+1], 0, 0 ) / 1000;
  }
  pwm_update();
  tim_print_cfg( TIM_EXA );

  return 0;
}


int cmd_tinit( int argc, const char * const * argv )
{
  tim_cfg();
  tim_print_cfg( TIM_EXA );

  return 0;
}

int cmd_servo( int argc, const char * const * argv )
{
  uint32_t psc = calc_TIM_psc_for_cnt_freq( TIM_EXA, 1000000 );
  uint32_t arr = calc_TIM_arr_for_base_psc( TIM_EXA, psc, 100 );
  UVAR('p') = psc;
  UVAR('a') = arr;
  UVAR('m') = 0;
  UVAR('o') = 0;
  UVAR('r') = 1;
  uint32_t v0 = ( UVAR('x') + UVAR('y') ) / 2;
  for( auto &ch : pwmc ) {
    ch.ccr = v0;
    ch.v   = v0;
  }
  tim_cfg();
  tim_print_cfg( TIM_EXA );
  on_servo = true;

  return 0;
}



//  ----------------------------- configs ----------------

void tim_cfg()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = UVAR('p');
  tim_h.Init.Period            = UVAR('a');
  tim_h.Init.ClockDivision     = 0;
  unsigned cmode = UVAR('m');
  if( cmode > n_countmodes ) {
    cmode = 0;
  }
  tim_h.Init.CounterMode       = countmodes[cmode];
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  pwm_recalc();
}

void pwm_recalc()
{
  int pbase = UVAR('a');
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = UVAR('o') ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  for( auto ch : pwmc ) {
    HAL_TIM_PWM_Stop( &tim_h, ch.ch );
    tim_oc_cfg.Pulse = UVAR('r') ? ( ch.v ) : ( ch.v * pbase / 100 ) ;
    if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, ch.ch ) != HAL_OK ) {
      UVAR('e') = 11 + ch.idx;
      return;
    }
    HAL_TIM_PWM_Start( &tim_h, ch.ch );
  }

}

void pwm_update()
{
  tim_h.Instance->PSC  = UVAR('p');
  int pbase = UVAR('a');
  tim_h.Instance->ARR  = pbase;

  for( auto ch : pwmc ) {
    ch.ccr = UVAR('r') ? ch.v : ( ch.v * pbase / 100 );
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

