#ifndef _OXC_GPIO_H
#define _OXC_GPIO_H

#include <functional>

#include <oxc_base.h>
#include <oxc_bitops.h>
#include <oxc_refptr.h>


// ---------- special mini-classes to strong parameters types
class GpioRegs;
extern inline GpioRegs *const GPIOs[];
class PinMask;
class PinNum;
class PortPin;


class PinMask {
  public:
   explicit constexpr PinMask( uint16_t mask_ ) : mask ( mask_ ) {};
   inline constexpr uint16_t  bitmask() const { return mask; }
   // TODO: iterator
  private:
   uint16_t mask;
};
static_assert( sizeof(PinMask) == sizeof( uint16_t ) );
constexpr PinMask operator""_mask( unsigned long long v ) {
  return PinMask( v );
}

class PinNum {
  public:
   explicit constexpr PinNum( uint8_t num_ ) : num ( num_ ) {};
   inline constexpr uint8_t  Num() const { return num; }
   inline constexpr PinMask Mask() const { return PinMask( 1 << num ); }
   inline constexpr uint16_t bitmask() const { return ( 1 << num ); }
   inline constexpr bool valid() const { return (num < 16); }
  private:
   uint8_t num;
};
static_assert( sizeof(PinNum) == sizeof( uint8_t ) );
constexpr PinNum operator""_pin( unsigned long long v ) {
  return PinNum( v );
}

constexpr inline PinMask make_pinMask( PinNum pin_num, uint8_t n ) { return PinMask( make_bit_mask( pin_num.Num(), n ) ) ; }


class PortPin {
  public:
   constexpr PortPin( uint8_t port_num_, PinNum pin_num ) : port_num( port_num_ ), num( pin_num ) {};
   explicit constexpr PortPin( const char *s ); // TODO: "A1" "F15" "HF"?
   inline constexpr uint8_t portNum() const { return port_num; };
   inline constexpr GpioRegs& port() const { return *GPIOs[port_num]; };
   inline constexpr PinNum pinNum() const { return num; };
   inline constexpr PinMask Mask() const { return num.Mask(); }
   inline constexpr uint16_t bitmask() const { return num.Mask().bitmask(); }
   inline constexpr bool valid() const { return (port_num < 16) && num.valid(); } // TODO: number of ports
   inline static constexpr PortPin Bad()  { return PortPin( 0xFF, PinNum(0xFF) ); }
  private:
   uint8_t port_num;
   PinNum  num;
};
static_assert( sizeof(PortPin) == sizeof( uint16_t ) );

// here PA0, PA1 ... PM15 are defined
#define _OXC_PORTPINS_READY_INCLUDE
#include <oxc_portpins.h>
#undef _OXC_PORTPINS_READY_INCLUDE

// ----------------- GPIO registers representation ------------------------------

constexpr inline uint8_t GpioIdx( const GpioRegs &gp )
{
  return (uint8_t)( ( reinterpret_cast<unsigned>(&gp) - GPIOA_BASE ) / ( GPIOB_BASE - GPIOA_BASE ) );
}

class GpioRegs {
  public:
   enum class Moder { in = 0, out = 1, af = 2, analog = 3 };
   enum class Pull {  no = 0,  up = 1, down = 2 };
   enum class ExtiEv { no = 0, up = 1, down = 2, updown = 3 };
   enum class ModeF1 { Analog = 0b0000, InFloat = 0b0100, InPull = 0b1000,
                        OutPP = 0b0011, OutOD   = 0b0111,
                         AFPP = 0b1011,  AFOD   = 0b1111  };

   GpioRegs()  = delete; // init only as ptr/ref to real GPIO area
   ~GpioRegs() = delete;
   constexpr inline uint8_t getIdx() const { return GpioIdx(*this); } // TODO: return PortPin
   void enableClk() const;
   inline void set( PinMask v ) // get given to '1' (OR)
   {
     BSSR = v.bitmask();
   }
   inline void reset( PinMask v ) // AND~
   {
     BSSR = (uint32_t)v.bitmask() << 16;
   }
   inline void sr( PinMask bits, bool doSet )
   {
     if( doSet ) {
       set( bits );
     } else {
       reset( bits );
     }
   }
   inline void toggle( PinMask v ) // XOR
   {
     ODR ^= v.bitmask();
   }

   template <typename F> void for_selected_pins( PinMask pins, F f )
   {
     for( uint16_t pb = 1, pin_num = 0; pb != 0; pb <<= 1, ++pin_num ) {
       if( pins.bitmask() & pb ) {
         f( PinNum(pin_num) );
       }
     }
   }

