#include <stdlib.h>
#include <errno.h>

#if USE_FREERTOS != 0
#include <FreeRTOS.h>
#include <task.h>
#endif

#include <oxc_gpio.h>
#include <oxc_devio.h>
#include <oxc_debug1.h>

// general buffers
char gbuf_a[GBUF_SZ];
char gbuf_b[GBUF_SZ]; // and log too
int user_vars[N_USER_VARS];

extern  const int _sdata, _edata, _sbss, _ebss, _end, _estack;

char* str2addr( const char *str )
{
  if( str[0] == 'a' && str[1] == '\0' ) {
    return gbuf_a;
  } else if ( str[0] == 'b' && str[1] == '\0' ) {
    return gbuf_b;
  }
  char *eptr;
  char *addr = (char*)( strtol( str, &eptr, 0 ) );
  if( *eptr == '\0' ) {
    return addr;
  }
  return (char*)(BAD_ADDR);
}



void dump8( const void *addr, int n )
{
  char b[8]; // for char2hex
  unsigned const char* ad = (unsigned const char*)(addr);
  if( !ad  ||  ad == BAD_ADDR ) {
    return;
  }
  unsigned const char* ad0 = ad;
  pr( NL );

  int i, row, bs;
  int nr = (n+15) >> 4; // non-full rows counting too
  for( row = 0; row < nr; ++row ) {
    pr_a( ad0 ); pr( ": " );
    bs = row << 4;
    for( i=0; i<16 && (i+bs)<n; ++i ) {
      pr( char2hex( ad[i+bs], b ) ); pr( " " );
      if( (i&3) == 3 ) {
        pr( ": " );
      }
    }

    pr( "|  " );
    b[1] = 0;
    for( i=0; i<16 && (i+bs)<n; ++i ) {
      b[0] = '.';
      if( ad[i+bs] >= ' ' ) {
        b[0] = ad[i+bs];
      }
      pr( b );
      if( (i&3) == 3 ) {
        pr( " " );
      }
    }
    pr( NL );
    ad0 += 16;

  }

  pr( "--------------------------------------" NL );
}


int log_buf_idx = 0;

// TODO: mutex
void log_add( const char *s )
{
  if( !s ) {
    return;
  }

  while( *s !=0  && log_buf_idx < GBUF_SZ-1 ) {
    gbuf_b[log_buf_idx++] = *s++;
  }
  gbuf_b[log_buf_idx] = '\0'; // not++
}

void log_add_bin( const char *s, uint16_t len )
{
  if( !s ) {
    return;
  }

  for( uint16_t i=0;  i<len && log_buf_idx < GBUF_SZ-1; ++i ) {
    gbuf_b[log_buf_idx++] = *s++;
  }
  gbuf_b[log_buf_idx] = '\0'; // not++
}

void log_reset()
{
  log_buf_idx = 0;
  gbuf_b[0] = '\0';
}

void log_print()
{
  if( log_buf_idx > 0 ) {
    pr( gbuf_b );
    pr( NL );
    pr_sd( "log_buf_idx", log_buf_idx );
    pr( NL );
    delay_ms( 100 );
  }
}

void print_user_var( int idx )
{
  if( idx < 0  ||  idx >= N_USER_VARS ) {
    pr_sd( NL "err: bad var index: ", idx );
    return;
  }
  char b[4] = "0= ";
  b[0] = (char)('a' + idx);
  pr( b ); pr_d( user_vars[idx] ); pr( " = " );
  pr_h( user_vars[idx] ); pr( NL );
}

