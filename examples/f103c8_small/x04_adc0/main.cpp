#include <cstring>

#include <oxc_auto.h>

#define Din_GPIO_Port GPIOB
#define Din_0_Pin GPIO_PIN_3
#define Din_1_Pin GPIO_PIN_4
#define Din_2_Pin GPIO_PIN_8
#define Din_3_Pin GPIO_PIN_9

void MX_DigitalIn_Init();
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
int vref_mv = 3339;

int main(void)
{
  STD_PROLOG_UART_NOCON;

  leds.write( 0 );
  MSTRF( os, 128, out_uart );

  MX_DigitalIn_Init();
  MX_DMA_Init();
  MX_ADC1_Init();

  uint32_t c_msp = __get_MSP();
  os << "# MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) ) << NL;
  os.flush();

  if( HAL_ADCEx_Calibration_Start( &hadc1 ) != HAL_OK )  {
    os << "HAL_ADCEx_Calibration_Start failed" NL; os.flush();
    die4led( 0 );
  }

  delay_ms( 10 );

  uint16_t v[4], vbin;
  char stat_str[4];

  uint32_t tm0 = HAL_GetTick(), tmc = tm0;

  for( int n=0; ; ++n ) {
    leds.toggle( BIT1 );

    adc_state = 0; stat_str[0] = ' '; stat_str[1] = ' '; stat_str[2] = ' '; stat_str[3] = '\0';
    // int jj = 0;

    if( HAL_ADC_Start_DMA( &hadc1, (uint32_t *)v, 4 ) != HAL_OK )   {
      stat_str[0] = 'S';
    }
    for( int j=0; adc_state == 0 && j<100; ++j ) {
      delay_ms( 1 ); //      jj = j;
    }
    if( adc_state != 1 )  {
      stat_str[1] = 'W';
    }

    int vi[4];
    for( int j=0; j<4; ++j ) {
      vi[j] = v[j] * vref_mv / 4096;
    }

    // vbin = n & 0x0F;
    uint16_t d_in_pure = GPIOB->IDR;
    vbin  = ( d_in_pure >> 3 ) & 0x03;
    vbin |= ( d_in_pure >> 6 ) & 0x0C;

    os << FmtInt( n * delay_val, 8, '0' ) << ' '
       << FloatMult( vi[0], 3 ) << ' '
       << FloatMult( vi[1], 3 ) << ' '
       << FloatMult( vi[2], 3 ) << ' '
       << FloatMult( vi[3], 3 ) << ' '
       << HexInt8( vbin )    << ' '  // << jj << ' '
       << ( HAL_GetTick() - tmc ) << ' ' << stat_str << NL;
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
  // leds.toggle( BIT3 );
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // NOP
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc_state = 2;
  // leds.toggle( BIT0 );
}

void MX_DigitalIn_Init()
{
  GPIO_InitTypeDef gio;
  __HAL_AFIO_REMAP_SWJ_NOJTAG(); // to use B3
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gio.Pin  = Din_0_Pin | Din_1_Pin | Din_2_Pin | Din_3_Pin;
  gio.Mode = GPIO_MODE_INPUT;
  gio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( GPIOB, &gio );
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

