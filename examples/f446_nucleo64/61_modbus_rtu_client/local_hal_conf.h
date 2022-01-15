#ifndef _LOCAL_HAL_CONF_H
#define _LOCAL_HAL_CONF_H

#define UART_MODBUS USART1
#define UART_MODBUS_CLK_ENABLE __HAL_RCC_USART1_CLK_ENABLE
#define UART_MODBUS_GPIO GpioA
#define UART_MODBUS_GPIO_PINS ( BIT9 | BIT10 )
#define UART_MODBUS_GPIO_AF GPIO_AF7_USART1
#define UART_MODBUS_IRQ USART1_IRQn
#define UART_MODBUS_IRQHANDLER USART1_IRQHandler

#define HAL_UART_USERINIT_FUN HAL_UART_UserInit

int MX_MODBUS_UART_Init(void);
// void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle );


#endif

