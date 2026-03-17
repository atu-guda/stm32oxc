#include <ranges>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_hd44780_i2c.h>

#include "main.h"


using namespace SMLRL;
namespace ranges = std::ranges;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

// USBCDC_CONSOLE_DEFINES;
BOARD_CONSOLE_DEFINES_UART;

int debug {0};

// auto out_q_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x3F );

const int k_v { 8375 };
// const int k_v { 84 };
const int adc_n { 10 };
volatile int adc_dma_end {0};
int32_t  adc_data[adc_n_ch];
uint16_t adc_buf[adc_n_ch];


// ------------------------   commands

const char* common_help_string = "vamini " __DATE__ " " __TIME__ NL;

// --- local commands;
DCL_CMD_REG( test0,       'T', " - test " );



void idle_main_task()
{
  leds[0].toggle();
}

// void on_sigint( int #<{(| c |)}># )
// {
//   tim_lwm_stop();
//   break_flag = 1;
//   ledsx[1].set();
// }

int main(void)
{
  // STD_PROLOG_START;
  // STD_PROLOG_USBCDC;
  STD_PROLOG_UART;

  UVAR_t =    50;
  UVAR_n =    20;

  leds.initHW();
  leds.reset( 0xFF_mask );

  UVAR_v = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;

  lcdt.init_4b();
  lcdt.on();
  lcdt.cls();
  lcdt.puts( " ptn-hlo!\n\t" );

  MX_DMA_Init();
  if( ! MX_ADC1_Init() ) {
    // std_out << "Err: ADC init"  NL;
    die4led( 3_mask );
  }

  BOARD_POST_INIT_BLINK;

  // oxc_add_aux_tick_fun( led_task_nortos );

  // dev_console.setOnSigInt( on_sigint );

  // std_main_loop_nortos( &srl, idle_main_task );

  char buf0[32];
  char buf1[32];

  char chx = 'x';
  uint32_t i = 0;
  while( true ) {
    measure_adc( adc_n );
    uint32_t w = adc_data[0] * adc_data[1] / 10000;
    chx = (i&1) ? ':' : '.';
    // i2dec( adc_data[0], buf0, 8 );
    ifcvt( adc_data[0], 1000, buf0, 3, 2 );
    buf0[7] = ' ';
    ifcvt( w, 1000, buf0+8, 3, 2 );
    buf0[15] = chx;  buf0[16] = '\0';

    // i2dec( adc_data[1], buf1, 8 );
    ifcvt( adc_data[1], 10000, buf1, 3, 2 );
    // i2dec( UVAR_i, buf1, 8 );
    lcdt.puts_xy( 0,0, buf0 );
    lcdt.puts_xy( 0,1, buf1 );
    delay_ms( 200 );
    ++i;
  }

  return 0;
}


int measure_adc( int nx )
{
  if( nx < 1 ) {
    nx = 1;
  }
  ranges::fill( adc_data, 0 );

  for( int i=0; i<nx; ++i ) {
    adc_dma_end = 0;
    if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)adc_buf, adc_n_ch ) != HAL_OK ) {
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
    for( unsigned j=0; j<std::size(adc_data); ++j ) {
      adc_data[j] +=  k_v * adc_buf[j];
    }
  }


  for( auto &x : adc_data ) {
    x /= nx;
  }

  adc_data[0] /= 1000;
  adc_data[1] /= 1000;

  return 1;
}



int cmd_test0( int argc, const char * const * argv )
{
  // char buf0[32];
  // char buf1[32];
  //
  // char chx = 'x';
  // for( int i=0; i<UVAR_n; ++i ) {
  //   measure_adc( adc_n );
  //   i2dec( adc_buf[0], buf0, 8 );
  //   i2dec( adc_data[0], buf1, 8 );
  //   // i2dec( UVAR_i, buf1, 8 );
  //   chx = (i&1) ? 'X' : '.';
  //   buf1[8] = chx;  buf1[9] = '\0';
  //   lcdt.puts_xy( 0,0, buf0 );
  //   lcdt.puts_xy( 0,1, buf1 );
  //   delay_ms( 500 );
  // }
  return 0;
}


// ------------------------------------ ADC ------------------------------------------------

DMA_HandleTypeDef hdma_adc1;
ADC_HandleTypeDef hadc1;

void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, 10, 0 );
  HAL_NVIC_EnableIRQ(   DMA2_Stream0_IRQn );
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &hdma_adc1 );
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
  hadc1.Init.NbrOfConversion       = 2;
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
    errno = 5001;
    return 0;
  }

  sConfig.Rank = 2;
  sConfig.Channel = ADC_CHANNEL_5;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    errno = 5002;
    return 0;
  }

  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC1 ) {
    return;
  }

  ADC_CLK_EN;
  ADC1_PIN0.cfgAnalog();
  ADC1_PIN1.cfgAnalog();

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
  UVAR_j |= 2;
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 ) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    ADC1_PIN0.cfgIn();
    ADC1_PIN1.cfgIn();
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}

void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef* hadc1 )
{
  adc_dma_end = 1;
  ++UVAR_i;
}


// ------------------------------------  ------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

