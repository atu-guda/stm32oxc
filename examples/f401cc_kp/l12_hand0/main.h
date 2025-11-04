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
#define TIM_LWM_GPIO_PINS ( TIM_LWM_GPIO_PIN_0 | TIM_LWM_GPIO_PIN_1 | TIM_LWM_GPIO_PIN_2 |  TIM_LWM_GPIO_PIN_3 )
#define TIM_LWM_GPIO_AF GPIO_AF1_TIM2
inline constexpr uint32_t tim_lwm_psc_freq   {  2000000 }; // 2 MHz
inline constexpr uint32_t tim_lwm_freq       {       50 }; // 50 Hz
inline constexpr uint32_t tim_lwm_t_us       { 1000000 / tim_lwm_freq };
extern uint32_t tim_lwm_arr;
int tim_lwm_cfg();
void tim_lwm_start();
void tim_lwm_stop();

inline auto& ADC1_GPIO { GpioA };
#define ADC1_PIN0  GPIO_PIN_4
#define ADC1_PIN1  GPIO_PIN_5
#define ADC1_PIN2  GPIO_PIN_6
#define ADC1_PINS  ( ADC1_PIN0 | ADC1_PIN1 | ADC1_PIN2 )
#define ADC_CLK_EN __HAL_RCC_ADC1_CLK_ENABLE();  __HAL_RCC_GPIOA_CLK_ENABLE();
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
void MX_DMA_Init(void);
int  MX_ADC1_Init(void);
void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle );
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle);

#endif

