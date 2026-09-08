#ifndef _OXC_TIM_PWM_D_H
#define _OXC_TIM_PWM_D_H

#include <array>
#include <ranges>

#include <oxc_tim.h>
#include <oxc_purecaps.h>

using std::span;
using std::array;

namespace oxc {

class  Tim_Pwm_Dev : public PwmBaseBasic {
  public:
   static constexpr size_t max_ch { 6 }; // but what if?
   constexpr Tim_Pwm_Dev( uintptr_t tim_addr_, span<const TimChPin> channels_,
       TIM_HandleTypeDef &t_h_,              // TODO: use own config
       uint32_t arr_max_ = 0xFFFF ) noexcept // TODO: timer traits
     : tim_addr( tim_addr_ ),
       channels( channels_ ),
       n_ch( channels_.size() ),
       arr_max( arr_max_ ), // TODO: timer traits
       t_h( t_h_ )
       {
         for( auto&& [ccr,ch] : std::views::zip( ccrs_a, channels ) ) {
           ccr = TimCh::getCCR_a( tim_addr, ch.ch );
         }
       };

   virtual ReturnCode init() noexcept override { return readCfg(); } //? TODO: more?
   void enable()  noexcept override { tim_p()->CR1 |=  1u; };
   void disable() noexcept override { tim_p()->CR1 &= ~1u; };
   bool isEnabled() const noexcept override { return (bool)(tim_p()->CR1 & 1u); };

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
   bool isBadCh( size_t ch ) const { return ( ch >= n_ch ) || ( ccrs_a[ch] == 0 )|| ( ccrs_a[ch] == SAFE_SINK_BASE ); }

   ReturnCode initHW();
   void initPins();
   inline reg32* pccr( std::size_t ch ) const { return reinterpret_cast<reg32*>(ccrs_a[ch]); };
   void setAllowPSCadj( bool allow ) { allowPSCadj = allow; };
   void setArrMin( uint32_t arr_min_ ) { arr_min = arr_min_; };

   uint32_t getFreqIn()   const noexcept { return freq_in;   }
   float    getFreqCnt()  const noexcept { return freq_cnt;  }
   float    getFreqBase() const noexcept { return freq_base; }

   // debug:
   // auto getCCR( std::size_t ch ) const { return ( ch < n_ch ) ? ccrs_a[ch] : 0 ; };
  protected:
   const uintptr_t tim_addr;
   span<const TimChPin> channels;
   const size_t n_ch; // cached channels.size()
   array<uintptr_t, max_ch> ccrs_a { SAFE_SINK_BASE };
   uint32_t arr_min { 50 };
   uint32_t arr_max;
   uint32_t fake_ccr { 0 }; // TODO: how to use it?
   bool allowPSCadj { true };
   // for init/cache
   TIM_HandleTypeDef &t_h; // TODO: remove, use own init
   uint32_t cfgv[2]; // 0 - ARR, 1 - PSC
   uint32_t &arr       { cfgv[0] }; // aliases
   uint32_t &psc       { cfgv[1] };
   uint32_t freq_in   { 0 };
   float    freq_cnt  { 0 };
   float    freq_base { 0 };
   uint32_t cmode { TIM_COUNTERMODE_UP };

};

}; // namespace oxc


#endif

