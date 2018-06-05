#ifndef _OXC_GPIO_H
#define _OXC_GPIO_H

#include <oxc_base.h>


void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );
void GPIO_enableClk( GPIO_TypeDef* gp );


class Pins
{
  public:
   Pins( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n )
     : gpio( gi ),
       start( a_start ), n( a_n ),
       mask( (uint16_t)((uint16_t)(0xFFFF) << (PORT_BITS - n)) >> (PORT_BITS - n - start) )
     {};
   uint16_t getMask() const { return mask; }
   void initHW() { GPIO_enableClk( gpio ); }
   const GPIO_TypeDef* dev() { return gpio; }
   uint16_t mv( uint16_t v ) const
   {
     return ((v << start) & mask );
   }
  protected:
   GPIO_TypeDef *gpio;
   const uint8_t start, n;
   const uint16_t mask;
};

class PinsOut : public Pins
{
  public:
   PinsOut( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n )
     : Pins( gi, a_start, a_n )
     {};
   void initHW();
   void write( uint16_t v )  // set to given, drop old
   {
     gpio->ODR = mv( v ) | ( gpio->ODR & (~mask) );
   }
   void set( uint16_t v )   // get given to '1' (OR)
   {
     gpio->SET_BIT_REG = mv( v );
   }
   void reset( uint16_t v ) // AND~
   {
     #if RESET_BIT_SHIFT == 0
     gpio->RESET_BIT_REG = mv( v );
     #else
     gpio->RESET_BIT_REG = mv( v ) << RESET_BIT_SHIFT;
     #endif
   }
   void toggle( uint16_t v ) // XOR
   {
     gpio->ODR ^= mv( v );
   }
  protected:
   // none for now
};
extern PinsOut leds;
void die4led( uint16_t n );

class PinsIn : public Pins
{
  public:
   PinsIn( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n, uint16_t a_pull = GPIO_NOPULL  )
     : Pins( gi, a_start, a_n ),
       pull( a_pull )
     {};
   void initHW();
   uint16_t read() const
   {
     return ( gpio->IDR & mask ) >> start;
   }
  protected:
   const uint16_t pull;
};

class IoPin {
  public:
   IoPin( GPIO_TypeDef *a_gpio, uint16_t a_pin )
     : gpio( a_gpio ), pin( a_pin ) {};
   void initHW();
   void sw1() { gpio->SET_BIT_REG = pin; };
   void sw0() {
     #if RESET_BIT_SHIFT == 0
     gpio->RESET_BIT_REG = pin;
     #else
     gpio->RESET_BIT_REG = pin << RESET_BIT_SHIFT;
     #endif
   };
   void set_sw0( bool s ) { if( s ) sw1(); else sw0(); }
   uint8_t rw() {
     delay_bad_mcs( 1 );
     return rw_raw();
   };
   uint8_t rw_raw() {
     return (gpio->IDR & pin) ? 1 : 0;
   };

  protected:
   GPIO_TypeDef *gpio;
   uint16_t pin;
};


#endif

// vim: path=.,/usr/share/stm32cube/inc
