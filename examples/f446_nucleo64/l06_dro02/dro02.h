#ifndef _DRO02_H
#define _DRO02_H


extern TIM_HandleTypeDef htim2;
extern volatile uint32_t tim2_catch;
int MX_TIM2_Init();

extern UART_HandleTypeDef huart1;
int MX_USART1_UART_Init();
int  MX_FC_UART_Init();

#endif

