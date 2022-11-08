#ifndef _MAIN_H
#define _MAIN_H

// #include <oxc_gpio.h> from auto

// #define TOWER_GPIO GpioB
inline auto& TOWER_GPIO { GpioB };
inline constexpr uint32_t TOWER_PIN0 { 0 };
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

inline constexpr uint32_t reg00_def_forv = 0x000001C1;
inline constexpr uint32_t reg00_def_rev  = 0x000001C9;
inline constexpr uint32_t reg10_def      = 0x00031F01;
inline constexpr uint32_t reg6C_def      = 0x15010053; // 8 mstep

inline constexpr uint32_t motor_step2turn  = 200;
inline constexpr uint32_t motor_mstep      =   8;

struct TaskData {
  // visible data
  int n_total {  100 }; // total turns
  int v_rot   {  500 }; // nominal rotation speed, in 1000* turns/sec
  int v_mov_o { 2000 }; // move-only speed, in um/sec
  // calculated data
};

extern TaskData td; // in main.cpp

#endif
