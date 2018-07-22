#include <oxc_base.h>

#include <board_sdram.h>

void HAL_SDRAM_MspInit( SDRAM_HandleTypeDef *hsdram )
{
  GPIO_InitTypeDef  GPIO_Init_Structure;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+--------------------+--------------------+
 +                       SDRAM pins assignment                                      +
 +-------------------+--------------------+--------------------+--------------------+
 | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0  <-> FMC_A10   |
 | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1  <-> FMC_A11   |
 | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG4  <-> FMC_BA0   |
 | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG5  <-> FMC_BA1   |
 | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    | PG8  <-> FMC_SDCLK |
 | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    | PG15 <-> FMC_NCAS  |
 | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FMC_NRAS  |--------------------+
 +-------------------| PE12 <-> FMC_D9    | PF12 <-> FMC_A6    |
                     | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |
                     | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |
                     | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |
 +-------------------+--------------------+--------------------+
 | PB5 <-> FMC_SDCKE1|
 | PB6 <-> FMC_SDNE1 |
 | PC0 <-> FMC_SDNWE |
 +-------------------+

*/

  /* Common GPIO configuration */
  GPIO_Init_Structure.Mode  = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_Init_Structure.Pull  = GPIO_NOPULL;
  GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;

  /* GPIOB configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);

  /* GPIOC configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

  /* GPIOD configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1  | GPIO_PIN_8 |
                            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
                            GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

  /* GPIOE configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7 |
                            GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                            GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

  /* GPIOF configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0  | GPIO_PIN_1 | GPIO_PIN_2 |
                            GPIO_PIN_3  | GPIO_PIN_4 | GPIO_PIN_5 |
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                            GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

  /* GPIOG configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |
                            GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init( GPIOG, &GPIO_Init_Structure );
}


void SDRAM_Initialization_Sequence( SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command )
{
  __IO uint32_t tmpmrd =0;
  /* Step 3:  Configure a clock configuration enable command */
  Command->CommandMode        = FMC_SDRAM_CMD_CLK_ENABLE;
  Command->CommandTarget      = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber    = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 4: Insert 100 ms delay */
  // HAL_Delay(100);
  delay_bad_ms( 100 );

  /* Step 5: Configure a PALL (precharge all) command */
  Command->CommandMode        = FMC_SDRAM_CMD_PALL;
  Command->CommandTarget        = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber    = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 6 : Configure a Auto-Refresh command */
  Command->CommandMode        = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command->CommandTarget      = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber    = 4;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 7: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command->CommandTarget      = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber    = 1;
  Command->ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 8: Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  HAL_SDRAM_ProgramRefreshRate( hsdram, REFRESH_COUNT );
}

int bsp_init_sdram( SDRAM_HandleTypeDef *phsdram )
{
  FMC_SDRAM_TimingTypeDef SDRAM_Timing;
  FMC_SDRAM_CommandTypeDef command;
  /*##-1- Configure the SDRAM device #########################################*/
  phsdram->Instance = FMC_SDRAM_DEVICE;

  // Timing configuration for 96 MHz of SDRAM clock frequency (192MHz/2)
  // all other must be scaled to 192/180 = 1.066666666
  SDRAM_Timing.LoadToActiveDelay    = 2; // TMRD: 2 Clock cycles
  SDRAM_Timing.ExitSelfRefreshDelay = 7; // TXSR: min=70ns (6x11.90ns)
  SDRAM_Timing.SelfRefreshTime      = 4; // TRAS: min=42ns (4x11.90ns) max=120k (ns)
  SDRAM_Timing.RowCycleDelay        = 7; // TRC:  min=63 (6x11.90ns)
  SDRAM_Timing.WriteRecoveryTime    = 2; // TWR:  2 Clock cycles
  SDRAM_Timing.RPDelay              = 2; // TRP:  15ns => 2x11.90ns
  SDRAM_Timing.RCDDelay             = 2; // TRCD: 15ns => 2x11.90ns

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

  if( HAL_SDRAM_Init( phsdram, &SDRAM_Timing ) != HAL_OK ) {
    return 0;
  }

  SDRAM_Initialization_Sequence( phsdram, &command );
  return 1;
}
