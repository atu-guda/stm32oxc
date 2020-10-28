#include <interpreter.h>

// #include <math.h>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_picoc_reghelpers.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"


void C_delay_ms( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_delay_ms( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  delay_ms( Param[0]->Val->Integer );
}

void C_delay_ms_brk( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_delay_ms_brk( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  int rc = delay_ms_brk( Param[0]->Val->Integer );
  ReturnValue->Val->Integer = rc;
}

void C_leds_set( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_leds_set( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  leds.set( Param[0]->Val->Integer );
}

void C_leds_reset( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_leds_reset( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  leds.reset( Param[0]->Val->Integer );
}

void C_leds_toggle( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_leds_toggle( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  leds.toggle( Param[0]->Val->Integer );
}

void C_pr_i( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_i( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  std_out << ( Param[0]->Val->Integer ) << ' ';
}

void C_pr_ih( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_ih( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  std_out << HexInt( Param[0]->Val->Integer ) << ' ';
}

void C_pr_i_l( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_i_l( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  struct Value *na = Param[0];
  for( int i=0; i<NumArgs; ++i ) {
    std_out << ( na->Val->Integer ) << ' ';
    na = (struct Value *)( (char *)na + MEM_ALIGN(sizeof(struct Value) + TypeStackSizeValue(na)) );
  }
}

void C_pr_i_n( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_i_n( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  int n  = Param[1]->Val->Integer;
  int *v = (int*)( Param[0]->Val->Pointer );
  // std_out << "## n= " << n << " v= " << HexInt((void*)(v)) << NL;
  for( int i=0; i<n; ++i ) {
    std_out << ( v[i] ) << ' ';
  }
}


void C_pr_c( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_c( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  std_out << char( Param[0]->Val->Integer );
}

void C_pr_s( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_s( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  std_out << (const char*)( Param[0]->Val->Pointer );
}

void C_pr_d( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr_d( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  std_out << (double)( Param[0]->Val->FP );
}

void C_pr( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_pr( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  struct Value *na = Param[0];
  char sep =  (char)( na->Val->Integer );
  for( int i=1; i<NumArgs; ++i ) {
    na = (struct Value *)( (char *)na + MEM_ALIGN(sizeof(struct Value) + TypeStackSizeValue(na)) );
    std_out << "na= " << HexInt( na ) << " Typ= " << HexInt( na->Typ );
    delay_ms( 100 );
    if( na->Typ ) {
       std_out << " Typ->Base= " << HexInt( na->Typ->Base );
    }
    delay_ms( 100 );
    std_out << " Val= " << HexInt( na->Val )
            << " Val->Typ=" << HexInt(na->Val->Typ)
            << " int= " << ( na->Val->Integer ) << ' ';
    delay_ms( 100 );
    // if( (uint32_t)(na->Val->Typ) > 0x08000000 &&  (uint32_t)(na->Val->Typ) < 0x30000000 ) { // real mem: flash or ram
    //   std_out << " Val->Typ->Base: " << HexInt(na->Val->Typ->Base);
    // }
    if( sep != '\0' ) {
      std_out << sep;
    }
    std_out << NL; // TMP: debug
    delay_ms( 100 );
  }
}

struct LibraryFunction oxc_picoc_misc_Functions[] =
{
  { C_delay_ms,      "void delay_ms(int);" },
  { C_delay_ms_brk,  "int delay_ms_brk(int);" },
  { C_leds_set,      "void leds_set(int);" },
  { C_leds_reset,    "void leds_reset(int);" },
  { C_leds_toggle,   "void leds_toggle(int);" },
  { C_pr_i,          "void pr_i(int);" },
  { C_pr_ih,         "void pr_ih(int);" },
  { C_pr_i_l,        "void pr_i_l(int,...);" },
  { C_pr_i_n,        "void pr_i_n(int*,int);" },
  { C_pr_c,          "void pr_c(int);" },
  { C_pr_s,          "void pr_s(char*);" },
  { C_pr_d,          "void pr_d(double);" },
  { C_pr,            "void pr(char,...);" },
  { NULL,            NULL }
};

void oxc_picoc_misc_SetupFunc( Picoc *pc );
void oxc_picoc_misc_SetupFunc( Picoc *pc )
{
}

void oxc_picoc_misc_init( Picoc *pc );
void oxc_picoc_misc_init( Picoc *pc )
{
  // VariableDefinePlatformVar( pc, NULL, "M_E"       , &pc->FPType, (union AnyValue *)&C_math_M_E       , FALSE );
  VariableDefinePlatformVar( pc, nullptr, "UVAR",      pc->IntArrayType, (union AnyValue *)(user_vars),  TRUE );

  IncludeRegister( pc, "oxc_misc.h", &oxc_picoc_misc_SetupFunc, oxc_picoc_misc_Functions, NULL);
}

