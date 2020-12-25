#include <cerrno>

#include <oxc_picoc_interpreter.h>

// #include <math.h>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_picoc_reghelpers.h>

using namespace std;

#pragma GCC diagnostic ignored "-Wunused-parameter"


void C_getTick( PICOC_FUN_ARGS );
void C_getTick( PICOC_FUN_ARGS )
{
  RV_INT = HAL_GetTick();
}

void C_delay_ms( PICOC_FUN_ARGS );
void C_delay_ms( PICOC_FUN_ARGS )
{
  delay_ms( ARG_0_INT );
}

void C_delay_ms_brk( PICOC_FUN_ARGS );
void C_delay_ms_brk( PICOC_FUN_ARGS )
{
  int rc = delay_ms_brk( ARG_0_INT );
  RV_INT = rc;
}

void C_delay_ms_until_brk( PICOC_FUN_ARGS );
void C_delay_ms_until_brk( PICOC_FUN_ARGS )
{
  int rc = delay_ms_until_brk( (uint32_t*)ARG_0_PTR, ARG_1_INT );
  RV_INT = rc;
}

void C_leds_set( PICOC_FUN_ARGS );
void C_leds_set( PICOC_FUN_ARGS )
{
  leds.set( ARG_0_INT );
}

void C_leds_reset( PICOC_FUN_ARGS );
void C_leds_reset( PICOC_FUN_ARGS )
{
  leds.reset( ARG_0_INT );
}

void C_leds_toggle( PICOC_FUN_ARGS );
void C_leds_toggle( PICOC_FUN_ARGS )
{
  leds.toggle( ARG_0_INT );
}

void C_pr_i( PICOC_FUN_ARGS );
void C_pr_i( PICOC_FUN_ARGS )
{
  std_out << ( ARG_0_INT ) << ' ';
}

void C_pr_ih( PICOC_FUN_ARGS );
void C_pr_ih( PICOC_FUN_ARGS )
{
  std_out << HexInt( ARG_0_INT ) << ' ';
}

void C_pr_p( PICOC_FUN_ARGS );
void C_pr_p( PICOC_FUN_ARGS )
{
  std_out << HexInt( ARG_0_PTR ) << ' ';
}


void C_pr_i_l( PICOC_FUN_ARGS );
void C_pr_i_l( PICOC_FUN_ARGS )
{
  struct Value *na = Param[0];
  for( int i=0; i<NumArgs; ++i ) {
    std_out << ( na->Val->Integer ) << ' ';
    na = (struct Value *)( (char *)na + MEM_ALIGN(sizeof(struct Value) + TypeStackSizeValue(na)) );
  }
}

void C_pr_i_n( PICOC_FUN_ARGS );
void C_pr_i_n( PICOC_FUN_ARGS )
{
  int n  = ARG_1_INT;
  int *v = (int*)( ARG_0_PTR );
  // std_out << "## n= " << n << " v= " << HexInt((void*)(v)) << NL;
  for( int i=0; i<n; ++i ) {
    std_out << ( v[i] ) << ' ';
  }
}


void C_pr_c( PICOC_FUN_ARGS );
void C_pr_c( PICOC_FUN_ARGS )
{
  std_out << char( ARG_0_INT );
}

void C_pr_s( PICOC_FUN_ARGS );
void C_pr_s( PICOC_FUN_ARGS )
{
  std_out << (const char*)( ARG_0_PTR );
}

void C_pr_d( PICOC_FUN_ARGS );
void C_pr_d( PICOC_FUN_ARGS )
{
  std_out << (double)( ARG_0_FP );
}

void C_pr( PICOC_FUN_ARGS );
void C_pr( PICOC_FUN_ARGS )
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

void C_char2hex( PICOC_FUN_ARGS );
void C_char2hex( PICOC_FUN_ARGS )
{
  uint8_t v =  (uint8_t)( ARG_0_INT );
  char *s = (char*)( ARG_1_PTR );
  RV_PTR = char2hex( v, s );
}

void C_short2hex( PICOC_FUN_ARGS );
void C_short2hex( PICOC_FUN_ARGS )
{
  uint16_t v =  (uint16_t)( ARG_0_INT );
  char *s = (char*)( ARG_1_PTR );
  RV_PTR = short2hex( v, s );
}


void C_word2hex( PICOC_FUN_ARGS );
void C_word2hex( PICOC_FUN_ARGS )
{
  uint32_t v =  (uint32_t)( ARG_0_INT );
  std_out << "## v= " << HexInt(v) << NL;
  char *s = (char*)( ARG_1_PTR );
  RV_PTR = word2hex( v, s );
}

void C_rev16( PICOC_FUN_ARGS );
void C_rev16( PICOC_FUN_ARGS )
{
  uint16_t v =  (uint16_t)( ARG_0_INT );
  RV_INT = __REV16( v );
}

void C_imin( PICOC_FUN_ARGS );
void C_imin( PICOC_FUN_ARGS )
{
  RV_INT = imin( ARG_0_INT, ARG_1_INT );
}

void C_imax( PICOC_FUN_ARGS );
void C_imax( PICOC_FUN_ARGS )
{
  RV_INT = imax( ARG_0_INT, ARG_1_INT );
}

void C_isign( PICOC_FUN_ARGS );
void C_isign( PICOC_FUN_ARGS )
{
  RV_INT = sign( ARG_0_INT );
}

void C_i2dec( PICOC_FUN_ARGS );
void C_i2dec( PICOC_FUN_ARGS )
{
  int n           = ARG_0_INT;
  char *s         = (char*)ARG_1_PTR;
  unsigned min_sz = ARG_2_INT;
  char fill_ch    = (char)(ARG_3_INT);
  RV_PTR = i2dec( n, s, min_sz, fill_ch );
}

void C_dump8( PICOC_FUN_ARGS );
void C_dump8( PICOC_FUN_ARGS )
{
  void *p         = ARG_0_PTR;
  unsigned n      = ARG_1_INT;
  bool  isAbs     = (bool)ARG_2_INT;
  dump8( p, n, isAbs );
}

void C_dump32( PICOC_FUN_ARGS );
void C_dump32( PICOC_FUN_ARGS )
{
  void *p         = ARG_0_PTR;
  unsigned n      = ARG_1_INT;
  bool  isAbs     = (bool)ARG_2_INT;
  dump32( p, n, isAbs );
}



struct LibraryFunction oxc_picoc_misc_Functions[] =
{
  { C_getTick,             "int getTick(void);" },
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
  { C_pr_ih,               "void pr_p(void*);" },
  { C_char2hex,            "char* char2hex(char,char*);" },
  { C_short2hex,           "char* short2hex(int,char*);" },
  { C_word2hex,            "char* word2hex(int,char*);" },
  { C_rev16,               "short rev16(short);" },
  { C_imin,                "int imin(int,int);" },
  { C_imin,                "int imax(int,int);" },
  { C_isign,               "int isign(int);" },
  { C_i2dec,               "char* i2dec(int,char*,int,char);" },
  { C_dump8,               "void dump8(void*,int,int);" },
  { C_dump32,              "void dump32(void*,int,int);" },
  { NULL,            NULL }
};

void oxc_picoc_misc_SetupFunc( Picoc *pc );
void oxc_picoc_misc_SetupFunc( Picoc *pc )
{
}

void oxc_picoc_misc_init( Picoc *pc );
void oxc_picoc_misc_init( Picoc *pc )
{
  VariableDefinePlatformVar( pc, nullptr, "UVAR",       pc->IntArrayType, (union AnyValue *)(user_vars),   TRUE );
  VariableDefinePlatformVar( pc, nullptr, "errno",      &pc->IntType,     (union AnyValue *)(&errno),      TRUE );
  VariableDefinePlatformVar( pc, nullptr, "break_flag", &pc->IntType,     (union AnyValue *)(&break_flag), TRUE );

  IncludeRegister( pc, "oxc_misc.h", &oxc_picoc_misc_SetupFunc, oxc_picoc_misc_Functions, NULL);
}

