#include <cstring>

#include <oxc_auto.h>

void MX_DMA_Init();
extern DMA_HandleTypeDef hdma_adc1; // in adcdma0.cpp
void MX_ADC1_Init();
extern ADC_HandleTypeDef hadc1;
volatile uint32_t adc_state = 0; // 0 - pre, 1 - done, 2 + -  error


using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah;
int out_uart( const char *d, unsigned n );

int out_uart( const char *d, unsigned n )
{
  return HAL_UART_Transmit( &uah, (uint8_t*)d, n, 10 ) == HAL_OK;
}

const int delay_val = 100;

int main(void)
{
  STD_PROLOG_UART_NOCON;

  leds.write( 0 );
  MSTRF( os, 128, out_uart );

  MX_DMA_Init();
  MX_ADC1_Init();
  if( HAL_ADCEx_Calibration_Start( &hadc1 ) != HAL_OK )  {
    os << "HAL_ADCEx_Calibration_Start failed" NL; os.flush();
    die4led( 0 );
  }

  uint32_t c_msp = __get_MSP();
  os << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) ) << NL;
  os.flush();

  uint32_t tm0 = HAL_GetTick(), tmc = tm0;
  int v[4], vbin;

  for( int n=0; ; ++n ) {
    leds.toggle( BIT1 );

    v[0] = 1000 + (n&0x0F) * 10; // Fake values for now
    v[1] = 2000 + (n&0x07) * 20;
    v[2] = 3000 + (n&0x07) * 30;
    v[3] =  100 + (n&0x07) *  1;

    adc_state = 0;
    if( HAL_ADC_PollForConversion( &hadc1, 10 ) != HAL_OK )  {
      os << "Err: HAL_ADC_PollForConversion" << NL;
      os.flush();
    }
    if( (HAL_ADC_GetState( &hadc1 ) & HAL_ADC_STATE_EOC_REG ) == HAL_ADC_STATE_EOC_REG ) {
      v[0] = HAL_ADC_GetValue( &hadc1 );
    }
    vbin = n & 0x0F;

    os << FmtInt( n * delay_val, 8, '0' ) << ' '
       << FloatMult( v[0], 3 ) << ' '
       << FloatMult( v[1], 3 ) << ' '
       << FloatMult( v[2], 3 ) << ' '
       << FloatMult( v[3], 3 ) << ' '
       << HexInt8( vbin )    << ' '
       << ( HAL_GetTick() - tmc ) << NL;
    os.flush();

    tmc += delay_val;
    uint32_t tm1 = HAL_GetTick();
    delay_ms( (tmc - tm1) < 1000000 ? (tmc - tm1) : 0 );
  }

  return 0;
}

void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *AdcHandle )
{
  adc_state = 1;
  /* Report to main program that ADC sequencer has reached its end */
  // ubSequenceCompleted = SET;
}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{

}

/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc_state = 2;
  // Error_Handler();
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

