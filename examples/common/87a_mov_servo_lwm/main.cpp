#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_pwmctltim.h>

#include <oxc_main.h>

#include <oxc_actu_servo_lwm.h>


#include <board_robo_cfg.h>

using namespace oxc;
using namespace SMLRL;

using std::begin;
using std::end;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test LWM servo" NL;

// --- local commands;
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setX,  'X',     " x [t_us] - set x"  );
DCL_CMD_REG(    testADC,  'A',     " test ADC"  ); // TODO: remove



ReturnCode init_hw_all();

void idle_main_task()
{
  leds.toggle( 1_mask );
}



TIM_HandleTypeDef tim_servolwm_h;
constinit PwmCtlTim pwm1( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h );
RoboPwmCtl q0_pwm( "q0_pwm", pwm1 );
LinearCoordTransform q0_coord_tr { pi_f/2, 0 }; // TODO: coeff (mech dependent) to header
ActuServoLWM q0_actu( q0_pwm, 0, q0_coord_tr );

ADC_HandleTypeDef hadc_sensor;
DMA_HandleTypeDef hdma_adc_sensor;
ADC_Info adc_s1 ( ADC_SENSOR, ADC_SENSOR_CHPINS ); // overkill?
ReturnCode init_adc_s1();
const constexpr uint32_t adc_n_ch = std::size( ADC_SENSOR_CHPINS ) - 1;
uint16_t adc_buf[ adc_n_ch ];

RoboDevice* hw_robo_devs[] {
  &q0_pwm,

};

RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( hw_robo_devs, robo_joints );


int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idLe after run
  UVAR_t =   20; // t_step, ms
  UVAR_n =   50;
  UVAR_p =  200; // t_pre,  ms
  UVAR_r = 1000; // t_run,  ms, def
  UVAR_o =  500; // t_post, ms

  init_hw_all();


  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  // q0:
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_SERVOLWM_BASE ), SERVOLWM_FREQ );
  pwm1.setAllowPSCadj( true );
  tim_servolwm_h.Instance = TIM_SERVOLWM;
  pwm1.setHardParams( psc_i, arr_i, TIM_COUNTERMODE_UP );
  pwm1.setPwm( 0, 0 );
  pwm1.enable();

  init_adc_s1();

  return robo.init_all();
}




CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  pwm1.setPwm( 0, pwm_v );

  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_SERVOLWM_BASE );

  std_out << "# freq:  "  << pwm1.getFreq() << NL;

  dump32( (void*)TIM_SERVOLWM_BASE, 0x60 );

  return 0;
}

CMD_FUNCTION( setfreq ) // F
{
  auto freq = arg2float_d( 1, argc, argv, 1, 0.01f );

  pwm1.setFreq( freq );

  std_out << "# freq: " << freq << " => " << pwm1.getFreq() << NL;

  return 0;
}

CMD_FUNCTION( pulse ) // U
{
  uint32_t pu = arg2ulong_d( 1, argc, argv, 0 );
  pwm1.setPulse( 0, pu );
  std_out << '#' << pu << ' ' << pwm1.getPwmRaw( 0 ) << NL;

  tim_print_cfg( TIM_SERVOLWM_BASE );

  return 0;
}


struct Data_setX
{
  float x;
};

ReturnCode run_x_loop( const RunLoopState &rls, const RunLoopData &rld, void *data )
{
  if( !data ) {
    return rcFatal;
  }
  auto d = static_cast<Data_setX*>(data);

  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }
  float x = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->x : 0;

  if( rls.stage & RunLoopState::stage_change_flag  ) {
    q0_actu.setQ( x );
    robo.commit_all();
  }

  std_out << FmtInt( rls.tc, 8 ) << ' '  << x << ' ' << pwm1.getPwmRaw( 0 )
    << ' ' << q0_actu.get_q_int() << ' ' << q0_actu.get_q_phy()
    << NL;

  return rcOk;
}


CMD_FUNCTION( setX ) // X
{
  struct Data_setX d;
  d.x = arg2float_d( 1, argc, argv, 0 );
  auto t_run = arg2ulong_d( 2, argc, argv, UVAR_r, 0 );

  RunLoopData rld( UVAR_t, UVAR_p, t_run, UVAR_o );

  auto rc = run_periodic( rld, run_x_loop, &d );

  if( UVAR_l ) {
    q0_actu.idle();
    robo.commit_all();
  }

  return rc.isOk() ? 0 : 2;
}

CMD_FUNCTION( testADC ) // A
{
  std::fill( begin(adc_buf), end(adc_buf), 0 );

  const uint32_t t_wait0 = 2; // ms?

  int rc = 0;
  if( UVAR_l ) {  leds[2].set(); }
  uint32_t r = adc_s1.start_DMA_wait( adc_n_ch, 1, t_wait0 );
  if( UVAR_l ) {  leds[2].reset(); }

  if( r != 0 ) {
    rc = 4;
  }

  for( auto v : adc_buf ) {
    std_out << v << ' ';
  }
  std_out << HexInt(ADC_CLOCK_SYNC_PCLK_DIV4) << NL;

  return rc;
}


// ------------------------ ADC -------------------------------
// BUG: arch-dependent part for now

ReturnCode init_adc_s1()
{
  const uint32_t unsigned stime_idx = adc_arch_sampletimes_n - 2;

  adc_s1.prepare_multi_ev( adc_n_ch, ADC_CLOCK_SYNC_PCLK_DIV4, adc_arch_sampletimes[stime_idx].code, ADC_SOFTWARE_START, BOARD_ADC_DEFAULT_RESOLUTION );

  if( ! adc_s1.init_common() ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

  // TODO: check H7 - DMA not work with ordinary memory ???
  adc_s1.data = adc_buf;
  adc_s1.reset_cnt();

  return rcOk;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC_SENSOR ) {
    return;
  }
  ADC_SENSOR_CLK_EN();
  ADC_SENSOR_DMA_CLK_EN();

  adc_s1.init_gpio_channels();

  adc_s1.DMA_reinit( DMA_NORMAL );

  HAL_NVIC_SetPriority( ADC_SENSOR_DMA_IRQ, ADC_SENSOR_DMA_IRQ_PRTY, 0 );
  HAL_NVIC_EnableIRQ(   ADC_SENSOR_DMA_IRQ );
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC_SENSOR ) {
    return;
  }
  ADC_SENSOR_CLK_DIS();
  ADC_SENSOR_DMA_CLK_DIS();
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds[1].set();
  // adc_s1.convHalfCpltCallback( hadc );
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds[1].set();
  adc_s1.convCpltCallback( hadc );
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // leds[0].set();
  adc_s1.errorCallback( hadc );
}

void BOARD_ADC_DMA_IRQHANDLER(void)
{
  // leds[0].set();
  HAL_DMA_IRQHandler( &adc_s1.hdma_adc );
}




// ------------------- TIM callbacks+init --------------------------

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_SERVOLWM ) {
    TIM_SERVOLWM_CLKEN();
    return;
  }
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_SERVOLWM ) {
    TIM_SERVOLWM_CLKDIS();
    return;
  }
}

