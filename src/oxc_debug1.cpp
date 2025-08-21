#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <algorithm>

#if defined(USE_FREERTOS)  &&  ( USE_FREERTOS != 0 )
#include <FreeRTOS.h>
#include <task.h>
#endif

#include <oxc_gpio.h>
#include <oxc_debug1.h>
#include <oxc_devio.h>
#include <oxc_outstream.h>

// general buffers
char gbuf_a[GBUF_SZ];
char gbuf_b[GBUF_SZ]; // and log too
int user_vars[N_USER_VARS];

bool (*print_var_hook)( const char *nm, int fmt ) = nullptr;
bool (*set_var_hook)( const char *nm, const char *s ) = nullptr;

static const Name2Addr dev_addrs[] = {
  { "TIM1", TIM1 },
  { "TIM2", TIM2 },
#ifdef TIM3
  { "TIM3", TIM3 },
#endif
#ifdef TIM4
  { "TIM4", TIM4 },
#endif
#ifdef TIM5
  { "TIM5", TIM5 },
#endif
#ifdef TIM6
  { "TIM6", TIM6 },
#endif
#ifdef TIM7
  { "TIM7", TIM7 },
#endif
#ifdef TIM8
  { "TIM8", TIM8 },
#endif
  { "USART1", USART1 },
#ifdef USART2
  { "USART2", USART2 },
#endif
#ifdef SPI1
  { "SPI1", SPI1 },
#endif
#ifdef SPI2
  { "SPI2", SPI2 },
#endif
};

char* str2addr( const char *str )
{
  if( !str || !*str ) {
    return (char*)(BAD_ADDR);
  }
  if( str[0] == 'a' && str[1] == '\0' ) {
    return gbuf_a;
  } else if ( str[0] == 'b' && str[1] == '\0' ) {
    return gbuf_b;
  }


  for( const auto sa : dev_addrs ) {
    if( strcmp( sa.name, str ) == 0 ) {
      return (char*)sa.addr;
    }
  };

  char *eptr;
  char *addr = (char*)( strtoul( str, &eptr, 0 ) );
  if( *eptr == '\0' ) {
    return addr;
  }
  return (char*)(BAD_ADDR);
}



void dump8( const void *addr, unsigned n, bool isAbs  )
{
  auto ad = (unsigned const char*)(addr);
  if( !ad  ||  ad == BAD_ADDR ) {
    return;
  }
  decltype(ad) ad0 = isAbs ? ad : nullptr; // left label
  std_out << NL;

  unsigned nr = (n+15) >> 4; // non-full rows counting too
  for( decltype(+nr) row = 0; row < nr && !break_flag; ++row, ad0 += 16 ) {
    std_out << HexInt( (void*)ad0 ) << ": ";
    unsigned bs = row << 4;
    for( unsigned i=0; i<16 && (i+bs)<n; ++i ) {
      std_out << HexInt8( ad[i+bs] ) << ' ';
      if( (i&3) == 3 ) {
        std_out << ": ";
      }
    }

    std_out << "|  ";
    for( unsigned i=0; i<16 && (i+bs)<n; ++i ) {
      char b = '.';
      char c = ad[i+bs];
      if( c >= ' ' && c < '\x7F' ) {
        b = c;
      }
      std_out.append( b );
      if( (i&3) == 3 ) {
        std_out << ' ';
      }
    }
    std_out << NL;
  }

  std_out << "--------------------------------------" NL;
}

void dump32( const void *addr, unsigned n, bool isAbs  )
{
  auto ad = (const uint32_t *)(addr);
  if( !ad  ||  ad == BAD_ADDR ) {
    return;
  }
  decltype(ad) ad0 = isAbs ? ad : nullptr; // left label
  std_out << NL;

  unsigned nr = (n+15) >> 4; // non-full rows counting too
  for( decltype(+nr) row = 0; row < nr && !break_flag; ++row, ad0 += 4 ) {
    std_out << HexInt( (void*)ad0 ) << ": ";
    unsigned bs = row << 2;
    for( unsigned i=0; i<4; ++i ) {
      std_out << HexInt( ad[i+bs] ) << ' ';
      if( (i&3) == 3 ) {
        std_out << ": ";
      }
    }
    std_out << NL;
  }

  std_out << "--------------------------------------" NL;
}


