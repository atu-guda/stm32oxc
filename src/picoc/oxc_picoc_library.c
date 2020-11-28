#include <oxc_picoc_interpreter.h>
// TODO: split
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
// #include <oxc_auto.h> // C++ required

#include <oxc_picoc_reghelpers.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"


void stm32oxc_SetupFunc( Picoc *pc );
void stm32oxc_SetupFunc( Picoc *pc )
{
}

void C_lineno( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lineno( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  ReturnValue->Val->Integer = Parser->Line;
}



void C_xprintf( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_xprintf( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  printf( "# dbg: NumArgs= %d\n", NumArgs );
  struct Value *na = Param[0];
  for( int i=0; i<NumArgs; ++i ) {
    printf( "# dbg: i= %d tp= %d size= %d \n", i, na->Typ->Base, na->Typ->Sizeof  );
    na = (struct Value *)( (char *)na + MEM_ALIGN(sizeof(struct Value) + TypeStackSizeValue(na)) );
  }
  ReturnValue->Val->Integer = printf( Param[0]->Val->Pointer );
  printf( "## dbg: \n" );
}



/* list of all library functions and their prototypes */
struct LibraryFunction stm32oxc_Functions[] =
{
  { C_lineno,      "int lineno();"       },
  { C_xprintf,      "int xprintf(char*,...);"       },
  { NULL,          NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
  IncludeRegister( pc, "picoc_stmoxc.h", &stm32oxc_SetupFunc, &stm32oxc_Functions[0], NULL);
}

