#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_gpio.h>


void GpioRegs::cfgOut_common( PinNum pin_num )
{
  #if defined (STM32F1)
    // unused
  #else
  cfg_set_MODER( pin_num, GpioModer::out );
  cfg_set_speed_max( pin_num );
  cfg_set_pull_no( pin_num );
  cfg_set_af0( pin_num );
  #endif
}

void GpioRegs::cfgOut( PinNum pin_num, bool od )
{
  #if defined (STM32F1)
    replace_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4, od ? (uint8_t)(ModeF1::OutOD) : (uint8_t)(ModeF1::OutPP) );
  #else
  cfgOut_common( pin_num );
  cfg_set_ppod( pin_num, od );
  #endif
}



void GpioRegs::cfgAF( PinNum pin_num, uint8_t af, bool od )
{
  #if defined (STM32F1)
    replace_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4,
                  od ? (uint8_t)(ModeF1::AFOD) : (uint8_t)(ModeF1::AFPP) );
  #else
  cfg_set_MODER( pin_num, GpioModer::af );
  cfg_set_speed_max( pin_num );
  cfg_set_ppod( pin_num, od );
  cfg_set_pull_no( pin_num );
  cfg_set_af( pin_num, af );
  #endif

}


void GpioRegs::cfgIn( PinNum pin_num, GpioPull p )
{
  #if defined (STM32F1)
    uint8_t idx = pin_num >> 3;
    pin_num &= 0x07;
    CR[idx] &= ~( 0x0F << ( pin_num << 2 ) );
    if( p == Pull::no ) {
      CR[idx] |=  ( (uint8_t)(GpioModeF1::InFloat)  << ( pin_num << 2 ) );
    } else {
      CR[idx] |=  ( (uint8_t)(GpioModeF1::InPull)   << ( pin_num << 2 ) );
      if( p == Pull::up ) {
        set_bit( ODR, pin_num );
      } else {
        reset_bit( ODR, pin_num );
      }
    }
  #else
  cfg_set_MODER( pin_num, GpioModer::in );
  cfg_set_speed_min( pin_num );
  cfg_set_pp( pin_num );
  cfg_set_pull( pin_num, p );
  cfg_set_af0( pin_num );
  #endif
}


void GpioRegs::cfgAnalog( PinNum pin_num )
{
  #if defined (STM32F1)
    reset_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4 );
  #else
  cfg_set_MODER( pin_num, GpioModer::analog );
  cfg_set_speed_min( pin_num );
  cfg_set_pp( pin_num );
  cfg_set_pull_no( pin_num );
  cfg_set_af0( pin_num );
  #endif
}



void GpioRegs::enableClk() const
{
  RCC->GPIO_EN_REG |= GPIO_EN_BIT0 << getIdx();
}

void GpioRegs::setEXTI( PinNum pin, ExtiEv ev )
{
  const unsigned reg_idx = pin.Num() >> 2;
  constexpr uint32_t exti_mask = (1<<EXTI_CFG_BITS) - 1;
  uint32_t tmp = EXTICFG_PLACE->EXTICR[ reg_idx ];
  tmp &= ~( exti_mask  << ( EXTI_CFG_BITS * ( pin.Num() & 0x03 ) ) ); // reset for this pin
  tmp |=  ( getIdx()   << ( EXTI_CFG_BITS * ( pin.Num() & 0x03 ) ) ); // and set given
  EXTICFG_PLACE->EXTICR[ reg_idx ] = tmp;

  uint32_t mask_pos = 1 << pin.Num();
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

[[ noreturn ]] void die4led( PinMask n )
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

#if defined(BOARD_BTN0)
  BOARD_BTN0.enableClk(); // TODO: config
  BOARD_BTN0.cfgIn( BOARD_BTN0_PULL );
  BOARD_BTN0.setEXTI( BOARD_BTN0_MODE );
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN0_IRQ, BOARD_BTN0_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN0_IRQ );
  }
#endif

#if defined(BOARD_BTN1)
  auto &gpio1 = BOARD_BTN0.port();
  BOARD_BTN1.enableClk();
  BOARD_BTN1.cfgIn( BOARD_BTN1_PULL );
  BOARD_BTN1.setEXTI( BOARD_BTN1_MODE );
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
  gpio.cfgOut_N( mask, true );
}

// --------------- EXTI mass init ------------------

unsigned EXTI_inits( const EXTI_init_info *exti_info, bool initIRQ )
{
  unsigned n = 0;
  for( auto ei = exti_info; ei->pinnum.Num() < 16; ++ei ) {
    ei->gpio.setEXTI( ei->pinnum, ei->dir );
    if( initIRQ && ei->exti_n >= EXTI0_IRQn ) {
      HAL_NVIC_SetPriority( ei->exti_n, ei->prty, ei->subprty );
      HAL_NVIC_EnableIRQ(   ei->exti_n );
      n += 1000;
    }
    ++n;
  }
  return n;
}

// vim: path=.,/usr/share/stm32cube/inc
