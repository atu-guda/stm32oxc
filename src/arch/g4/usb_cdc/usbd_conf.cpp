// reworked usbd_conf.c (+more) from  STMicroelectronics
#include <errno.h>
#include <oxc_base.h>
#include <oxc_gpio.h>

#include <usbd_core.h>


extern "C" {
  USBD_StatusTypeDef USBD_LL_PrepareReceive( USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t sz );
  USBD_StatusTypeDef USBD_LL_Transmit( USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t sz );
}

PCD_HandleTypeDef hpcd;
void default_USBFS_MspInit(void); // *hpcd?
USBD_StatusTypeDef USBD_Get_USB_Status( HAL_StatusTypeDef hal_status );

#ifndef OXC_NEED_SPECIAL_PCD_MSPINIT
void HAL_PCD_MspInit( PCD_HandleTypeDef* /* hpcd */ )
{
  default_USBFS_MspInit();
}
#endif

void default_USBFS_MspInit(void)
{
  // if( pcdHandle->Instance != BOARD_USB_DEFAULT_INSTANCE ) { // ???????????????????????
  //   return;
  // }

  #ifdef BOARD_USB_DEFAULT_GPIO_AF
    BOARD_USB_DEFAULT_GPIO.cfgAF_N( BOARD_USB_DEFAULT_DPDM_PINS, BOARD_USB_DEFAULT_GPIO_AF );
  #endif

  #ifdef BOARD_USB_DEFAULT_VBUS_PIN
    BOARD_USB_DEFAULT_GPIO.cfgIn_N( BOARD_USB_DEFAULT_VBUS_PIN );
  #endif

  #ifdef BOARD_USB_DEFAULT_ID_PIN
    BOARD_USB_DEFAULT_GPIO.cfgAF_N( BOARD_USB_DEFAULT_ID_PIN, BOARD_USB_DEFAULT_GPIO_AF, true );
  #endif

  // special for G4. TODO: config
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  memset( &PeriphClkInit, 0, sizeof(PeriphClkInit) );
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection    = RCC_USBCLKSOURCE_PLL;
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK ) {
    errno = 1008;
    return;
  }

  BOARD_USB_DEFAULT_ENABLE;

  // HAL_NVIC_SetPriority(   OTG_FS_EP1_OUT_IRQn, BOARD_USB_DEFAULT_IRQ_PRTY, 0 );
  // HAL_NVIC_SetPriority(    OTG_FS_EP1_IN_IRQn, BOARD_USB_DEFAULT_IRQ_PRTY, 0 );
  HAL_NVIC_SetPriority( BOARD_USB_DEFAULT_IRQ, BOARD_USB_DEFAULT_IRQ_PRTY, 0 );

  // HAL_NVIC_EnableIRQ( OTG_FS_EP1_OUT_IRQn );
  // HAL_NVIC_EnableIRQ( OTG_FS_EP1_IN_IRQn );
  HAL_NVIC_EnableIRQ( BOARD_USB_DEFAULT_IRQ );

}


void BOARD_USB_DEFAULT_IRQHANDLER(void)
{
  // leds.set( BIT0 );
  HAL_PCD_IRQHandler( &hpcd );
  // leds.reset( BIT0 );
}

#if defined(STM32H7)
void OTG_FS_EP1_OUT_IRQHandler(void)
{
  // leds.set( BIT1 );
  HAL_PCD_IRQHandler( &hpcd );
  // leds.reset( BIT1 );
}

void OTG_FS_EP1_IN_IRQHandler(void)
{
  // leds.set( BIT2 );
  HAL_PCD_IRQHandler( &hpcd );
  // leds.reset( BIT2 );
}
#endif

USBD_StatusTypeDef USBD_Get_USB_Status( HAL_StatusTypeDef hal_status );
void SystemClockConfig_Resume(void);
extern void SystemClock_Config(void); // TODO: drop

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/


