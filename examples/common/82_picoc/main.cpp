#include <oxc_auto.h>
#include <oxc_main.h>

#include <stdarg.h>

#include <oxc_picoc.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test picoc interpratator ;cmd" NL;

#define PICOC_STACK_SIZE (32*1024)
int picoc_cmdline_handler( char *s );
Picoc pc;
int init_picoc( Picoc *ppc );
// TMP here: move to system include
extern "C" {
void oxc_picoc_math_init( Picoc *pc );
}
double d_arr[4] = { 1.234, 9.87654321e-10, 5.432198765e12, 1.23456789e-100 };
double *d_ptr = d_arr;
char a_char[] = "ABCDE";
char *p_char = a_char;
void oxc_picoc_misc_init( Picoc *pc );

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test something 0"  );
DCL_CMD_REG( init_picoc, 'I', " - init picoc interpretator"  );


void idle_main_task()
{
  leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('a') =  42;
  UVAR('b') =  17;
  UVAR('t') = 100;
  UVAR('n') =  20;
  UVAR('z') = 123;

  cmdline_handlers[0] = picoc_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  pc.InteractiveHead = nullptr;
  init_picoc( &pc );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int a = arg2long_d( 1, argc, argv,   2,    0, 127 );
  std_out << "# Test0: a= " << a << " sz:va_list= " << sizeof(va_list) << NL;

  unsigned tsize = pc.GlobalTable.Size;
  std_out << "# &pc= " << HexInt(&pc) << " size= " << tsize << " OnHeap= " << pc.GlobalTable.OnHeap << NL;

  Table *gtab = &pc.GlobalTable;
  TableEntry **ppte = gtab->HashTable;
  std_out << "# ppte= " << HexInt(ppte) << NL;

  for( unsigned hi=0; hi<tsize; ++hi ) {
    for( TableEntry* te = gtab->HashTable[hi]; te != nullptr; te = te->Next ) {
      std_out << "# hi= " << hi << " key= \"" << te->p.v.Key << "\"" << " file= \"" << te->DeclFileName;
      Value *v = te->p.v.Val;
      if( v ) {
        std_out << "\" typ= " << v->Typ->Base;
      }
      std_out  << NL;
    }
  }


  return 0;
}

int picoc_cmdline_handler( char *s )
{
  // static int nnn = 0;

  if( !s  ||  s[0] != ';' ) { // not my
    return -1;
  }

  const char *cmd = s + 1;
  std_out << NL "# C: cmd= \"" << cmd << '"' << NL;
  delay_ms( 10 );
  int ep_rc =  PicocPlatformSetExitPoint( &pc );
  if( ep_rc == 0 ) {
    PicocParse( &pc, "cmd", cmd, strlen(cmd), TRUE, TRUE, FALSE, TRUE );
  } else {
    std_out << "## Exit point: " << ep_rc << NL;
  }

  int rc = 0;

  return rc;

}

int init_picoc( Picoc *ppc )
{
  if( ppc->InteractiveHead != nullptr ) {
    PicocCleanup( ppc );
  }
  PicocInitialise( ppc, PICOC_STACK_SIZE );
  oxc_picoc_math_init( ppc );
  oxc_picoc_misc_init( ppc );
  PicocIncludeAllSystemHeaders( ppc );
  VariableDefinePlatformVar( ppc, nullptr, "__a",         &(ppc->IntType), (union AnyValue *)&(UVAR('a')), TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "d_arr",      ppc->FPArrayType, (union AnyValue *)d_arr,        TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "d_ptr",        ppc->FPPtrType, (union AnyValue *)&d_ptr,       TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "a_char",   ppc->CharArrayType, (union AnyValue *)a_char,       TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "p_char",     ppc->CharPtrType, (union AnyValue *)&p_char,      TRUE );
  return 0;
}

int cmd_init_picoc( int argc, const char * const * argv )
{
  init_picoc( &pc );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

