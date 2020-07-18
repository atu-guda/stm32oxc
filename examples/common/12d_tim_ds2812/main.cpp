#include <cstring>
#include <iterator>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test DS2812 LED controller with timer and DMA" NL;

TIM_HandleTypeDef tim_h;
DMA_HandleTypeDef hdma_tim_ch1;

PinOut dbg_pin( GpioA, 9 );


void MX_DMA_Init();
void tim_cfg();
uint16_t t_min, t_max; // timings got 0/1 on wire (in times ticks) min1-max0 = 0, max1-min0 = 1
const uint32_t max_leds = 128; // each LED require 3*8 = 24 bit, each require halfword
uint16_t ledbuf[max_leds*24];


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10;
  UVAR('n') = 1;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  dbg_pin.initHW();
  dbg_pin.reset();
  MX_DMA_Init();
  tim_cfg();
  memset( ledbuf, 0, sizeof(ledbuf) );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  unsigned v = arg2long_d( 1, argc, argv, 0, 0, 100 );
  unsigned td = arg2long_d( 2, argc, argv, UVAR('t'), 0 );
  unsigned n = UVAR('n');

  for( unsigned nled=0; nled < n; ++nled ) {
    unsigned ofs = nled * 24;
    for( unsigned i=0; i<8; ++i ) {
      ledbuf[ofs+i] = t_max;
    }
    ofs += 8;
    for( unsigned i=0; i<8; ++i ) {
      ledbuf[ofs+i] = t_min;
    }
    ofs += 8;
    for( unsigned i=0; i<8; ++i ) {
      ledbuf[ofs+i] = t_min;
    }
  }

  uint16_t s = 0;
  switch( v ) {
    case 0: s = 0;     break;
    case 1: s = t_min; break;
    case 2: s = t_max; break;
    default: s = v; break;
  }

  std_out << "# Test: s= " << s << "  td= " << td <<  NL;

  dbg_pin.set();
  delay_mcs( 20 );

  UVAR('r') = HAL_TIM_PWM_Start_DMA( &tim_h, TIM_CHANNEL_1, (uint32_t*)ledbuf, n*24 );
  //TIM_EXA->CCR1 = s;

  delay_ms( td );
  HAL_TIM_PWM_Stop_DMA( &tim_h, TIM_CHANNEL_1 );
  TIM_EXA->CCR1 = 0;
  TIM_EXA->CNT = 0;

  // delay_ms( td );
  delay_mcs( 20 );
  dbg_pin.reset();


  // uint32_t tm0 = HAL_GetTick();
  // break_flag = 0;
  // for( unsigned i=0; i<n && !break_flag; ++i ) {
  //   ch.ccr = ch.v * pbase / 256;
  //   if( UVAR('d') > 0 ) {
  //     std_out << NL;
  //   }
  //
  //   delay_ms_until_brk( &tm0, t_step );
  // }

  return 0;
}

//  ----------------------------- configs ----------------

void tim_cfg()
{
  auto pbase = calc_TIM_arr_for_base_psc( TIM_EXA, 0, 800000 );
  UVAR('a') = pbase;
  t_min = (uint16_t)( pbase * 7 / 25 ); // * 0.28
  t_max = (uint16_t)( pbase - t_min );

  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = 0;
  tim_h.Init.Period            = pbase;
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }
  UVAR('x') = 1;

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );
  UVAR('x') = 2;

  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 3; // like error
    return;
  }
  UVAR('x') = 3;

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = 0; // pbase / 2;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }

  TIM_EXA->CCR1 = 0;
  TIM_EXA->CNT  = 0;
  UVAR('x') = 11;

}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim )
{
  UVAR('z') = 1;
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PIN1, TIM_EXA_GPIOAF );

  hdma_tim_ch1.Instance                 = DMA2_Stream1;
  hdma_tim_ch1.Init.Channel             = DMA_CHANNEL_6;
  hdma_tim_ch1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tim_ch1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tim_ch1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tim_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_tim_ch1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_tim_ch1.Init.Mode                = DMA_NORMAL;
  hdma_tim_ch1.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_tim_ch1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_tim_ch1 ) != HAL_OK ) {
    UVAR('e') = 14;
    return;
  }

  __HAL_LINKDMA( &tim_h, hdma[TIM_DMA_ID_CC1], hdma_tim_ch1 );
  UVAR('z') = 2;
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PINS );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}

void MX_DMA_Init()
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA2_Stream1_IRQn, 8, 0 );
  HAL_NVIC_EnableIRQ( DMA2_Stream1_IRQn );

}

void DMA2_Stream1_IRQHandler()
{
  leds.set( BIT0 );
  HAL_DMA_IRQHandler( &hdma_tim_ch1 );
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

