#include <oxc_base.h>

#include <board_sdram.h>



void SDRAM_Initialization_Sequence( SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *cmd )
{
  __IO uint32_t tmpmrd =0;
  /* Step 3:  Configure a clock configuration enable command */
  cmd->CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  cmd->CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
  cmd->AutoRefreshNumber      = 1;
  cmd->ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand( hsdram, cmd, 0x1000 );

  /* Step 4: Insert 100 ms delay */
  delay_bad_ms( 100 );

  /* Step 5: Configure a PALL (precharge all) command */
  cmd->CommandMode          = FMC_SDRAM_CMD_PALL;
  cmd->CommandTarget        = FMC_SDRAM_CMD_TARGET_BANK2;
  cmd->AutoRefreshNumber    = 1;
  cmd->ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(hsdram, cmd, 0x1000);

  /* Step 6 : Configure a Auto-Refresh command */
  cmd->CommandMode        = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  cmd->CommandTarget      = FMC_SDRAM_CMD_TARGET_BANK2;
  cmd->AutoRefreshNumber    = 4;
  cmd->ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(hsdram, cmd, 0x1000);

  /* Step 7: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  cmd->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  cmd->CommandTarget      = FMC_SDRAM_CMD_TARGET_BANK2;
  cmd->AutoRefreshNumber    = 1;
  cmd->ModeRegisterDefinition = tmpmrd;
  HAL_SDRAM_SendCommand(hsdram, cmd, 0x1000);

  /* Step 8: Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  HAL_SDRAM_ProgramRefreshRate( hsdram, REFRESH_COUNT );
}

int bsp_init_sdram( SDRAM_HandleTypeDef *phsdram )
{
  FMC_SDRAM_CommandTypeDef command;
  /*##-1- Configure the SDRAM device #########################################*/
  phsdram->Instance = FMC_SDRAM_DEVICE;

  // Timing configuration for 96 MHz of SDRAM clock frequency (192MHz/2)
  // all other must be scaled to 192/180 = 1.066666666
  FMC_SDRAM_TimingTypeDef sd_tmng;
  sd_tmng.LoadToActiveDelay    = 2; // TMRD: 2 Clock cycles
  sd_tmng.ExitSelfRefreshDelay = 7; // TXSR: min=70ns (6x11.90ns)
  sd_tmng.SelfRefreshTime      = 4; // TRAS: min=42ns (4x11.90ns) max=120k (ns)
  sd_tmng.RowCycleDelay        = 7; // TRC:  min=63 (6x11.90ns)
  sd_tmng.WriteRecoveryTime    = 2; // TWR:  2 Clock cycles
  sd_tmng.RPDelay              = 2; // TRP:  15ns => 2x11.90ns
  sd_tmng.RCDDelay             = 2; // TRCD: 15ns => 2x11.90ns

  phsdram->Init.SDBank             = FMC_SDRAM_BANK2;
  phsdram->Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  phsdram->Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  phsdram->Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  phsdram->Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  phsdram->Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  phsdram->Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  phsdram->Init.SDClockPeriod      = SDCLOCK_PERIOD;
  phsdram->Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
  phsdram->Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

  if( HAL_SDRAM_Init( phsdram, &sd_tmng ) != HAL_OK ) {
    return 0;
  }

  SDRAM_Initialization_Sequence( phsdram, &command );
  return 1;
}
