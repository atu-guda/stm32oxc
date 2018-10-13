#ifndef _MEAS0_H
#define _MEAS0_H

OutStream& operator<<( OutStream &os, float rhs );

struct D_in_sources {
  decltype( GPIOA ) gpio;
  uint16_t           bit;
};

// global in, out, ... vars : part of datas (TODO)

extern float    time_f;
extern int      time_i;

inline const int n_uin = 8;
extern float     uin[n_uin];     // user input float
extern int       nu_uin;         // number of used uin elements

inline const int n_uin_i = 8;
extern int       uin_i[n_uin_i]; // user input int
extern int       nu_uin_i;       // number of used uin_i elements

inline const int n_uout = 8;
extern float     uout[n_uout];     // user output float
extern int       nu_uout;          // number of used uout elements

inline const int n_uout_i = 8;
extern int       uout_i[n_uout_i]; // user output int
extern int        nu_uout_i;        // number of used uout_i elements

extern float vref_in;
extern float vref_out;

inline const int n_adc = 4;
extern float adc[n_adc];
extern int   adc_i[n_adc];
// adc_u16 - exists only for DMA in main

inline const int n_dac    = 2;
extern float dac[n_dac];

inline const int n_din    = 4;
extern D_in_sources d_ins[n_din];
extern int din[n_din];
extern int dins; // combined

inline const int n_pwm    = 4;
extern float pwm[n_pwm]; // output
extern float pwm_f;

inline const int n_din_f = 2; // freq
extern float din_f[n_din_f];

inline const int n_din_dc = 2; // duty cycle
extern float din_dc[n_din_dc];

inline const int n_din_c = 2; // count
extern int   din_c[n_din_c];

inline const int n_lcd = 4;
extern float lcd[n_lcd];

inline const int n_lcd_b = 4; // bits
extern int   lcd_b[n_lcd_b];

inline const int n_tmp = 32;
extern float     tmp[n_tmp];

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;

int MX_BTN_Init();

int MX_TIM1_Init();
int MX_TIM2_Init();
int MX_TIM3_Init();
int MX_TIM4_Init();
int MX_TIM5_Init();
int MX_TIM8_Init();

void pwm_output();

extern DAC_HandleTypeDef hdac;
int  MX_DAC_Init(void);
void dac_output();

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
int  MX_ADC1_Init();
int  MX_DMA_Init(); // used for ADC
int  dma_subinit();

#endif

