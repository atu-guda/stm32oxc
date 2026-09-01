#ifndef _OXC_TIM_PWM_D_H
#define _OXC_TIM_PWM_D_H

#include <array>
#include <ranges>

#include <oxc_tim.h>
#include <oxc_capabilities.h>

using std::span;
using std::array;

namespace oxc {

class  Tim_Pwm_Dev : public PwmPureCapability {
  public:
   static constexpr size_t max_ch { 8 }; // really 6, but what if?
   constexpr Tim_Pwm_Dev( uintptr_t tim_addr_, span<const TimChPin> channels_, TIM_HandleTypeDef &t_h_ ) noexcept
     : tim_addr( tim_addr_ ),
       channels( channels_ ),
       n_ch( channels_.size() ),
       t_h( t_h_ )
       {
         for( auto&& [ccr,ch] : std::views::zip( ccrs_a, channels ) ) {
           ccr = TimCh::getCCR_a( tim_addr, ch.ch );
         }
       };

   virtual uint32_t duty2raw(  float duty ) const noexcept override;
   virtual uint32_t pulse2raw( float pu_s ) const noexcept override;
   virtual uint32_t shift2raw( float pu_s ) const noexcept override;
   virtual ReturnCode freq2cfgs( float freq, std::span<uint32_t> cfgs ) const noexcept override;
   virtual float_er  cfg2freq( std::span<const uint32_t> cfgs ) const noexcept override;
   // low-level interface
   virtual ReturnCode setDutyRaw(  size_t ch, int32_t dr ) noexcept override;
   virtual ReturnCode setShiftRaw( size_t ch, int32_t sr ) noexcept override;
   virtual int32_t_er getDutyRaw(  size_t ch )  noexcept override;
   virtual int32_t_er getShiftRaw( size_t ch )  noexcept override;
   virtual ReturnCode applyCfg( std::span<const uint32_t> cfgs )  noexcept override;
   virtual ReturnCode storeCfg( std::span<      uint32_t> cfgs ) const noexcept override;

   ReturnCode readCfg() noexcept;

   TIM_TypeDef* tim_p() const { return reinterpret_cast<TIM_TypeDef*>( tim_addr ); };
   bool isBadCh( size_t ch ) const { return ( ch >= n_ch ) || ( ccrs_a[ch] == 0 )|| ( ccrs_a[ch] == tim_addr ); }

   ReturnCode initHW() ;
   void initPins();
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
   uint32_t cfgv[4]; // 0 - ARR, 1 - PSC, 2 - freq_cnt, 3 - freq_base
   uint32_t &arr       { cfgv[0] }; // aliases
   uint32_t &psc       { cfgv[1] };
   uint32_t &freq_cnt  { cfgv[2] }; // BAD, need float, only [0] [1] - bin param
   uint32_t &freq_base { cfgv[3] };
   uint32_t cmode { TIM_COUNTERMODE_UP };

};

}; // namespace oxc


#endif

