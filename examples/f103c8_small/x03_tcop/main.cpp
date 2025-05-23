#include <cstring>

#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

void MX_GPIO_Init(void);
void MX_inp_Init();
int MX_USART1_UART_Init(void);


PinsOut led0( GpioC, 13, 1 );

extern "C" {
void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );
} // extern "C"

UART_HandleTypeDef uah_console;
const int TX_BUF_SZ = 128;
char tx_buf[TX_BUF_SZ];
int prs( const char *s );

SPI_HandleTypeDef hspi1;
void MX_SPI1_Init();
void HAL_SPI_MspDeInit( SPI_HandleTypeDef* spiHandle );

const uint8_t MAX31855_FAIL = 0x01; // v[2]
const uint8_t MAX31855_BRK  = 0x01; // v[0]
const uint8_t MAX31855_GND  = 0x02;
const uint8_t MAX31855_VCC  = 0x04;

volatile uint32_t loop_delay = 1000;
const uint16_t    tx_wait  = 100;
const uint16_t    spi_wait = 100;
const uint32_t loop_delays[6] = { 100, 500, 1000, 2000, 10, 20 }; // last 2 is fake

int main(void)
{
  HAL_Init();

  leds.initHW();
  led0.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  leds.write( 0x00 );  delay_bad_ms( 200 );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  MX_inp_Init();
  if( ! MX_USART1_UART_Init() ) {
      die4led( 1 );
  }
  MX_SPI1_Init();

  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4 , GPIO_PIN_SET );

  // xTaskCreate( task_leds, "leds", 1*def_stksz, 0, 1, 0 );
  xTaskCreate( task_send, "send", 2*def_stksz, 0, 1, 0 );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

int prs( const char *s )
{
  if( !s || !*s) {
    return 0;
  }
  int l = strlen( s );
  int rc;
  if( ( rc = HAL_UART_Transmit( &uah_console, (uint8_t*)(s), l, tx_wait )) != HAL_OK ) {
    // leds.toggle( BIT3 );
    return 0;
  }
  return l;
}


void task_send( void *prm UNUSED_ARG )
{
  char buf[32];
  uint8_t v[4];
  uint8_t lbits;
  unsigned n = 0;
  uint16_t ipo;

  prs( "# Start\r\n" );

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  while( 1 )
  {
    TickType_t tcc = xTaskGetTickCount();

    lbits = (uint8_t)( ( n & 1 ) << 3 );
    ipo = ( GPIOA->IDR >> 1 ) & 0x0003;
    loop_delay = loop_delays[ipo];

    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4 , GPIO_PIN_RESET );
    delay_bad_mcs( 100 );
    int last_rc = HAL_SPI_Receive( &hspi1, (uint8_t*)(v), sizeof(v), spi_wait ); // v[0] is MOST significant!
    if( last_rc != HAL_OK ) {
      v[1] |= MAX31855_FAIL; // force error;
    }
    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4 , GPIO_PIN_SET );

    tx_buf[0] = '\0';
    i2dec( tcc - tc00, buf, 8, '0' );  strncat( tx_buf, buf, 12 ); strncat( tx_buf, " ", 2 );

    // try even if error
    int32_t tif =  ( v[3] >> 4 ) | ( v[2] << 4 );
    if( tif & 0x0800 ) {
      tif |= 0xFFFFF000;
    }
    int32_t tid4 = tif * 625;
    ifcvt( tid4, 10000, buf, 4 ); strncat( tx_buf, buf, 14 ); strncat( tx_buf, " ", 2 );

    int32_t tof =  ( v[1] >> 2 ) | ( v[0] << 6 );
    if( tof & 0x2000 ) {
      tof |= 0xFFFFC000;
    }
    int tod4 = tof * 25;
    ifcvt( tod4, 100, buf, 2 ); strncat( tx_buf, buf, 14 ); strncat( tx_buf, " ", 2 );

    if( v[1] & MAX31855_FAIL ) {
      lbits |= 1;
      strncat( tx_buf, "FAIL,", 6 );
      if( v[3] & MAX31855_BRK ) {
        strncat( tx_buf, "BREAK,", 8 );
      }
      if( v[3] & MAX31855_GND ) {
        strncat( tx_buf, "GND,", 6 );
        lbits |= 4;
      }
      if( v[3] & MAX31855_VCC ) {
        strncat( tx_buf, "VCC", 6 );
        lbits |= 2;
      }
      strncat( tx_buf, " ", 2 );
    }

    i2dec( loop_delay, buf ); strncat( tx_buf, buf, 10 ); strncat( tx_buf, " ", 2 );

    // debug
    // word2hex( *(uint32_t*)(v), buf );  strncat( tx_buf, buf, 10 ); strncat( tx_buf, " ", 1 );
    // order is reversed: v[0] is MOST significant!
    // char2hex( v[0], buf );  strncat( tx_buf, buf, 4 ); strncat( tx_buf, " ", 1 );
    // char2hex( v[1], buf );  strncat( tx_buf, buf, 4 ); strncat( tx_buf, " ", 1 );
    // char2hex( v[2], buf );  strncat( tx_buf, buf, 4 ); strncat( tx_buf, " ", 1 );
    // char2hex( v[3], buf );  strncat( tx_buf, buf, 4 ); strncat( tx_buf, " ", 1 );

    strncat( tx_buf, "\r\n", 3 );
    prs( tx_buf );
    // leds.toggle( BIT3 );
    leds.write( lbits );
    led0.toggle( BIT0 );
    vTaskDelayUntil( &tc0, loop_delay );
    ++n;
  }
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

