#ifndef _OXC_TIM_H
#define _OXC_TIM_H

// common timer related functions

#include <oxc_base.h>

uint32_t get_TIM_in_freq( TIM_TypeDef *tim ); // TODO: to lib
uint32_t get_TIM_cnt_freq( TIM_TypeDef *tim );
uint32_t calc_TIM_psc_for_cnt_freq( TIM_TypeDef *tim, uint32_t cnt_freq );
uint32_t get_TIM_base_freq( TIM_TypeDef *tim );

#endif