void HAL_PCD_MspDeInit( PCD_HandleTypeDef* pcdHandle )
{
  if( pcdHandle->Instance != USB ) {
    return;
  }
  __HAL_RCC_USB_CLK_DISABLE();

  HAL_NVIC_DisableIRQ( USB_LP_IRQn );
}

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SetupStageCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_SetupStage( (USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup );
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_DataOutStageCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_DataOutStage( (USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff );
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_DataInStageCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_DataInStage( (USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff );
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SOFCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_SOF( (USBD_HandleTypeDef*)hpcd->pData );
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResetCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL; // atu: only full
  USBD_LL_SetSpeed( (USBD_HandleTypeDef*)hpcd->pData, speed );
  USBD_LL_Reset( (USBD_HandleTypeDef*)hpcd->pData );
}

/**
  * @brief  Suspend callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SuspendCallback( PCD_HandleTypeDef *hpcd )
{
  /* Inform USB library that core enters in suspend Mode. */
  USBD_LL_Suspend( (USBD_HandleTypeDef*)hpcd->pData );
  /* Enter in STOP mode. */
  if( hpcd->Init.low_power_enable ) {
    /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
}

/**
  * @brief  Resume callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResumeCallback( PCD_HandleTypeDef *hpcd )
{
  if( hpcd->Init.low_power_enable ) {
    /* Reset SLEEPDEEP bit of Cortex System Control Register. */
    SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    SystemClockConfig_Resume();
  }

  USBD_LL_Resume( (USBD_HandleTypeDef*)hpcd->pData );
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_ISOOUTIncompleteCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_IsoOUTIncomplete( (USBD_HandleTypeDef*)hpcd->pData, epnum );
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_ISOINIncompleteCallback( PCD_HandleTypeDef *hpcd, uint8_t epnum )
{
  USBD_LL_IsoINIncomplete( (USBD_HandleTypeDef*)hpcd->pData, epnum );
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ConnectCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_DevConnected( (USBD_HandleTypeDef*)hpcd->pData );
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_DisconnectCallback( PCD_HandleTypeDef *hpcd )
{
  USBD_LL_DevDisconnected( (USBD_HandleTypeDef*)hpcd->pData );
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/
/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Init( USBD_HandleTypeDef *pdev )
{

  hpcd.pData                    = pdev;
  pdev->pData                   = &hpcd;

  hpcd.Instance                 = BOARD_USB_DEFAULT_INSTANCE;
  hpcd.Init.dev_endpoints       = 4;
  hpcd.Init.speed               = PCD_SPEED_FULL;
  hpcd.Init.phy_itface          = PCD_PHY_EMBEDDED;
  hpcd.Init.Sof_enable          = DISABLE;
  hpcd.Init.low_power_enable    = DISABLE;
  hpcd.Init.lpm_enable          = DISABLE;
  hpcd.Init.battery_charging_enable = DISABLE;

#if defined(STM32H7)
  hpcd.Init.battery_charging_enable = DISABLE;
#endif

  #if ( USE_HAL_PCD_REGISTER_CALLBACKS != 0 )
  /* register Msp Callbacks (before the Init) atu: not now*/
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_MSPINIT_CB_ID,   PCD_MspInit );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_MSPDEINIT_CB_ID, PCD_MspDeInit );
  #endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

  if( HAL_PCD_Init( &hpcd ) != HAL_OK ) {
    errno = 50000;
    return USBD_FAIL;
  }

#if ( USE_HAL_PCD_REGISTER_CALLBACKS != 0 )
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_SOF_CB_ID, PCD_SOFCallback );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_RESET_CB_ID, PCD_ResetCallback );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback );
  HAL_PCD_RegisterCallback( &hpcd, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback );

  HAL_PCD_RegisterDataOutStageCallback( &hpcd, PCD_DataOutStageCallback );
  HAL_PCD_RegisterDataInStageCallback(  &hpcd, PCD_DataInStageCallback );
  HAL_PCD_RegisterIsoOutIncpltCallback( &hpcd, PCD_ISOOUTIncompleteCallback );
  HAL_PCD_RegisterIsoInIncpltCallback(  &hpcd, PCD_ISOINIncompleteCallback );

#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

  HAL_PCDEx_PMAConfig( (PCD_HandleTypeDef*)pdev->pData , 0x00 , PCD_SNG_BUF,  0x18 );
  HAL_PCDEx_PMAConfig( (PCD_HandleTypeDef*)pdev->pData , 0x80 , PCD_SNG_BUF,  0x58 );
  HAL_PCDEx_PMAConfig( (PCD_HandleTypeDef*)pdev->pData , 0x81 , PCD_SNG_BUF,  0xC0 );
  HAL_PCDEx_PMAConfig( (PCD_HandleTypeDef*)pdev->pData , 0x01 , PCD_SNG_BUF, 0x110 );
  HAL_PCDEx_PMAConfig( (PCD_HandleTypeDef*)pdev->pData , 0x82 , PCD_SNG_BUF, 0x100 );
  return USBD_OK;
}

/**
  * @brief  De-Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_DeInit( USBD_HandleTypeDef *pdev )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_DeInit( (PCD_HandleTypeDef*)pdev->pData ) );
  return rc;
}

/**
  * @brief  Starts the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Start( USBD_HandleTypeDef *pdev )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_Start( (PCD_HandleTypeDef*)pdev->pData ) );
  return rc;
}

/**
  * @brief  Stops the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Stop( USBD_HandleTypeDef *pdev )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_Stop( (PCD_HandleTypeDef*)pdev->pData ) );
  return rc;
}

/**
  * @brief  Opens an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_type: Endpoint type
  * @param  ep_mps: Endpoint max packet size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_OpenEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_Open( (PCD_HandleTypeDef*)pdev->pData, ep_addr, ep_mps, ep_type ) );
  return rc;
}

/**
  * @brief  Closes an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_CloseEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_Close( (PCD_HandleTypeDef*)pdev->pData, ep_addr ) );
  return rc;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_FlushEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_Flush( (PCD_HandleTypeDef*)pdev->pData, ep_addr ) );
  return rc;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_StallEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_SetStall( (PCD_HandleTypeDef*)pdev->pData, ep_addr ) );
  return rc;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_ClrStall( (PCD_HandleTypeDef*)pdev->pData, ep_addr ) );
  return rc;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;

  if( (ep_addr & 0x80 ) == 0x80 ) {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  } else {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress( USBD_HandleTypeDef *pdev, uint8_t dev_addr )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_SetAddress( (PCD_HandleTypeDef*)pdev->pData, dev_addr ) );
  return rc;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Transmit( USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_Transmit( (PCD_HandleTypeDef*)pdev->pData, ep_addr, pbuf, size ) );
  return rc;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive( USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size )
{
  USBD_StatusTypeDef rc = USBD_Get_USB_Status( HAL_PCD_EP_Receive( (PCD_HandleTypeDef*)pdev->pData, ep_addr, pbuf, size ) );
  return rc;
}

/**
  * @brief  Returns the last transferred packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Received Data Size
  */
uint32_t USBD_LL_GetRxDataSize( USBD_HandleTypeDef *pdev, uint8_t ep_addr )
{
  return HAL_PCD_EP_GetRxCount( (PCD_HandleTypeDef*) pdev->pData, ep_addr );
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay( uint32_t delay )
{
  // HAL_Delay( delay );
  delay_ms( delay );
  // delay_bad_ms( delay );
}


/**
  * @brief  Configures system clock after wake-up from USB resume callBack:
  *         enable HSI, PLL and select PLL as system clock source.
  * @retval None
  */
void SystemClockConfig_Resume(void)
{
  // TODO: use my SystemClock_Config();
}

/**
  * @brief  Returns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
USBD_StatusTypeDef USBD_Get_USB_Status( HAL_StatusTypeDef hal_status )
{
  USBD_StatusTypeDef usb_status = USBD_OK;

  switch (hal_status)
  {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT :
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}


