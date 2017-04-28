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
int cmd_cat( int argc, const char * const * argv );
CmdInfo CMDINFO_CAT { "cat", 0, cmd_cat, " path [max [noout]] - output file contents to stdout"  };
int cmd_appstr( int argc, const char * const * argv );
CmdInfo CMDINFO_APPSTR { "appstr", 0, cmd_appstr, " file string  - append string to file"  };
int cmd_wblocks( int argc, const char * const * argv );
CmdInfo CMDINFO_WBLOCKS { "wblocks", 'W', cmd_wblocks, " file [n_blocks]  - write blocks to file"  };
int cmd_rm( int argc, const char * const * argv );
CmdInfo CMDINFO_RM { "rm", 0, cmd_rm, " file - remove file"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_MOUNT,
  &CMDINFO_UMOUNT,
  &CMDINFO_FSINFO,
  &CMDINFO_LS,
  &CMDINFO_CAT,
  &CMDINFO_APPSTR,
  &CMDINFO_WBLOCKS,
  &CMDINFO_RM,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
int init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

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

  if( ! init_uart( &uah ) ) {
      die4led( 1 );
  }
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

  return 0;
}

int cmd_mount( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    strncpy( fspath, argv[1], sizeof( fspath ) - 1 );
  }
  pr( "Try to mount \"" ); pr( fspath ); pr( "\""  NL);
  FRESULT fr = f_mount( &fs, fspath, 1 );

  return fr;
}

int cmd_umount( int argc, const char * const * argv )
{
  pr( "Try to umount \"" ); pr( fspath ); pr( "\""  NL);
  FRESULT fr = f_mount( nullptr, fspath, 1 );

  return fr;
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
      pr( " " ); pr_d( finfo.fsize );
      pr( NL );
    }
    f_closedir( &dir );
  } else {
    pr( "f_opendir error: " ); pr_d( r ); pr( NL );
  }

  return r;
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

  return 0;
}

int cmd_cat( int argc, const char * const * argv )
{
  const char *fn = "";
  if( argc > 1 ) {
    fn = argv[1];
  }
  uint32_t max_sz = arg2long_d( 2, argc, argv, 1024, 1, 0x7FFFFFFF );
  bool do_out = ! arg2long_d( 3, argc, argv, 0, 1, 1 );
  pr( "cat file \"" ); pr( fn ); pr( "\"  max_sz= " ); pr_d( max_sz ); pr( NL );

  unsigned nr = 0, to_read, was_read; // number of read bytes
  FIL f;
  FRESULT r = f_open( &f, fn, FA_READ );
  TickType_t tc0 = xTaskGetTickCount(), tc1 = tc0;
  if( r == FR_OK ) {
    while( nr < max_sz ) { // or break
      to_read = max_sz - nr;
      if( to_read > sizeof( sd_buf ) ) {
        to_read = sizeof( sd_buf );
      }
      r = f_read( &f, sd_buf, to_read, &was_read );
      // pr( " to_read= " ); pr_d( to_read ); pr( " was_read= " ); pr_d( was_read ); pr( " r= " ); pr_d( r ); pr( NL );
      if( r != FR_OK || was_read < 1 ) {
        break;
      }
      if( do_out ) {
        sendBlock( 1, (const char*)sd_buf, was_read );
      }
      nr += was_read;
    }
    f_close( &f );
    tc1 = xTaskGetTickCount();
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }

  pr( NL "cat end, read " ); pr_d( nr);
  pr( " bytes, r =  " ); pr_d( r );
  pr( " time =  " ); pr_d( tc1 - tc0 ); pr( NL );
  return 0;
}

int cmd_appstr( int argc, const char * const * argv )
{
  if( argc < 3 ) {
    pr( "Error: need filename and string" NL );
    return 1;
  }

  const char *fn = argv[1], *s = argv[2];
  unsigned  was_wr = 0;
  FIL f;
  FRESULT r = f_open( &f, fn, FA_WRITE | FA_OPEN_ALWAYS );
  // float zz = 0.123456;
  if( r == FR_OK ) {
    r = f_lseek( &f, f.fsize );
    was_wr = f_puts( s, &f );
    // f_printf( &f, "more %d %f string\r\n", f.fsize, zz );
    f_close( &f );
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }

  pr( NL "appstr end, r= " ); pr_d( r ); pr( " was_wr= "); pr_d( was_wr ); pr( NL );
  return 0;
}


int cmd_wblocks( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Error: need filename [n_sect]" NL );
    return 1;
  }
  uint32_t n_sect = arg2long_d( 2, argc, argv, 1, 1, 1024*2048 );

  const char *fn = argv[1];
  unsigned  was_wr = 0, w;
  memset( sd_buf, '1', sizeof( sd_buf ) );
  FIL f;
  FRESULT r = f_open( &f, fn, FA_WRITE | FA_OPEN_ALWAYS );
  TickType_t tc0 = xTaskGetTickCount(), tc1 = tc0;
  if( r == FR_OK ) {
    for( unsigned i=0; i < n_sect; ++i ) {
      r = f_write( &f, sd_buf, sizeof( sd_buf ), &w );
      was_wr += w;
      if( r != FR_OK ) {
        break;
      }
    }
    tc1 = xTaskGetTickCount();
    f_close( &f );
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }

  pr( NL "wblocks end, r= " ); pr_d( r ); pr( " was_wr= "); pr_d( was_wr );
  pr( " time =  " ); pr_d( tc1 - tc0 ); pr( NL );
  return 0;
}

int cmd_rm( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Error: need filename to delete" NL );
    return 1;
  }

  const char *fn = argv[1];
  FRESULT r = f_unlink( fn );
  if( r != FR_OK ) {
    pr( "f_unlink error: " ); pr_d( r ); pr( NL );
  }

  return r;
}



//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

