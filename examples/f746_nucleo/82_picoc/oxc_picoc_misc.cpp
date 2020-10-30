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

void C_delay_ms_until_brk( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_delay_ms_until_brk( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  int rc = delay_ms_until_brk( (uint32_t*)Param[0]->Val->Pointer, Param[1]->Val->Integer );
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
    if( ! na->Typ ) {
      std_out << "?X?";
      continue;
    }


    switch( na->Typ->Base ) {
      case TypeInt: case TypeShort: case TypeLong: case TypeUnsignedInt:
      case TypeUnsignedShort: case TypeUnsignedChar: case TypeUnsignedLong: case TypeEnum:
        std_out << na->Val->Integer;
        break;
      case TypeChar:
        std_out << (char)(na->Val->Integer);
        break;
      case TypePointer:
        if( !na->Val->Pointer ) {
          std_out << "null";
          break;
        }
        if( na->Typ->FromType->Base == TypeChar ) {
          std_out << (const char*)(na->Val->Pointer);
        } else {
          std_out << "?P?";
        }
        break;
      case TypeFP:
        std_out << na->Val->FP;
        break;
      default:
        std_out << '?' << (int)(na->Typ->Base) << '?';
        break;
    }
    if( sep != '\0' ) {
      std_out << sep;
    }
  }
}

void C_char2hex( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_char2hex( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  uint8_t v =  (uint8_t)( Param[0]->Val->Integer );
  char *s = (char*)( Param[1]->Val->Pointer );
  ReturnValue->Val->Pointer = char2hex( v, s );
}

void C_short2hex( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_short2hex( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  uint16_t v =  (uint16_t)( Param[0]->Val->Integer );
  char *s = (char*)( Param[1]->Val->Pointer );
  ReturnValue->Val->Pointer = short2hex( v, s );
}


void C_word2hex( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_word2hex( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  uint32_t v =  (uint32_t)( Param[0]->Val->Integer );
  std_out << "## v= " << HexInt(v) << NL;
  char *s = (char*)( Param[1]->Val->Pointer );
  ReturnValue->Val->Pointer = word2hex( v, s );
}

void C_rev16( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_rev16( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  uint16_t v =  (uint16_t)( Param[0]->Val->Integer );
  ReturnValue->Val->Integer = __REV16( v );
}

void C_imin( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_imin( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  ReturnValue->Val->Integer = imin( Param[0]->Val->Integer, Param[1]->Val->Integer );
}

void C_imax( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_imax( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  ReturnValue->Val->Integer = imax( Param[0]->Val->Integer, Param[1]->Val->Integer );
}

void C_isign( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_isign( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  ReturnValue->Val->Integer = sign( Param[0]->Val->Integer );
}

void C_i2dec( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs);
void C_i2dec( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
  int n           = Param[0]->Val->Integer;
  char *s         = (char*)Param[1]->Val->Pointer;
  unsigned min_sz = Param[2]->Val->Integer;
  char fill_ch    = (char)(Param[3]->Val->Integer);
  ReturnValue->Val->Pointer = i2dec( n, s, min_sz, fill_ch );
}


struct LibraryFunction oxc_picoc_misc_Functions[] =
{
  { C_delay_ms,            "void delay_ms(int);" },
  { C_delay_ms_brk,        "int delay_ms_brk(int);" },
  { C_delay_ms_until_brk,  "int delay_ms_until_brk(int*,int);" },
  { C_leds_set,            "void leds_set(int);" },
  { C_leds_reset,          "void leds_reset(int);" },
  { C_leds_toggle,         "void leds_toggle(int);" },
  { C_pr_i,                "void pr_i(int);" },
  { C_pr_ih,               "void pr_ih(int);" },
  { C_pr_i_l,              "void pr_i_l(int,...);" },
  { C_pr_i_n,              "void pr_i_n(int*,int);" },
  { C_pr_c,                "void pr_c(int);" },
  { C_pr_s,                "void pr_s(char*);" },
  { C_pr_d,                "void pr_d(double);" },
  { C_pr,                  "void pr(char,...);" },
  { C_char2hex,            "char* char2hex(char,char*);" },
  { C_short2hex,           "char* short2hex(int,char*);" },
  { C_word2hex,            "char* word2hex(int,char*);" },
  { C_rev16,               "short rev16(short);" },
  { C_imin,                "int imin(int,int);" },
  { C_imin,                "int imax(int,int);" },
  { C_isign,               "int isign(int);" },
  { C_i2dec,               "char* i2dec(int,char*,int,char);" },
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

