#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <ff_gen_drv_st.h>
#include <usbh_diskio.h>
#include <ff.h>

#include <oxc_fs_cmd0.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

const char* common_help_string = "Appication to test USB host msc (flash)" NL;

USBH_HandleTypeDef hUSB_Host;
char USBDISKPath[8]; // USB Host logical drive path
uint8_t sd_buf[512]; // one sector
FATFS fs;
int isUSBH_on = 0, isMSC_ready = 0;

void USBH_HandleEvent( USBH_HandleTypeDef *phost, uint8_t id );

// --- local commands;
DCL_CMD_REG( test0, 'T', " - "  );

void idle_main_task()
{
  if( isUSBH_on ) {
    // leds.toggle( 1 );
    USBH_Process( &hUSB_Host );
  };
}



int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 100;
  UVAR('n') =  20;
  fs.fs_type = 0; // none
  fspath[0] = '\0';

  BOARD_POST_INIT_BLINK;
  leds.reset( 0xFF );

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  // uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 100000000 ); // number of series

  std_out << "# Test: " << NL;

  int rc = FATFS_LinkDriver( &USBH_Driver, USBDISKPath );
  if( rc != 0 ) {
    std_out << "# Error LinkDriver: " << rc << NL;
    return 1;
  }

  // std_out << "# USBH_Init: " NL;
  // delay_ms( 10 );
  USBH_Init( &hUSB_Host, USBH_HandleEvent, 0);

  // std_out << "# USBH_RegisterClass" NL;
  // delay_ms( 10 );
  USBH_RegisterClass( &hUSB_Host, USBH_MSC_CLASS );

  // std_out << "# USBH_Start: " NL;
  // delay_ms( 10 );
  isUSBH_on = 1;
  USBH_Start( &hUSB_Host );


  return 0;
}

// on: 4,3,2 off: 5
void USBH_HandleEvent( USBH_HandleTypeDef *phost, uint8_t id )
{
  // leds.toggle( BIT1 );
  // std_out << "### UP " << (int)id << NL;

  switch( id ) {
    case HOST_USER_SELECT_CONFIGURATION: // 1
      break;

    case HOST_USER_CLASS_ACTIVE:         // 2
      isMSC_ready = 1;
      // leds.set( BIT2 );
      break;

    case HOST_USER_CLASS_SELECTED:       // 3
      break;

    case HOST_USER_CONNECTION:           // 4
      break;

    case HOST_USER_DISCONNECTION:        // 5
      // leds.reset( BIT2 );
      f_mount( nullptr, (TCHAR const*)"", 0 );
      isMSC_ready = 0;
      break;

    case HOST_USER_UNRECOVERED_ERROR:    // 6
      // leds.set( BIT0 );
      errno = 7555;
      break;

    default:
      break;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