char* log_buf = gbuf_b;
unsigned log_buf_size = GBUF_SZ;
unsigned log_buf_idx = 0;

void set_log_buf( char *buf, unsigned buf_sz )
{
  if( buf == nullptr ) { // default values
    log_buf = gbuf_b;
    log_buf_size = GBUF_SZ;
  } else {
    log_buf = buf;
    log_buf_size = buf_sz;
  }
  log_buf_idx = 0;
}

// TODO: mutex
void log_add( const char *s )
{
  if( !s ) {
    return;
  }

  while( *s !=0  && log_buf_idx < log_buf_size-1 ) {
    log_buf[log_buf_idx++] = *s++;
  }
  log_buf[log_buf_idx] = '\0'; // not++
}

void log_add_hex( uint32_t v )
{
  //              0123456789ABCDEF
  char s[16]; // ' 0x01234567 '
  s[0] = ' '; s[1] = '0'; s[2] = 'x';
  word2hex( v, s+3 );
  s[11] = ' ';
  s[12] = '\0';
  log_add_bin( s, 12 );
}

void log_add_bin( const char *s, uint16_t len )
{
  if( !s ) {
    return;
  }

  for( uint16_t i=0;  i<len && log_buf_idx < log_buf_size-1; ++i ) {
    log_buf[log_buf_idx++] = *s++;
  }
  log_buf[log_buf_idx] = '\0'; // not++
}

void log_reset()
{
  log_buf_idx = 0;
  log_buf[0] = '\0';
}

void log_print()
{
  if( log_buf_idx > 0 ) {
    std_out <<  log_buf <<  NL <<  "log_buf_idx " <<  log_buf_idx << NL;
    delay_ms( 100 );
  }
}

void print_user_var( int idx )
{
  if( idx < 0  ||  idx >= (int)N_USER_VARS ) {
    std_out << NL "err: bad var index: " << idx;
    return;
  }
  std_out << "#> " << (char)( 'a' + idx ) << " = "  << HexInt( user_vars[idx], true ) << " = "  << ( user_vars[idx] ) << NL;
}

void test_delays_misc( int n, uint32_t t_step, int tp )
{
  static const char *const f_nm[] {
      "delay_ms"             , // 0
      "delay_ms_brk"         , // 1
      "delay_ms_until_brk"   , // 2
      "HAL_Delay"            , // 3
      "delay_ms_until_brk_ex", // 4
      "delay_bad_ms"         , // 5
      "?6"                   , // 6
      "?7"                   , // 7
      "?8"                   , // 8
      "delay_ms-noirq"       , // 9
      "?a"                     // 10
  };
  delay_ms( 10 );
  tp = std::clamp( tp, 0, int(std::size(f_nm)-1) );

  uint32_t tm0 = HAL_GetTick();

  TickType tc0 = GET_OS_TICK(), tc00 = tc0;

  std_out << "#i t_os t_hal dt_os_hal dt_hal " << f_nm[tp] << NL;

  uint32_t tmc_prev = tc0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    TickType tcc = GET_OS_TICK();
    uint32_t tmc = HAL_GetTick();
    std_out << i << ' ' << ( tcc - tc00 )
            << ' ' << ( tmc - tm0 ) << ' ' << ( tmc - tmc_prev ) << ' ' << ( tcc - tmc ) << NL;

    if( UVAR('w') ) {
      std_out.flush();
    }

    leds.toggle( 1 );
    tmc_prev = tcc;

    switch( tp ) {
      case 0:  delay_ms( t_step );                break;
      case 1:  delay_ms_brk( t_step );             break;
      case 2:  delay_ms_until_brk( &tc0, t_step ); break;
      case 3:  HAL_Delay( t_step );                break;
      case 4:  delay_ms_until_brk_ex( nullptr, t_step, false ); break;
      case 5:  delay_bad_ms( t_step ); break;
      case 9:  at_disabled_irq( [t_step](){ delay_ms( t_step ); } );  break;
      default: break; // no delay ;-)
    }

  }
}

