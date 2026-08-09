#ifndef _OXC_TIM_PWM_D_H
#define _OXC_TIM_PWM_D_H

#include <array>
#include <ranges>

#include <oxc_tim.h>
#include <oxc_capabilities.h>

using std::span;
using std::array;

namespace oxc {

class  Tim_Pwm_Dev : public PwmCapability {
  public:
   static constexpr size_t max_ch { 8 }; // really 6, but what if?
   constexpr Tim_Pwm_Dev( uintptr_t tim_addr_, span<const TimChPin> channels_, size_t bitsz_, TIM_HandleTypeDef &t_h_,
        const ValFiTrans1xN &tr_f_, const ValFiTrans1x1 &tr_d_ = globalUnityValFiTrans
       ) noexcept
     : PwmCapability( channels_.size(), bitsz_, tr_f_, tr_d_ ),
       tim_addr( tim_addr_ ),
       channels( channels_ ),
       n_ch( channels_.size() ),
       t_h( t_h_ )
       {
         for( auto [i,ch] : std::views::enumerate( channels ) ) {
           ccrs_a[i] = TimCh::getCCR_a( tim_addr, ch.ch );
         }
       };

   // PwmPureCapability:
   virtual ReturnCode setDuty( size_t ch, float duty ) noexcept ;
   virtual ReturnCode setFreq( float freq )            noexcept ;
   virtual float getFreq() const                       noexcept ;

   // virtual uint32_t pwm2raw( float pwm ) override;
   // virtual uint32_t pulse2raw( uint32_t us ) override;

   TIM_TypeDef* tim_p() const { return reinterpret_cast<TIM_TypeDef*>( tim_addr ); };
   bool isBadCh( size_t ch ) const { return ( ch >= n_ch ) || ( ccrs_a[ch] == 0 )|| ( ccrs_a[ch] == tim_addr ); }

   ReturnCode initHW() ;
   void initPins();
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
   const uintptr_t tim_addr;
   span<const TimChPin> channels;
   const size_t n_ch; // cached channels.size()
   array<uintptr_t, max_ch> ccrs_a { 0 };
   uint32_t arr_max { 10 };
   uint32_t fake_ccr { 0 };
   bool allowPSCadj { true };
   // for init/cache
   TIM_HandleTypeDef &t_h;
   uint32_t psc { 0 };
   uint32_t arr { 1 };
   uint32_t cmode { TIM_COUNTERMODE_UP };

};

}; // namespace oxc


#endif

