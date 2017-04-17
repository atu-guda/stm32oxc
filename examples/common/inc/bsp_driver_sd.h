// bsp_driver_sd.h
#ifndef _BSP_DRIVER_SD_H
#define _BSP_DRIVER_SD_H

#ifdef __cplusplus
extern "C" {
#endif

// #include <stm32f4xx_hal.h>
#include <oxc_base.h>

#ifndef BSP_SD_CardInfo
  #define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef
#endif

#define   MSD_OK                        ((uint8_t)0x00)
#define   MSD_ERROR                     ((uint8_t)0x01)
#define   SD_TRANSFER_OK                ((uint8_t)0x00)
#define   SD_TRANSFER_BUSY              ((uint8_t)0x01)
#define SD_PRESENT                      ((uint8_t)0x01)
#define SD_NOT_PRESENT                  ((uint8_t)0x00)
#define SD_DATATIMEOUT                  ((uint32_t)100000000)


uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_ITConfig(void);
void    BSP_SD_DetectIT(void);
void    BSP_SD_DetectCallback(void);
uint8_t BSP_SD_ReadBlocks(      uint32_t *pData, uint32_t  ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout );
uint8_t BSP_SD_WriteBlocks(     uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout );
uint8_t BSP_SD_ReadBlocks_DMA(  uint32_t *pData, uint32_t  ReadAddr, uint32_t NumOfBlocks );
uint8_t BSP_SD_WriteBlocks_DMA( uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks );
uint8_t BSP_SD_Erase( uint32_t StartAddr, uint32_t EndAddr );
void    BSP_SD_IRQHandler(void);
void    BSP_SD_DMA_Tx_IRQHandler(void);
void    BSP_SD_DMA_Rx_IRQHandler(void);
uint8_t BSP_SD_GetCardState(void);
void    BSP_SD_GetCardInfo( HAL_SD_CardInfoTypeDef *CardInfo );
uint8_t BSP_SD_IsDetected(void);

#ifdef __cplusplus
}
#endif

#endif // _BSP_DRIVER_SD_H