void test_output_rate( int n, int sl, int do_flush )
{
  const int max_len = 512;
  std_out << "# test_rate: n= " << n << " sl= " << sl << NL;
  char buf[max_len+4]; // for ends and align

  for( int i=0; i<sl-2; ++i ) {
    buf[i] = (char)( '@' + ( i & 0x3F ) );
  }
  buf[sl-2] = '\r';
  buf[sl-1] = '\n';
  buf[sl]   = '\0'; // sic, transfer sl bytes,

  uint32_t tm0 = HAL_GetTick();

  unsigned n_lines = 0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    buf[0] =  (char)( '@' + ( i & 0x3F ) );
    std_out << buf;
    if( do_flush ) {
      std_out.flush();
    }
    ++n_lines;
  }
  uint32_t tm1 = HAL_GetTick();
  delay_ms( 500 ); // settle
  unsigned dt = tm1 - tm0;
  unsigned n_ch = sl * n_lines;
  std_out << NL "dt= " << dt << " ms, chars: " << n_ch << "  lines: " << n_lines
          << "  cps: " << ( 1000*n_ch / dt ) << "  lps: " << ( 1000*n_lines / dt ) << NL;

}

const CmdInfo CMDINFO_TEST_RATE { "test_rate", 0, cmd_test_rate, "[ n [len [flush] ] ] - test output rate"  };

int cmd_test_rate( int argc, const char * const * argv )
{
  const int max_len = 512;
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int sl = arg2long_d( 2, argc, argv, 64, 0, max_len );
  int do_flush = arg2long_d( 3, argc, argv, 0, 0, 1 );
  test_output_rate( n, sl, do_flush );

  return 0;
}

int cmd_test_delays( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = arg2long_d( 2, argc, argv, UVAR('t'), 0, 100000 );
  int tp = arg2long_d( 3, argc, argv, 0, 0, 20 );
  std_out <<  "# test_delays : n= " << n << " t= " << t_step << " tp= " << tp << NL;

  test_delays_misc( n, t_step, tp );

  return 0;
}

const CmdInfo CMDINFO_TEST_DELAYS { "test_delays", '\0', cmd_test_delays, "[n] [step_ms] [type] - test delays [w=flush]"  };



//----------------------------------------------------------------------


#if  defined (STM32F0) || defined (STM32F1)

static const char *pin_moder_name[] = { "Inp", "O10", "O02", "O50", "?m?" };
static const char *pin_cr_i_name[]  = { "Ana", "Flt", "Pud", "xxx", "?c?" };
static const char *pin_cr_o_name[]  = { "OPP", "ODD", "APP", "AOD", "?c?" };

void gpio_pin_info( GPIO_TypeDef *gi, uint16_t pin, char *s )
{
  if( !gi || !s || pin >= PORT_BITS ) { return; }
  int j = 0;
  uint32_t cr = ( pin > 7 ) ? gi->CRH : gi->CRL;
  uint16_t p4 = ( pin & 7 ) << 2;

  uint16_t mod = ( cr >> p4 ) & 0x03;

  for( int i=0; i<3; ++i ) {
    s[j++] = pin_moder_name[mod][i];
  }
  s[j++] = '.';

  uint16_t cn = ( cr >> (p4+2) ) & 0x03;
  const char* pin_cname = ( mod == 0 ) ? ( pin_cr_i_name[cn] ) : ( pin_cr_o_name[cn] );
  for( int i=0; i<3; ++i ) {
    s[j++] = pin_cname[i];
  }

  s[j++] = ' ';
  s[j++] = '='; s[j++] = 'i';
  s[j++] = ( ( gi->IDR >> pin ) & 1 ) ? '1' : '0';
  s[j++] = ','; s[j++] = 'o';
  s[j++] = ( ( gi->ODR >> pin ) & 1 ) ? '1' : '0';
  s[j++] = 0;
}

