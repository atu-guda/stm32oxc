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
    replace_bits( CR[pin_num >> 3], ( pin_num & 7 ) << 2, 4, od ? (uint8_t)(ModeF1::AFOD) : (uint8_t)(ModeF1::AFPP) );
    // uint8_t idx = pin_num >> 3;
    // pin_num &= 0x07;
    // CR[idx] &= ~( 0x0F << ( pin_num << 2 ) );
    // CR[idx] |=  ( ( od ? (uint8_t)(ModeF1::AFOD) : (uint8_t)(ModeF1::AFPP) ) << ( pin_num << 2 ) );
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
        ODR |=  ( 1 << pin_num );
      } else {
        ODR &= ~( 1 << pin_num );
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
    uint8_t idx = pin_num >> 3;
    pin_num &= 0x07;
    CR[idx] &= ~( 0x0F << ( pin_num << 2 ) );
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

  if( (uint8_t)(ev) & (uint8_t)(ExtiEv::up) ) {
    EXTI->RTSR |=  ( 1 << pin );
  } else {
    EXTI->RTSR &= ~( 1 << pin );
  }

  if( (uint8_t)(ev) & (uint8_t)(ExtiEv::down) ) {
    EXTI->FTSR |=  ( 1 << pin );
  } else {
    EXTI->FTSR &= ~( 1 << pin );
  }

  EXTI->IMR  = 1 << pin;
}

// ********************************************************

void GPIO_enableClk( GPIO_TypeDef* gp )
{
  if( gp == GPIOA ) {
    __GPIOA_CLK_ENABLE();
    return;
  }
  if( gp == GPIOB ) {
    __GPIOB_CLK_ENABLE();
    return;
  }
  if( gp == GPIOC ) {
    __GPIOC_CLK_ENABLE();
    return;
  }
  if( gp == GPIOD ) {
    __GPIOD_CLK_ENABLE();
    return;
  }
  #ifdef GPIOE
  if( gp == GPIOE ) {
    __GPIOE_CLK_ENABLE();
    return;
  }
  #endif
  #ifdef GPIOF
  if( gp == GPIOF ) {
    __GPIOF_CLK_ENABLE();
    return;
  }
  #endif
  #ifdef GPIOG
  if( gp == GPIOG ) {
    __GPIOG_CLK_ENABLE();
    return;
  }
  #endif
  #ifdef GPIOH
  if( gp == GPIOH ) {
    __GPIOH_CLK_ENABLE();
    return;
  }
  #endif
  #ifdef GPIOI
  if( gp == GPIOI ) {
    __GPIOI_CLK_ENABLE();
    return;
  }
  #endif
  #ifdef GPIOJ
  if( gp == GPIOJ ) {
    __GPIOJ_CLK_ENABLE();
    return;
  }
  #endif
}



void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask )
{
  GPIOx->ODR = ( PortVal & mask ) | ( GPIOx->ODR & (~mask) );
}

void PinsOut::initHW()
{
  Pins::initHW();
  GPIO_InitTypeDef gpi;

  gpi.Pin = mask;
  gpi.Mode = GPIO_MODE_OUTPUT_PP;
  gpi.Pull = GPIO_NOPULL;
  gpi.Speed = GPIO_SPEED_MAX;
  HAL_GPIO_Init( gpio, &gpi );
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
  GPIO_InitTypeDef gpi;
  gpi.Speed = GPIO_SPEED_MAX;

#ifdef BOARD_BTN0_EXIST
  GPIO_enableClk( BOARD_BTN0_GPIO );
  gpi.Pin   = BOARD_BTN0_BIT;
  gpi.Mode  = BOARD_BTN0_MODE;
  gpi.Pull  = BOARD_BTN0_PULL;
  HAL_GPIO_Init( BOARD_BTN0_GPIO, &gpi );
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN0_IRQ, BOARD_BTN0_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN0_IRQ );
  }
#endif

#ifdef BOARD_BTN1_EXIST
  GPIO_enableClk( BOARD_BTN1_GPIO );
  gpi.Pin   = BOARD_BTN1_BIT;
  gpi.Mode  = BOARD_BTN1_MODE;
  gpi.Pull  = BOARD_BTN1_PULL;
  HAL_GPIO_Init( BOARD_BTN1_GPIO, &gpi );
  if( needIRQ ) {
    HAL_NVIC_SetPriority( BOARD_BTN1_IRQ, BOARD_BTN1_IRQPRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_BTN1_IRQ );
  }
#endif

}

// ----------------------------- PinsOut -----------------------------

void PinsIn::initHW()
{
  Pins::initHW();
  GPIO_InitTypeDef gpi;

  gpi.Pin   = mask;
  gpi.Mode  = GPIO_MODE_INPUT;
  gpi.Pull  = pull;
  gpi.Speed = GPIO_SPEED_LOW; // unused
  HAL_GPIO_Init( gpio, &gpi );
};

// ----------------------------- IoPin -----------------------------

void IoPin::initHW()
{
  GPIO_enableClk( gpio );
  GPIO_InitTypeDef gio;
  gio.Pin   = pin;
  gio.Mode  = GPIO_MODE_OUTPUT_OD;
  gio.Pull  = GPIO_NOPULL;
  gio.Speed = GPIO_SPEED_MAX;
  HAL_GPIO_Init( gpio, &gio );
}


// vim: path=.,/usr/share/stm32cube/inc
