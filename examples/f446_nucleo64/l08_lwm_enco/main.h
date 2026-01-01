#ifndef _MAIN_H
#define _MAIN_H

// Encoder: T2.{1.2} A0,A1
#define TIM_CNT       TIM2
#define TIM_CNT_EN    __HAL_RCC_TIM2_CLK_ENABLE();
#define TIM_CNT_DIS   __HAL_RCC_TIM2_CLK_DISABLE();
#define TIM_CNT_PIN0  PA0
#define TIM_CNT_PIN1  PA1
#define TIM_CNT_AF    GPIO_AF1_TIM2

// LWM: TIM3.4 C9
#define TIM_LWM       TIM3
#define TIM_LWM_EN    __HAL_RCC_TIM3_CLK_ENABLE();
#define TIM_LWM_DIS   __HAL_RCC_TIM3_CLK_DISABLE();
#define TIM_LWM_PIN   PC9
#define TIM_LWM_AF    GPIO_AF2_TIM3
#define TIM_LWM_CHANNEL TIM_CHANNEL_4
#define LWM_CCR CCR4
inline constexpr uint32_t tim_lwm_psc_freq   {  2000000 }; //  2 MHz
inline constexpr uint32_t tim_lwm_freq       {       50 }; // 50  Hz
inline constexpr uint32_t tim_lwm_t_us       { 1000000 / tim_lwm_freq };
inline constexpr uint32_t tim_lwm_dt_ns      { 1000000000L / tim_lwm_psc_freq };
extern uint32_t tim_lwm_arr;

extern TIM_HandleTypeDef htim_cnt; // TIM2
extern TIM_HandleTypeDef htim_lwm; // TIM3

int MX_TIM_CNT_Init(void);
int MX_TIM_LWM_Init(void);


#endif

