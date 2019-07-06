#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

// TODO: propagate
#ifdef __cplusplus
 extern "C" {
#endif
extern uint32_t SystemCoreClock;
#ifdef __cplusplus
}
#endif

// All hooks must be extern "C"


#define configUSE_PREEMPTION              1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                ( SystemCoreClock )
#define configTICK_RATE_HZ                ( 1000 )
#define configMAX_PRIORITIES              ( 5 )
#define configMINIMAL_STACK_SIZE          ( 256 )
#define configTOTAL_HEAP_SIZE             ( 4 * 1024 )
//#define configTOTAL_HEAP_SIZE             ( 10 * 1024 )
#define configMAX_TASK_NAME_LEN           ( 16 )
#define configUSE_16_BIT_TICKS            0
#define configIDLE_SHOULD_YIELD           1
// def=1
#define configUSE_TASK_NOTIFICATIONS      0
/* Co-routine definitions. */
#define configUSE_CO_ROUTINES             0
#define configMAX_CO_ROUTINE_PRIORITIES   ( 2 )

#define configUSE_MUTEXES                 1
#define configUSE_RECURSIVE_MUTEXES       0
#define configUSE_COUNTING_SEMAPHORES     0
#define configQUEUE_REGISTRY_SIZE         8
#define configUSE_QUEUE_SETS              0
#define configUSE_TIME_SLICING            0
#define configUSE_NEWLIB_REENTRANT        0
#define configENABLE_BACKWARD_COMPATIBILITY  0
// #define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

// void vApplicationIdleHook( void );
#define configUSE_IDLE_HOOK               0
// void vApplicationTickHook( void );
#define configUSE_TICK_HOOK               0
#define configCHECK_FOR_STACK_OVERFLOW    0
#define configUSE_MALLOC_FAILED_HOOK      0

#define configGENERATE_RUN_TIME_STATS     0
#define configUSE_TRACE_FACILITY          0

// #define configUSE_TIMERS                        1
// #define configTIMER_TASK_PRIORITY               3
// #define configTIMER_QUEUE_LENGTH                10
// #define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE



/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    0
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       0
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  0
#define INCLUDE_pcTaskGetTaskName               1
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        0
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_xTimerPendFunctionCall          0

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
  /* __NVIC_PRIO_BITS will be specified when CMSIS is being used. */
  #define configPRIO_BITS                 __NVIC_PRIO_BITS
#else
  #define configPRIO_BITS                 4  /* 15 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY                        0xf

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY        5


/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY                 255
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY         191 /* equivalent to 0xb0, or priority 11. */


/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY        15


/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
/* IMPORTANT: This define MUST be commented when used with STM32Cube firmware,
              to prevent overwriting SysTick_Handler defined within STM32Cube HAL */
/* #define xPortSysTickHandler SysTick_Handler */

#endif /* FREERTOS_CONFIG_H */

