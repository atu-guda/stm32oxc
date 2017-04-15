#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <ff.h>
#include <fatfs.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS_EX;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;
const int fspath_sz = 32;
char fspath[fspath_sz];

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_mount( int argc, const char * const * argv );
CmdInfo CMDINFO_MOUNT { "mount", 'M', cmd_mount, " [path] - mount FAT filesystem"  };
int cmd_umount( int argc, const char * const * argv );
CmdInfo CMDINFO_UMOUNT { "umount", 'U', cmd_umount, " - umount FAT filesystem"  };
int cmd_fsinfo( int argc, const char * const * argv );
CmdInfo CMDINFO_FSINFO { "fsinfo", 'I', cmd_fsinfo, " info about FAT filesystem"  };
int cmd_ls( int argc, const char * const * argv );
CmdInfo CMDINFO_LS { "ls", 0, cmd_ls, " [path] - list directory contents"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_MOUNT,
  &CMDINFO_UMOUNT,
  &CMDINFO_FSINFO,
  &CMDINFO_LS,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL_EX );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL_EX );
    return 0;
  }

  HAL_Delay( 200 ); // delay_bad_ms( 200 );
  leds.write( 0x00 ); delay_ms( 200 );
  leds.write( BOARD_LEDS_ALL_EX );  HAL_Delay( 200 );

  init_uart( &uah );
  leds.write( 0x0A );  delay_bad_ms( 200 );


  MX_SDIO_SD_Init();

  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_Init();
  UVAR('s') = HAL_SD_GetState( &hsd );
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';

  leds.write( BOARD_LEDS_ALL );  HAL_Delay( 200 );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_UART_AS_STDIO( usartio );

  default_main_loop();
  vTaskDelete(NULL);
}




// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int start = arg2long_d( 1, argc, argv, 0, 0 );
  // uint32_t t_step = UVAR('t');
  pr( NL "Test0: start= " ); pr_d( start ); // pr( " t= " ); pr_d( t_step );
  pr( NL );

  HAL_StatusTypeDef rc;

  memset( sd_buf, 0, sizeof( sd_buf ) );
  auto card_state = HAL_SD_GetCardState( &hsd );
  auto sd_err = HAL_SD_GetError( &hsd );
  rc = HAL_SD_ReadBlocks( &hsd, sd_buf, start, 1, 1000 );
  auto sd_state2 = HAL_SD_GetState( &hsd );

  dump8( sd_buf, sizeof(sd_buf) );
  pr_sdx( rc );
  // pr_sdx( sd_state );
  pr_sdx( card_state );
  pr( "sd_err= " ); pr_h( sd_err ); pr( NL );
  pr_sdx( sd_state2 );
  pr( "type= " ); pr_d( cardInfo.CardType );
  pr( " ver= " ); pr_d( cardInfo.CardVersion );
  pr( " class = " ); pr_d( cardInfo.Class );
  pr( " blocks = " ); pr_d( cardInfo.BlockNbr );
  pr( " bsz = " ); pr_d( cardInfo.BlockSize );
  pr( NL );


  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_mount( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    strncpy( fspath, argv[1], sizeof( fspath ) - 1 );
  }
  pr( "Try to mount \"" ); pr( fspath ); pr( "\""  NL);
  FRESULT fr = f_mount( &fs, fspath, 1 );
  pr( "Result: " ); pr_d( fr ); pr( NL );


  break_flag = 0;  idle_flag = 1;
  pr( NL "mount end." NL );
  return 0;
}

int cmd_umount( int argc, const char * const * argv )
{
  pr( "Try to umount \"" ); pr( fspath ); pr( "\""  NL);
  FRESULT fr = f_mount( nullptr, fspath, 1 );
  pr( "Result: " ); pr_d( fr ); pr( NL );

  break_flag = 0;  idle_flag = 1;
  pr( NL "umount end." NL );
  return 0;
}

int cmd_ls( int argc, const char * const * argv )
{
  const char *dname = "/";
  if( argc > 1 ) {
    dname = argv[1];
  }
  pr( "dir \"" ); pr( dname ); pr( "\"" NL );

  DIR dir;
  FILINFO finfo;
  FRESULT r = f_opendir( &dir, dname );
  if( r == FR_OK ) {
    while( ( r = f_readdir( &dir, &finfo ) )  ==  FR_OK ) {
      if( ! finfo.fname[0] ) { // end of dir
        break;
      }
      pr( finfo.fname );
      if( finfo.fattrib & AM_DIR ) {
        pr( "/" );
      }
      pr( NL );
    }
  } else {
    pr( "f_opendir error: " ); pr_d( r ); pr( NL );
  }

  break_flag = 0;  idle_flag = 1;
  pr( NL "ls end." NL );
  return 0;
}

int cmd_fsinfo( int argc, const char * const * argv )
{
  const char *fsn = "";
  if( argc > 1 ) {
    fsn = argv[1];
  }
  pr( "fsinfo about \"" ); pr( fsn ); pr( "\"" NL "fs_type= " ); pr_d( fs.fs_type ); pr( NL );
  pr( "drv= " ); pr_d( fs.drv ); pr( NL );
  pr( "csize= " ); pr_d( fs.csize ); pr( NL );
  pr( "n_fats= " ); pr_d( fs.n_fats ); pr( NL );
  pr( "wflag= " ); pr_d( fs.wflag ); pr( NL );
  pr( "fsi_flag= " ); pr_d( fs.fsi_flag ); pr( NL );
  pr( "id= " ); pr_d( fs.id ); pr( NL );

  char vol_buf[32]; vol_buf[0] = '\0';
  uint32_t vsn = 0, n_f_clust = 0;
  FATFS *lfs;
  if( fs.fs_type > 0 ) {
    f_getlabel( fsn, vol_buf, &vsn );
    pr( "volume_label: \"" ); pr( vol_buf ); pr( "\" vsn=" ); pr_d( vsn ); pr( NL );
    f_getfree( fsn, &n_f_clust, &lfs );
    pr( "free_clust: " ); pr_d( n_f_clust ); pr( " kB:" ); pr_d( n_f_clust * fs.csize / 2 ) ;pr( NL );
  }

  break_flag = 0;  idle_flag = 1;
  pr( NL "ls end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc
