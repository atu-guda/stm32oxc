#include <iterator>
#include <array>
#include <memory>

#include <oxc_auto.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>
#include <oxc_pwmctltim.h>
#include <oxc_main.h>


using std::size;
using std::size_t;
using namespace oxc;
using namespace SMLRL;

#include "main.h"

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as PWM source v2" NL;

TIM_HandleTypeDef tim_h;


void tim_cfg( uint32_t psc, uint32_t arr, uint32_t cmode );
void pwm_recalc();

// --- local commands;
DCL_CMD_REG( test0,      'T',  " - test PWM vals"  );
DCL_CMD_REG( test_u16,   '\0', " - test PWM u16 vals"  );
DCL_CMD_REG( tinit,      'I',  " - reinit timer"  );
DCL_CMD_REG( servo,      'S',  " - prepare to servo control"  );
DCL_CMD_REG( go_servo,   'G',  " v0 v1 v2 v3 - set servo 0-1000"  );
DCL_CMD_REG( pulse,      'U',  " []- test pulse in us"  );
DCL_CMD_REG( tinfo,      'P',  " print info"  );
DCL_CMD_REG( setfreq,    'F',  " Hz - set freq"  );
DCL_CMD_REG( xtest,      'X',  " unknown test"  );


const uint32_t countmodes[] = {
  TIM_COUNTERMODE_UP,
  TIM_COUNTERMODE_DOWN,
  TIM_COUNTERMODE_CENTERALIGNED1,
  TIM_COUNTERMODE_CENTERALIGNED2,
  TIM_COUNTERMODE_CENTERALIGNED3
};

bool on_servo { false };

std::array<float,size(tim_exa_chspins) > pwm_f;
constinit PwmCtlTim pwm1( TIM_EXA_BASE, tim_exa_chspins );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 10;
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq(TIM_EXA), 1 );
  UVAR_p = psc_i;
  UVAR_a = arr_i;
  UVAR_m = 0;    // mode: 0: up, 1: down, 2: updown
  UVAR_o = 0;    // pOlarity 0: high 1: low
  UVAR_x =  500;    // servo start value (us)
  UVAR_y = 2500;    // servo end value (us)

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  std::ranges::generate( pwm_f, [i = 0] () mutable { return (++i) * 0.2f; });

  pwm1.setAllowPSCadj( true );
  tim_h.Instance = TIM_EXA;
  pwm1.initHW( tim_h, psc_i, arr_i ); // do not really good
  pwm1.setPwms( pwm_f );
  pwm1.initPins();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


CMD_FUNCTION( test0 ) // T
{
  for( size_t i=0; i<pwm_f.size(); ++i ) {
    pwm_f[i] = arg2float_d( i+1, argc, argv, pwm_f[i], 0.0f, 1.0f );
    std_out << i << ' ' << pwm_f[i] << NL;
  }

  pwm1.setPwms( pwm_f );

  tim_print_cfg( TIM_EXA );

  return 0;
}

CMD_FUNCTION( test_u16 ) //
{
  for( size_t i=0; i<pwm_f.size(); ++i ) {
    uint16_t v = ( i + 1 ) * 0x2000;
    pwm1.setPwmU16( i, v );
    std_out << i << ' ' << pwm1.getPwmRaw( i ) << NL;
  }

  tim_print_cfg( TIM_EXA );

  return 0;
}

CMD_FUNCTION( pulse ) // U
{
  for( size_t i=0; i<pwm_f.size(); ++i ) {
    uint32_t pu = arg2ulong_d( i+1, argc, argv, 0 );
    pwm1.setPulse( i, pu );
    std_out << i << ' ' << pu << ' ' << pwm1.getPwmRaw( i ) << NL;
  }

  tim_print_cfg( TIM_EXA );

  return 0;
}



CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_EXA );

  std_out << "# freq:  "  << pwm1.getFreq() << NL;

  dump32( TIM_EXA, 0x60 );

  // pwm1.disable();
  // delay_ms( 5000 );
  // pwm1.enable();

  // std_out << "#p1: " << HexInt( pwm1.getCCR( 0 ) ) << NL;

  return 0;
}

CMD_FUNCTION( setfreq ) // F
{
  auto freq = arg2float_d( 1, argc, argv, 1, 0.01f );

  pwm1.setFreq( freq );

  std_out << "# freq: " << freq << " => " << pwm1.getFreq() << NL;

  return 0;
}

CMD_FUNCTION( xtest ) // X
{
  // std::array tims { TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8, TIM9, TIM10, TIM11, TIM12, TIM13, TIM14 };
  // __TIM2_CLK_ENABLE();
  // __TIM3_CLK_ENABLE();
  // __TIM4_CLK_ENABLE();
  // __TIM5_CLK_ENABLE();
  // __TIM6_CLK_ENABLE();
  // __TIM7_CLK_ENABLE();
  // __TIM8_CLK_ENABLE();
  // __TIM9_CLK_ENABLE();
  // __TIM10_CLK_ENABLE();
  // __TIM11_CLK_ENABLE();
  // __TIM12_CLK_ENABLE();
  // __TIM13_CLK_ENABLE();
  // __TIM14_CLK_ENABLE();
  //
  // // test: is32bit - better to use traits
  // for( auto [i,tim] : std::views::enumerate(tims) ) {
  //   RestoreAtLeave _( tim->CR1 );
  //   RestoreAtLeave _( tim->ARR );
  //   tim->CR1 = 0x00000001;
  //   tim->ARR = 0xFFFFFFFF;
  //   std_out << FmtInt(i+1,2) << ' ' << HexInt(tim) << ' ' <<  HexInt(tim->CR1) << ' ' << HexInt( tim->ARR ) << NL;
  // }

  return 0;
}



CMD_FUNCTION( go_servo ) // G
{
  if( !on_servo ) {
    cmd_servo( argc, argv ); // fake args - unused
  }
  if( !on_servo ) {
    return 1;
  }

  uint32_t scale = UVAR_y - UVAR_x;
  for( size_t ch = 0; ch < pwm1.size(); ++ch ) {
    float q = arg2float_d( ch+1, argc, argv, 0.5f, 0.0f, 1.0f );
    uint32_t pu = UVAR_x + (uint32_t)( scale * q );
    pwm1.setPulse( ch, pu );
    std_out << ch << ' ' << pu << NL;
  }

  return 0;
}


CMD_FUNCTION( tinit ) // I
{
  auto mode_idx = std::min( (size_t)(UVAR_m), std::size(countmodes)-1 );
  pwm1.initHW( tim_h, UVAR_p, UVAR_a, countmodes[mode_idx] );
  tim_print_cfg( TIM_EXA );

  return 0;
}

CMD_FUNCTION( servo ) // S
{
  pwm1.setFreq( 50 );

  uint32_t v0 = ( UVAR_x + UVAR_y ) / 2;
  for( size_t ch=0; ch<pwm1.size(); ++ch ) {
    pwm1.setPulse( ch, v0 );
  }

  tim_print_cfg( TIM_EXA );
  on_servo = true;

  return 0;
}



//  ----------------------------- configs ----------------



void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

