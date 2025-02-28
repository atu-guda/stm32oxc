#ifndef _WHEELS_PINS_H
#define _WHEELS_PINS_H

// on-board button
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

// my LEDs - auto init by PinsOut
#define LED0_Pin GPIO_PIN_0
#define LED0_GPIO_Port GpioC
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GpioC
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GpioC
#define LED3_Pin GPIO_PIN_3
#define LED3_GPIO_Port GpioC

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GpioA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GpioA

// T1: 1,2 - motor PWM, 3 - US pulse, 4 - US echo
#define T1_1_M_Right_Pin GPIO_PIN_8
#define T1_1_M_Right_GPIO_Port GpioA
#define T1_2_M_Left_Pin GPIO_PIN_9
#define T1_2_M_Left_GPIO_Port GpioA
#define T1_3_US_Pulse_Pin GPIO_PIN_10
#define T1_3_US_Pulse_GPIO_Port GpioA
#define T1_4_US_Echo_Pin GPIO_PIN_11
#define T1_4_US_Echo_GPIO_Port GpioA
#define T1_ALL_GPIO_Port GpioA

#define TIM_N_L TIM4
#define TIM_N_R TIM3
#define TIM_SERVO TIM14

// TIM3: 1 = A6 = right wheel counter
#define T3_1_M_count_R_Pin GPIO_PIN_6
#define T3_1_M_count_R_GPIO_Port GpioA

// TIM4: 1 = B6 = left wheel counter
#define T4_1_M_count_l_Pin GPIO_PIN_6
#define T4_1_M_count_l_GPIO_Port GpioB

// TIM14: 1 = A7 = servo contorl PIN
#define T14_1_servo_Pin GPIO_PIN_7
#define T14_1_servo_GPIO_Port GpioA

// Motor direction
#define M_dir_r_Pin GPIO_PIN_5
#define M_dir_r_GPIO_Port GpioC
#define M_dir_rn_Pin GPIO_PIN_6
#define M_dir_rn_GPIO_Port GpioC
#define M_dir_aux_Pin GPIO_PIN_7
#define M_dir_aux_GPIO_Port GpioC
#define M_dir_l_Pin GPIO_PIN_8
#define M_dir_l_GPIO_Port GpioC
#define M_dir_ln_Pin GPIO_PIN_9
#define M_dir_ln_GPIO_Port GpioC
enum motor_bits {
  motor_bit_r  = 0x01,
  motor_bit_rn = 0x02,
  motor_bits_r = motor_bit_r | motor_bit_rn,
  // bit 0x04 is reserved
  motor_bit_l  = 0x08,
  motor_bit_ln = 0x10,
  motor_bits_l = motor_bit_l | motor_bit_ln,
  motor_bits   = motor_bits_r | motor_bits_l
};

// IR proxy sensors pins
#define prox_fr_Pin GPIO_PIN_12
#define prox_fr_GPIO_Port GpioB
#define prox_fl_Pin GPIO_PIN_13
#define prox_fl_GPIO_Port GpioB
#define prox_br_Pin GPIO_PIN_14
#define prox_br_GPIO_Port GpioB
#define prox_bl_Pin GPIO_PIN_15
#define prox_bl_GPIO_Port GpioB
enum {
  PROXY_FL = 1, PROXY_FR = 2, PROXY_BR = 4, PROXY_BL = 8,
  PROXY_FA = PROXY_FL | PROXY_FR,
  PROXY_BA = PROXY_BL | PROXY_BR,
  PROXY_ALL = PROXY_FA | PROXY_BA

};

// system
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GpioA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GpioA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GpioB

extern TIM_HandleTypeDef tim1_h, tim3_h, tim4_h, tim14_h;
extern int us_dir_zero; // CCR units

struct RunStepData {
  int l_r, l_l; // in mm
  int p_c;      // central pwm, %, 0 = scan
  int t_max;    // max time, ms;
  int p_c0;     // initial pwm, %, if <0 = get previous?
  int t_0;      // initial ramp time
                // // flags
  void print( OutStream &os ) const;
  void parse_args( int argc, const char * const * argv, int base );
};

inline OutStream& operator<<( OutStream &os, const RunStepData &d ) { d.print( os ); return os; };

struct RunTestElem {
  const RunStepData *d;
  int ne;
};

const int tick_per_turn { 20 };
const int wheel_len { 204 }; // = (D=65mm*pi)

struct RunState {
  int r_w, l_w;
  uint16_t prox;
  void reset() { r_w = l_w = 0; prox = 0; };
};


#endif

