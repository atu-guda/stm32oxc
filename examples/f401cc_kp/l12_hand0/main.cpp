#include <cerrno>
#include <algorithm>
#include <cmath>
#include <array>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_atleave.h>
#include <oxc_outstr.h>
#include <oxc_as5600.h>

#include "main.h"

// using namespace std;
namespace ranges = std::ranges;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

// ------------------------------- Coords -----------------------------------


struct CoordInfo {
  float x_min, x_max;
  float v_max;
  float x_cur; // TODO: separate, other - const
};

CoordInfo coords[] {
  {  0.10f, 0.90f, 0.20f, 0.50f }, // rotate
  {  0.45f, 0.80f, 0.50f, 0.50f }, // arm1
  {  0.45f, 0.80f, 0.50f, 0.50f }, // arm2
  {  0.00f, 0.70f, 0.30f, 0.30f }, // grip
};
constexpr size_t n_coords = std::size(coords);

// ------------------------------- Movers -----------------------------------

enum class MoverType : uint8_t {
  direct = 0, // x -> position
  integr = 1, // x -> speed
  step   = 2  // ??? 
};


// MoverType::integr,
// MoverType::direct,
// MoverType::direct,
// MoverType::direct,




// ------------------------   end coords
int debug {0};
int dry_run {0};

PinsOut ledsx( LEDSX_GPIO, LEDSX_START, LEDSX_N );



const char* common_help_string = "hand0 " __DATE__ " " __TIME__ NL;

// TODO: to motor objects
TIM_HandleTypeDef tim_lwm_h;
uint32_t tim_lwm_arr { 39999 }; // near init value, will be recalculated
int lwm_t_min {  500 }; // min pulse width in us
int lwm_t_max { 2500 }; // max pulse width in us

// --- local commands;
DCL_CMD ( test0,  'T', " [val] [ch] [k_v] - test move 1 ch" );
DCL_CMD ( stop,   'P', " - stop pwm" );
DCL_CMD ( mtest,  'M', " - test AS5600" );
DCL_CMD ( mcoord, 'C', " - measure and store coords" );

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_test0,
  &CMDINFO_stop,
  &CMDINFO_mtest,
  &CMDINFO_mcoord,
  nullptr
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );
SensorAS5600 sens_enc( ang_sens );

int adc_n {100};
volatile int adc_dma_end {0};
// uint16_t adc_buf[4]; // really need 3, but for alignment
// uint32_t adc_data[4]; // collected and divided data (by adc.measure)
SensorAdc sens_adc( 3 );

std::array<Sensor*,2> sensors { &sens_adc, &sens_enc };
int measure_store_coords( int nm );

// TaskData td;


#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_IOBJ_TD(x) constexpr NamedInt   ob_##x { #x, &td.x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
#define ADD_FOBJ_TD(x) constexpr NamedFloat ob_##x { #x, &td.x }

// ADD_IOBJ_TD( n_total );
// ADD_FOBJ_TD( d_wire  );
ADD_IOBJ   ( debug   );
ADD_IOBJ   ( dry_run   );
ADD_IOBJ   ( adc_n   );
ADD_IOBJ   ( lwm_t_min   );
ADD_IOBJ   ( lwm_t_max   );

#undef ADD_IOBJ
#undef ADD_IOBJ_TD


constexpr const NamedObj *const objs_info[] = {
  & ob_debug,
  & ob_dry_run,
  & ob_adc_n,
  & ob_lwm_t_min,
  & ob_lwm_t_max,
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
  // if( cstate_go == 0 && ostate_go != 0 ) {
  //   uint32_t cur_start_tick = HAL_GetTick();
  //   if( cur_start_tick - last_start_tick > 100 ) {
  //     leds.toggle( 1 );
  //     if( global_smallrl != nullptr && global_smallrl->get()[0] == '\0' ) {
  //       ungets( 0, "G\n" );
  //     }
  //     last_start_tick = cur_start_tick;
  //   }
  // }
  // ostate_go = cstate_go;
}

