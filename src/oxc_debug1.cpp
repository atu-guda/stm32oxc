#include <stdlib.h>
#include <errno.h>

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
const char* (*get_var_name_hook)( unsigned i ) = nullptr;

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
  char *eptr;
  char *addr = (char*)( strtoul( str, &eptr, 0 ) );
  if( *eptr == '\0' ) {
    return addr;
  }
  return (char*)(BAD_ADDR);
}



void dump8( const void *addr, int n, bool isAbs  )
{
  unsigned const char* ad = (unsigned const char*)(addr);
  if( !ad  ||  ad == BAD_ADDR ) {
    return;
  }
  unsigned const char* ad0 = isAbs ? ad : nullptr; // left label
  STDOUT_os;
  os << NL;

  int i, row, bs;
  int nr = (n+15) >> 4; // non-full rows counting too
  for( row = 0; row < nr; ++row ) {
    os << HexInt( (void*)ad0 ) << ": ";
    bs = row << 4;
    for( i=0; i<16 && (i+bs)<n; ++i ) {
      os << HexInt8( ad[i+bs] ) << ' ';
      if( (i&3) == 3 ) {
        os << ": ";
      }
    }

    os << "|  ";
    char b;
    for( i=0; i<16 && (i+bs)<n; ++i ) {
      b = '.';
      if( ad[i+bs] >= ' ' ) {
        b = ad[i+bs];
      }
      os.append( b );
      if( (i&3) == 3 ) {
        os << ' ';
      }
    }
    os << NL;
    os.flush();
    ad0 += 16;

  }

  os << "--------------------------------------" NL;
  os.flush();
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
    STDOUT_os;
    os <<  gbuf_b <<  NL <<  "log_buf_idx " <<  log_buf_idx << NL;
    delay_ms( 100 );
  }
}

void print_user_var( int idx )
{
  STDOUT_os;
  if( idx < 0  ||  idx >= (int)N_USER_VARS ) {
    os << NL "err: bad var index: " << idx;
    return;
  }
  char b[] = "0 = ";
  b[0] = (char)( 'a' + idx );
  os << b << ( user_vars[idx] ) << " = "  << HexInt( user_vars[idx], true ) << NL;
}

//----------------------------------------------------------------------

static const char *pin_moder_name[] = { "Inp", "Out", "AFn", "Ana", "?m?" };
static const char *pin_speed_name[] = { "Low", "Lo1", "Med", "Hig", "?s?" };
static const char *pin_pupdr_name[] = { "No", "Up", "Dn", "Xx", "?p" };