#elif defined (STM32F2) || defined (STM32F3) || defined (STM32F4) || defined (STM32F7)  || defined (STM32H5) || defined (STM32H7) || defined (STM32G4)

static const char *pin_moder_name[] = { "Inp", "Out", "AFn", "Ana", "?m?" };
static const char *pin_speed_name[] = { "Low", "Lo1", "Med", "Hig", "?s?" };
static const char *pin_pupdr_name[] = { "No", "Up", "Dn", "Xx", "?p" };

void gpio_pin_info( GPIO_TypeDef *gi, uint16_t pin, char *s )
{
  if( !gi || !s || pin >= PORT_BITS ) { return; }
  int j = 0;
  uint16_t p2 = pin << 1;
  uint16_t mod = ( gi->MODER >> p2 ) & 0x03;
  for( int i=0; i<3; ++i ) {
    s[j++] = pin_moder_name[mod][i];
  }

  s[j++] = '.';
  if( ( gi->OTYPER >> pin ) & 1 ) {
    s[j++] = 'O'; s[j++] = 'D';
  } else {
    s[j++] = 'P'; s[j++] = 'P';
  }

  s[j++] = '.';
  uint16_t spe = ( gi->OSPEEDR >> p2 ) & 0x03;
  for( int i=0; i<3; ++i ) {
    s[j++] = pin_speed_name[spe][i];
  }

  s[j++] = '.';
  uint16_t pupd = ( gi->PUPDR >> p2 ) & 0x03;
  for( int i=0; i<2; ++i ) {
    s[j++] = pin_pupdr_name[pupd][i];
  }

  s[j++] = ':'; s[j++] = 'A'; s[j++] = 'F';
  uint32_t afnr = ( pin > 7 ) ? gi->AFR[1] : gi->AFR[0];
  uint16_t afn = ( afnr >> ( ( pin & 0x07 )<<2) ) & 0x0F;
  s[j++] = hex_digits[afn];

  s[j++] = '='; s[j++] = 'i';
  s[j++] = ( ( gi->IDR >> pin ) & 1 ) ? '1' : '0';
  s[j++] = ','; s[j++] = 'o';
  s[j++] = ( ( gi->ODR >> pin ) & 1 ) ? '1' : '0';

  s[j++] = 0;
}
#else
  #warning "Unknown MCU family"
#endif