void on_sigint( int /* c */ )
{
  tim_lwm_stop();
  ledsx[1].set();
}

int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') =    50;
  UVAR('n') =    20;

  ledsx.initHW();
  ledsx.reset( 0xFF );

  UVAR('v') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

  MX_DMA_Init();
  if( ! MX_ADC1_Init() ) {
    std_out << "Err: ADC init"  NL;
    die4led( 3 );
  }

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  if( ! tim_lwm_cfg() ) {
    std_out << "Err: timer LWM init"  NL;
    die4led( 2 );
  }
  tim_lwm_start();

  init_EXTI();

  ranges::for_each( sensors, [](auto ps) { ps->init(); } );
  sens_adc.init();
  sens_enc.init();
  sens_enc.set_zero_val( 2381 ); // mech param: init config?


  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  dev_console.setOnSigInt( on_sigint );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}



void init_EXTI()
{
  //   ei.gpio.setEXTI( ei.pin, ei.dir );
  //   HAL_NVIC_SetPriority( ei.exti_n, EXTI_IRQ_PRTY, 0 );
  //   HAL_NVIC_EnableIRQ(   ei.exti_n );
}

int cmd_test0( int argc, const char * const * argv )
{
  const unsigned  ch = arg2long_d(  2, argc, argv, 1, 0, n_coords );
  auto &mo = coords[ch];
  const float x_e = arg2float_d( 1, argc, argv, 0.5f, mo.x_min, mo.x_max );
  const float k_v = arg2float_d( 3, argc, argv, 0.5f,    0.01f, 2.0f );

  const uint32_t t_step = UVAR('t');
  const float   x_0  = mo.x_cur;
  const float  x_dlt = x_e - x_0;
  const float x_adlt = fabsf( x_dlt );
  const float      v = k_v * mo.v_max;

  const int n = std::clamp( int( x_adlt/( v * t_step * 1e-3f ) ), 1, 10000 );
  const float dx = x_dlt / n;

  ledsx.reset ( 0xFF );

  std_out
    <<  "# Test0: ch= " << ch << " x_0= " << x_0 << " x_e= " << x_e << " v=" << v << " n= " << n
    << " dx= " << dx << " dt= " << t_step << NL;

  static const decltype( &TIM_LWM->CCR1 ) ccrs[] { &TIM_LWM->CCR1, &TIM_LWM->CCR2, &TIM_LWM->CCR3, &TIM_LWM->CCR4 };

  std_out <<  "#  i   tick     dt      x    ccr" NL;

  tim_lwm_start();


  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;


  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t  tcb = HAL_GetTick();

    auto adc_nread = sens_adc.measure( adc_n );
    if( !adc_nread ) {
      std_out << "# Err adc_n : " << adc_nread << " != " << adc_n << NL;
      break;
    }
    sens_enc.measure( 1 );

    const float x = ( i < (n-1) ) ? ( x_0 + dx * (i+1) ) : x_e; // TODO: fun
    const uint32_t vi = std::clamp(                            // TODO: motor
        (uint32_t) (lwm_t_min + (lwm_t_max-lwm_t_min) * x),
        (uint32_t)lwm_t_min, (uint32_t) lwm_t_max
    );
    uint32_t ccr = (uint32_t) tim_lwm_arr * vi / tim_lwm_t_us;

    if( !dry_run ) {
      *ccrs[ch] = ccr;
    }
    mo.x_cur = x;

    ledsx[2].set();

    uint32_t  tcc = HAL_GetTick();
    std_out << FmtInt(i,4) << ' ' << FmtInt( tcc - tc00, 6 ) << ' ' << FmtInt( tcc - tcb, 6 )
      << ' ' << FltFmt(x, cvtff_auto, 8, 4)  << ' ' << ccr
      << ' ' << sens_adc.getUint(0) << ' ' << sens_adc.getUint(1) << ' ' << sens_enc.get(0) << NL;

    delay_ms_until_brk( &tc0, t_step );
    ledsx[2].reset();
  }


  if( debug > 0 ) {
    tim_print_cfg( TIM_LWM );
  }

  return 0;
}