   #if ! defined (STM32F1)
   inline void cfg_set_MODER( PinNum pin_num, Moder val )
   {
     uint32_t t = MODER;
     t &= ~( 3u  << ( pin_num.Num() * 2 ) ); // TODO: use bitopts
     t |=  ( (uint8_t)(val) << ( pin_num.Num() * 2 ) );
     MODER = t;
   }
   inline void cfg_set_pp( PinNum pin_num )
   {
     OTYPER  &= ~( 1u << pin_num.Num() );
   }

   inline void cfg_set_od( PinNum pin_num )
   {
     OTYPER  |= ( 1u << pin_num.Num() );
   }
   inline void cfg_set_ppod( PinNum pin_num, bool od )
   {
     if( od ) {
       cfg_set_od( pin_num );
     } else {
       cfg_set_pp( pin_num );
     }
   }
   inline void cfg_set_speed_max( PinNum pin_num )
   {
     OSPEEDR |=  ( 3u << ( pin_num.Num() * 2 ) );
   }
   inline void cfg_set_speed_min( PinNum pin_num )
   {
     OSPEEDR &= ~( 3u << ( pin_num.Num() * 2 ) );
   }
   inline void cfg_set_pull( PinNum pin_num, Pull p )
   {
     PUPDR   &= ~( 3u << ( pin_num.Num() * 2 ) );
     PUPDR   |=  ( (uint8_t)p << ( pin_num.Num() * 2 ) );
   }
   inline void cfg_set_pull_no( PinNum pin_num )
   {
     PUPDR   &= ~( 3u << ( pin_num.Num() * 2 ) );
   }
   inline void cfg_set_pull_up( PinNum pin_num )
   {
     PUPDR   &= ~( 3u << ( pin_num.Num() * 2 ) );
     PUPDR   |=  ( 1u << ( pin_num.Num() * 2 ) );
   }
   inline void cfg_set_pull_down( PinNum pin_num )
   {
     PUPDR   &= ~( 3u << ( pin_num.Num() * 2 ) );
     PUPDR   |=  ( 2u << ( pin_num.Num() * 2 ) );
   }
   inline void cfg_set_af0( PinNum pin_num )
   {
     const uint8_t idx = pin_num.Num() >> 3;
     const uint8_t nn  = ( pin_num.Num() & 0x07 ) * 4;
     AFR[idx] &= ~( 0x0F << nn );
   }
   inline void cfg_set_af( PinNum pin_num, uint8_t af )
   {
     const uint8_t idx = pin_num.Num() >> 3;
     const uint8_t nn  = ( pin_num.Num() & 0x07 ) * 4;
     AFR[idx] &= ~( 0x0F << nn );
     AFR[idx] |=  (   af << nn );
   }
   #endif

   void cfgOut_common( PinNum pin_num );
   void cfgOut( PinNum pin_num, bool od = false );
   void cfgOut_N( PinMask pins, bool od = false )
   {
     for_selected_pins( pins, std::bind( &GpioRegs::cfgOut, this, std::placeholders::_1, od ) );
   }

   void cfgAF( PinNum pin_num, uint8_t af, bool od = false );
   void cfgAF_N( PinMask pins, uint8_t af, bool od = false )
   {
     for_selected_pins( pins, std::bind( &GpioRegs::cfgAF, this, std::placeholders::_1, af, od ) );
   }

   void cfgIn( PinNum pin_num, Pull p = Pull::no );
   void cfgIn_N( PinMask pins, Pull p = Pull::no )
   {
     for_selected_pins( pins, std::bind( &GpioRegs::cfgIn, this, std::placeholders::_1, p ) );
   }

   void cfgAnalog( PinNum pin_num );
   void cfgAnalog_N( PinMask pins )
   {
     for_selected_pins( pins, std::bind( &GpioRegs::cfgAnalog, this, std::placeholders::_1 ) );
   }


