#include "stm32f1xx_hal.h"

extern void Error_Handler(void);


void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );

  /* HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0); */
  /* HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0); */
  /* HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0); */
  /* HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0); */
  /* HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0); */
  /* HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0); */
  /* HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0); */

  /**NOJTAG: JTAG-DP Disabled and SW-DP Enabled  */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

}

