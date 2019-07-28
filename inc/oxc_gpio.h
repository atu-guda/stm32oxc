#ifndef _OXC_GPIO_H
#define _OXC_GPIO_H

#include <oxc_base.h>


inline constexpr uint16_t make_gpio_mask( uint8_t start, uint8_t n ) {
  return (uint16_t) ( (uint16_t)(0xFFFF) << (PORT_BITS - n) ) >> ( PORT_BITS - n - start );
}


class GpioRegs {
  public:
   enum class Moder { in = 0, out = 1, af = 2, analog = 3 };
   enum class Pull {  no = 0,  up = 1, down = 2 };
   GpioRegs()  = delete; // init only as ptr/ref to real GPIO area
   ~GpioRegs() = delete;
   void enableClk() const;
   inline void set( uint16_t v ) // get given to '1' (OR)
   {
     BSSR = v;
   }
   inline void reset( uint16_t v ) // AND~
   {
     BSSR = v << 16;
   }
   inline void sr( uint16_t bits, bool doSet ) {
     if( doSet ) {
       set( bits );
     } else {
       reset( bits );
     }
   }
   inline void toggle( uint16_t v ) // XOR
   {
     ODR ^= v;
   }

   inline void cfg_set_MODER( uint8_t pin_num, Moder val )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     uint32_t t = MODER;
     t &= ~( 3u  << ( pin_num * 2 ) );
     t |=  ( (uint8_t)(val) << ( pin_num * 2 ) );
     MODER = t;
     #endif
   }
   inline void cfg_set_pp( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     OTYPER  &= ~( 1u << pin_num );
     #endif
   }
   inline void cfg_set_od( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     OTYPER  |= ( 1u << pin_num );
     #endif
   }
   inline void cfg_set_ppod( uint8_t pin_num, bool od )
   {
     if( od ) {
       cfg_set_od( pin_num );
     } else {
       cfg_set_pp( pin_num );
     }
   }
   inline void cfg_set_speed_max( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     OSPEEDR |=  ( 3u << ( pin_num * 2 ) );
     #endif
   }
   inline void cfg_set_speed_min( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     OSPEEDR &= ~( 3u << ( pin_num * 2 ) );
     #endif
   }
   inline void cfg_set_pull( uint8_t pin_num, Pull p )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     PUPDR   &= ~( 3u << ( pin_num * 2 ) );
     PUPDR   |=  ( (uint8_t)p << ( pin_num * 2 ) );
     #endif
   }
   inline void cfg_set_pull_no( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     PUPDR   &= ~( 3u << ( pin_num * 2 ) );
     #endif
   }
   inline void cfg_set_pull_up( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     PUPDR   &= ~( 3u << ( pin_num * 2 ) );
     PUPDR   |=  ( 1u << ( pin_num * 2 ) );
     #endif
   }
   inline void cfg_set_pull_down( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     PUPDR   &= ~( 3u << ( pin_num * 2 ) );
     PUPDR   |=  ( 2u << ( pin_num * 2 ) );
     #endif
   }
   inline void cfg_set_af0( uint8_t pin_num )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     uint8_t idx = pin_num >> 3;
     pin_num  &= 0x07;
     AFR[idx] &= ~( 0x0F << ( pin_num * 4 ) );
     #endif
   }
   inline void cfg_set_af( uint8_t pin_num, uint8_t af )
   {
     #if defined (STM32F1)
       #error "Unimplemeted for now"
     #else
     uint8_t idx = pin_num >> 3;
     pin_num  &= 0x07;
     AFR[idx] &= ~( 0x0F << ( pin_num * 4 ) );
     AFR[idx] |=  (   af << ( pin_num * 4 ) );
     #endif
   }

   void cfgOut_common( uint8_t pin_num );
   void cfgOut( uint8_t pin_num, bool od = false );
   void cfgOut_N( uint16_t pins, bool od = false );

   void cfgAF( uint8_t pin_num, uint8_t af, bool od = false  );
   void cfgAF_N( uint16_t pins, uint8_t af, bool od = false  );

   void cfgIn( uint8_t pin_num, Pull p = Pull::no );
   void cfgIn_N( uint16_t pins,  Pull p = Pull::no );

   void cfgAnalog( uint8_t pin_num );
   void cfgAnalog_N( uint16_t pins );

   template <typename F> void for_selected_pins( uint16_t pins, F f )
   {
     for( uint16_t pb = 1, pin_num = 0; pb != 0; pb <<= 1, ++pin_num ) {
       if( pins & pb ) {
         f( pin_num );
       }
     }
   }


   #if defined (STM32F1)
   __IO uint32_t CR[2];   // CRL + CRH (16*(2+2))
   __IO uint32_t IDR;
   __IO uint32_t ODR;
   __IO uint32_t BSSR;    // 16 set (low) + 16 reset (high)
   __IO uint32_t BRR;
   __IO uint32_t LCKR;
   #else
   __IO uint32_t MODER;   // (2*16): 00 = IN, 01 = GP_out, 10 = AF, 11 = Analog
   __IO uint32_t OTYPER;  // (1*16):  0 = PP, 1 = OD
   __IO uint32_t OSPEEDR; // (2*16): 00 = min, 11 = max
   __IO uint32_t PUPDR;   // (2*16); 00 = NO, 01 = Up, 10 = down, 11 - reserved
   __IO uint32_t IDR;
   __IO uint32_t ODR;
   __IO uint32_t BSSR;    // 16 set (low) + 16 reset (high)
   __IO uint32_t LCKR;
   __IO uint32_t AFR[2];  // 32*4: AF number
   #endif
   // TODO: ASCR on L4?????

};