//----------------------------------------------------------------------
// common commands
//
int cmd_info( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  std_out << NL "# **** " PROJ_NAME " **** " NL;

  std_out << "# HalVersion= " << HexInt( HAL_GetHalVersion() )
          << " REVID= "       << HexInt( HAL_GetREVID() )
          << " DEVID= "       << HexInt( HAL_GetDEVID() ) << NL;

  std_out << "# SYSCLK: " << HAL_RCC_GetSysClockFreq()
     << " HCLK: "  << HAL_RCC_GetHCLKFreq()
     << " PCLK1: " << HAL_RCC_GetPCLK1Freq()
     << " PCLK2: " << HAL_RCC_GetPCLK2Freq()
     << " HSE_VALUE: " << HSE_VALUE
     << " SystemCoreClock: " << SystemCoreClock << NL;

  std_out << "# errno= " << errno << " sigint_count= "  << sigint_count << " DCV= " << delay_calibrate_value << NL
     << "# dbg_val0= "  << dbg_val0 <<  " = "  << HexInt( dbg_val0, true )
     << " dbg_val1= " << dbg_val1 <<  " = "  << HexInt( dbg_val1, true ) << NL
     << "# dbg_val2= "  << dbg_val2 <<  " = "  << HexInt( dbg_val2, true )
     << " dbg_val3= " << dbg_val3 <<  " = "  << HexInt( dbg_val3, true ) << NL;

  std_out << "# _sdata= 0x" << HexInt( (uint32_t)(&_sdata) ) << " _edata= 0x"  << HexInt( (uint32_t)(&_edata)  )
     << " _sbss= 0x" << HexInt( (uint32_t)(&_sbss)  ) << " _ebss=  0x"  << HexInt( (uint32_t)(&_ebss)   )
     << " _end= 0x"  << HexInt( (uint32_t)(&_end)   ) << " _estack= 0x" << HexInt( (uint32_t)(&_estack) );

  uint32_t c_msp = __get_MSP(), c_psp = __get_PSP();
  std_out
    << NL "# MSP=   " << HexInt( c_msp, true ) <<  " PSP= " << HexInt( c_psp, true )
    << "  __heap_top=   " << HexInt( (uint32_t)__heap_top, true )
    << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) )
    << NL;

  uint32_t prio_grouping = HAL_NVIC_GetPriorityGrouping();
  std_out << "# prio_grouping= " << prio_grouping << NL;

  uint32_t prio_preempt, prio_sub;
  struct OutIrqName {
    IRQn_Type IRQn;
    const char* const nm;
  };
  const OutIrqName irqs[] = {
    { EXTI0_IRQn,   "EXTI0  " },
    { SysTick_IRQn, "SysTick" },
    #if defined( BOARD_UART_DEFAULT_IRQ )
    { BOARD_UART_DEFAULT_IRQ,  oxc_uart_name(BOARD_UART_DEFAULT) },
    #else
    { USART2_IRQn,  "USART2 " },
    #endif
    #ifdef TIM_EXA_IRQ
    { TIM_EXA_IRQ, TIM_EXA_STR },
    #endif
    // { OTG_FS_IRQn,  "OTG_FS " }, TODO: depend in MCU type
    #ifdef BOARD_SPI_DEFAULT_IRQ
    { BOARD_SPI_DEFAULT_IRQ,  BOARD_SPI_DEFAULT_NAME  },
    #endif
    { I2C1_EV_IRQn, "I2C1_EV" }
  };

  for( const auto& iqn : irqs ) {
    HAL_NVIC_GetPriority( iqn.IRQn, prio_grouping, &prio_preempt, &prio_sub );
    std_out << "# " << iqn.nm << " (" << iqn.IRQn <<  ")  preempt= " <<  prio_preempt << " sub= " <<  prio_sub << NL;
  }
  delay_ms( 50 );



  #if defined(USE_FREERTOS) && ( USE_FREERTOS != 0 )
    const char *nm = pcTaskGetName( 0 );
    std_out <<  "# task: \"" <<  nm << "\" tick_count: " << xTaskGetTickCount() << "  prty: " << uxTaskPriorityGet( 0 )
       << " highStackWaterMark= " << uxTaskGetStackHighWaterMark( 0 ) << NL;
  #endif
  errno = 0;
  return 0;
}
const CmdInfo CMDINFO_INFO {  "info",  0, cmd_info,       " - Output general info" };

int cmd_echo( int argc, const char * const * argv )
{
  std_out << NL;
  for( int i=1; i<argc; ++i ) {
    std_out << argv[i] << ' ';
  }
  std_out << NL;

  if( UVAR('d') < 1 ) {
    return 0;
  }

  std_out << "# argc= " << argc << NL;
  for( int i=0; i<argc; ++i ) {
    std_out << "# arg" << i << " = \"" << argv[i] << "\" " << strlen(argv[i]) << NL;
  }
  return 0;
}
const CmdInfo CMDINFO_ECHO { "echo",  0, cmd_echo,       " [args] - output args" };

const char* common_help_string = "Default help " NL;

