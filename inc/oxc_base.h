#ifndef _OXC_BASE_H
#define _OXC_BASE_H


#if defined (STM32F1)
  #include "stm32f10x_conf.h"
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define NVIC_FOR_FREERTOS NVIC_SetPriorityGrouping( NVIC_PriorityGroup_4 );
#elif defined (STM32F2)
  #include "stm32f2xx_conf.h"
 #define SET_BIT_REG   BSRRL
 #define RESET_BIT_REG BSRRH
 #define NVIC_FOR_FREERTOS NVIC_SetPriorityGrouping( NVIC_PriorityGroup_4 );
#elif defined (STM32F3)
  #include "stm32f30x_conf.h"
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define NVIC_FOR_FREERTOS NVIC_SetPriorityGrouping( NVIC_PriorityGroup_4 );
#elif defined (STM32F4)
  #include "stm32f4xx_conf.h"
 #define SET_BIT_REG   BSRRL
 #define RESET_BIT_REG BSRRH
 #define NVIC_FOR_FREERTOS NVIC_SetPriorityGrouping( NVIC_PriorityGroup_4 );
#else
  #error "Unsupported MCU"
#endif

#if REQ_MCBASE != MCBASE
  #error "Required and give MCBASE is not equal"
#endif

#define UNUSED __attribute__((unused))


#define PORT_BITS 16
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000


typedef __IO uint32_t reg32;
typedef const char *const ccstr;
#define BAD_ADDR ((void*)(0xFFFFFFFF))

#define ARR_SZ(x) (sizeof(x) / sizeof(x[0]))
#define ARR_AND_SZ(x) x, (sizeof(x) / sizeof(x[0]))


// timings for bad loop delays TODO: for other too
// for 72MHz
#if REQ_SYSCLK_FREQ == 24
  #define T_MKS_MUL    2
  #define T_MS_MUL  2660
  #define T_S_MUL   3420459
#elif REQ_SYSCLK_FREQ == 36
  #define T_MKS_MUL    4
  #define T_MS_MUL  3990
  #define T_S_MUL   5130689
#elif REQ_SYSCLK_FREQ == 48
  #define T_MKS_MUL    5
  #define T_MS_MUL  5319
  #define T_S_MUL   6840918
#elif REQ_SYSCLK_FREQ == 56
  #define T_MKS_MUL    8
  #define T_MS_MUL  7979
  #define T_S_MUL   10261378
#elif REQ_SYSCLK_FREQ == 72
  #define T_MKS_MUL    8
  #define T_MS_MUL  7979
  #define T_S_MUL   10261378
#elif REQ_SYSCLK_FREQ == 168
  #define T_MKS_MUL 32
  #define T_MS_MUL  33845
  #define T_S_MUL   33845000
#endif

#ifdef __cplusplus
template<typename T> class _ShowType; // to output decucted type
                                      // _ShowType< decltype(XXXX) > xType;
 extern "C" {
#endif

// void die4led( uint16_t n );
void taskYieldFun(void);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
// misc functions
void die( uint16_t n );
void delay_ms( uint32_t ms ); // base on vTaskDelay - switch to sheduler
void delay_mcs( uint32_t mcs );
// dumb delay fuctions - loop based - for use w/o timer and for small times
void delay_bad_n( uint32_t n );
void delay_bad_s( uint32_t s );
void delay_bad_ms( uint32_t ms );
void delay_bad_mcs( uint32_t mcs );

// RCC registers for enable devices
enum RCC_Bus { // indexes in RCC_enr
  RCC_Bus0 = 0, RCC_APB1 = 0,
  RCC_Bus1 = 1, RCC_APB2 = 1,
  RCC_Bus2 = 2, RCC_AHB = 2, RCC_AHB1  = 2,
  RCC_Bus3 = 3, RCC_AHB2  = 3,
  RCC_Bus4 = 4, RCC_AHB3  = 4,
  RCC_NBUS = 5
};

enum PinModeNum {
  pinMode_NONE = 0,
  pinMode_AN,
  pinMode_INF,
  pinMode_IPU,
  pinMode_IPD,
  pinMode_Out_PP,
  pinMode_Out_OD,
  pinMode_AF_PP,
  pinMode_AF_OD,
  pinMode_AFIU,
  pinMode_MAX, // size
  // aliases (default, may be other)
  pinMode_TIM_Capt   = pinMode_INF,
  pinMode_TIM_Out    = pinMode_Out_PP,
  pinMode_USART_TX   = pinMode_AF_PP,
  pinMode_USART_RX   = pinMode_AFIU,
  pinMode_SPI_SCK    = pinMode_AF_PP,
  pinMode_SPI_MOSI   = pinMode_AF_PP,
  pinMode_SPI_MISO   = pinMode_INF,
  pinMode_SPI_NSS    = pinMode_INF,
  pinMode_I2C_SCK    = pinMode_AF_OD,
  pinMode_I2C_SDA    = pinMode_AF_OD,
  pinMode_I2C_SMBA   = pinMode_AF_OD
};

#if defined(STM32F1)
#define GPIO_DFL_Speed GPIO_Speed_50MHz

#elif defined(STM32F2)
#define GPIO_DFL_Speed GPIO_Speed_100MHz

#elif defined(STM32F3)
#define  GPIO_DFL_Speed GPIO_Speed_Level_3

#elif defined(STM32F4)
#define  GPIO_DFL_Speed  GPIO_High_Speed
#else
  #error "Unknow MCU type"
#endif
extern GPIO_InitTypeDef GPIO_Modes[pinMode_MAX];


void devPinsConf( GPIO_TypeDef* GPIOx, enum PinModeNum mode_num, uint16_t pins );
/** write some (mask based) bits to port, keep all other */
void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask );

/** return position of first setted bit LSB=0, or FF if 0 */
uint8_t numFirstBit( uint32_t a );

extern const char hex_digits[];
extern const char dec_digits[];

// 64/log_2[10] \approx 20
#define INT_STR_SZ_DEC 24
#define INT_STR_SZ_HEX 20

// converts char to hex represenration (2 digits+EOL, store to s)
char* char2hex( char c, char *s );
// converts uint32 to hex represenration (8(64=16) digits+EOL, store to s)
char* word2hex( uint32_t d,  char *s );
// 64/log_2[10] \approx 20
#define INT_STR_SZ 24
// if s == 0 returns ptr to inner static buffer
char* i2dec( int n, char *s );

#ifdef __cplusplus
}
#endif


#endif