#if  defined (STM32F0) || defined (STM32F1)
void gpio_pin_info( GPIO_TypeDef *gi, uint16_t pin, char *s )
{
  if( !gi || !s || pin >= PORT_BITS ) { return; }
  s[0] = '?'; s[1] = 0;
}
#elif defined (STM32F2) || defined (STM32F3) || defined (STM32F4) || defined (STM32F7)
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
  STDOUT_os;
  os << NL "**** " PROJ_NAME " **** " NL;

  os << "SYSCLK: " << HAL_RCC_GetSysClockFreq()
     << " HCLK: "  << HAL_RCC_GetHCLKFreq()
     << " PCLK1: " << HAL_RCC_GetPCLK1Freq()
     << " PCLK2: " << HAL_RCC_GetPCLK2Freq()
     << " HSE_VALUE: " << HSE_VALUE
     << " SystemCoreClock: " << SystemCoreClock << NL;

  os << "errno= " << errno << " sigint_count="  << sigint_count << NL
     << "dbg_val0= "  << dbg_val0 <<  " = "  << HexInt( dbg_val0, true )
     << " dbg_val1= " << dbg_val1 <<  " = "  << HexInt( dbg_val1, true ) << NL
     << "dbg_val2= "  << dbg_val2 <<  " = "  << HexInt( dbg_val2, true )
     << " dbg_val3= " << dbg_val3 <<  " = "  << HexInt( dbg_val3, true ) << NL;

  os << "_sdata= 0x" << HexInt( (uint32_t)(&_sdata) ) << " _edata= 0x"  << HexInt( (uint32_t)(&_edata)  )
     << " _sbss= 0x" << HexInt( (uint32_t)(&_sbss)  ) << " _ebss=  0x"  << HexInt( (uint32_t)(&_ebss)   )
     << " _end= 0x"  << HexInt( (uint32_t)(&_end)   ) << " _estack= 0x" << HexInt( (uint32_t)(&_estack) );

  uint32_t c_msp = __get_MSP(), c_psp = __get_PSP();
  os << NL "MSP=   " << HexInt( c_msp, true ) <<  " PSP= " << HexInt( c_psp, true )
     << "  __heap_top=   " << HexInt( (uint32_t)__heap_top, true )
     << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) )
     << NL;

  uint32_t prio_grouping = HAL_NVIC_GetPriorityGrouping();
  os << "prio_grouping= " << prio_grouping << NL;

  uint32_t prio_preempt, prio_sub;
  struct OutIrqName {
    IRQn_Type IRQn;
    const char* nm;
  };
  const OutIrqName irqs[] = {
    { SysTick_IRQn, "SysTick" },
    { EXTI0_IRQn,   "EXTI0  " },
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

  for( auto iqn : irqs ) {
    HAL_NVIC_GetPriority( iqn.IRQn, prio_grouping, &prio_preempt, &prio_sub );
    os << iqn.nm << " (" << iqn.IRQn <<  ")  preempt= " <<  prio_preempt << " sub= " <<  prio_sub << NL;
  }
  delay_ms( 50 );



  #if defined(USE_FREERTOS) && ( USE_FREERTOS != 0 )
    const char *nm = pcTaskGetName( 0 );
    os <<  "task: \"" <<  nm << "\" tick_count: " << xTaskGetTickCount() << "  prty: " << uxTaskPriorityGet( 0 )
       << " highStackWaterMark= " << uxTaskGetStackHighWaterMark( 0 ) << NL;
  #endif
  errno = 0;
  return 0;
}
CmdInfo CMDINFO_INFO {  "info",  0, cmd_info,       " - Output general info" };

int cmd_echo( int argc, const char * const * argv )
{
  STDOUT_os;
  os << NL << "argc= " << argc << NL;
  for( int i=0; i<argc; ++i ) {
    os << " arg" << i << " = \"" << argv[i] << "\"" NL;
  }
  return 0;
}
CmdInfo CMDINFO_ECHO { "echo",  0, cmd_echo,       " [args] - output args" };

const char* common_help_string = "Default help " NL;

int cmd_help( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  STDOUT_os;
  os << common_help_string;
  os << NL "commands:" NL;
  char b1[2]; b1[0] = b1[1] = 0;

  for( int i=0; global_cmds[i] && i<CMDS_NMAX; ++i ) {
    if( global_cmds[i]->name == 0 ) {
      break;
    }
    os << global_cmds[i]->name << ' ' << global_cmds[i]->hint << ' ';
    if( global_cmds[i]->acr != 0 ) {
      os << " (" << global_cmds[i]->acr << ')';
    }
    os << NL;
  }
  // see oxc_smallrl.cpp : SMLRL::SmallRL::handle_nl
  os <<  ".h - history " NL
     <<  ".v - more verbose " NL
     <<  ".q - no verbose " NL;
  return 0;
}
CmdInfo CMDINFO_HELP { "help",  'h', cmd_help, " - List of commands and arguments"  };

int cmd_dump( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  STDOUT_os;
  const char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    os << "** error: dump: bad address \""  <<  argv[1] << "\"" NL;
    return 2;
  }

  int n = arg2long_d( 2, argc, argv, 1, 1, 0x8000 );
  int isAbs = arg2long_d( 3, argc, argv, 0, 0, 1 );

  os << NL "** dump: argc=" << argc << " addr=" << HexInt( (void*)addr ) << " n= " << n << NL;
  dump8( addr, n, isAbs );
  return 0;
}
CmdInfo CMDINFO_DUMP { "hd",  0, cmd_dump, " {a|b|addr} [n] [abs:0:1]- HexDumps given area"  };

