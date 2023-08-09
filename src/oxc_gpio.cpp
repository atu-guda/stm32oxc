#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_gpio.h>


void GpioRegs::cfgOut_common( uint8_t pin_num )
{
  #if defined (STM32F1)
    // unused
  #else
  cfg_set_MODER( pin_num, Moder::out );
  cfg_set_speed_max( pin_num );
  cfg_set_pull_no( pin_num );
  cfg_set_af0( pin_num );
  #endif
}

void GpioRegs::cfgOut( uint8_t pin_num, bool od )
{
  #if defined (STM32F1)
    replace_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4, od ? (uint8_t)(ModeF1::OutOD) : (uint8_t)(ModeF1::OutPP) );
  #else
  cfgOut_common( pin_num );
  cfg_set_ppod( pin_num, od );
  #endif
}



void GpioRegs::cfgAF( uint8_t pin_num, uint8_t af, bool od )
{
  #if defined (STM32F1)
    replace_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4,
                  od ? (uint8_t)(ModeF1::AFOD) : (uint8_t)(ModeF1::AFPP) );
  #else
  cfg_set_MODER( pin_num, Moder::af );
  cfg_set_speed_max( pin_num );
  cfg_set_ppod( pin_num, od );
  cfg_set_pull_no( pin_num );
  cfg_set_af( pin_num, af );
  #endif

}


void GpioRegs::cfgIn( uint8_t pin_num, Pull p )
{
  #if defined (STM32F1)
    uint8_t idx = pin_num >> 3;
    pin_num &= 0x07;
    CR[idx] &= ~( 0x0F << ( pin_num << 2 ) );
    if( p == Pull::no ) {
      CR[idx] |=  ( (uint8_t)(ModeF1::InFloat)  << ( pin_num << 2 ) );
    } else {
      CR[idx] |=  ( (uint8_t)(ModeF1::InPull)   << ( pin_num << 2 ) );
      if( p == Pull::up ) {
        set_bit( ODR, pin_num );
      } else {
        reset_bit( ODR, pin_num );
      }
    }
  #else
  cfg_set_MODER( pin_num, Moder::in );
  cfg_set_speed_min( pin_num );
  cfg_set_pp( pin_num );
  cfg_set_pull( pin_num, p );
  cfg_set_af0( pin_num );
  #endif
}


void GpioRegs::cfgAnalog( uint8_t pin_num )
{
  #if defined (STM32F1)
    reset_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4 );
  #else
  cfg_set_MODER( pin_num, Moder::analog );
  cfg_set_speed_min( pin_num );
  cfg_set_pp( pin_num );
  cfg_set_pull_no( pin_num );
  cfg_set_af0( pin_num );
  #endif
}



void GpioRegs::enableClk() const
{
  RCC->GPIO_EN_REG |= GPIO_EN_BIT0 << GpioIdx( *this );
}

void GpioRegs::setEXTI( uint8_t pin, ExtiEv ev )
{
  uint32_t tmp = EXTICFG_PLACE->EXTICR[ pin >> 2 ];
  tmp &= ~( 0x0F             << ( 4 * ( pin & 0x03 ) ) );
  tmp |=  ( GpioIdx( *this ) << ( 4 * ( pin & 0x03 ) ) );
  EXTICFG_PLACE->EXTICR[ pin >> 2 ] = tmp;

  uint32_t mask_pos = 1 << pin;
  uint32_t mask_neg = ~mask_pos;
  if( (uint8_t)(ev) & (uint8_t)(ExtiEv::up) ) {
    EXTI->EXTIREG_RTSR |=  mask_pos;
  } else {
    EXTI->EXTIREG_RTSR &=  mask_neg;
  }

  if( (uint8_t)(ev) & (uint8_t)(ExtiEv::down) ) {
    EXTI->EXTIREG_FTSR |=  mask_pos;
  } else {
    EXTI->EXTIREG_FTSR &=  mask_neg;
  }

  EXTI->EXTIREG_IMR  |= mask_pos;
}

// ********************************************************



void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask )
{
  GPIOx->ODR = ( PortVal & mask ) | ( GPIOx->ODR & (~mask) );
}

// **************** PinsOut ********************************

void PinsOut::initHW()
{
  Pins::initHW();
  gpio.cfgOut_N( mask );
};

[[ noreturn ]] void die4led( uint16_t n )
{
  #ifdef USE_FREERTOS
  taskDISABLE_INTERRUPTS();
  #endif

  leds.set( n );
  oxc_disable_interrupts();
  while(1) {
    delay_bad_ms( 100 );
    leds.toggle( LED_BSP_ERR );
  }
}


void board_def_btn_init( bool needIRQ )
{

#ifdef BOARD_BTN0_EXIST
  BOARD_BTN0_GPIO.enableClk(); // TODO: config
  BOARD_BTN0_GPIO.cfgIn( BOARD_BTN0_N, BOARD_BTN0_PULL );
  BOARD_BTN0_GPIO.setEXTI( BOARD_BTN0_N, BOARD_BTN0_MODE );
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN0_IRQ, BOARD_BTN0_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN0_IRQ );
  }
#endif

#ifdef BOARD_BTN1_EXIST
  BOARD_BTN1_GPIO.enableClk();
  BOARD_BTN1_GPIO.cfgIn( BOARD_BTN1_N, BOARD_BTN1_PULL );
  BOARD_BTN1_GPIO.setEXTI( BOARD_BTN1_N, BOARD_BTN1_MODE );
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN1_IRQ, BOARD_BTN1_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN1_IRQ );
  }
#endif

}

// ----------------------------- PinsIn -----------------------------

void PinsIn::initHW()
{
  Pins::initHW();
  gpio.cfgIn_N( mask, pull );
};

// ----------------------------- IoPin -----------------------------

void IoPin::initHW()
{
  gpio.enableClk();
  gpio.cfgOut_N( pin, true );
}


// vim: path=.,/usr/share/stm32cube/inc
