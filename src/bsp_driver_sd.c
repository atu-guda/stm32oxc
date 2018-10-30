#include <oxc_base.h>
#include <bsp_driver_sd.h>

extern SD_HandleTypeDef hsd;

uint8_t BSP_SD_Init(void)
{
  uint8_t sd_state = MSD_OK;
  if( BSP_SD_IsDetected() != SD_PRESENT )  {
    return MSD_ERROR;
  }
  sd_state = HAL_SD_Init(&hsd);

/* #ifdef BUS_4BITS */
/*   #<{(| Configure SD Bus width |)}># */
/*   if (sd_state == MSD_OK) */
/*   { */
/*     #<{(| Enable wide operation |)}># */
/*     if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK) */
/*     { */
/*       sd_state = MSD_ERROR; */
/*     } */
/*   } */
/* #endif */

  return sd_state;
}

uint8_t BSP_SD_ITConfig(void)
{
  return (uint8_t)0;
}

void BSP_SD_DetectIT(void)
{
}

__weak void BSP_SD_DetectCallback(void)
{
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read
  * @param  Timeout: Timeout for read operation
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks( uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout )
{
  uint8_t sd_state = MSD_OK;
  if( HAL_SD_ReadBlocks(&hsd, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK )   {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  uint8_t sd_state = MSD_OK;

  if( HAL_SD_WriteBlocks( &hsd, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK )   {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
  uint8_t sd_state = MSD_OK;

  if( HAL_SD_ReadBlocks_DMA(&hsd, (uint8_t *)pData, ReadAddr, NumOfBlocks) != HAL_OK )  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks )
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t *)pData, WriteAddr, NumOfBlocks) != HAL_OK ) {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_Erase(&hsd, StartAddr, EndAddr) != HAL_OK)  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

void BSP_SD_IRQHandler(void)
{
  HAL_SD_IRQHandler(&hsd);
}

/** @brief  Handles SD DMA Tx transfer interrupt request.  */
void BSP_SD_DMA_Tx_IRQHandler(void)
{
  HAL_DMA_IRQHandler( hsd.hdmatx );
}

/**  @brief  Handles SD DMA Rx transfer interrupt request.  */
void BSP_SD_DMA_Rx_IRQHandler(void)
{
  HAL_DMA_IRQHandler( hsd.hdmarx );
}

/**
  * @brief  Gets the current SD card data status.
  * @param  None
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
uint8_t BSP_SD_GetCardState(void)
{
  return ( (HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY );
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  * @retval None
  */
void BSP_SD_GetCardInfo( HAL_SD_CardInfoTypeDef *CardInfo )
{
  HAL_SD_GetCardInfo( &hsd, CardInfo );
}
/* USER CODE END 0 */

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @param  None
 * @retval Returns if SD is detected or not
 */
uint8_t BSP_SD_IsDetected(void)
{
  __IO uint8_t status = SD_PRESENT;
  return status;
}

