#ifndef _MAIN_H
#define _MAIN_H

// level0: base TB6612FNG + AS5600 C4:C5 - dir+mode
inline constexpr PortPin L0_Ctrl_Pin { PC4  };
inline constexpr PortPin L0_Stop_Pin { PC12 };
// PWM: TIM3.1 C6 AF2
#define TIM_PWM       TIM3
#define TIM_PWM_EN    __HAL_RCC_TIM3_CLK_ENABLE();
#define TIM_PWM_DIS   __HAL_RCC_TIM3_CLK_DISABLE();
#define TIM_PWM_PIN   PC6
#define TIM_PWM_AF    GPIO_AF2_TIM3
#define TIM_PWM_CHANNEL TIM_CHANNEL_1
#define PWM_CCR CCR1
inline constexpr uint32_t tim_pwm_psc_freq   { 42'000'000 }; // 42 MHz - init, TODO: adj after test
inline constexpr uint32_t tim_pwm_freq       {     20'000 }; // 20 kHz - init
extern uint32_t tim_pwm_arr;

extern TIM_HandleTypeDef htim_pwm; // TIM3

int MX_TIM_PWM_Init(void);

// description see at main.cpp
extern uint32_t measure_tick;
extern uint32_t measure_idle_step;
extern int      t_pre;
extern int      t_post;
extern int      t_meas;
extern int      t_step;
extern int      have_magn;
extern int      stopsw;
extern int      l0_freq;
extern int      l0_freq_min;
extern int      l0_freq_max;
extern int      l0_freq_n;
extern int      l0_v_n;
extern int      q0_i;
extern float    q0;
extern float    q0_g;
extern float    q0_0;
extern float    nu0;

void set_l0_freq( uint32_t freq );
void set_l0_pwm( float pwm );
void set_l0_mode( bool i1, bool i2 );
void set_l0_v( float v ); // +-1
float get_l0_v();

bool measure_all();
bool measure_speed( float v );

inline constexpr uint32_t t_lab_max    {    100'000 }; // 100 s

int lab_init( int x ); //* x - first argument (at will), returns 0 - ok, >0 - error, >1 - emerg. stop
int lab_step( uint32_t tc ); //* tc - current time in ms from start, returns:0 - next, 1 - end, > 1 - err + end


#endif

