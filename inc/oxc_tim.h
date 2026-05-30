#ifndef _OXC_TIM_H
#define _OXC_TIM_H

// common timer related functions

#include <span>

#include <oxc_gpio.h>

namespace oxc {

inline constexpr std::size_t tim_max_ccrs { 8 };

// TODO: autogenerate
#if defined(STM32F0) || defined(STM32F1) || defined(STM32F2) || defined(STM32F4) || defined(STM32L0) || defined(STM32L1)
  inline constexpr std::uintptr_t tim_ccr_offsets[tim_max_ccrs]  {
    0x34, 0x38, 0x3C, 0x40,      0,    0,   0, 0
  };

#elif defined(STM32F3) || defined(STM32F7) || defined(STM32L4) || defined(STM32WB) || defined(STM32H7)
  inline constexpr std::uintptr_t tim_ccr_offsets[tim_max_ccrs]  {
    0x34, 0x38, 0x3C, 0x40,   0x58, 0x5C,   0, 0
  };

#elif defined(STM32H5) || defined(STM32G4)
  inline constexpr std::uintptr_t tim_ccr_offsets[tim_max_ccrs]  {
    0x34, 0x38, 0x3C, 0x40,   0x48, 0x4C,   0, 0
  };

#else
  #error "Unknown MCU for timers definitions"
#endif

using tim_ch_type = decltype( TIM_CHANNEL_1 );

struct TimCh {
  enum TimChNum : uint8_t {
     TimChN1 = 0, TimChN2, TimChN3, TimChN4, TimChN5, TimChN6, TimChN7, TimChN8 // 7,8- unused for now
  };
  static constexpr oxc::tim_ch_type hal_ch_tab[] {
    TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4,
    #ifdef TIM_CHANNEL_5
    TIM_CHANNEL_5,
    #endif
    #ifdef TIM_CHANNEL_6
    TIM_CHANNEL_6,
    #endif
  };
  static constexpr std::uintptr_t getCCR_a( std::uintptr_t  tim_addr, TimCh ch )
  {
    return tim_addr + tim_ccr_offsets[ ch.n ];
  }
  static constexpr oxc::tim_ch_type ch2hal_ch( TimCh ch ) { return hal_ch_tab[ ch.n ]; };
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
float get_TIM_base_freq_f(  TIM_TypeDef *tim ); // after ARR
uint32_t calc_TIM_psc_for_cnt_freq(  TIM_TypeDef *tim, uint32_t cnt_freq );
uint32_t calc_TIM_arr_for_base_freq( TIM_TypeDef *tim, uint32_t base_freq );
uint32_t calc_TIM_arr_for_base_psc(  TIM_TypeDef *tim, uint32_t psc, uint32_t base_freq ); // given PSC, not from timer

// returns { psc, arr}, { 0xFFFFFFFF, 0 } - if bad
std::pair<uint32_t,uint32_t> calc_tim_psc_arr( float f_in, float f_out, uint32_t arr_min = 100, uint32_t arr_max = 0xFFFF );
ReturnCode tim_pwm_cfg_default( TIM_HandleTypeDef &t_h, uint32_t psc, uint32_t arr, std::span<const oxc::TimChPin> chpins );

void tim_print_cfg( TIM_TypeDef *tim ); // real if USE_OXC_DEBUG

static const constexpr TIM_OC_InitTypeDef tim_oc_cfg_default_pwm1 {
  .OCMode       = TIM_OCMODE_PWM1,
  .Pulse        = 0,
  .OCPolarity   = TIM_OCPOLARITY_HIGH,
  .OCNPolarity  = TIM_OCNPOLARITY_HIGH,
  .OCFastMode   = TIM_OCFAST_DISABLE,
  .OCIdleState  = TIM_OCIDLESTATE_RESET,
  .OCNIdleState = TIM_OCNIDLESTATE_RESET,
};
static const constexpr TIM_ClockConfigTypeDef sClockSourceConfig_def {
  .ClockSource    = TIM_CLOCKSOURCE_INTERNAL,
  .ClockPolarity  = 0, // ignored
  .ClockPrescaler = 0,
  .ClockFilter    = 0
};
static const constexpr TIM_MasterConfigTypeDef sMasterConfig_def {
  .MasterOutputTrigger  = TIM_TRGO_UPDATE,
  #ifdef TIM_TRGO2_ENABLE
  .MasterOutputTrigger2 = TIM_TRGO2_UPDATE,
  #endif
  .MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE,
};

#endif

