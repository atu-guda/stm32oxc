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

class Sensor {
  public:
   explicit Sensor( unsigned n_ch_ ) : n_ch( n_ch_ ) {};
   virtual ~Sensor() = default;
   unsigned getNch() const { return n_ch; }
   virtual int measure( int nx ) = 0;
   virtual int init() = 0;
   virtual uint32_t getUint( unsigned ch ) = 0;
   virtual float get( unsigned ch ) = 0;
  protected:
   unsigned n_ch;
};

class SensorAdc : public Sensor {
  public:
   explicit SensorAdc( unsigned n_ch_ ) : Sensor( std::min( n_ch_, max_n_ch ) ) {};
   virtual ~SensorAdc() = default;
   virtual int measure( int nx ) override;
   virtual int init() override;
   virtual uint32_t getUint( unsigned ch ) override { return ( ch < n_ch ) ? adc_data[ch] : 0; }
   virtual float get( unsigned ch ) override { return getUint(ch) * k_a[ch] + k_b[ch]; }
  protected:
   static const unsigned max_n_ch { 4 };
   uint32_t adc_data[max_n_ch];  // collected and divided data (by adc_measure)
   uint16_t adc_buf[max_n_ch];   //
   float k_a[max_n_ch] { 1.0f, 1.0f, 1.0f, 1.0f };
   float k_b[max_n_ch] { 0.0f, 0.0f, 0.0f, 0.0f };
};

class SensorAS5600 : public Sensor {
  public:
   SensorAS5600( AS5600 &dev_) : Sensor( 1 ), dev( dev_ ) {};
   virtual ~SensorAS5600() = default;
   virtual int measure( int nx ) override;
   virtual int init() override;
   virtual uint32_t getUint( unsigned /* ch */ ) override { return iv; }
   virtual float get( unsigned /* ch */ ) override { return v; };
   void set_zero_val( int zv ) { zero_val = zv; }
  protected:
   AS5600 &dev;
   float v;
   int zero_val { 0 };
   uint16_t iv;
};



#endif

