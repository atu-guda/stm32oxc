#include <interpreter.h>
// TODO: split
#define __USE_GNU
#define __USE_MISC
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
// #include <oxc_auto.h> // C++ required

#pragma GCC diagnostic ignored "-Wunused-parameter"

#define FUN_XX_HEAD(nm) \
  void C_ ## nm( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs); \
  void C_ ## nm( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)

#define FUN_D2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->FP = nm( Param[0]->Val->FP ); \
  }

#define REG_FUN_D2D(nm) \
  { C_ ## nm,         "double " #nm "(double);" }


#define FUN_DD2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->FP = nm( Param[0]->Val->FP, Param[1]->Val->FP ); \
  }

#define REG_FUN_DD2D(nm) \
  { C_ ## nm,         "double " #nm "(double,double);" }


void stm32oxc_SetupFunc( Picoc *pc );
void stm32oxc_SetupFunc( Picoc *pc )
{
}

void C_lineno (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_lineno (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = Parser->Line;
}

const double C_math_M_E        = 2.7182818284590452354;  /* e */
const double C_math_M_LOG2E    = 1.4426950408889634074;  /* log_2 e */
const double C_math_M_LOG10E   = 0.43429448190325182765; /* log_10 e */
const double C_math_M_LN2      = 0.69314718055994530942; /* log_e 2 */
const double C_math_M_LN10     = 2.30258509299404568402; /* log_e 10 */
const double C_math_M_PI       = 3.14159265358979323846; /* pi */
const double C_math_M_PI_2     = 1.57079632679489661923; /* pi/2 */
const double C_math_M_PI_4     = 0.78539816339744830962; /* pi/4 */
const double C_math_M_1_PI     = 0.31830988618379067154; /* 1/pi */
const double C_math_M_2_PI     = 0.63661977236758134308; /* 2/pi */
const double C_math_M_2_SQRTPI = 1.12837916709551257390; /* 2/sqrt(pi) */
const double C_math_M_SQRT2    = 1.41421356237309504880; /* sqrt(2) */
const double C_math_M_SQRT1_2  = 0.70710678118654752440; /* 1/sqrt(2) */

FUN_D2D(acos);
FUN_D2D(asin);
FUN_D2D(atan);
FUN_DD2D(atan2);
FUN_D2D(cbrt);
FUN_D2D(ceil);
FUN_D2D(cos);
FUN_D2D(erf);
FUN_D2D(exp);
FUN_D2D(exp2);
FUN_D2D(expm1);
FUN_D2D(fabs);
FUN_DD2D(fdim);
FUN_D2D(floor); // +fma
FUN_DD2D(fmax);
FUN_DD2D(fmin);
FUN_DD2D(fmod);
FUN_DD2D(hypot);
FUN_D2D(log);
FUN_D2D(log10);
FUN_D2D(log2);
FUN_DD2D(pow);
FUN_D2D(rint);
FUN_D2D(round);
FUN_D2D(sin);
FUN_D2D(sqrt); // + sqrt0
FUN_D2D(tan);
FUN_D2D(trunc);


void C_xprintf (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_xprintf (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
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
  REG_FUN_D2D(acos),
  REG_FUN_D2D(asin),
  REG_FUN_D2D(atan),
  REG_FUN_DD2D(atan2),
  REG_FUN_D2D(cbrt),
  REG_FUN_D2D(ceil),
  REG_FUN_D2D(cos),
  REG_FUN_D2D(erf),
  REG_FUN_D2D(exp),
  REG_FUN_D2D(exp2),
  REG_FUN_D2D(expm1),
  REG_FUN_D2D(fabs),
  REG_FUN_DD2D(fdim),
  REG_FUN_D2D(floor), // +fma
  REG_FUN_DD2D(fmax),
  REG_FUN_DD2D(fmin),
  REG_FUN_DD2D(fmod),
  REG_FUN_DD2D(hypot),
  REG_FUN_D2D(log),
  REG_FUN_D2D(log10),
  REG_FUN_D2D(log2),
  REG_FUN_DD2D(pow),
  REG_FUN_D2D(rint),
  REG_FUN_D2D(round),
  REG_FUN_D2D(sin),
  REG_FUN_D2D(sqrt), // + sqrt0
  REG_FUN_D2D(tan),
  REG_FUN_D2D(trunc),
  { C_lineno,      "int lineno();"       },
  { C_xprintf,      "int xprintf(char*,...);"       },
  { NULL,          NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
  VariableDefinePlatformVar( pc, NULL, "M_E"       , &pc->FPType, (union AnyValue *)&C_math_M_E       , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_LOG2E"   , &pc->FPType, (union AnyValue *)&C_math_M_LOG2E   , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_LOG10E"  , &pc->FPType, (union AnyValue *)&C_math_M_LOG10E  , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_LN2"     , &pc->FPType, (union AnyValue *)&C_math_M_LN2     , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_LN10"    , &pc->FPType, (union AnyValue *)&C_math_M_LN10    , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_PI"      , &pc->FPType, (union AnyValue *)&C_math_M_PI      , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_PI_2"    , &pc->FPType, (union AnyValue *)&C_math_M_PI_2    , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_PI_4"    , &pc->FPType, (union AnyValue *)&C_math_M_PI_4    , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_1_PI"    , &pc->FPType, (union AnyValue *)&C_math_M_1_PI    , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_2_PI"    , &pc->FPType, (union AnyValue *)&C_math_M_2_PI    , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_2_SQRTPI", &pc->FPType, (union AnyValue *)&C_math_M_2_SQRTPI, FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_SQRT2"   , &pc->FPType, (union AnyValue *)&C_math_M_SQRT2   , FALSE );
  VariableDefinePlatformVar( pc, NULL, "M_SQRT1_2" , &pc->FPType, (union AnyValue *)&C_math_M_SQRT1_2 , FALSE );

  IncludeRegister( pc, "picoc_stmoxc.h", &stm32oxc_SetupFunc, &stm32oxc_Functions[0], NULL);
}