int cmd_stop( int argc, const char * const * argv )
{
  tim_lwm_stop();
  return 0;
}

int cmd_mtest( int argc, const char * const * argv )
{
  const int  set_pos = arg2long_d( 1, argc, argv, 0, 0, 1 );
  sens_enc.measure( 1 );
  auto alp_i = sens_enc.getUint(0);
  auto alp_v = sens_enc.get( 0 );

  std_out
      << alp_i << ' ' << alp_v
      << ' ' << ang_sens.getN_turn() << ' ' << ang_sens.getOldVal() << NL;

  std_out << "=== AGC: " << ang_sens.getAGCSetting() << " cordic: " <<  ang_sens.getCORDICMagnitude()
    << " detect: "  << ang_sens.isMagnetDetected() << " status: " << HexInt8( ang_sens.getStatus() ) << NL;
  if( set_pos ) {
    ang_sens.setStartPosCurr();
  }
  return 0;
}

int cmd_mcoord( int argc, const char * const * argv )
{
  const int n_meas = arg2long_d(  1, argc, argv, adc_n, 1, 10000 );
  return measure_store_coords( n_meas );
}

int measure_store_coords( int nm )
{
  for( auto ps : sensors ) {
    auto rc = ps->measure( nm );
    if( rc < 1 ) {
      return 0;
    }
  }

  return 1;
}

// -------------------- Timers ----------------------------------------------------

int tim_lwm_cfg()
{
  const uint32_t psc { calc_TIM_psc_for_cnt_freq( TIM_LWM, tim_lwm_psc_freq ) };
  tim_lwm_arr = calc_TIM_arr_for_base_psc( TIM_LWM, psc, tim_lwm_freq );
  UVAR('a') = psc;
  UVAR('b') = tim_lwm_arr;

  auto &t_h { tim_lwm_h };
  t_h.Instance               = TIM_LWM;
  t_h.Init.Prescaler         = psc;
  t_h.Init.Period            = tim_lwm_arr;
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
  tim_oc_cfg.Pulse        = 0; // tim_lwm_arr / 2; // TMP to test, need 0;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Stop( &t_h, ch );
    if( HAL_TIM_PWM_ConfigChannel( &t_h, &tim_oc_cfg, ch ) != HAL_OK ) {
      UVAR('e') = 3000 + ch;
      return 0;
    }
  }
  return 1;
}


void tim_lwm_start()
{
  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Start( &tim_lwm_h, ch );
  }
}

void tim_lwm_stop()
{
  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Stop( &tim_lwm_h, ch );
  }
}



bool read_sensors()
{
  return true;
}


void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_LWM ) {
    TIM_LWM_EN;
    GpioA.cfgAF_N( TIM_LWM_GPIO_PINS, TIM_LWM_GPIO_AF );
    // HAL_NVIC_SetPriority( TIM_LWM_IRQn, 8, 0 );
    // HAL_NVIC_EnableIRQ( TIM_LWM_IRQn );
    return;
  }


}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_LWM ) {
    TIM_LWM_DIS;
    GpioA.cfgIn_N( TIM_LWM_GPIO_PINS );
    // HAL_NVIC_DisableIRQ( TIM_LWM_IRQn );
    return;
  }
}

// void TIM_LWM_IRQ_HANDLER()
// {
//   HAL_TIM_IRQHandler( &tim_lwm_h );
// }


void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  // read_sensors();
  // uint32_t pa = porta_sensors_bits & sensor_flags;
  //
  // if( htim->Instance == TIM_LWM ) {
  //   ++UVAR('y');
  //   // ledsx.toggle( 2 );
  //   ++tim_lwm_pulses;
  //   if( tim_lwm_need > 0 && tim_lwm_pulses >= tim_lwm_need ) {
  //     tim_lwm_stop();
  //   }
  //   return;
  // }

}