   void setEXTI( PinNum pin, ExtiEv ev );



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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"

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
#ifdef GPIOL_BASE
inline GpioRegs_ref GpioL = *reinterpret_cast<GpioRegs_ptr_c>(GPIOL_BASE);
#endif
#ifdef GPIOM_BASE
inline GpioRegs_ref GpioM = *reinterpret_cast<GpioRegs_ptr_c>(GPIOM_BASE);
#endif


inline GpioRegs *const  GPIOs[] {
  reinterpret_cast<GpioRegs_ptr_c>(GPIOA_BASE),
  reinterpret_cast<GpioRegs_ptr_c>(GPIOB_BASE),
  reinterpret_cast<GpioRegs_ptr_c>(GPIOC_BASE),
  reinterpret_cast<GpioRegs_ptr_c>(GPIOD_BASE),
  #ifdef GPIOE
     reinterpret_cast<GpioRegs_ptr_c>(GPIOE),
  #endif
  #ifdef GPIOF
     reinterpret_cast<GpioRegs_ptr_c>(GPIOF),
  #endif
  #ifdef GPIOG
     reinterpret_cast<GpioRegs_ptr_c>(GPIOG),
  #endif
  #ifdef GPIOH
     reinterpret_cast<GpioRegs_ptr_c>(GPIOH),
  #endif
  #ifdef GPIOI
     reinterpret_cast<GpioRegs_ptr_c>(GPIOI),
  #endif
  #ifdef GPIOJ
     reinterpret_cast<GpioRegs_ptr_c>(GPIOJ),
  #endif
  #ifdef GPIOK
     reinterpret_cast<GpioRegs_ptr_c>(GPIOK,
  #endif
  #ifdef GPIOL
     reinterpret_cast<GpioRegs_ptr_c>(GPIOL),
  #endif
  #ifdef GPIOM
     reinterpret_cast<GpioRegs_ptr_c>(GPIOM),
  #endif
};
inline constexpr std::size_t GPIOs_n { std::size( GPIOs ) };

constexpr auto Gpios { RefPtr(GPIOs) };


#pragma GCC diagnostic pop



// --------------- Pins ----------------------------------------

void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );
void board_def_btn_init( bool needIRQ );

class Pins
{
  public:
   constexpr Pins( GpioRegs &gi, PinNum a_start, uint8_t a_n )
     : gpio( gi ),
       start( a_start ), n( a_n ),
       mask( make_pinMask( a_start, n ) ),
       maskR( mask.bitmask() << 16 )
     {};
   constexpr Pins( PortPin pp, uint8_t a_n )
     : Pins( pp.port(),  pp.pinNum(), a_n )
     {};
   PinMask getMask() const { return mask; }
   void initHW() { gpio.enableClk(); }
   GpioRegs& dev() { return gpio; }
   constexpr inline uint16_t mv( PinMask v ) const
   {
     return ( ( v.bitmask() << start.Num() ) & mask.bitmask() );
   }
   inline PinMask read() const
   {
     return PinMask( ( gpio.IDR & mask.bitmask() ) >> start.Num() );
   }
  protected:
   GpioRegs &gpio;
   const PinNum start;
   const uint8_t n;
   const PinMask mask;
   const uint32_t maskR;
};

// --------------- PinsOut ----------------------------------------
// PinsOut p( GpioC, 12, 4 ) ; // GPIOD 12:15
// p.write( 0x0C ); p.set( 0x0F ); p.reset( 0x05 ); p.sr( 0xAA, true ); p.toggle( 0x55 );
// p.setbit( 0 ); p.resetbit( 15 ); // even read()
// check:
// p[1] = 1; p |= 0x0F; p ^= 0xAA;  p %= 0x77; // (reset: not &=, ~&= )
// p[1].set(); p[0].reset();
// p = 0x0C; // =write

class PinsOut : public Pins
{
  public:
   constexpr PinsOut( GpioRegs &gi, PinNum a_start, uint8_t a_n )
     : Pins( gi, a_start, a_n )
     {};
   constexpr PinsOut( PortPin pp, uint8_t a_n )
     : PinsOut( pp.port(),  pp.pinNum(), a_n )
     {};
   void initHW();
   inline void write( PinMask v )  // set all bits to given, drop old
   {
     gpio.BSSR = mv( v ) | maskR;
   }
   inline void operator=( PinMask v ) { write( v ); }
   inline void set( PinMask v )   // set given bits to '1' (OR)
   {
     gpio.BSSR = mv( v );
   }
   inline void operator|=( PinMask v ) { set( v ); }
   inline void reset( PinMask v ) // reset given bits to '0' AND~
   {
     gpio.BSSR = mv( v ) << 16;
   }
   inline void operator%=( PinMask v ) { reset( v ); }
   inline void setbit( PinNum i )   // set given (by pos) 1 bit to '1' (OR)
   {
     set( make_pinMask( i, 1 ) );
   }
   inline void resetbit( PinNum i )   // reset given (by pos) 1 bit to '0' (AND~)
   {
     reset( make_pinMask( i, 1 ) );
   }
   inline void sr( PinMask bits, bool doSet ) {
     if( doSet ) {
       set( bits );
     } else {
       reset( bits );
     }
   }
   inline void srbit( PinNum i, bool doSet ) {
     if( doSet ) {
       setbit( i );
     } else {
       resetbit( i );
     }
   }
   inline void toggle( PinMask v ) // XOR
   {
     gpio.ODR ^= mv( v );
   }
   inline void operator^=( PinMask v ) { toggle( v ); }
   inline void togglebit( PinNum i ) // XOR 1 bit
   {
     gpio.ODR ^= mv( make_pinMask( i, 1 ) );
   }
   struct bitpos {
     PinsOut &pins;
     PinNum i;
     inline void operator=( bool b ) { pins.srbit( i, b ); };
     inline void set() { pins.setbit( i ); }
     inline void reset() { pins.resetbit( i ); }
     inline void toggle() { pins.togglebit( i ); }
   };
   bitpos operator[]( PinNum i ) { return { *this, i }; }

