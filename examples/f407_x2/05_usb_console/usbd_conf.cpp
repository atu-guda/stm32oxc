#include <stm32f4xx_hal.h>
#include <usbd_core.h>

#include <oxc_base.h>

/* Private variables ---------------------------------------------------------*/
PCD_HandleTypeDef hpcd;
void default_USBFS_MspInit(void);


void default_USBFS_MspInit(void)
{
  GPIO_InitTypeDef  gpi;
  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpi.Pin = GPIO_PIN_11 | GPIO_PIN_12; // D-, D+
  gpi.Mode = GPIO_MODE_AF_PP;
  gpi.Pull = GPIO_NOPULL;
  gpi.Speed = GPIO_SPEED_HIGH;
  gpi.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init( GPIOA, &gpi );

  gpi.Pin = GPIO_PIN_9; // VBUS ??
  gpi.Mode = GPIO_MODE_INPUT;
  gpi.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( GPIOA, &gpi );

  gpi.Pin = GPIO_PIN_10; // ID ?
  gpi.Mode = GPIO_MODE_AF_OD;
  gpi.Pull = GPIO_PULLUP;
  gpi.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init( GPIOA, &gpi );

  __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  HAL_NVIC_SetPriority( OTG_FS_IRQn, 15, 0 ); // TODO: =configLIBRARY_KERNEL_INTERRUPT_PRIORITY

  /* Enable USBFS Interrupt */
  HAL_NVIC_EnableIRQ( OTG_FS_IRQn );

}



/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/


void HAL_PCD_SetupStageCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_SetupStage( (USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup );
}

void HAL_PCD_DataOutStageCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_DataOutStage( (USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff );
}

void HAL_PCD_DataInStageCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_DataInStage( (USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff );
}

void HAL_PCD_SOFCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_SOF( (USBD_HandleTypeDef*)hpcd->pData );
}

void HAL_PCD_ResetCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL; // atu: only full
  USBD_LL_SetSpeed( (USBD_HandleTypeDef*)hpcd->pData, speed );
  USBD_LL_Reset( (USBD_HandleTypeDef*)hpcd->pData );
}

void HAL_PCD_SuspendCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_Suspend( (USBD_HandleTypeDef*)hpcd->pData );
}

void HAL_PCD_ResumeCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_Resume( (USBD_HandleTypeDef*)hpcd->pData );
}

void HAL_PCD_ISOOUTIncompleteCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_IsoOUTIncomplete( (USBD_HandleTypeDef*)hpcd->pData, epnum );
}

void HAL_PCD_ISOINIncompleteCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_IsoINIncomplete( (USBD_HandleTypeDef*)hpcd->pData, epnum );
}

void HAL_PCD_ConnectCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_DevConnected( (USBD_HandleTypeDef*)hpcd->pData );
}

void HAL_PCD_DisconnectCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_DevDisconnected( (USBD_HandleTypeDef*)hpcd->pData );
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/
/**
  * @brief  Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef  USBD_LL_Init( USBD_HandleTypeDef *pdev )
{
  /* Change Systick prioity */
  // NVIC_SetPriority ( SysTick_IRQn, 0 ); // TODO: atu: check!

  /*Set LL Driver parameters */
  hpcd.Instance = USB_OTG_FS;
  hpcd.Init.dev_endpoints = 4;
  hpcd.Init.use_dedicated_ep1 = 0;
  hpcd.Init.ep0_mps = 0x40;
  hpcd.Init.dma_enable = 0;
  hpcd.Init.low_power_enable = 0;
  hpcd.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd.Init.Sof_enable = 0;
  hpcd.Init.speed = PCD_SPEED_FULL;
  hpcd.Init.vbus_sensing_enable = 1; // TODO: no?
  /* Link The driver to the stack */
  hpcd.pData = pdev;
  pdev->pData = &hpcd;
  /*Initialize LL Driver */
  HAL_PCD_Init( &hpcd );

  HAL_PCDEx_SetRxFiFo( &hpcd, 0x80 );
  HAL_PCDEx_SetTxFiFo( &hpcd, 0, 0x40 );
  HAL_PCDEx_SetTxFiFo( &hpcd, 1, 0x80 );

  return USBD_OK;
}

/**
  * @brief  De-Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_DeInit( USBD_HandleTypeDef *pdev )
{
  HAL_PCD_DeInit( (PCD_HandleTypeDef*)pdev->pData );
  return USBD_OK;
}

/**
  * @brief  Starts the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Start( USBD_HandleTypeDef *pdev )
{
  HAL_PCD_Start( (PCD_HandleTypeDef*)pdev->pData );
  return USBD_OK;
}

/**
  * @brief  Stops the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Stop( USBD_HandleTypeDef *pdev )
{
  HAL_PCD_Stop( (PCD_HandleTypeDef*)pdev->pData );
  return USBD_OK;
}

/**
  * @brief  Opens an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  ep_type: Endpoint Type
  * @param  ep_mps: Endpoint Max Packet Size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_OpenEP( USBD_HandleTypeDef *pdev,
                                  uint8_t  ep_addr,
                                  uint8_t  ep_type,
                                  uint16_t ep_mps )
{
  HAL_PCD_EP_Open( (PCD_HandleTypeDef*)pdev->pData, ep_addr, ep_mps, ep_type );
  return USBD_OK;
}

/**
  * @brief  Closes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_CloseEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  HAL_PCD_EP_Close( (PCD_HandleTypeDef*)pdev->pData, ep_addr );
  return USBD_OK;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_FlushEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  HAL_PCD_EP_Flush( (PCD_HandleTypeDef*)pdev->pData, ep_addr );
  return USBD_OK;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_StallEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  HAL_PCD_EP_SetStall( (PCD_HandleTypeDef*)pdev->pData, ep_addr );
  return USBD_OK;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  HAL_PCD_EP_ClrStall( (PCD_HandleTypeDef*)pdev->pData, ep_addr );
  return USBD_OK;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Stall (1: yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*)pdev->pData;

  if( (ep_addr & 0x80 ) == 0x80 )
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns an USB address to the device
  * @param  pdev: Device handle
  * @param  dev_addr: USB address
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress( USBD_HandleTypeDef *pdev, uint8_t dev_addr )
{
  HAL_PCD_SetAddress( (PCD_HandleTypeDef*)pdev->pData, dev_addr );
  return USBD_OK;
}

/**
  * @brief  Transmits data over an endpoint
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Transmit( USBD_HandleTypeDef *pdev,
                                    uint8_t  ep_addr,
                                    uint8_t  *pbuf,
                                    uint16_t  size )
{
  HAL_PCD_EP_Transmit( (PCD_HandleTypeDef*)pdev->pData, ep_addr, pbuf, size );
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_PrepareReceive( USBD_HandleTypeDef *pdev,
                                          uint8_t  ep_addr,
                                          uint8_t  *pbuf,
                                          uint16_t  size )
{
  HAL_PCD_EP_Receive( (PCD_HandleTypeDef*)pdev->pData, ep_addr, pbuf, size );
  return USBD_OK;
}

uint32_t USBD_LL_GetRxDataSize( USBD_HandleTypeDef *pdev, uint8_t  ep_addr )
{
  return HAL_PCD_EP_GetRxCount( (PCD_HandleTypeDef*)pdev->pData, ep_addr );
}

void  USBD_LL_Delay( uint32_t delay )
{
  HAL_Delay( delay );
  // delay_ms( delay );
}