void HAL_GPIO_EXTI_Callback( uint16_t pin_bit )
{
  ++UVAR('i');
  // bool need_stop { false };

  switch( pin_bit ) {
    // case USER_STOP_BIT:
    //   need_stop = true;
    //   break_flag = (int)(BreakNum::cbreak);
    //   break;

    default:
      ledsx.toggle( 1 );
      ++UVAR('j');
      break;
  }

  // if( need_stop ) {
  //   tims_stop( TIM_BIT_ALL );
  // }
}


void EXTI2_IRQHandler()
{
  // HAL_GPIO_EXTI_IRQHandler( TOWER_BIT_UP );
}

// ------------------------------------ ADC ------------------------------------------------

DMA_HandleTypeDef hdma_adc1;
ADC_HandleTypeDef hadc1;

void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, 10, 0 );
  HAL_NVIC_EnableIRQ(   DMA2_Stream0_IRQn );
  UVAR('j') |= 4;
}

void DMA2_Stream0_IRQHandler(void)
{
  ledsx[1].set();
  HAL_DMA_IRQHandler( &hdma_adc1 );
  ledsx[1].reset();
  UVAR('j') |= 8;
}

int MX_ADC1_Init(void)
{
  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK ) {
    errno = 5000; return 0;
  }

  ADC_ChannelConfTypeDef sConfig {
    .Channel      = ADC_CHANNEL_4,
    .Rank         = 1,
    .SamplingTime = ADC_SAMPLETIME_144CYCLES,
    .Offset       = 0
  };
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    errno = 5001; return 0;
  }

  sConfig.Rank = 2;
  sConfig.Channel = ADC_CHANNEL_5;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    errno = 5002; return 0;
  }

  sConfig.Rank = 3;
  sConfig.Channel = ADC_CHANNEL_6;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    errno = 5003; return 0;
  }
  UVAR('j') |= 1;
  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC1 ) {
    return;
  }

  ADC_CLK_EN;
  ADC1_GPIO.cfgAnalog_N( ADC1_PINS );

  hdma_adc1.Instance                 = DMA2_Stream0;
  hdma_adc1.Init.Channel             = DMA_CHANNEL_0;
  hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_adc1.Init.Mode                = DMA_NORMAL;
  hdma_adc1.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_adc1 ) != HAL_OK ) {
    errno = 5010; return;
  }

  __HAL_LINKDMA( adcHandle, DMA_Handle, hdma_adc1 );
  UVAR('j') |= 2;

}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 ) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    ADC1_GPIO.cfgIn_N( ADC1_PINS );
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}

void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef* hadc1 )
{
  ++UVAR('i');
  ledsx[0].toggle();
  adc_dma_end = 1;
}


// ------------------------------------ Sensors classes ---------------------------------

int SensorAdc::init()
{
  return 1;
}

int SensorAdc::measure( int nx )
{
  if( nx < 1 ) {
    nx = 1;
  }
  std::ranges::fill( adc_data, 0 );

  for( int i=0; i<nx; ++i ) {
    adc_dma_end = 0;
    if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)adc_buf, 3 ) != HAL_OK ) {
      errno = 4567;
      return 0;
    }
    for( int i=0; i<100000; ++i ) {
      if( adc_dma_end ) {
        break;
      }
    }
    if( ! adc_dma_end ) {
      errno = 4568;
      return 0;
    }
    for( int j=0; j<4; ++j ) {
      adc_data[j] += adc_buf[j];
    }
  }

  for( auto &x : adc_data ) {
    x /= nx;
  }

  return 1;
}



int SensorAS5600::init()
{
  ang_sens.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  return 1;
}

int SensorAS5600::measure( int /*nx*/ )
{
  iv = dev.getAngle();
  v  = ( (float) iv  - zero_val ) * 1.1553e-1f; // TODO: what?
  return 1;
}

// ------------------------------------  ------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

