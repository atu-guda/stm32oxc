#ifndef _OXC_PWMCTL_H
#define _OXC_PWMCTL_H

#include <ranges>

#include <oxc_types.h>

namespace oxc {

//* abstraction for real PWM controllers
class PwmCtl {
  public:
   constexpr explicit PwmCtl( std::size_t n_ch_ ) : n_ch( n_ch_ ) {};
   constexpr std::size_t size() const     { return n_ch; };
   virtual ReturnCode initHW() = 0;
   virtual bool setFreq( float freq ) { return false; };
   virtual float getFreq() const { return 0; };
   virtual bool setPwmU16( std::size_t ch, uint16_t pwm ) = 0; //* pwm: 0: 65535
   virtual bool setPwm(    std::size_t ch, float pwm )    = 0; //* pwm: 0: 1
   virtual bool setPulse(  std::size_t ch, uint32_t us )  = 0; //*
   virtual bool setPwmRaw( std::size_t ch, uint32_t r )   = 0; //* to allow commit() from Robo
   virtual uint32_t pwm2raw( float pwm )                  = 0; //* ==
   virtual uint32_t pulse2raw( uint32_t us )              = 0; //* ==
   bool setPwms( std::span<float> pwms )
   {
     for( auto [ch,v] : std::views::enumerate( pwms )  )
     {
       if( (size_t)ch >= size() ) {
         break;
       }
       if( !setPwm( ch, v ) ) {
         return false;
       }
     }
     return true;
   };
   bool setPwmsRaw( std::span<uint32_t> pwmsr )
   {
     for( auto [ch,v] : std::views::enumerate( pwmsr )  )
     {
       if( (size_t)ch >= size() ) {
         break;
       }
       if( !setPwmRaw( ch, v ) ) {
         return false;
       }
     }
     return true;
   };
  protected:
   std::size_t n_ch;
};

}; // namespace oxc


#endif

