#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_gpio.h>

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
}



void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask )
{
  GPIOx->ODR = ( PortVal & mask ) | ( GPIOx->ODR & (~mask) );
}

void PinsOut::initHW()
{
  GPIO_enableClk( gpio );
  GPIO_InitTypeDef gpi;

  gpi.Pin = mask;
  gpi.Mode = GPIO_MODE_OUTPUT_PP;
  gpi.Pull = GPIO_NOPULL;
  gpi.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init( gpio, &gpi );
};

void die4led( uint16_t n )
{
  #ifdef USE_FREERTOS
  taskDISABLE_INTERRUPTS();
  #endif

  leds.set( n );
  while(1) {
    delay_bad_ms( 100 );
    leds.toggle( BIT0 );
  }
}

void IoPin::initHW()
{
  GPIO_enableClk( gpio );
  GPIO_InitTypeDef gio;
  gio.Pin   = pin;
  gio.Mode  = GPIO_MODE_OUTPUT_OD;
  gio.Pull  = GPIO_NOPULL;
  gio.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init( gpio, &gio );
}


// vim: path=.,/usr/share/stm32cube/inc
