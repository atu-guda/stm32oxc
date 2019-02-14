#ifndef _BOARD_SDRAM_H
#define _BOARD_SDRAM_H

// ---------------- SDRAM -------------------------------------------------------------------------
#define REFRESH_COUNT       ((uint32_t)0x056A)   /* SDRAM refresh counter (90MHz SDRAM clock) */
#define SDRAM_BANK_ADDR                 ((uint32_t)0xD0000000)
#define SDRAM_ADDR ((uint8_t *)(SDRAM_BANK_ADDR))

#define SDRAM_CMD_WAIT 0x1000
#define SDRAM_COMMAND_TARGET  FMC_SDRAM_CMD_TARGET_BANK2
#define SDRAM_MODEREG_VAL SDRAM_MODEREG_BURST_LENGTH_2     | \
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   | \
                     SDRAM_MODEREG_CAS_LATENCY_3           | \
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD | \
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE

/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8 */
#define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_16

/* #define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2 */
#define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)


const inline FMC_SDRAM_TimingTypeDef sd_tmng_default = {
  .LoadToActiveDelay    = 2, // TMRD: 2 Clock cycles
  .ExitSelfRefreshDelay = 7, // TXSR: min=70ns (6x11.90ns)
  .SelfRefreshTime      = 4, // TRAS: min=42ns (4x11.90ns) max=120k (ns)
  .RowCycleDelay        = 7, // TRC:  min=63 (6x11.90ns)
  .WriteRecoveryTime    = 2, // TWR:  2 Clock cycles
  .RPDelay              = 2, // TRP:  15ns => 2x11.90ns
  .RCDDelay             = 2, // TRCD: 15ns => 2x11.90ns
};

const inline FMC_SDRAM_InitTypeDef  sdram_init_default = {
  .SDBank             = FMC_SDRAM_BANK2,
  .ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8,
  .RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12,
  .MemoryDataWidth    = SDRAM_MEMORY_WIDTH,
  .InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4,
  .CASLatency         = FMC_SDRAM_CAS_LATENCY_3,
  .WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
  .SDClockPeriod      = SDCLOCK_PERIOD,
  .ReadBurst          = FMC_SDRAM_RBURST_DISABLE,
  .ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1
};


#endif
