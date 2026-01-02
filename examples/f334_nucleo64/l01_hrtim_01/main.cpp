#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <stm32f3xx_hal_hrtim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

HRTIM_HandleTypeDef hhrtim1;
bool MX_HRTIM1_Init();
void HAL_HRTIM_MspInit( HRTIM_HandleTypeDef* hrtimHandle );
void HAL_HRTIM_MspPostInit( HRTIM_HandleTypeDef* hrtimHandle );

const char* common_help_string = "Appication to HRTIM 01" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test HRTIM"  );
DCL_CMD_REG( cmp1, 'C', " A B C D E - set cmp1 values"  );


void idle_main_task()
{
  // leds[1].toggle();
}



int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 100;
  UVAR_n =  20;

  BOARD_POST_INIT_BLINK;

  if( ! MX_HRTIM1_Init() ) {
    die4led( BIT2M );
  }

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  std_out << "################### sMasterRegs " NL;
  dump32( &(HRTIM1->sMasterRegs), sizeof(HRTIM_Master_TypeDef), false );

  std_out << "################### sTimerxRegs[0] = A " NL;
  dump32( &(HRTIM1->sTimerxRegs[0]), sizeof(HRTIM_Timerx_TypeDef), false );
  std_out << "################### sTimerxRegs[1] = B " NL;
  dump32( &(HRTIM1->sTimerxRegs[1]), sizeof(HRTIM_Timerx_TypeDef), false );
  // std_out << "################### sTimerxRegs[2] = C " NL;
  // dump32( &(HRTIM1->sTimerxRegs[2]), sizeof(HRTIM_Timerx_TypeDef), false );
  // std_out << "################### sTimerxRegs[3] = D " NL;
  // dump32( &(HRTIM1->sTimerxRegs[3]), sizeof(HRTIM_Timerx_TypeDef), false );
  // std_out << "################### sTimerxRegs[4] = E " NL;
  // dump32( &(HRTIM1->sTimerxRegs[4]), sizeof(HRTIM_Timerx_TypeDef), false );

  // std_out << "################### sCommonRegs " NL;
  // dump8( &(HRTIM1->sCommonRegs), sizeof(HRTIM_Common_TypeDef), false );
  //
  // std_out << "# A.CNTxR=" << HexInt( HRTIM1->sTimerxRegs[0].CNTxR ) << NL;
  // std_out << "# A.CNTxR=" << HexInt( HRTIM1->sTimerxRegs[0].CNTxR ) << NL;

  std_out << "# MCR=      " << HexInt( HRTIM1->sMasterRegs.MCR ) << NL;

  std_out << "# A.OUTxR=  " << HexInt( HRTIM1->sTimerxRegs[0].OUTxR  ) << NL;
  std_out << "# B.TIMxCR= " << HexInt( HRTIM1->sTimerxRegs[1].TIMxCR ) << NL;
  std_out << "# B.OUTxR=  " << HexInt( HRTIM1->sTimerxRegs[1].OUTxR  ) << NL;
  std_out << "# B.DTxR =  " << HexInt( HRTIM1->sTimerxRegs[1].DTxR   ) << NL;

  // 0x40017400
  //
  // dump8(  HRTIM1, 0x48, false ); // 0x40017400
  // dump32( HRTIM1, 0x48, false ); // 0x40017400
  // int n = arg2long_d( 1, argc, argv, UVAR_n, 0 );
  // uint32_t t_step = UVAR_t;
  // std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;
  //
  // uint32_t tm0 = HAL_GetTick();
  //
  // uint32_t tc0 = tm0, tc00 = tm0;
  //
  // break_flag = 0;
  // for( int i=0; i<n && !break_flag; ++i ) {
  //   uint32_t  tcc = HAL_GetTick();
  //   uint32_t tmc = HAL_GetTick();
  //   std_out <<  " Fake Action i= " << i << "  tick: " << ( tcc - tc00 )
  //           << "  ms_tick: " << ( tmc - tm0 ) << NL;
  //   delay_ms_until_brk( &tc0, t_step );
  // }

  return 0;
}

