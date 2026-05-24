#ifndef _OXC_TIM_H
#define _OXC_TIM_H

// common timer related functions

#include <oxc_gpio.h>

namespace oxc {

using tim_ch_type = decltype( TIM_CHANNEL_1 );

struct TimCh {
  enum TimChNum : uint8_t {
     TimChN1 = 0, TimChN2, TimChN3, TimChN4, TimChN5, TimChN6, TimChN7, TimChN8 // 7,8- unused for now
  };
  static constexpr reg32* getCCR( TIM_TypeDef *tim, TimCh ch ) // really not constexpr as int->ptr is reinterpret_cast
  {
    switch( ch.n ) {
      case TimChN1: return &(tim->CCR1);
      case TimChN2: return &(tim->CCR2);
      case TimChN3: return &(tim->CCR3);
      case TimChN4: return &(tim->CCR4);
      #ifdef TIM_CCMR3_OC5FE_Pos
      case TimChN5: return &(tim->CCR5);
      case TimChN6: return &(tim->CCR6);
      #endif
      #ifdef TIM_CCMR4_OC7FE_Pos
      case TimChN7: return &(tim->CCR7);
      case TimChN8: return &(tim->CCR8);
      #endif
      default: return &fake_ccr;
    }
  }
  static bool isFakeCCR( const reg32 *ccr ) { return ( ccr == &fake_ccr ) || ( ccr == nullptr ); }
  static reg32 fake_ccr;
  TimChNum n;
};

constexpr inline TimCh TimCh1 { TimCh::TimChN1 };
constexpr inline TimCh TimCh2 { TimCh::TimChN2 };
constexpr inline TimCh TimCh3 { TimCh::TimChN3 };
constexpr inline TimCh TimCh4 { TimCh::TimChN4 };
constexpr inline TimCh TimCh5 { TimCh::TimChN5 };
constexpr inline TimCh TimCh6 { TimCh::TimChN6 };
constexpr inline TimCh TimCh7 { TimCh::TimChN7 };
constexpr inline TimCh TimCh8 { TimCh::TimChN8 };

struct TimChPin {
  TimCh ch;
  uint8_t af;
  PortPin pin;
};
static_assert( sizeof(TimChPin) == 4 );

}; // namespace oxc

struct PwmCh {
  uint16_t idx;
  const oxc::tim_ch_type ch;
  __IO uint32_t &ccr;
  unsigned v;
};

uint32_t get_TIM_in_freq( TIM_TypeDef *tim );   // from bus, before prescaler
inline uint32_t get_TIM_cnt_freq( TIM_TypeDef *tim )  // after precaler
{
  return get_TIM_in_freq( tim ) / (1 + tim->PSC);
}
uint32_t get_TIM_base_freq( TIM_TypeDef *tim ); // after ARR
float get_TIM_base_freq_f( TIM_TypeDef *tim ); // after ARR
uint32_t calc_TIM_psc_for_cnt_freq( TIM_TypeDef *tim, uint32_t cnt_freq );
uint32_t calc_TIM_arr_for_base_freq( TIM_TypeDef *tim, uint32_t base_freq );
uint32_t calc_TIM_arr_for_base_psc( TIM_TypeDef *tim, uint32_t psc, uint32_t base_freq ); // given PSC, not from timer

// returns { psc, arr}, { 0xFFFFFFFF, 0 } - if bad
std::pair<uint32_t,uint32_t> calc_tim_psc_arr( float f_in, float f_out, uint32_t arr_min = 100, uint32_t arr_max = 0xFFFF );

void tim_print_cfg( TIM_TypeDef *tim ); // real if USE_OXC_DEBUG

#endif
