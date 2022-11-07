#ifndef _MAIN_H
#define _MAIN_H

#define TOWER_GPIO GpioB
#define TOWER_PIN0     0
#define TOWER_N        3

#define TOWER_BIT_UP   BIT0
#define TOWER_BIT_CE   BIT1
#define TOWER_BIT_DW   BIT2


#define SWLIM_GPIO GpioA
#define SWLIM_PIN0     4
#define SWLIM_N        4

#define SWLIM_BIT_LR   BIT4
#define SWLIM_BIT_LL   BIT5
#define SWLIM_BIT_OR   BIT6
#define SWLIM_BIT_OL   BIT7


#define DIAG_GPIO GpioB
#define DIAG_PIN0     8
#define DIAG_N        2

#define DIAG_BIT_ROT   BIT8
#define DIAG_BIT_MOV   BIT9


#define USER_START_GPIO GpioB
#define USER_START_PIN0    10
#define USER_START_N        1

#define USER_START_BIT   BIT10

#define USER_STOP_GPIO GpioA
#define USER_STOP_PIN0     3
#define USER_STOP_N        1

#define USER_STOP_BIT   BIT3

#endif

