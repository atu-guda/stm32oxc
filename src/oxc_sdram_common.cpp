#include <oxc_sdram_common.h>

#include <board_sdram.h>


SDRAM_HandleTypeDef hsdram_main;

int SDRAM_Initialization_Sequence( SDRAM_HandleTypeDef *hsdram )
{
  FMC_SDRAM_CommandTypeDef cmd;

  // Configure a clock configuration enable
  cmd.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  cmd.CommandTarget          = SDRAM_COMMAND_TARGET;
  cmd.AutoRefreshNumber      = 1;
  cmd.ModeRegisterDefinition = 0;
  if( HAL_SDRAM_SendCommand( hsdram, &cmd, SDRAM_CMD_WAIT ) != HAL_OK ) {
    return 0;
  }

  delay_bad_ms( 100 );

  // Configure a PALL (precharge all)
  cmd.CommandMode            = FMC_SDRAM_CMD_PALL;
  if( HAL_SDRAM_SendCommand( hsdram, &cmd, SDRAM_CMD_WAIT ) != HAL_OK ) {
    return 0;
  }

  // Configure a Auto-Refresh
  cmd.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  cmd.AutoRefreshNumber      = 4;
  if( HAL_SDRAM_SendCommand( hsdram, &cmd, SDRAM_CMD_WAIT ) != HAL_OK ) {
    return 0;
  }

  // Program the external memory mode register
  cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  cmd.AutoRefreshNumber    = 1;
  cmd.ModeRegisterDefinition = SDRAM_MODEREG_VAL;
  if( HAL_SDRAM_SendCommand( hsdram, &cmd, SDRAM_CMD_WAIT ) != HAL_OK ) {
    return 0;
  }

  // Set the refresh rate counter
  if( HAL_SDRAM_ProgramRefreshRate( hsdram, SDRAM_REFRESH_COUNT ) != HAL_OK ) {
    return 0;
  }
  return 1;
}

int bsp_init_sdram()
{
  hsdram_main.Instance = FMC_SDRAM_DEVICE;

  hsdram_main.Init = sdram_init_default;

  if( HAL_SDRAM_Init( &hsdram_main, const_cast<FMC_SDRAM_TimingTypeDef*>(&sd_tmng_default) ) != HAL_OK ) {
    return 0;
  }

  return SDRAM_Initialization_Sequence( &hsdram_main );
}
