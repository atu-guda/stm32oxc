#ifndef _MAIN_H
#define _MAIN_H

extern int debug;

void init_EXTI();


inline auto& LEDSX_GPIO { GpioB };
inline constexpr uint32_t LEDSX_START { 12 };
inline constexpr uint32_t LEDSX_N { 4 };

inline auto& LWM_GPIO { GpioA };
inline constexpr uint32_t LWM_PIN0 { 0 };
inline constexpr uint32_t LWM_PIN1 { 1 };
inline constexpr uint32_t LWM_PIN2 { 2 };
inline constexpr uint32_t LWM_PIN3 { 3 };

int tim_lwm_cfg();
#define TIM_LWM TIM2
#define TIM_LWM_EN  __GPIOA_CLK_ENABLE(); __TIM2_CLK_ENABLE();
#define TIM_LWM_DIS __TIM2_CLK_DISABLE();
#define TIM_LWM_GPIO_PIN_0 GPIO_PIN_0
#define TIM_LWM_GPIO_PIN_1 GPIO_PIN_1
#define TIM_LWM_GPIO_PIN_2 GPIO_PIN_2
#define TIM_LWM_GPIO_PIN_3 GPIO_PIN_3
#define TIM_LWM_GPIO_AF GPIO_AF1_TIM2
inline constexpr uint32_t tim_lwm_psc_freq   {  2000000 }; // 2 MHz
inline constexpr uint32_t tim_lwm_freq       {       50 }; // 50 Hz
extern uint32_t tim_lwm_arr;
int tim_lwm_cfg();
void tim_lwm_start();
void tim_lwm_stop();


#endif

