#ifndef _OXC_PWMCTLTIM_H
#define _OXC_PWMCTLTIM_H

#include <span>
#include <array>
#include <ranges>

#include <oxc_tim.h>
#include <oxc_pwmctl.h>

namespace oxc {

//* PWM controller based on STM32 timers with pwm capability
class PwmCtlTim : public PwmCtl {
  public:
   static constexpr std::size_t max_ch { 8 }; // really 6, but wat if?
   constexpr PwmCtlTim( TIM_TypeDef *tim_, std::span<const TimChPin> channels_ ) // not constexpr ;-(
     : PwmCtl( channels_.size() ),
       tim( tim_ ),
       channels( channels_ )
       {
         for( auto [chp,pccr] : std::views::zip( channels, ccrs ) ) {
            pccr = TimCh::getCCR( tim, chp.ch );
         }
       };
   virtual bool setFreq( uint32_t freq_ ) override;
   virtual uint32_t getFreq() const override;
   virtual bool setPwmU16( std::size_t ch, uint16_t pwm ) override;
   virtual bool setPwm(    std::size_t ch, float pwm )    override;

   std::size_t initPins();
   void enable()  { tim->CR1 |=  1u; };
   void disable() { tim->CR1 &= ~1u; };
   bool isEnabled() const { return (bool)(tim->CR1 & 1u); };
   void setAllowPSKadj( bool allow ) { allowPSKadj = allow; };

   // debug:
   reg32* getCCR( std::size_t ch ) const { return ( ch < n_ch ) ? ccrs[ch] : nullptr ; };
  protected:
   TIM_TypeDef *tim;
   std::span<const TimChPin> channels;
   std::array<reg32*,max_ch> ccrs { nullptr };
   bool allowPSKadj { false };

};

}; // namespace oxc


#endif