using GpioRegs_ptr   = GpioRegs*;
using GpioRegs_ptr_c = GpioRegs *const;
using GpioRegs_ref   = GpioRegs&;

inline GpioRegs_ref GpioA = *reinterpret_cast<GpioRegs_ptr_c>(GPIOA_BASE);
inline GpioRegs_ref GpioB = *reinterpret_cast<GpioRegs_ptr_c>(GPIOB_BASE);
inline GpioRegs_ref GpioC = *reinterpret_cast<GpioRegs_ptr_c>(GPIOC_BASE);
#ifdef GPIOD_BASE
inline GpioRegs_ref GpioD = *reinterpret_cast<GpioRegs_ptr_c>(GPIOD_BASE);
#endif
#ifdef GPIOE_BASE
inline GpioRegs_ref GpioE = *reinterpret_cast<GpioRegs_ptr_c>(GPIOE_BASE);
#endif
#ifdef GPIOF_BASE
inline GpioRegs_ref GpioF = *reinterpret_cast<GpioRegs_ptr_c>(GPIOF_BASE);
#endif
#ifdef GPIOG_BASE
inline GpioRegs_ref GpioG = *reinterpret_cast<GpioRegs_ptr_c>(GPIOG_BASE);
#endif
#ifdef GPIOH_BASE
inline GpioRegs_ref GpioH = *reinterpret_cast<GpioRegs_ptr_c>(GPIOH_BASE);
#endif
#ifdef GPIOI_BASE
inline GpioRegs_ref GpioI = *reinterpret_cast<GpioRegs_ptr_c>(GPIOI_BASE);
#endif
#ifdef GPIOJ_BASE
inline GpioRegs_ref GpioJ = *reinterpret_cast<GpioRegs_ptr_c>(GPIOJ_BASE);
#endif
#ifdef GPIOK_BASE
inline GpioRegs_ref GpioK = *reinterpret_cast<GpioRegs_ptr_c>(GPIOK_BASE);
#endif

constexpr inline uint8_t GpioIdx( const GpioRegs &gp )
{
  return (uint8_t)( ( reinterpret_cast<unsigned>(&gp) - GPIOA_BASE ) / ( GPIOB_BASE - GPIOA_BASE ) );
}


// --------------- old part ----------------------------------------

void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );
void GPIO_enableClk( GPIO_TypeDef* gp );
void board_def_btn_init( bool needIRQ );

class Pins
{
  public:
   constexpr Pins( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n )
     : gpio( gi ),
       start( a_start ), n( a_n ),
       mask( make_gpio_mask( start, n ) )
     {};
   uint16_t getMask() const { return mask; }
   void initHW() { GPIO_enableClk( gpio ); }
   const GPIO_TypeDef* dev() { return gpio; }
   uint16_t mv( uint16_t v ) const
   {
     return ((v << start) & mask );
   }
  protected:
   GPIO_TypeDef *const gpio;
   const uint8_t start, n;
   const uint16_t mask;
};

class PinsOut : public Pins
{
  public:
   constexpr PinsOut( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n )
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
   void sr( uint16_t bits, bool doSet ) {
     if( doSet ) {
       set( bits );
     } else {
       reset( bits );
     }
   }
   void toggle( uint16_t v ) // XOR
   {
     gpio->ODR ^= mv( v );
   }
  protected:
   // none for now
};

extern PinsOut leds;

[[ noreturn ]] void die4led( uint16_t n );

class PinsIn : public Pins
{
  public:
   constexpr PinsIn( GPIO_TypeDef *gi, uint8_t a_start, uint8_t a_n, uint16_t a_pull = GPIO_NOPULL  )
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
   constexpr IoPin( GPIO_TypeDef *a_gpio, uint16_t a_pin )
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
   GPIO_TypeDef *const gpio;
   const uint16_t pin;
};


#endif

// vim: path=.,/usr/share/stm32cube/inc
