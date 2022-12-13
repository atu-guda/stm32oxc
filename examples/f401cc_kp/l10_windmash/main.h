#ifndef _MAIN_H
#define _MAIN_H

// #include <oxc_gpio.h> from auto

// #define TOWER_GPIO GpioB
inline auto& TOWER_GPIO { GpioB };
inline constexpr uint32_t TOWER_PIN0 { 0 };
inline constexpr uint32_t TOWER_PIN_UP { TOWER_PIN0 };
inline constexpr uint32_t TOWER_PIN1 { TOWER_PIN0 + 1 };
inline constexpr uint32_t TOWER_PIN_CE { TOWER_PIN1 };
inline constexpr uint32_t TOWER_PIN2 { TOWER_PIN0 + 2 };
inline constexpr uint32_t TOWER_PIN_DW { TOWER_PIN2 };
inline constexpr uint32_t TOWER_N    { 3 };
inline constexpr uint32_t TOWER_BIT_UP { 1 << TOWER_PIN0 };
inline constexpr uint32_t TOWER_BIT_CE { TOWER_BIT_UP << 1 };
inline constexpr uint32_t TOWER_BIT_DW { TOWER_BIT_CE << 1 };
inline constexpr uint32_t TOWER_BITS_ALL { TOWER_BIT_UP | TOWER_BIT_CE | TOWER_BIT_DW  };


inline auto&  SWLIM_GPIO { GpioA };
inline constexpr uint32_t  SWLIM_PIN0 { 4 };
inline constexpr uint32_t  SWLIM_N    { 4 };
// last 2 chars: first S - switch, O - opto, L/R - left/right
inline constexpr uint32_t SWLIM_BIT_SR { 1 << SWLIM_PIN0 };
inline constexpr uint32_t SWLIM_BIT_SL { SWLIM_BIT_SR << 1 };
inline constexpr uint32_t SWLIM_BIT_OR { SWLIM_BIT_SL << 1 };
inline constexpr uint32_t SWLIM_BIT_OL { SWLIM_BIT_OR << 1 };
inline constexpr uint32_t SWLIM_BITS_ALL { SWLIM_BIT_SR | SWLIM_BIT_SL | SWLIM_BIT_OR | SWLIM_BIT_OL };
inline constexpr uint32_t SWLIM_BITS_SW  { SWLIM_BIT_SR | SWLIM_BIT_SL };
inline constexpr uint32_t SWLIM_BITS_OP  { SWLIM_BIT_OR | SWLIM_BIT_OL };


inline auto&  DIAG_GPIO { GpioB };
inline constexpr uint32_t  DIAG_PIN0     { 8 };
inline constexpr uint32_t  DIAG_N        { 2 };
inline constexpr uint32_t  DIAG_BIT_ROT  { 1 << DIAG_PIN0 };
inline constexpr uint32_t  DIAG_BIT_MOV  { DIAG_BIT_ROT << 1 };
inline constexpr uint32_t  DIAG_BITS_ALL { DIAG_BIT_ROT | DIAG_BIT_MOV };


inline auto&  USER_START_GPIO { GpioB };
inline constexpr uint32_t  USER_START_PIN0 { 10 };
inline constexpr uint32_t  USER_START_N    {  1 };
inline constexpr uint32_t  USER_START_BIT  {  1 << USER_START_PIN0 };

inline auto&  USER_STOP_GPIO { GpioA };
inline constexpr uint32_t  USER_STOP_PIN0  { 3 };
inline constexpr uint32_t  USER_STOP_N     { 1 };
inline constexpr uint32_t  USER_STOP_BIT   { 1 << USER_STOP_PIN0 };

#define TIM_ROT TIM2
#define TIM_MOV TIM5
#define TIM_ROT_CH TIM_CHANNEL_2
#define TIM_MOV_CH TIM_CHANNEL_3
#define TIM_ROT_EN  __GPIOA_CLK_ENABLE(); __TIM2_CLK_ENABLE();
#define TIM_MOV_EN  __GPIOA_CLK_ENABLE(); __TIM5_CLK_ENABLE();
#define TIM_ROT_DIS __TIM2_CLK_DISABLE();
#define TIM_MOV_DIS __TIM5_CLK_DISABLE();
#define TIM_ROT_IRQn TIM2_IRQn
#define TIM_MOV_IRQn TIM5_IRQn
#define TIM_ROT_IRQ_HANDLER TIM2_IRQHandler
#define TIM_MOV_IRQ_HANDLER TIM5_IRQHandler
#define TIM_ROT_GPIO_PIN GPIO_PIN_1
#define TIM_MOV_GPIO_PIN GPIO_PIN_2
#define TIM_ROT_GPIO_AF GPIO_AF1_TIM2
#define TIM_MOV_GPIO_AF GPIO_AF2_TIM5

inline constexpr uint32_t reg00_def_forv = 0x000001C1;
inline constexpr uint32_t reg00_def_rev  = 0x000001C9;
inline constexpr uint32_t reg10_def      = 0x00041F01;
inline constexpr uint32_t reg6C_def      = 0x15010053; // 8 mstep
inline constexpr uint32_t reg6C_off      = 0x15010050; // 8 mstep + OFF

inline constexpr uint32_t motor_step2turn  = 200;
inline constexpr uint32_t motor_mstep      =   8;

inline constexpr uint32_t TMC2209_R6F_badflags  = 0x00000F3F; // shorts + overtemp

inline constexpr float speed_scale = 1.0e-6f;

enum class BreakNum {
  none = 0,
  cbreak = 1,
  limits = 2,
  tower_top = 3,
  tower_bot = 4,
  drv_flags_rot = 5,
  drv_smin_rot  = 6,
  drv_flags_mov = 7,
  drv_smin_mov  = 8,
  max
};

struct TaskData {
  // input data
  int n_total {   100 }; // total turns
  int d_wire  {   210 }; // wire diameter + extra space in um
  int w_len   { 50000 }; // wirering length (in um)
  // mech
  int v_rot   {   500000 }; // nominal rotation speed, * speed_scale
  int v_mov_o {  2000000 }; // move-only speed
  int w_len_m {    50000 }; // max wirering length (in um)
  int s_rot_m {       40 }; // minimal S stall value for rotation
  int s_mov_m {       20 }; // minimal S stall value for movement
  int      dt {       20 }; // time step durung work in ms
  int check_top {      1 }; // check top sensor in tower (wire break or end)
  int check_bot {      1 }; // check bottom sensor in tower (stall)
  // calculated
  int n_lay   {        0 }; // number of layers
  int n_2lay  {        0 }; // turns per layer
  int v_mov   {        0 }; // speed during main work, in um/sec
  // status
  int n_done  {        0 }; // total done
  int n_ldone {        0 }; // turns done per layer
  int p_ldone {        0 }; // pulses done per layer
  int p_ltask {        0 }; // task in pulses for current run
  int p_move  {        0 }; // number of "move" pulses in current run
  int c_lay   {        0 }; // current layer
  // funcs
  int calc( int n_tot, int d_w, int w_l, bool even );
};

extern TaskData td; // in main.cpp

#endif

