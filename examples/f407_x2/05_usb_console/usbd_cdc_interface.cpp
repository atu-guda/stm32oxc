// #include "main.h"
#include <stdint.h>
#include <oxc_base.h>

#include <oxc_gpio.h> // debug

#include "usbd_desc.h"
#include <usbd_cdc.h>

/* Private define ------------------------------------------------------------*/
#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048

/* Private variables ---------------------------------------------------------*/
USBD_CDC_LineCodingTypeDef LineCoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };

uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
uint32_t BuffLength;
uint32_t UserTxBufPtrIn = 0;/* Increment this pointer or roll it back to
                               start address when data are received over USART */
uint32_t UserTxBufPtrOut = 0; /* Increment this pointer or roll it back to
                                 start address when data are sent over USB */

/* USB handler declaration */
extern USBD_HandleTypeDef  USBD_Dev;

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC_Itf_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{

  /*##-5- Set Application Buffers ############################################*/
  USBD_CDC_SetTxBuffer( &USBD_Dev, UserTxBuffer, 0 );
  USBD_CDC_SetRxBuffer( &USBD_Dev, UserRxBuffer );

  return USBD_OK;
}

/**
  * @brief  CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_DeInit(void)
{
  return USBD_OK;
}

/**
  * @brief  CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length UNUSED_ARG )
{
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Add your code here */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Add your code here */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_SET_LINE_CODING:
    LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                            (pbuf[2] << 16) | (pbuf[3] << 24));
    LineCoding.format     = pbuf[4];
    LineCoding.paritytype = pbuf[5];
    LineCoding.datatype   = pbuf[6];

    /* Set the new configuration */
    // ComPort_Config();
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(LineCoding.bitrate);
    pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    /* Add your code here */
    break;

  case CDC_SEND_BREAK:
     /* Add your code here */
    break;

  default:
    break;
  }

  return USBD_OK;
}

/**
  * @brief  TIM period elapsed callback
  * @param  htim: TIM handle
  * @retval None
  */
// see USED functions 
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
//   uint32_t buffptr;
//   uint32_t buffsize;
//
//   if(UserTxBufPtrOut != UserTxBufPtrIn)
//   {
//     if(UserTxBufPtrOut > UserTxBufPtrIn) /* Rollback */
//     {
//       buffsize = APP_RX_DATA_SIZE - UserTxBufPtrOut;
//     }
//     else
//     {
//       buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
//     }
//
//     buffptr = UserTxBufPtrOut;
//
//     USBD_CDC_SetTxBuffer( &USBD_Dev, (uint8_t*)&UserTxBuffer[buffptr], buffsize );
//
//     if(USBD_CDC_TransmitPacket(&USBD_Dev) == USBD_OK)
//     {
//       UserTxBufPtrOut += buffsize;
//       if (UserTxBufPtrOut == APP_RX_DATA_SIZE)
//       {
//         UserTxBufPtrOut = 0;
//       }
//     }
//   }
// }

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//   /* Increment Index for buffer writing */
//   UserTxBufPtrIn++;
//
//   /* To avoid buffer overflow */
//   if(UserTxBufPtrIn == APP_RX_DATA_SIZE)
//   {
//     UserTxBufPtrIn = 0;
//   }
//
//   /* Start another reception: provide the buffer pointer with offset and the buffer size */
//   HAL_UART_Receive_IT(huart, (uint8_t *)(UserTxBuffer + UserTxBufPtrIn), 1);
// }

/**
  * @brief  CDC_Itf_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Receive( uint8_t* Buf UNUSED_ARG , uint32_t *Len UNUSED_ARG )
{
  // HAL_UART_Transmit_DMA(&UartHandle, Buf, *Len);
  leds.toggle( 0x08 );

  USBD_CDC_ReceivePacket( &USBD_Dev ); // ???
  return USBD_OK;
}

/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
// void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
// {
//   /* Initiate next USB packet transfer once UART completes transfer (transmitting data over Tx line) */
//   USBD_CDC_ReceivePacket( &USBD_Dev );
// }