int cmd_fill( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    return 1;
  }

  STDOUT_os;
  char* addr = str2addr( argv[1] );
  if( addr == BAD_ADDR ) {
    os << "** error: fill: bad address \"" << argv[1] << "\"" NL;
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

  os <<  "** fill: addr=" << HexInt( (void*)addr )
     << " v= " << v << " n= " << n << " stp= " << stp << NL;

  for( int i=0; i<n; ++i, ++addr ) {
    *addr = v; v+=stp;
  }
  os << NL "---------- done---------------" NL;
  return 0;
}
CmdInfo CMDINFO_FILL { "fill",  0, cmd_fill, " {a|b|addr} val [n] [stp] - Fills memory by value"  };


int cmd_pvar( int argc, const char * const * argv )
{
  STDOUT_os;

  if( argc < 2 ) { // all
    for( unsigned i=0; i<N_USER_VARS; ++i ) {
      print_user_var( i );
    }
    if( get_var_name_hook != nullptr  && print_var_hook != nullptr ) {
      for( unsigned i=0; ; ++i ) {
        auto s = get_var_name_hook( i );
        if( !s ) {
          break;
        }
        print_var_hook( s, 0 );
      }
    }
    return 0;
  }

  int fmt = 0;
  if( argc > 2 ) {
    fmt = strtol( argv[2], 0, 0 );
  }

  if( print_var_hook != nullptr && print_var_hook( argv[1], fmt ) ) {
    return 0;
  }


  // build-in int vars with one-char named
  char c = argv[1][0];
  if( argv[1][1] != '\0' || c < 'a' || c > 'z' ) {
    os << "# Error: problem with name \"" << argv[1] << '"' << NL;
    return 2;
  }
  int idx = c - 'a';
  print_user_var( idx );
  return 0;
}
CmdInfo CMDINFO_PVAR { "print", 'p', cmd_pvar, "name - print user var a-z"  };


int cmd_svar( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc != 3 ) {
    os << "# Error: bad number of arguments: s var value" << NL;
    return 1;
  }

  if( set_var_hook != nullptr  &&  set_var_hook( argv[1], argv[2] ) ) {
    return 0;
  }

  int idx = argv[1][0] - 'a';
  if( idx < 0 || idx >= (int)N_USER_VARS || argv[1][1] != '\0' ) {
    os << "# Error: problem with name \"" << argv[1] << '"' << NL;
    return 2;
  }

  if( argv[2][0] == '-' ) {
    user_vars[idx] = strtol( argv[2], 0, 0 );
  } else {
    user_vars[idx] = strtoul( argv[2], 0, 0 );
  }
  print_user_var( idx );
  return 0;
}
CmdInfo CMDINFO_SVAR { "set", 's', cmd_svar,  "name value - set var a-z"  };


[[ noreturn ]] int cmd_die( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 0, 0, 0xFF );
  die4led( v );
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

  STDOUT_os;
  os << NL "Port " << pstr << " addr: " << HexInt( (void*)gi ) << NL;

  for( uint16_t p = pin, i=0; p<16 && i<n; ++p, ++i ) {
    gpio_pin_info( gi, p, s );
    os << " pin: " <<  p << ": " << s << NL;
  }
  os.flush();
  dump8( gi, (sizeof(*gi)+15) & 0xF0 );

  return 0;
}
CmdInfo CMDINFO_PIN_INFO { "pinfo",  0, cmd_pin_info,       " [A-I] [0-15] - info about pin" };

int cmd_set_leds_step( int argc, const char * const * argv )
{
  uint32_t nstep = arg2long_d( 1, argc, argv, 50, 1, 100000 ); // number output series
  task_leds_step = nstep;
  STDOUT_os;
  os << "LEDS step is set to " << task_leds_step << " = "  << task_leds_step * TASK_LEDS_QUANT << " ms" NL;
  return 0;
}
CmdInfo CMDINFO_LSTEP { "leds_step", 0, cmd_set_leds_step, " [N] - set leds step in 10 ms "  };

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc
