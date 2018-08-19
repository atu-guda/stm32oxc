#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test SDIO in low-level model" NL;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};




int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;

  MX_SDIO_SD_Init();
  // UVAR('e') = HAL_SD_Init( &hsd );

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

  STDOUT_os;
  os <<  NL "Test0: start= "  <<  start <<  NL;

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
  os << "rc= " << rc << " sd_state= " << sd_state << " card_state= " << card_state << NL;
  os <<  "sd_err= "  << HexInt(  sd_err )  <<  " sd_state2= " << sd_state2
     <<  "type= "  <<  cardInfo.CardType
     <<  " ver= "  <<  cardInfo.CardVersion
     <<  " class = "  <<  cardInfo.Class
     <<  " blocks = "  <<  cardInfo.BlockNbr
     <<  " bsz = "  <<  cardInfo.BlockSize << NL;

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

