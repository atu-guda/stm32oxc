#ifndef _MAIN_H
#define _MAIN_H

// first pins block: conn1
inline constexpr PortPin AUX2_PIN       { PB10 };
inline constexpr PortPin DIR_PIN        { PB2  };
inline constexpr PortPin PWM_FLOW_PIN   { PA1  }; // T5.2 !!! realy here
inline constexpr PortPin STEP_PIN       { PB0  }; // T3.3

// second pin block: conn2
inline constexpr PortPin ADC1_PIN1      { PA5 };
inline constexpr PortPin ADC1_PIN0      { PA4 };
inline constexpr PortPin STANDBY_PIN    { PA3 };
inline constexpr PortPin MEAS_FREQ_PIN  { PA2 }; // T2.3 TIM_FREQ_MEAS
inline constexpr PortPin AUX1_PIN       { PA0 };


// aux pin block: conn7
// extra leds
inline constexpr PortPin  LEDSX_START      { PB12 };
inline constexpr uint32_t LEDSX_N          {    4 };
inline constexpr PortPin  BTN_USER_PIN     {  PA8 };
// user button
inline constexpr uint32_t BTN_USER_BIT      { BTN_USER_PIN.pinNum().bitmask() };
inline constexpr uint16_t BTN_USER_IRQ_PRTY { 12 };
inline constexpr auto     BTN_USER_IRQ_N    { EXTI9_5_IRQn };
inline constexpr auto     BTN_USER_EXTI_DIR { ExtiEv::down };


inline constexpr std::size_t adc_n_ch { 2 };
extern int32_t  adc_data[adc_n_ch];  // collected and divided data (by adc_measure)
extern uint16_t adc_buf[adc_n_ch];   // buffer for DMA
#define ADC_CLK_EN __HAL_RCC_ADC1_CLK_ENABLE();
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
void MX_DMA_Init(void);
int  MX_ADC1_Init(void);
void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle );
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle);

// timers
extern TIM_HandleTypeDef tim_freq_meas_h;
#define TIM_FREQ_MEAS TIM2
#define TIM_FREQ_MEAS_EN  __GPIOA_CLK_ENABLE(); __TIM2_CLK_ENABLE();
#define TIM_FREQ_MEAS_DIS __TIM2_CLK_DISABLE();
#define TIM_FREQ_MEAS_GPIO_AF GPIO_AF1_TIM2
#define TIM_FREQ_MEAS_CHANNEL TIM_CHANNEL_3
int tim_freq_meas_cfg();
inline constexpr uint32_t tim_freq_meas_psc   { 0 }; // 72 MHz = max
//
extern TIM_HandleTypeDef tim_step_h;
#define TIM_STEP TIM3
#define TIM_STEP_EN  __GPIOB_CLK_ENABLE(); __TIM3_CLK_ENABLE();
#define TIM_STEP_DIS __TIM3_CLK_DISABLE();
#define TIM_STEP_GPIO_AF GPIO_AF1_TIM3
#define TIM_STEP_CHANNEL TIM_CHANNEL_3
int tim_step_cfg();
inline constexpr uint32_t tim_step_psc   {     0 };
inline constexpr uint32_t tim_step_arr   { 36000 }; // to 2 kHz
//
extern TIM_HandleTypeDef tim_flow_pwm_h;
#define TIM_FLOW_PWM TIM5
#define TIM_FLOW_PWM_EN  __GPIOA_CLK_ENABLE(); __TIM5_CLK_ENABLE();
#define TIM_FLOW_PWM_DIS __TIM5_CLK_DISABLE();
#define TIM_FLOW_PWM_GPIO_AF GPIO_AF1_TIM5
#define TIM_FLOW_PWM_CHANNEL TIM_CHANNEL_2
int tim_flow_cfg();
inline constexpr uint32_t tim_flow_psc   {     0 };
inline constexpr uint32_t tim_flow_arr   {  3600 }; // to 20 kHz

// funcs

void init_EXTI();
int measure_adc( int nx );
int measure_press();
bool default_loop( bool can_stop = false );
void default_out( int i );

inline const std::size_t buf_sz_i2c { 32 }; // 20 char + reserve
inline const std::size_t buf_n_i2c  {  4 }; // 4 lines
extern char buf_i2c[buf_n_i2c][buf_sz_i2c];
inline auto& buf_i2c_0 { buf_i2c[0] };
inline auto& buf_i2c_1 { buf_i2c[1] };
inline auto& buf_i2c_2 { buf_i2c[2] };
inline auto& buf_i2c_3 { buf_i2c[3] };

extern uint32_t f_in;
extern uint32_t pressure;
extern uint32_t t_00;
extern uint32_t t_c;
extern uint32_t t_step;

#endif