int cmd_help( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  std_out << common_help_string;
  std_out << NL "commands:" NL;
  char b1[2]; b1[0] = b1[1] = 0;

  for( int i=0; global_cmds[i] && i<CMDS_NMAX; ++i ) {
    if( global_cmds[i]->name == 0 ) {
      break;
    }
    std_out << global_cmds[i]->name << ' ' << global_cmds[i]->hint << ' ';
    if( global_cmds[i]->acr != 0 ) {
      std_out << " (" << global_cmds[i]->acr << ')';
    }
    std_out << NL;
  }
  // see oxc_smallrl.cpp : SMLRL::SmallRL::handle_nl
  std_out <<  ".h - history " NL
     <<  ".v - more verbose " NL
     <<  ".q - no verbose " NL;
  return 0;
}
const CmdInfo CMDINFO_HELP { "help",  'h', cmd_help, " - List of commands and arguments"  };

int cmd_dump( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  const char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    std_out << "** error: dump: bad address \""  <<  argv[1] << "\"" NL;
    return 2;
  }

  int n = arg2long_d( 2, argc, argv, 1, 1, 0x8000 );
  int isAbs = arg2long_d( 3, argc, argv, 0, 0, 1 );

  std_out << NL "** dump: argc=" << argc << " addr=" << HexInt( (void*)addr ) << " n= " << n << NL;
  break_flag = 0;
  dump8( addr, n, isAbs );
  return 0;
}
const CmdInfo CMDINFO_DUMP { "hd",  0, cmd_dump, " {a|b|addr} [n] [abs:0:1]- HexDumps given area"  };

int cmd_dump32( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  const char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    std_out << "** error: dump: bad address \""  <<  argv[1] << "\"" NL;
    return 2;
  }

  int n = arg2long_d( 2, argc, argv, 1, 1, 0x8000 );
  int isAbs = arg2long_d( 3, argc, argv, 0, 0, 1 );

  std_out << NL "** dump32: argc=" << argc << " addr=" << HexInt( (void*)addr ) << " n= " << n << NL;
  break_flag = 0;
  dump32( addr, n, isAbs );
  return 0;
}
const CmdInfo CMDINFO_DUMP32 { "hd32",  0, cmd_dump32, " {a|b|addr} [n] [abs:0:1]- HexDumps given area as 32bit"  };


int cmd_fill( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    std_out << "** error: fill: bad address \"" << argv[1] << "\"" NL;
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

  int n = arg2long_d( 3, argc, argv, 1, 1, 0xFFFF );
  uint8_t stp = (uint8_t)arg2long_d( 4, argc, argv, 0, 0, 0xFF );

  std_out <<  "** fill: addr=" << HexInt( (void*)addr )
     << " v= " << v << " n= " << n << " stp= " << stp << NL;

  for( int i=0; i<n; ++i, ++addr ) {
    *addr = v; v+=stp;
  }
  std_out << NL "---------- done---------------" NL;
  return 0;
}
const CmdInfo CMDINFO_FILL { "fill",  0, cmd_fill, " {a|b|addr} val [n] [stp] - Fills memory by value"  };


int cmd_pvar( int argc, const char * const * argv )
{
  if( argc < 2 ) { // all
    for( unsigned i=0; i<N_USER_VARS; ++i ) {
      print_user_var( i );
    }
    if( print_var_hook != nullptr ) {
      print_var_hook( "", 0 );
    }
    return 0;
  }

  int fmt = 0;
  if( argc > 2 ) {
    fmt = strtol( argv[2], 0, 0 );
  }

  if( argv[1][1] != '\0' &&  print_var_hook != nullptr ) {
    return print_var_hook( argv[1], fmt ) ? 0: 2;
  }


  // build-in int vars with one-char name
  char c = argv[1][0];
  if( argv[1][1] != '\0' || c < 'a' || c > 'z' ) {
    std_out << "# Error: problem with name \"" << argv[1] << '"' << NL;
    return 2;
  }
  int idx = c - 'a';
  print_user_var( idx );
  return 0;
}
const CmdInfo CMDINFO_PVAR { "print", 'p', cmd_pvar, "name - print user var a-z"  };


