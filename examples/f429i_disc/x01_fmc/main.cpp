#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

BOARD_DEFINE_LEDS;

SDRAM_HandleTypeDef hsdram;
FMC_SDRAM_TimingTypeDef SDRAM_Timing;
FMC_SDRAM_CommandTypeDef command;

void SDRAM_Initialization_Sequence( SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command );


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

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


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART1 );
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART1_SEND_TASK( usartio );
// STD_USART1_RECV_TASK( usartio );
STD_USART1_IRQ( usartio );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();

  leds.write( 0x03 );  delay_bad_ms( 200 );
  // leds.write( 0x00 );  HAL_Delay( 200 );
  leds.write( 0x00 );  delay_ms( 200 );
  leds.write( 0x03 );  delay_bad_ms( 200 );
  init_uart( &uah );


  leds.write( 0x0A );  delay_bad_ms( 200 );

  // HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );

  // usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart1_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
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

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  /*##-1- Configure the SDRAM device #########################################*/
  hsdram.Instance = FMC_SDRAM_DEVICE;

  /* Timing configuration for 90 MHz of SDRAM clock frequency (180MHz/2) */
  /* TMRD: 2 Clock cycles */
  SDRAM_Timing.LoadToActiveDelay    = 2; // 2; // atu: 4:my
  /* TXSR: min=70ns (6x11.90ns) */
  SDRAM_Timing.ExitSelfRefreshDelay = 7;
  /* TRAS: min=42ns (4x11.90ns) max=120k (ns) */
  SDRAM_Timing.SelfRefreshTime      = 4;
  /* TRC:  min=63 (6x11.90ns) */
  SDRAM_Timing.RowCycleDelay        = 7;
  /* TWR:  2 Clock cycles */
  SDRAM_Timing.WriteRecoveryTime    = 2;
  /* TRP:  15ns => 2x11.90ns */
  SDRAM_Timing.RPDelay              = 2;
  /* TRCD: 15ns => 2x11.90ns */
  SDRAM_Timing.RCDDelay             = 2;

  hsdram.Init.SDBank             = FMC_SDRAM_BANK2;
  hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
  hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

  /* Initialize the SDRAM controller */
  if( HAL_SDRAM_Init( &hsdram, &SDRAM_Timing ) != HAL_OK ) {
    // Error_Handler( 4 );
    die4led( 1 );
  }

  /* Program the SDRAM external device */
  SDRAM_Initialization_Sequence( &hsdram, &command );
  // uint8_t sdram_rc = BSP_SDRAM_Init();
  // UVAR('z') = sdram_rc;

  default_main_loop();
  vTaskDelete(NULL);
}



//  ----------------------------- configs ----------------



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