int cmd_cmp1( int argc, const char * const * argv )
{
  uint32_t ca = arg2long_d( 1, argc, argv, 0x1000, 0, 0xFFEE );
  uint32_t cb = arg2long_d( 2, argc, argv, 0x1000, 0, 0xFFEE );
  uint32_t cc = arg2long_d( 3, argc, argv, 0x1000, 0, 0xFFEE );
  uint32_t cd = arg2long_d( 4, argc, argv, 0x1000, 0, 0xFFEE );
  uint32_t ce = arg2long_d( 5, argc, argv, 0x1000, 0, 0xFFEE );
  HRTIM1->sTimerxRegs[0].CMP1xR = ca;
  HRTIM1->sTimerxRegs[1].CMP1xR = cb;
  HRTIM1->sTimerxRegs[2].CMP1xR = cc;
  HRTIM1->sTimerxRegs[3].CMP1xR = cd;
  HRTIM1->sTimerxRegs[4].CMP1xR = ce;
  return 0;
}


bool MX_HRTIM1_Init()
{
  hhrtim1.Instance                     = HRTIM1;
  hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
  hhrtim1.Init.SyncOptions             = HRTIM_SYNCOPTION_NONE;
  if( HAL_HRTIM_Init( &hhrtim1 ) != HAL_OK )  {
    errno = 70001;
    return false;
  }
  if( HAL_HRTIM_DLLCalibrationStart( &hhrtim1, HRTIM_CALIBRATIONRATE_14 ) != HAL_OK ) {
    errno = 70002;
    return false;
  }
  if( HAL_HRTIM_PollForDLLCalibration( &hhrtim1, 10 ) != HAL_OK ) {
    errno = 70003;
    return false;
  }
  if( HAL_HRTIM_EventPrescalerConfig( &hhrtim1, HRTIM_EVENTPRESCALER_DIV1 ) != HAL_OK ) {
    errno = 70004;
    return false;
  }

  HRTIM_EventCfgTypeDef pEventCfg;
  pEventCfg.Source      = HRTIM_EVENTSRC_1;
  pEventCfg.Polarity    = HRTIM_EVENTPOLARITY_HIGH;
  pEventCfg.Sensitivity = HRTIM_EVENTSENSITIVITY_LEVEL;
  pEventCfg.Filter      = HRTIM_EVENTFILTER_NONE;
  if( HAL_HRTIM_EventConfig( &hhrtim1, HRTIM_EVENT_10, &pEventCfg ) != HAL_OK ) {
    errno = 70005;
    return false;
  }
  if( HAL_HRTIM_FaultPrescalerConfig( &hhrtim1, HRTIM_FAULTPRESCALER_DIV1 ) != HAL_OK ) {
    errno = 70006;
    return false;
  }

  HRTIM_FaultCfgTypeDef pFaultCfg;
  pFaultCfg.Source   = HRTIM_FAULTSOURCE_DIGITALINPUT;
  pFaultCfg.Polarity = HRTIM_FAULTPOLARITY_HIGH;
  pFaultCfg.Filter   = HRTIM_FAULTFILTER_NONE;
  pFaultCfg.Lock     = HRTIM_FAULTLOCK_READWRITE;
  if( HAL_HRTIM_FaultConfig( &hhrtim1, HRTIM_FAULT_1, &pFaultCfg ) != HAL_OK ) {
    errno = 70007;
    return false;
  }
  HAL_HRTIM_FaultModeCtl( &hhrtim1, HRTIM_FAULT_1, HRTIM_FAULTMODECTL_ENABLED );

  HRTIM_TimeBaseCfgTypeDef pTimeBaseCfg;
  pTimeBaseCfg.Period            = 0xB400;
  pTimeBaseCfg.RepetitionCounter = 0x00;
  pTimeBaseCfg.PrescalerRatio    = HRTIM_PRESCALERRATIO_MUL32;
  pTimeBaseCfg.Mode              = HRTIM_MODE_CONTINUOUS;
  if( HAL_HRTIM_TimeBaseConfig( &hhrtim1, HRTIM_TIMERINDEX_MASTER, &pTimeBaseCfg ) != HAL_OK ) {
    errno = 70008;
    return false;
  }

  HRTIM_TimerCfgTypeDef pTimerCfg;
  pTimerCfg.InterruptRequests = HRTIM_MASTER_IT_NONE;
  pTimerCfg.DMARequests       = HRTIM_MASTER_DMA_NONE;
  pTimerCfg.DMASrcAddress     = 0x0000;
  pTimerCfg.DMADstAddress     = 0x0000;
  pTimerCfg.DMASize           = 0x1;
  pTimerCfg.HalfModeEnable    = HRTIM_HALFMODE_DISABLED;
  pTimerCfg.StartOnSync       = HRTIM_SYNCSTART_DISABLED;
  pTimerCfg.ResetOnSync       = HRTIM_SYNCRESET_DISABLED;
  pTimerCfg.DACSynchro        = HRTIM_DACSYNC_NONE;
  pTimerCfg.PreloadEnable     = HRTIM_PRELOAD_DISABLED;
  pTimerCfg.UpdateGating      = HRTIM_UPDATEGATING_INDEPENDENT;
  pTimerCfg.BurstMode         = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
  pTimerCfg.RepetitionUpdate  = HRTIM_UPDATEONREPETITION_DISABLED;
  if( HAL_HRTIM_WaveformTimerConfig( &hhrtim1, HRTIM_TIMERINDEX_MASTER, &pTimerCfg ) != HAL_OK ) {
    errno = 70009;
    return false;
  }
  if( HAL_HRTIM_TimeBaseConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimeBaseCfg ) != HAL_OK ) {
    errno = 70010;
    return false;
  }

  pTimerCfg.InterruptRequests     = HRTIM_TIM_IT_NONE;
  pTimerCfg.DMARequests           = HRTIM_TIM_DMA_NONE;
  pTimerCfg.DMASrcAddress         = 0x0000;
  pTimerCfg.DMADstAddress         = 0x0000;
  pTimerCfg.DMASize               = 0x1;
  pTimerCfg.PushPull              = HRTIM_TIMPUSHPULLMODE_DISABLED;
  pTimerCfg.FaultEnable           = HRTIM_TIMFAULTENABLE_NONE;
  pTimerCfg.FaultLock             = HRTIM_TIMFAULTLOCK_READWRITE;
  pTimerCfg.DeadTimeInsertion     = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
  pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
  pTimerCfg.UpdateTrigger         = HRTIM_TIMUPDATETRIGGER_NONE;
  pTimerCfg.ResetTrigger          = HRTIM_TIMRESETTRIGGER_NONE;
  pTimerCfg.ResetUpdate           = HRTIM_TIMUPDATEONRESET_DISABLED;
  if( HAL_HRTIM_WaveformTimerConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimerCfg ) != HAL_OK ) {
    errno = 70011;
    return false;
  }

  pTimerCfg.DMASrcAddress = 0x0000;
  pTimerCfg.DMADstAddress = 0x0000;
  pTimerCfg.DMASize       = 0x1;
  pTimerCfg.DeadTimeInsertion     = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
  if( HAL_HRTIM_WaveformTimerConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimerCfg ) != HAL_OK ) {
    errno = 70012;
    return false;
  }

  pTimerCfg.DMASrcAddress = 0x0000;
  pTimerCfg.DMADstAddress = 0x0000;
  pTimerCfg.DMASize       = 0x1;
  if( HAL_HRTIM_WaveformTimerConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &pTimerCfg ) != HAL_OK ) {
    errno = 70013;
    return false;
  }

  pTimerCfg.DMASrcAddress         = 0x0000;
  pTimerCfg.DMADstAddress         = 0x0000;
  pTimerCfg.DMASize               = 0x1;
  pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;
  if( HAL_HRTIM_WaveformTimerConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &pTimerCfg ) != HAL_OK ) {
    errno = 70014;
    return false;
  }

  pTimerCfg.InterruptRequests = HRTIM_MASTER_IT_NONE;
  pTimerCfg.DMASrcAddress     = 0x0000;
  pTimerCfg.DMADstAddress     = 0x0000;
  pTimerCfg.DMASize           = 0x1;
  if( HAL_HRTIM_WaveformTimerConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_E, &pTimerCfg ) != HAL_OK ) {
    errno = 70015;
    return false;
  }

  HRTIM_CompareCfgTypeDef pCompareCfg;
  pCompareCfg.CompareValue = 0x1000;
  if( HAL_HRTIM_WaveformCompareConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &pCompareCfg ) != HAL_OK ) {
    errno = 70016;
    return false;
  }

  pCompareCfg.CompareValue       = 0x7000;
  pCompareCfg.AutoDelayedMode    = HRTIM_AUTODELAYEDMODE_REGULAR;
  pCompareCfg.AutoDelayedTimeout = 0x0000;
  if( HAL_HRTIM_WaveformCompareConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, &pCompareCfg ) != HAL_OK ) {
    errno = 70017;
    return false;
  }

  HRTIM_DeadTimeCfgTypeDef pDeadTimeCfg;
  pDeadTimeCfg.Prescaler       = HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL8;
  /// pDeadTimeCfg.Prescaler       = HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV8;
  pDeadTimeCfg.RisingValue     = 0x0FF;
  pDeadTimeCfg.RisingSign      = HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE;
  pDeadTimeCfg.RisingLock      = HRTIM_TIMDEADTIME_RISINGLOCK_WRITE;
  pDeadTimeCfg.RisingSignLock  = HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE;
  pDeadTimeCfg.FallingValue    = 0x077;
  pDeadTimeCfg.FallingSign     = HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;
  pDeadTimeCfg.FallingLock     = HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE;
  pDeadTimeCfg.FallingSignLock = HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE;
  if( HAL_HRTIM_DeadTimeConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pDeadTimeCfg ) != HAL_OK ) {
    errno = 70018;
    return false;
  }

  HRTIM_OutputCfgTypeDef pOutputCfg;
  pOutputCfg.Polarity              = HRTIM_OUTPUTPOLARITY_HIGH;
  pOutputCfg.SetSource             = HRTIM_OUTPUTSET_TIMPER;
  pOutputCfg.ResetSource           = HRTIM_OUTPUTRESET_TIMCMP1;
  pOutputCfg.IdleMode              = HRTIM_OUTPUTIDLEMODE_NONE;
  pOutputCfg.IdleLevel             = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
  pOutputCfg.FaultLevel            = HRTIM_OUTPUTFAULTLEVEL_NONE;
  pOutputCfg.ChopperModeEnable     = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &pOutputCfg ) != HAL_OK ) {
    errno = 70019;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB1, &pOutputCfg ) != HAL_OK ) {
    errno = 70020;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC1, &pOutputCfg ) != HAL_OK ) {
    errno = 70021;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD1, &pOutputCfg ) != HAL_OK ) {
    errno = 70022;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_OUTPUT_TE1, &pOutputCfg ) != HAL_OK ) {
    errno = 70023;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, &pOutputCfg ) != HAL_OK ) {
    errno = 70024;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB2, &pOutputCfg ) != HAL_OK ) {
    errno = 70025;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC2, &pOutputCfg ) != HAL_OK ) {
    errno = 70026;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD2, &pOutputCfg ) != HAL_OK ) {
    errno = 70027;
    return false;
  }
  if( HAL_HRTIM_WaveformOutputConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_OUTPUT_TE2, &pOutputCfg ) != HAL_OK ) {
    errno = 70028;
    return false;
  }

  pTimeBaseCfg.Period = 0xB400;
  if( HAL_HRTIM_TimeBaseConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimeBaseCfg ) != HAL_OK ) {
    errno = 70029;
    return false;
  }

  pCompareCfg.CompareValue = 0x3000;
  if( HAL_HRTIM_WaveformCompareConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, &pCompareCfg ) != HAL_OK ) {
    errno = 70030;
    return false;
  }

  pTimeBaseCfg.Period = 0xB400;
  if( HAL_HRTIM_TimeBaseConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &pTimeBaseCfg ) != HAL_OK ) {
    errno = 70031;
    return false;
  }
  if( HAL_HRTIM_TimeBaseConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &pTimeBaseCfg ) != HAL_OK ) {
    errno = 70032;
    return false;
  }
  if( HAL_HRTIM_TimeBaseConfig( &hhrtim1, HRTIM_TIMERINDEX_TIMER_E, &pTimeBaseCfg ) != HAL_OK ) {
    errno = 70033;
    return false;
  }

  HRTIM1->sTimerxRegs[0].OUTxR = 0x00020000;
  // HRTIM1->sTimerxRegs[1].OUTxR = 0x00000002;
  // HRTIM1->sTimerxRegs[1].DTxR  = 0x00000000; // disable deadtime
  HRTIM1->sTimerxRegs[0].CMP1xR = 0x2000;
  HRTIM1->sTimerxRegs[1].CMP1xR = 0x4000;

  HAL_HRTIM_MspPostInit( &hhrtim1 );

  // if( HAL_HRTIM_SimpleBaseStart( &hhrtim1, 0 ) != HAL_OK )  {
  //   errno = 70040;
  //   return false;
  // }
  // HAL_HRTIM_SimpleOCStart( &hhrtim1, 0, 0 );
  //HAL_HRTIM_SimpleOCStart( &hhrtim1, 0, 1 );

  // if( HAL_HRTIM_SimpleBaseStart( &hhrtim1, 1 ) != HAL_OK )  {
  //   errno = 70042;
  //   return false;
  // }
  //HAL_HRTIM_SimpleOCStart( &hhrtim1, 1, 0 );
  //HAL_HRTIM_SimpleOCStart( &hhrtim1, 1, 1 );

  HRTIM1->sCommonRegs.OENR |=
    HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 |
    HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2 | HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2 |
    HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2;
  // __HAL_HRTIM_ENABLE( &hhrtim1, HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B );
  HRTIM1->sMasterRegs.MCR |=
    HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B  | HRTIM_TIMERID_TIMER_C |
    HRTIM_TIMERID_TIMER_D | HRTIM_TIMERID_TIMER_E;

  return true;
}

