#ifndef _OXC_TIM_H
#define _OXC_TIM_H

// common timer related functions

#include <oxc_base.h>

struct PwmCh {
  uint16_t idx;
  const decltype(TIM_CHANNEL_1) ch;
  __IO uint32_t &ccr;
  unsigned v;
};

uint32_t get_TIM_in_freq( TIM_TypeDef *tim );   // from bus, before prescaler
uint32_t get_TIM_cnt_freq( TIM_TypeDef *tim );  // after precaler
uint32_t get_TIM_base_freq( TIM_TypeDef *tim ); // after ARR
uint32_t calc_TIM_psc_for_cnt_freq( TIM_TypeDef *tim, uint32_t cnt_freq );
uint32_t calc_TIM_arr_for_base_freq( TIM_TypeDef *tim, uint32_t base_freq );
uint32_t calc_TIM_arr_for_base_psc( TIM_TypeDef *tim, uint32_t psc, uint32_t base_freq ); // given PSC, not from timer

#ifdef USE_OXC_DEBUG
void tim_print_cfg( TIM_TypeDef *tim );
#endif

#endif
