#include <cstring>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_fs_cmd0.h>

#include <fatfs_sd_st.h>
#include <ff.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test SDIO with FATFS w/o FreeRTOS" NL;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;


// --- local commands;
DCL_CMD_REG( test0, 'T', " - test something SDCARD"  );
DCL_CMD_REG( sdinit, 0, " - init SDIO"  );
DCL_CMD_REG( sddeinit, 0, " - deinit SDIO"  );



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;

  MX_SDIO_SD_Init();

  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('s') = HAL_SD_GetState( &hsd );
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
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


int cmd_sdinit( int /*argc*/, const char * const * /*argv*/ )
{
  MX_SDIO_SD_Init();

  int rc_i =  HAL_SD_Init( &hsd );
  UVAR('e') = rc_i;
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('s') = HAL_SD_GetState( &hsd );
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';
  return rc_i;
}


int cmd_sddeinit( int /*argc*/, const char * const * /*argv*/ )
{
  int rc_d =  HAL_SD_DeInit( &hsd );
  UVAR('e') = rc_d;
  delay_ms( 10 );
  return rc_d;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