// common commands
//
int cmd_info( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  pr( NL "**** " PROJ_NAME " **** " NL );

  pr( "SYSCLK: " );  pr_d( HAL_RCC_GetSysClockFreq()  );
  pr( "  HCLK: " );  pr_d( HAL_RCC_GetHCLKFreq()  );
  pr( "  PCLK1: " ); pr_d( HAL_RCC_GetPCLK1Freq() );
  pr( "  PCLK2: " ); pr_d( HAL_RCC_GetPCLK2Freq() );
  pr( "  HSE_VALUE: " ); pr_d( HSE_VALUE );
  pr_sdx( SystemCoreClock );

  pr( NL "errno= "); pr_d( errno );
  pr( NL "dbg_val0= 0x" ); pr_h( dbg_val0 ); pr( " = " ); pr_d( dbg_val0 );
  pr(   " dbg_val1= 0x" ); pr_h( dbg_val1 ); pr( " = " ); pr_d( dbg_val1 );
  pr( NL "dbg_val2= 0x" ); pr_h( dbg_val2 ); pr( " = " ); pr_d( dbg_val2 );
  pr(   " dbg_val3= 0x" ); pr_h( dbg_val3 ); pr( " = " ); pr_d( dbg_val3 );

  pr( NL "_sdata= " ); pr_h( (int)(&_sdata) ); pr(  " _edata=  " ); pr_h( (int)(&_edata) );
  pr( NL "_sbss=  " ); pr_h( (int)(&_sbss) );  pr(  " _ebss=   " ); pr_h( (int)(&_ebss) );
  pr( NL "_end=   " ); pr_h( (int)(&_end) );   pr(  " _estack= " ); pr_h( (int)(&_estack) );

  uint32_t c_msp = __get_MSP(), c_psp = __get_PSP();
  pr( NL "MSP=   " ); pr_h( c_msp );   pr(  " PSP= " ); pr_h( c_psp );
  pr( "  __heap_top=   " ); pr_h( (int)__heap_top );
  pr( NL );

  uint32_t prio_grouping = HAL_NVIC_GetPriorityGrouping();
  pr_sdx( prio_grouping );

  uint32_t prio_preempt, prio_sub;
  struct OutIrqName {
    IRQn_Type IRQn;
    const char* nm;
  };
  const OutIrqName irqs[] = {
    { SysTick_IRQn, "SysTick" },
    { EXTI0_IRQn,   "EXTI0  " },
    { I2C1_EV_IRQn, "I2C1_EV" },
    { USART2_IRQn,  "USART2 " },
    // { OTG_FS_IRQn,  "OTG_FS " }, TODO: depend in MCU type
    { SPI1_IRQn,    "SPI1   " }
  };

  for( auto iqn : irqs ) {
    HAL_NVIC_GetPriority( iqn.IRQn, prio_grouping, &prio_preempt, &prio_sub );
    pr( iqn.nm ); pr( " (" ); pr_d( iqn.IRQn );
    pr( ")  preempt= " ); pr_d( prio_preempt );
    pr( "  sub= " ); pr_d( prio_sub );
    pr( NL );
  }
  delay_ms( 100 );

  unsigned max_malloc_sz = 0;
  for( unsigned sz = 1024; sz < 1024 * 1024; sz += 1024 ) {
    void *p = malloc( sz );
    if( !p ) {
      break;
    }
    free( p );
    max_malloc_sz = sz;
    // pr_sdx( sz );
    // delay_ms( 50 );
  }
  pr_sdx( max_malloc_sz );


  #if USE_FREERTOS != 0
    const char *nm = pcTaskGetTaskName( 0 );
    pr( "task: \"" ); pr( nm ); pr( "\" tick_count: "  );
    int tick_count = xTaskGetTickCount();
    pr_d( tick_count );
    int prty = uxTaskPriorityGet( 0 );
    pr( "  prty: " );    pr_d( prty );
    // uint32_t free_heap = xPortGetFreeHeapSize(); // not in heap_3.c
    // pr( "  free_heap: " );    pr_d( free_heap );
    uint32_t highStackWaterMark = uxTaskGetStackHighWaterMark( 0 );
    pr_sdx( highStackWaterMark );
  #endif
  errno = 0;
  return 0;
}
CmdInfo CMDINFO_INFO {  "info",  'i', cmd_info,       " - Output general info" };

int cmd_echo( int argc, const char * const * argv )
{
  pr( NL ); pr_sdx( argc ); pr( NL );
  int i;
  for( i=0; i<argc; ++i ) {
    pr( "  arg" ); pr_d( i ); pr( " = \"" ) ; pr( argv[i] ); pr( "\"" NL );
  }
  return 0;
}
CmdInfo CMDINFO_ECHO { "echo",  'e', cmd_echo,       " [args] - output args" };

