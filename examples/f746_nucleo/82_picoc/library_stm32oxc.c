#include <interpreter.h>
#include <math.h>
// #include <oxc_auto.h> // C++ required

void stm32oxc_SetupFunc(Picoc *pc);
void stm32oxc_SetupFunc(Picoc *pc)
{
}

void C_test (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_test (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  printf("test(%d)\n", Param[0]->Val->Integer);
  Param[0]->Val->Integer = 1234;
}

void C_lineno (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_lineno (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = Parser->Line;
}

void C_sin( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_sin( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  ReturnValue->Val->FP = sin( Param[0]->Val->FP );
}

void C_cos( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_cos( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  ReturnValue->Val->FP = cos( Param[0]->Val->FP );
}

void C_tan( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_tan( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  ReturnValue->Val->FP = tan( Param[0]->Val->FP );
}

/* list of all library functions and their prototypes */
struct LibraryFunction stm32oxc_Functions[] =
{
  { C_test,        "void test(int);" },
  { C_lineno,      "int lineno();" },
  { C_sin,         "double sin(double);" },
  { C_cos,         "double cos(double);" },
  { C_tan,         "double tan(double);" },
  { NULL,          NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
  IncludeRegister(pc, "picoc_stmoxc.h", &stm32oxc_SetupFunc, &stm32oxc_Functions[0], NULL);
}
