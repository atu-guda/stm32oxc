#ifndef _OXC_PWMCTLTIM_H
#define _OXC_PWMCTLTIM_H

#include <array>
#include <ranges>

#include <oxc_tim.h>
#include <oxc_pwmctl.h>

namespace oxc {

//* PWM controller based on STM32 timers with pwm capability
class PwmCtlTim : public PwmCtl {
  public:
   static constexpr std::size_t max_ch { 8 }; // really 6, but what if?
   constexpr PwmCtlTim( std::uintptr_t tim_addr_, std::span<const TimChPin> channels_, TIM_HandleTypeDef &t_h_ )
     : PwmCtl( channels_.size() ),
       tim_addr( tim_addr_ ),
       channels( channels_ ),
       t_h( t_h_ )
       {
         for( auto [i,ch] : std::views::enumerate( channels ) ) {
           ccrs_a[i] = TimCh::getCCR_a( tim_addr, ch.ch );
         }
       };
   virtual ReturnCode initHW() override;
   virtual bool setFreq( float freq ) override;
   virtual float getFreq() const override;
   virtual bool setPwmU16( std::size_t ch, uint16_t pwm ) override;
   virtual bool setPwm(    std::size_t ch, float pwm )    override;
   virtual bool setPulse(  std::size_t ch, uint32_t us )  override;
   virtual bool setPwmRaw( std::size_t ch, uint32_t r )   override;
   virtual uint32_t pwm2raw( float pwm ) override;
   virtual uint32_t pulse2raw( uint32_t us ) override;

   TIM_TypeDef* tim_p() const { return reinterpret_cast<TIM_TypeDef*>( tim_addr ); };
   bool isBadCh( std::size_t ch ) const { return ( ch >= n_ch ) || ( ccrs_a[ch] == 0 )|| ( ccrs_a[ch] == tim_addr ); }
   uint32_t getPwmRaw(  std::size_t ch ) const;

   std::size_t initPins(); // called from initHW
   ReturnCode setHardParams( uint32_t psc, uint32_t arr, uint32_t cmode = TIM_COUNTERMODE_UP );
   inline reg32* pccr( std::size_t ch ) const { return reinterpret_cast<reg32*>(ccrs_a[ch]); };
   void enable()  { tim_p()->CR1 |=  1u; };
   void disable() { tim_p()->CR1 &= ~1u; };
   bool isEnabled() const { return (bool)(tim_p()->CR1 & 1u); };
   void setAllowPSCadj( bool allow ) { allowPSCadj = allow; };
   void setArrMax( uint32_t arr_m ) { arr_max = arr_m; };

   // debug:
   // auto getCCR( std::size_t ch ) const { return ( ch < n_ch ) ? ccrs_a[ch] : 0 ; };
  protected:
   std::uintptr_t tim_addr;
   std::span<const TimChPin> channels;
   std::array<std::uintptr_t, max_ch> ccrs_a { 0 };
   uint32_t arr_max { 10 };
   uint32_t fake_ccr { 0 };
   bool allowPSCadj { false };
   // for init/cache
   TIM_HandleTypeDef &t_h;
   uint32_t psc { 0 };
   uint32_t arr { 1 };
   uint32_t cmode { TIM_COUNTERMODE_UP };

};

}; // namespace oxc


#endif

