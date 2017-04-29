#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

UART_CONSOLE_DEFINES( USART2 );

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}


int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  MX_SDIO_SD_Init();
  // UVAR('e') = HAL_SD_Init( &hsd );

  delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

  CREATE_STD_TASKS( task_usart2_send );

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
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
  __HAL_SD_ENABLE( &hsd );

  memset( sd_buf, 0, sizeof( sd_buf ) );
  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 100 );
  auto sd_state = HAL_SD_GetState( &hsd );
  HAL_SD_CardInfoTypeDef cardInfo;
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  auto card_state = HAL_SD_GetCardState( &hsd );
  auto sd_err = HAL_SD_GetError( &hsd );
  rc = HAL_SD_ReadBlocks( &hsd, sd_buf, start, 1, 1000 );
  auto sd_state2 = HAL_SD_GetState( &hsd );

  __HAL_SD_DISABLE( &hsd );

  dump8( sd_buf, sizeof(sd_buf) );
  pr_sdx( rc );
  pr_sdx( sd_state );
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


//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