int cmd_help( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG)
{
  pr( "commands:" NL );
  char b1[2]; b1[0] = b1[1] = 0;
  for( int i=0; global_cmds[i] && i<CMDS_NMAX; ++i ) {
    if( global_cmds[i]->name == 0 ) {
      break;
    }
    pr( global_cmds[i]->name ); pr( " " );
    pr( global_cmds[i]->hint ); pr( " " );
    if( global_cmds[i]->acr != 0 ) {
      pr( " (" ); b1[0] = global_cmds[i]->acr; pr( b1 ); pr( ")" );
    }
    pr( NL );
  }
  return 0;
}
CmdInfo CMDINFO_HELP { "help",  'h', cmd_help, " - List of commands and arguments"  };

int cmd_dump( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  const char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    pr( "** error: dump: bad address \"" );  pr( argv[1] );  pr( "\"" NL );
    return 2;
  }

  int n = 1;
  if( argc >= 3 ) {
    n = strtol( argv[2], 0, 0 );
  }

  pr( NL "** dump: argc=" ); pr_d( argc ); pr( " addr=" ); pr_a( addr );
  pr_sdx( n );
  dump8( addr, n );
  return 0;
}
CmdInfo CMDINFO_DUMP { "dump",  'd', cmd_dump, " {a|b|addr} [n] - HexDumps given area"  };

int cmd_fill( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    pr( "** error: fill: bad address \"" );  pr( argv[1] );  pr( "\"" NL );
    return 2;
  }

  uint8_t v = 0;
  if( argc >= 3 ) {
    if( argv[2][0] == '\'' ) {
      v = argv[2][1];
    } else {
      v = ( uint8_t ) ( strtol( argv[2], 0, 0 ) );
    }
  }

  int n = 1;
  if( argc >= 4 ) {
    n = strtol( argv[3], 0, 0 );
  }

  uint8_t stp = 0;
  if( argc >= 5 ) {
    stp = ( uint8_t ) ( strtol( argv[4], 0, 0 ) );
  }


  pr( "** fill: addr=" ); pr_a( addr );
  pr_sdx( v );
  pr_sdx( n );
  pr_sdx( stp );

  for( int i=0; i<n; ++i, ++addr ) {
    *addr = v; v+=stp;
  }
  pr( NL "---------- done---------------" NL );
  return 0;
}
CmdInfo CMDINFO_FILL { "fill",  'f', cmd_fill, " {a|b|addr} val [n] [stp] - Fills memory by value"  };

int cmd_pvar( int argc, const char * const * argv )
{
  uint8_t start = 0, end = N_USER_VARS;
  if( argc > 1 ) {
    start = argv[1][0]-'a';
    if( argv[1][1] >= 'a' ) {
      end = argv[1][1]-'a';
    } else {
      end = 1;
    }

  }
  if( end <= start ) {
    end = start+1;
  }

  for( int i=start; i<end; ++i ) {
    print_user_var( i );
  }
  return 0;
}
CmdInfo CMDINFO_PVAR { "print", 'p', cmd_pvar, "name - print user var a-z"  };

int cmd_svar( int argc, const char * const * argv )
{
  if( argc < 3 ) {
    return 1;
  }
  int idx = argv[1][0] - 'a';
  if( idx < 0 || idx >= N_USER_VARS ) {
    return 2;
  }
  user_vars[idx] = strtol( argv[2], 0, 0 );
  return 0;
}
CmdInfo CMDINFO_SVAR { "set", 's', cmd_svar,  "name value - set var a-z"  };


int cmd_die( int argc, const char * const * argv )
{
  int v = 0;
  if( argc > 1 ) {
    v = strtol( argv[1], 0, 0 );
  }
  die4led( v );
  return 0; // never ;-)
}
CmdInfo CMDINFO_DIE { "die",    0,  cmd_die, " [val] - die with value"  };

int cmd_reboot( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG)
{
  NVIC_SystemReset();
  return 0; // never ;-)
}
CmdInfo CMDINFO_REBOOT { "reboot", 0,  cmd_reboot,     " reboot system"  };

int cmd_log_print( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  log_print();
  return 0;
}
CmdInfo CMDINFO_LOG_PRINT { "lp", 0,  cmd_log_print, "  - print log buffer"  };

int cmd_log_reset( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  log_reset();
  return 0;
}
CmdInfo CMDINFO_LOG_RESET { "lr",     0,  cmd_log_reset,  "  - reset log buffer"  };

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../inc
