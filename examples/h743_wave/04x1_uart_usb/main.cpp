#include <oxc_auto.h>

#include <usbd_cdc.h>

#include "usbd_cdc_if.h"
#include "usbd_conf.h"
#include "usbd_desc.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

extern PCD_HandleTypeDef hpcd;
USBD_HandleTypeDef usb_dev;
void default_USBFS_MspInit(void);
// extern USBD_DescriptorsTypeDef VCP_Desc;

// void MX_USB_DEVICE_Init(void);

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
  UVAR('n') = 100;

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

  // MX_USB_DEVICE_Init();

  // // default_USBFS_MspInit();
  std_out <<  "# Test USBD_Init ... ";
  if( USBD_Init( &usb_dev, &VCP_Desc, BOARD_USB_DEFAULT_TYPE ) != USBD_OK ) { // 0 = DEVICE_FS 1 = DEVICE_HS
    errno = 2000;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  // delay_ms( 50 );

  std_out <<  "# Test USBD_RegisterClass ... ";
  if( USBD_RegisterClass( &usb_dev, USBD_CDC_CLASS ) != USBD_OK ) {
    errno = 2001;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  // delay_ms( 50 );

  std_out <<  "# Test USBD_CDC_RegisterInterface ... ";
  if( USBD_CDC_RegisterInterface( &usb_dev, &cdc_fops ) != USBD_OK ) {
    errno = 2002;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  // delay_ms( 50 );

  std_out <<  "# Test USBD_Start ... ";
  if( USBD_Start( &usb_dev ) != USBD_OK ) {
    errno = 2003;
    std_out <<  " == fail" NL;
    return 1;
  }
  std_out <<  " .. OK" NL;
  delay_ms( 200 );
  std_out <<  "# ========== OK ================== " NL;

  for( int i=0; i<n && !break_flag; ++i ) {
    CDC_Transmit_FS( (uint8_t*)("ADCD\r\n"), 6 );
    std_out << "# " << i << NL;
    delay_ms_brk( 1000 );
  }

  return 0;
}

// ----------------------------------------------------------------------------------------------
//


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

