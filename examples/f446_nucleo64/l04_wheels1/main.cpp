#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

#include "wheels_pins.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const int go_tick = 100; // 0.1 s

PinsOut motor_dir( GPIOC, 5, 5 );
const int motor_bits_r = 0x03; // bit 0x04 is reserved
const int motor_bits_l = 0x18;
const int motor_bits   = motor_bits_r | motor_bits_l;

PinsIn proxy_sens( GPIOB, 12, 4 );
enum {
  PROXY_FL = 1, PROXY_FR = 2, PROXY_BR = 4, PROXY_BL = 8,
  PROXY_FA = PROXY_FL | PROXY_FR,
  PROXY_BA = PROXY_BL | PROXY_BR
};

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle );
TIM_HandleTypeDef tim1_h, tim3_h, tim4_h;
void tim1_cfg(); // PWM (1,2), US: (pulse: 3, echo: 4 )
void tim3_cfg(); // count( 1, 2 )
void tim4_cfg(); // servo( 1 )
const int tim1_period = 8500; // approx 20Hz
void set_motor_pwm( int r, int l );
void set_us_dir( int dir ); // -90:90
int us_dir_zero = 1420;
int us_dir_scale = 10;
const int us_scan_min = -80, us_scan_max = 80, us_scan_step = 10,
          us_scan_n = 1 + ( (us_scan_max - us_scan_min) / us_scan_step ) ;
int us_scans[ us_scan_n ];