int cmd_svar( int argc, const char * const * argv )
{
  if( argc != 3 ) {
    std_out << "# Error: bad number of arguments: s var value" << NL;
    return 1;
  }

  if( argv[1][1] == '\0' ) {
    unsigned idx = (unsigned)(argv[1][0]) - 'a';
    if( idx < N_USER_VARS ) {
      if( argv[2][0] == '-' ) {
        user_vars[idx] = strtol( argv[2], 0, 0 );
      } else {
        user_vars[idx] = strtoul( argv[2], 0, 0 );
      }
      print_user_var( idx );
      return 0;
    }
  }

  if( set_var_hook != nullptr  &&  set_var_hook( argv[1], argv[2] ) ) {
    return 0;
  }
  return 1;
}
const CmdInfo CMDINFO_SVAR { "set", 's', cmd_svar,  "name value - set var a-z"  };


[[ noreturn ]] int cmd_die( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 0, 0, 0xFF );
  die4led( v );
}
const CmdInfo CMDINFO_DIE { "die",    0,  cmd_die, " [val] - die with value"  };

int cmd_reboot( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG)
{
  NVIC_SystemReset();
  return 0; // never ;-)
}
const CmdInfo CMDINFO_REBOOT { "reboot", 0,  cmd_reboot,     " reboot system"  };

int cmd_log_print( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  log_print();
  return 0;
}
const CmdInfo CMDINFO_LOG_PRINT { "lp", 0,  cmd_log_print, "  - print log buffer"  };

int cmd_log_reset( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  log_reset();
  return 0;
}
const CmdInfo CMDINFO_LOG_RESET { "lr",     0,  cmd_log_reset,  "  - reset log buffer"  };

int cmd_pin_info( int argc, const char * const * argv )
{
  char s[32];
  uint16_t pin = arg2long_d( 2, argc, argv, 0, 0, 15 );
  uint16_t n   = arg2long_d( 3, argc, argv, 1, 1, 16 );
  char pstr[2] = "A";
  GPIO_TypeDef *gi = GPIOA;
  if( argc > 1 ) {
    switch( argv[1][0] ) {
      case 'B': case 'b':  gi = GPIOB; pstr[0] = 'B'; break;
      case 'C': case 'c':  gi = GPIOC; pstr[0] = 'C'; break;
      case 'D': case 'd':  gi = GPIOD; pstr[0] = 'D'; break;
      #ifdef GPIOE
      case 'E': case 'e':  gi = GPIOE; pstr[0] = 'E'; break;
      #endif
      #ifdef GPIOF
      case 'F': case 'f':  gi = GPIOF; pstr[0] = 'F'; break;
      #endif
      #ifdef GPIOG
      case 'G': case 'g':  gi = GPIOG; pstr[0] = 'G'; break;
      #endif
      #ifdef GPIOI
      case 'I': case 'i':  gi = GPIOI; pstr[0] = 'I'; break;
      #endif
      #ifdef GPIOH
      case 'H': case 'h':  gi = GPIOH; pstr[0] = 'H'; break;
      #endif
    }
  }

  std_out << NL "Port " << pstr << " addr: " << HexInt( (void*)gi ) << NL;

  for( uint16_t p = pin, i=0; p<16 && i<n; ++p, ++i ) {
    gpio_pin_info( gi, p, s );
    std_out << " pin: " <<  p << ": " << s << NL;
  }
  std_out.flush();
  dump8( gi, (sizeof(*gi)+15) & 0xF0 );

  return 0;
}
const CmdInfo CMDINFO_PIN_INFO { "pinfo",  0, cmd_pin_info,       " [A-I] [0-15] - info about pin" };

int cmd_set_leds_step( int argc, const char * const * argv )
{
  uint32_t nstep = arg2long_d( 1, argc, argv, 50, 1, 100000 ); // number output series
  task_leds_step = nstep;
  std_out << "LEDS step is set to " << task_leds_step << " = "  << task_leds_step * TASK_LEDS_QUANT << " ms" NL;
  return 0;
}
const CmdInfo CMDINFO_LSTEP { "leds_step", 0, cmd_set_leds_step, " [N] - set leds step in 10 ms "  };

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc
