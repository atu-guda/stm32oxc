#ifndef _BOARD_SDRAM_H
#define _BOARD_SDRAM_H

// ---------------- SDRAM -------------------------------------------------------------------------
#define SDRAM_BANK_ADDR     ((uint32_t)0xC0000000)
#define SDRAM_ADDR          ((uint8_t *)(SDRAM_BANK_ADDR))
#define SDRAM_DEVICE_SIZE   ((uint32_t)32*1024*1024)  // SDRAM device size in bytes
#define SDRAM_REFRESH_COUNT ((uint32_t)0x0603)    // SDRAM refresh counter (100Mhz SD clock)

#define SDRAM_CMD_WAIT 0x1000
#define SDRAM_COMMAND_TARGET  FMC_SDRAM_CMD_TARGET_BANK1
#define SDRAM_MODEREG_VAL SDRAM_MODEREG_BURST_LENGTH_2     | \
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   | \
                     SDRAM_MODEREG_CAS_LATENCY_3           | \
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD | \
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE


#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)


const inline FMC_SDRAM_TimingTypeDef sd_tmng_default = {
  .LoadToActiveDelay    = 2, // TMRD: 2 Clock cycles
  .ExitSelfRefreshDelay = 7, // TXSR: min=70ns (6x11.90ns)
  .SelfRefreshTime      = 4, // TRAS: min=42ns (4x11.90ns) max=120k (ns)
  .RowCycleDelay        = 6, //
  .WriteRecoveryTime    = 2, // TWR:  2 Clock cycles
  .RPDelay              = 2, // TRP:  15ns => 2x11.90ns
  .RCDDelay             = 2, // TRCD: 15ns => 2x11.90ns
};

const inline FMC_SDRAM_InitTypeDef  sdram_init_default = {
  .SDBank             = FMC_SDRAM_BANK1,
  .ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8,
  .RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13,
  .MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16,
  .InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4,
  .CASLatency         = FMC_SDRAM_CAS_LATENCY_3,
  .WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
  .SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2,
  .ReadBurst          = FMC_SDRAM_RBURST_DISABLE,
  .ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_2
};


#endif