void HAL_HRTIM_MspInit( HRTIM_HandleTypeDef* hrtimHandle )
{
  //  if( hrtimHandle->Instance==HRTIM1 )
  __HAL_RCC_HRTIM1_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  //* HRTIM1 GPIO Configuration (first part)
  //  PC6     ------> HRTIM1_EEV10
  //  PA12     ------> HRTIM1_FLT1
  PC6.cfgAF(  GPIO_AF3_HRTIM1 );
  PA12.cfgAF( GPIO_AF13_HRTIM1 );
}

void HAL_HRTIM_MspPostInit( HRTIM_HandleTypeDef* hrtimHandle )
{
  // if( hrtimHandle->Instance==HRTIM1)

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /**HRTIM1 GPIO Configuration
    B12    ------> HRTIM1_CHC1
    B13    ------> HRTIM1_CHC2
    B14    ------> HRTIM1_CHD1
    B15    ------> HRTIM1_CHD2
    C8     ------> HRTIM1_CHE1
    C9     ------> HRTIM1_CHE2
    A8     ------> HRTIM1_CHA1
    A9     ------> HRTIM1_CHA2
    A10    ------> HRTIM1_CHB1
    A11    ------> HRTIM1_CHB2
    */
  GpioB.cfgAF_N( 12_mask | 13_mask | 14_mask | 15_mask, GPIO_AF13_HRTIM1 );
  GpioC.cfgAF_N( 8_mask | 9_mask, GPIO_AF3_HRTIM1 );
  GpioA.cfgAF_N( 8_mask | 9_mask | 10_mask | 11_mask, GPIO_AF13_HRTIM1 );
}

void HAL_HRTIM_MspDeInit( HRTIM_HandleTypeDef* hrtimHandle )
{
  __HAL_RCC_HRTIM1_CLK_DISABLE();
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