volatile int us_dir = 0;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_go( int argc, const char * const * argv );
CmdInfo CMDINFO_GO { "go", 'g', cmd_go, " time [right=50] [left=right]"  };
int cmd_us_dir( int argc, const char * const * argv );
CmdInfo CMDINFO_US_DIR { "us_dir", 0, cmd_us_dir, " [dir=0] (-90:90)"  };
int cmd_us_scan( int argc, const char * const * argv );
CmdInfo CMDINFO_US_SCAN { "us_scan", 'U', cmd_us_scan, " - scan via US sensor"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GO,
  &CMDINFO_US_DIR,
  &CMDINFO_US_SCAN,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device


int main(void)
{
  BOARD_PROLOG;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  UVAR('t') = 1000;
  UVAR('n') = 10;

  motor_dir.initHW();
  motor_dir.reset( 0x1F );
  proxy_sens.initHW();

  // UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  // i2c_dbg = &i2cd;

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  tim1_cfg();
  tim3_cfg();
  tim4_cfg();

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int st_a = arg2long_d( 1, argc, argv,   2,    0, 127 );
  int en_a = arg2long_d( 2, argc, argv, 127, st_a, 127 );
  pr( NL "Test0: st_a= " ); pr_h( st_a ); pr( " en_a= " ); pr_h( en_a );
  pr( NL "TIM1: in_freq= "); pr_d( get_TIM_in_freq( TIM1 ) );
  pr( " cnt_freq= " ); pr_d( get_TIM_cnt_freq( TIM1 ) );
  pr( " base_freq= " ); pr_d( get_TIM_base_freq( TIM1 ) );
  pr( NL );
  pr( NL "TIM3: in_freq= "); pr_d( get_TIM_in_freq( TIM3 ) );
  pr( " cnt_freq= " ); pr_d( get_TIM_cnt_freq( TIM3 ) );
  pr( " base_freq= " ); pr_d( get_TIM_base_freq( TIM3 ) );
  pr( NL );
  pr( NL "TIM4: in_freq= "); pr_d( get_TIM_in_freq( TIM4 ) );
  pr( " cnt_freq= " ); pr_d( get_TIM_cnt_freq( TIM4 ) );
  pr( " base_freq= " ); pr_d( get_TIM_base_freq( TIM4 ) );
  pr( NL );

  return 0;
}

void set_motor_pwm( int r, int l )
{
  HAL_TIM_PWM_Stop( &tim1_h, TIM_CHANNEL_1 ); // TODO: use like us_dir: CCR1, CCR2
  HAL_TIM_PWM_Stop( &tim1_h, TIM_CHANNEL_2 );
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = tim1_period * abs(r) / 100;
  HAL_TIM_PWM_ConfigChannel( &tim1_h, &tim_oc_cfg, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = tim1_period * abs(l) / 100;
  HAL_TIM_PWM_ConfigChannel( &tim1_h, &tim_oc_cfg, TIM_CHANNEL_2 );
  HAL_TIM_PWM_Start( &tim1_h, TIM_CHANNEL_1 );
  HAL_TIM_PWM_Start( &tim1_h, TIM_CHANNEL_2 );
}

int cmd_go( int argc, const char * const * argv )
{
  int t    = arg2long_d( 1, argc, argv, 1000,  0, 10000 );
  int r_w  = arg2long_d( 2, argc, argv,   50, -100, 100 );
  int l_w  = arg2long_d( 3, argc, argv,  r_w, -100, 100 );
  pr( NL "go: t= " ); pr_d( t ); pr( " r= " ); pr_d( r_w ); pr( " l= " ); pr_d( l_w ); pr ( NL );

  uint8_t bits = r_w > 0 ?    1 : 0;
  bits        |= r_w < 0 ?    2 : 0;
  bits        |= l_w > 0 ?    8 : 0;
  bits        |= l_w < 0 ? 0x10 : 0;

  if( us_dir != 0 ) {
    set_us_dir( 0 );
    delay_ms( 500 );
  }
  set_motor_pwm( r_w, l_w );

  motor_dir.write( bits );

  bool proxy_flag = false;
  for( ; t > 0 && !break_flag && !proxy_flag; t -= go_tick ) {
    uint16_t prox = ~proxy_sens.read() & 0x0F; // inverse senors
    if( prox ) {
      pr( "Prox: " ); pr_h( prox ); pr( NL );
      if( ( r_w > 0 && prox & PROXY_FR ) ||
          ( r_w < 0 && prox & PROXY_BR ) ||
          ( l_w > 0 && prox & PROXY_FL ) ||
          ( l_w < 0 && prox & PROXY_BL ) )
      {
        proxy_flag = true; // break?
      }
    }
    delay_ms( t > go_tick ? go_tick : t );
  }

  motor_dir.reset( motor_bits );
  set_motor_pwm( 0, 0 );
  if( break_flag ) {
    pr( "Break!" NL );
  }

  return 0;
}

void set_us_dir( int dir )
{
  if( dir < -100 ) { dir = -100; }
  if( dir >  100 ) { dir =  100; }
  uint32_t d = us_dir_zero + dir * us_dir_scale; // TODO: calibrate

  TIM4->CCR1 = d;
  delay_ms( 10 );

  UVAR('d') = d;
  us_dir = dir;
}


int cmd_us_dir( int argc, const char * const * argv )
{
  int dir  = arg2long_d( 1, argc, argv,  0,  -100, 100 );

  set_us_dir( dir );

  pr( NL "us_dir: " ); pr_d( us_dir );  pr ( NL );


  return 0;
}

int cmd_us_scan( int argc, const char * const * argv )
{
  pr( NL "us_scan: " ); pr_d( us_dir );  pr ( NL );
  set_us_dir( us_scan_min ); // to settle before
  delay_ms( 300 );
  for( int i=0, d = us_scan_min; i < us_scan_n && d <= us_scan_max; ++i, d += us_scan_step ) {
    set_us_dir( d );
    delay_ms( 200 );
    int l = UVAR('c');
    us_scans[ i ] = l;
    pr_d( d ); pr( " " ); pr_d( l ); pr( NL );
  }
  set_us_dir( 0 );
  delay_ms( 500 );

  return 0;
}

void tim1_cfg()
{
  tim1_h.Instance               = TIM1;
  // US defines tick: 5.8 mks approx 1mm 170000 = v_c/2 in mm/s, 998 or 846
  tim1_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM1, 170000 );
  tim1_h.Init.Period            = tim1_period; // F approx 20Hz: for  motor PWM
  tim1_h.Init.ClockDivision     = 0;
  tim1_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim1_h ) != HAL_OK ) {
    UVAR('e') = 111; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim1_h, &sClockSourceConfig );

  HAL_TIM_PWM_Init( &tim1_h );


  set_motor_pwm( 0, 0 );

  HAL_TIM_PWM_Stop( &tim1_h, TIM_CHANNEL_3 );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = 3; // 16 us
  HAL_TIM_PWM_ConfigChannel( &tim1_h, &tim_oc_cfg, TIM_CHANNEL_3 );

  TIM_IC_InitTypeDef  tim_ic_cfg;
  // tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_RISING;
  tim_ic_cfg.ICPolarity  = TIM_ICPOLARITY_BOTHEDGE; // rising - start, falling - stop
  tim_ic_cfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_ic_cfg.ICPrescaler = TIM_ICPSC_DIV1;
  tim_ic_cfg.ICFilter    = 0; // 0 - 0x0F
  if( HAL_TIM_IC_ConfigChannel( &tim1_h, &tim_ic_cfg, TIM_CHANNEL_4 ) != HAL_OK ) {
    UVAR('e') = 21;
    return;
  }
  HAL_NVIC_SetPriority(TIM1_CC_IRQn, 5, 0); // TODO: coorect for FreeRTOS
  HAL_NVIC_EnableIRQ( TIM1_CC_IRQn );
  if( HAL_TIM_IC_Start_IT( &tim1_h, TIM_CHANNEL_4 ) != HAL_OK ) {
    UVAR('e') = 23;
  }

  HAL_TIM_PWM_Start( &tim1_h, TIM_CHANNEL_3 );
}

