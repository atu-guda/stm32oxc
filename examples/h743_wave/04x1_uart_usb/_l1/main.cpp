#include <oxc_auto.h>

#include <usbd_cdc.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

USBD_HandleTypeDef usb_dev;
void default_USBFS_MspInit(void);
extern USBD_DescriptorsTypeDef VCP_Desc;
int8_t CDC_Itf_Init();
int8_t CDC_Itf_DeInit();
int8_t CDC_Itf_Control( uint8_t cmd, uint8_t* pbuf, uint16_t length );
int8_t CDC_Itf_Receive( uint8_t* pbuf, uint32_t *Len );
USBD_CDC_ItfTypeDef cdc_fops =
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive
};
const unsigned RX_BUF_SIZE = 2048;
const unsigned TX_BUF_SIZE = 2048;
char rx_buf[RX_BUF_SIZE];
char tx_buf[TX_BUF_SIZE];
USBD_CDC_LineCodingTypeDef lineCoding = {
  115200, /* baud rate*/ 0x00, /* stop bits-1*/  0x00,   /* parity - none*/  0x08 /* nb. of bits 8 */
};

const char* common_help_string = "Appication to test USBCDC via UART" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 100;
  UVAR('n') =  20;

  BOARD_POST_INIT_BLINK;


  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  // default_USBFS_MspInit();
  std_out <<  "# Test USBD_Init ... ";
  if( USBD_Init( &usb_dev, &VCP_Desc, BOARD_USB_DEFAULT_TYPE ) != USBD_OK ) { // 0 = DEVICE_FS 1 = DEVICE_HS
    errno = 2000;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  delay_ms( 2000 );

  std_out <<  "# Test USBD_RegisterClass ... ";
  if( USBD_RegisterClass( &usb_dev, USBD_CDC_CLASS ) != USBD_OK ) {
    errno = 2001;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  delay_ms( 2000 );

  std_out <<  "# Test USBD_CDC_RegisterInterface ... ";
  if( USBD_CDC_RegisterInterface( &usb_dev, &cdc_fops ) != USBD_OK ) {
    errno = 2002;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  delay_ms( 2000 );

  std_out <<  "# Test USBD_Start ... ";
  if( USBD_Start( &usb_dev ) != USBD_OK ) {
    errno = 2003;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  delay_ms( 2000 );
  std_out <<  "# ========== OK ================== " NL;

  return 0;
}

// ----------------------------------------------------------------------------------------------
//

void HAL_PCD_MspInit( PCD_HandleTypeDef *hpcd UNUSED_ARG )
{
  default_USBFS_MspInit();
}

int8_t CDC_Itf_Init()
{
  // leds.set( BIT1 );

  USBD_CDC_SetTxBuffer( &usb_dev, (uint8_t*)tx_buf, 0 );
  USBD_CDC_SetRxBuffer( &usb_dev, (uint8_t*)rx_buf );

  // leds.reset( BIT1 );

  return USBD_OK;
}

int8_t CDC_Itf_DeInit()
{
  // leds.set( BIT2 );

  // leds.reset( BIT2 );

  return USBD_OK;
}

int8_t CDC_Itf_Control( uint8_t cmd, uint8_t* pbuf, uint16_t length UNUSED_ARG )
{
  // leds.set( BIT1 );

  switch( cmd ) {
    case CDC_SEND_ENCAPSULATED_COMMAND:
      // NOP
      break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
      // NOP
      break;

    case CDC_SET_COMM_FEATURE:
      // NOP
      /* Add your code here */
      break;

    case CDC_GET_COMM_FEATURE:
      // NOP
      break;

    case CDC_CLEAR_COMM_FEATURE:
      // NOP
      break;

    case CDC_SET_LINE_CODING:
      // leds.toggle( BIT1 );
      lineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
          (pbuf[2] << 16) | (pbuf[3] << 24));
      lineCoding.format     = pbuf[4];
      lineCoding.paritytype = pbuf[5];
      lineCoding.datatype   = pbuf[6];

      break;

    case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)( lineCoding.bitrate       );
      pbuf[1] = (uint8_t)( lineCoding.bitrate >>  8 );
      pbuf[2] = (uint8_t)( lineCoding.bitrate >> 16 );
      pbuf[3] = (uint8_t)( lineCoding.bitrate >> 24 );
      pbuf[4] = lineCoding.format;
      pbuf[5] = lineCoding.paritytype;
      pbuf[6] = lineCoding.datatype;
      break;

    case CDC_SET_CONTROL_LINE_STATE:
      // leds.toggle( BIT2 );

      {
        // TODO: may be callback here
        USBD_SetupReqTypedef *req = (USBD_SetupReqTypedef *)pbuf;
        if( ( req->wValue & 0x0001 ) != 0 )  { // check DTR
          // static_usbcdcio->ready_transmit = true;
          // leds.set( BIT1 );
        } else {
          // static_usbcdcio->ready_transmit = false;
          // leds.reset( BIT1 );
        }
      }

      break;

    case CDC_SEND_BREAK:
      // leds.toggle( BIT1 );
      break;

    default:
      break;
  }

  // leds.reset( BIT2 );
  std_out << "# " << __FUNCTION__ << " END" NL;

  return USBD_OK;
}

int8_t CDC_Itf_Receive( uint8_t* Buf, uint32_t *Len )
{
  // if( static_usbcdcio && Buf && Len && *Len ) {
  //   static_usbcdcio->charsFromIrq( (char*)Buf, *Len );
  // }
  // leds.set( BIT2 );

  USBD_CDC_ReceivePacket( &usb_dev ); // acq
  return USBD_OK;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

