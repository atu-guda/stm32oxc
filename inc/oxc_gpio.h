#ifndef _OXC_GPIO_H
#define _OXC_GPIO_H

#include <oxc_base.h>


void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );
void GPIO_enableClk( GPIO_TypeDef* gp );


class PinsOut
{
  public:
   PinsOut( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n )
     : gpio( gi ),
       start( a_start ), n( a_n ),
       mask( ((uint16_t)(0xFFFF) << (PORT_BITS - n)) >> (PORT_BITS - n - start) )
     {};
   uint16_t getMask() const { return mask; }
   void initHW();
   const GPIO_TypeDef* dev() { return gpio; }
   uint16_t mv( uint16_t v ) const
   {
     return ((v << start) & mask );
   }
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
   GPIO_TypeDef *gpio;
   const uint8_t start, n;
   const uint16_t mask;
};
extern PinsOut leds;
void die4led( uint16_t n );


#endif

// vim: path=.,/usr/share/stm32cube/inc