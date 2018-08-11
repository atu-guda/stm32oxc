#ifndef _OXC_USBCDCIO_H
#define _OXC_USBCDCIO_H

#include <oxc_devio.h>

#include <usbd_cdc.h>

extern PCD_HandleTypeDef hpcd;
extern USBD_DescriptorsTypeDef VCP_Desc;
extern USBD_HandleTypeDef USBD_Dev;

class UsbcdcIO : public DevIO {
  public:
   UsbcdcIO()
    { pusb_dev = &usb_dev; static_usbcdcio = this; }
   // virtual ~UsbcdcIO() override {};
   // virtual void reset() override {}
   int init();

   virtual int write_s( const char *s, int l ) override;
   //{ usb->transmit( s, l ); return l; } // TODO: check return status

   virtual Chst getc_p( int w_tick = 0 ) override
   {
     return getc( w_tick ); // no special
   };

   USBD_CDC_LineCodingTypeDef* getCdcLineCoding() { return &lineCoding; }

   static int8_t CDC_Itf_Init();
   static int8_t CDC_Itf_DeInit();
   static int8_t CDC_Itf_Control( uint8_t cmd, uint8_t* pbuf, uint16_t length );
   static int8_t CDC_Itf_Receive( uint8_t* pbuf, uint32_t *Len );


  protected:
   USBD_HandleTypeDef usb_dev;
   static USBD_HandleTypeDef *pusb_dev;
   static UsbcdcIO *static_usbcdcio;
   USBD_CDC_LineCodingTypeDef lineCoding =
   { 115200, /* baud rate*/ 0x00, /* stop bits-1*/  0x00,   /* parity - none*/  0x08 /* nb. of bits 8*/
   };
   char rx_buf[TX_BUF_SIZE];
   char tx_buf[TX_BUF_SIZE];
   USBD_CDC_ItfTypeDef cdc_fops =
   {
     CDC_Itf_Init,
     CDC_Itf_DeInit,
     CDC_Itf_Control,
     CDC_Itf_Receive
   };
};


#define USBCDC_CONSOLE_DEFINES \
  UsbcdcIO dev_console; \
  SmallRL srl( smallrl_exec ); \
  STD_POST_EXEC;

#define SET_USBCDC_AS_STDIO(dev_console) \
  dev_console.setOnSigInt( sigint ); \
  devio_fds[0] = &dev_console;  \
  devio_fds[1] = &dev_console;  \
  devio_fds[2] = &dev_console;  \
  srl.setPostExecFun( standart_post_exec ); \
  delay_ms( 50 );

#endif
// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include