  protected:
   // none for now
};

extern PinsOut leds;

// --------------- PinOut ----------------------------------------
// single output pin
// PinOut p1( GpioA, 8 ); // GPIOA 8
// p1.write( true ); p1.set(); p1.reset(); p1.sr( false ); p1.toggle(); p1 = false;
class PinOut
{
  public:
   constexpr PinOut( GpioRegs &gi, PinNum a_start )
     : gpio( gi ),
       start( a_start ),
       mask( make_gpio_mask( start.Num(), 1 ) ),
       maskR( mask.bitmask() << 16 )
     {};
   constexpr PinOut( PortPin pp )
     : PinOut( pp.port(),  pp.pinNum() )
     {};
   void initHW() { gpio.enableClk(); gpio.cfgOut_N( mask );}
   GpioRegs& dev() { return gpio; }
   inline void write( bool doSet )
   {
     sr( doSet );
   }
   inline void set()
   {
     gpio.BSSR = mask.bitmask();
   }
   inline void reset()
   {
     gpio.BSSR = maskR;
   }
   inline void sr( bool doSet ) {
     if( doSet ) {
       set();
     } else {
       reset();
     }
   }
   inline void toggle()
   {
     gpio.ODR ^= mask.bitmask();
   }
   inline bool operator=( bool b ) { sr( b ); return b; }
   inline uint8_t read_in() {
     return ( gpio.IDR & mask.bitmask() ) ? 1 : 0;
   };
   inline uint8_t read_out() {
     return ( gpio.ODR & mask.bitmask() ) ? 1 : 0;
   };
  protected:
   GpioRegs &gpio;
   const PinNum start;
   const PinMask mask;
   const uint32_t maskR;
};

[[ noreturn ]] void die4led( PinMask n );


// --------------- PinsIn ----------------------------------------
// PinsIn pi( GpioB, 12, 4 [, GPIO_{NOPULL,} ] );
class PinsIn : public Pins
{
  public:
   constexpr PinsIn( GpioRegs &gi, PinNum a_start, uint8_t a_n, GpioRegs::Pull a_pull = GpioRegs::Pull::no )
     : Pins( gi, a_start, a_n ),
       pull( a_pull )
     {};
   constexpr PinsIn( PortPin pp, uint8_t a_n, GpioRegs::Pull a_pull = GpioRegs::Pull::no )
     : PinsIn( pp.port(),  pp.pinNum(), a_n, a_pull )
     {};
   void initHW();
  protected:
   const GpioRegs::Pull pull;
};

// --------------- IoPin ?? really pin, not pins? --------------------------------
// IoPin io( GpioE, 0x11 );
// io.sw1(); io.sw0(); io.set_sw0( true ); x = io.rw(); y = io.rw_raw();
class IoPin {
  public:
   constexpr IoPin( GpioRegs &gi, PinMask a_mask )
     : gpio( gi ), mask( a_mask ) {};
   constexpr IoPin( PortPin pp, uint8_t a_n = 1 )
     : IoPin( pp.port(), make_pinMask( pp.pinNum(), a_n ) )
     {};
   void initHW();
   inline void sw1() { gpio.BSSR = mask.bitmask(); };
   inline void sw0() {
     gpio.BSSR = mask.bitmask() << 16;
   };
   void set_sw0( bool s ) { if( s ) sw1(); else sw0(); }
   uint8_t rw() {
     delay_bad_mcs( 1 );
     return rw_raw();
   };
   inline uint8_t rw_raw() {
     return ( gpio.IDR & mask.bitmask() ) ? 1 : 0;
   };

  protected:
   GpioRegs &gpio;
   const PinMask mask;
};

// ------------------ mass EXTI init

struct EXTI_init_info {
  decltype(GpioA) gpio;
  PinNum pinnum; // number, not bit. if > 15 - end;
  decltype(GpioRegs::ExtiEv::updown) dir;
  decltype(EXTI0_IRQn) exti_n;
  uint8_t prty, subprty;
};

unsigned EXTI_inits( const EXTI_init_info *exti_info, bool initIRQ = true );

#endif

// vim: path=.,/usr/share/stm32cube/inc