void TIM1_CC_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &tim1_h );
  // leds.toggle( BIT2 );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  uint32_t cap2;
  static uint32_t c_old = 0xFFFFFFFF;
  if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4 )  {
    leds.toggle( BIT1 );
    cap2 = HAL_TIM_ReadCapturedValue( htim, TIM_CHANNEL_4 );
    if( cap2 > c_old ) {
      uint32_t l = cap2 - c_old;
      UVAR('c') = l;
      if( us_dir == 0 ) {
        UVAR('l') = l;
      }
      // leds.toggle( BIT2 );
    }
    c_old = cap2;
    // UVAR('m') = cap2;
    // UVAR('z') = htim->Instance->CNT;
  }
}

void tim3_cfg()
{
}

void tim4_cfg()
{
  tim4_h.Instance               = TIM4;
  // cnt_freq: 1MHz,
  tim4_h.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM4, 1000000 );
  tim4_h.Init.Period            = 9999; // 100 Hz, pwm in us 500:2500
  tim4_h.Init.ClockDivision     = 0;
  tim4_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim4_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim4_h ) != HAL_OK ) {
    UVAR('e') = 114; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim4_h, &sClockSourceConfig );

  HAL_TIM_PWM_Init( &tim4_h );

  HAL_TIM_PWM_Stop( &tim4_h, TIM_CHANNEL_1 );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = us_dir_zero;
  HAL_TIM_PWM_ConfigChannel( &tim4_h, &tim_oc_cfg, TIM_CHANNEL_1 );

  HAL_TIM_PWM_Start( &tim4_h, TIM_CHANNEL_1 );
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  GPIO_InitTypeDef gio;
  if( tim_baseHandle->Instance == TIM1 ) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    /** TIM1:  PA11     ------> TIM1_CH4     */
    gio.Pin       = T1_4_US_Echo_Pin;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gio.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init( T1_4_US_Echo_GPIO_Port, &gio );

  }
  else if( tim_baseHandle->Instance == TIM3 ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    /**TIM3:  PA6     ------> TIM3_CH1    PA7     ------> TIM3_CH2    */
    gio.Pin       = T3_1_M_count_R_Pin | T3_2_count_l_Pin;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_FREQ_LOW;
    gio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init( GPIOA, &gio );

    HAL_NVIC_SetPriority( TIM3_IRQn, 5, 0 );
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }
  else if( tim_baseHandle->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  }
  HAL_TIM_MspPostInit( tim_baseHandle );
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
  GPIO_InitTypeDef gio;
  if( timHandle->Instance == TIM1 )  {
    /**TIM1 GPIO Configuration
     PA8     ------> TIM1_CH1
     PA9     ------> TIM1_CH2
     PA10     ------> TIM1_CH3     */
    gio.Pin = T1_1_M_Right_Pin | T1_2_M_Left_Pin | T1_3_US_Pulse_Pin;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gio.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init( GPIOA, &gio );
  } else if( timHandle->Instance == TIM4 )  {
    /** TIM4 GPIO Configuration   PB6     ------> TIM4_CH1     */
    gio.Pin       = T4_1_servo_Pin;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gio.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init( T4_1_servo_GPIO_Port, &gio );
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM1 )   {
    HAL_NVIC_DisableIRQ( TIM1_CC_IRQn );
    __HAL_RCC_TIM1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, T1_1_M_Right_Pin | T1_2_M_Left_Pin | T1_3_US_Pulse_Pin | T1_4_US_Echo_Pin );
  } else if(tim_baseHandle->Instance == TIM3 ) {
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, T3_1_M_count_R_Pin|T3_2_count_l_Pin);
  } else if( tim_baseHandle->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_DISABLE();
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc


