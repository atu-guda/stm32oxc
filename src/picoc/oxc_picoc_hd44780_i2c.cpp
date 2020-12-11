#include <oxc_picoc_interpreter.h>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>
// #include <oxc_picoc_reghelpers.h>

using namespace std;

#pragma GCC diagnostic ignored "-Wunused-parameter"

extern HD44780_i2c *p_lcdt;


void C_lcd_cls( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_cls( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->cls();
  }
}

void C_lcd_home( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_home( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->home();
  }
}

// int rc = delay_ms_brk( Param[0]->Val->Integer );


void C_lcd_putch( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_putch( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->putch( (char)Param[0]->Val->Integer );
  }
}


void C_lcd_putxych( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_putxych( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->putxych( Param[0]->Val->Integer, Param[1]->Val->Integer, (char)Param[2]->Val->Integer );
  }
}

void C_lcd_gotoxy( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_gotoxy( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->gotoxy( Param[0]->Val->Integer, Param[1]->Val->Integer );
  }
}


void C_lcd_puts( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_puts( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->puts( (const char*)Param[0]->Val->Pointer );
  }
}


void C_lcd_puts_xy( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_puts_xy( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->puts_xy( Param[0]->Val->Integer, Param[1]->Val->Integer, (const char*)Param[2]->Val->Pointer  );
  }
}


void C_lcd_curs_on( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_curs_on( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->curs_on();
  }
}

void C_lcd_curs_off( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_curs_off( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->curs_off();
  }
}


void C_lcd_led_on( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_led_on( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->led_on();
  }
}

void C_lcd_led_off( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs );
void C_lcd_led_off( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs )
{
  if( p_lcdt ) {
    p_lcdt->led_off();
  }
}



struct LibraryFunction oxc_picoc_hd44780_i2c_Functions[] =
{
  { C_lcd_cls,            "void lcd_cls(void);" },
  { C_lcd_home,           "void lcd_home(void);" },
  { C_lcd_putch,          "void lcd_putch(char);" },
  { C_lcd_putxych,        "void lcd_putxych(int,int,char);" },
  { C_lcd_gotoxy,         "void lcd_gotoxy(int,int);" },
  { C_lcd_puts,           "void lcd_puts(char*);" },
  { C_lcd_puts_xy,        "void lcd_puts_xy(int,int,char*);" },
  { C_lcd_curs_on,        "void lcd_curs_on(void);" },
  { C_lcd_curs_off,       "void lcd_curs_off(void);" },
  { C_lcd_led_on,         "void lcd_led_on(void);" },
  { C_lcd_led_off,        "void lcd_led_off(void);" },
  { NULL,            NULL }
};

void oxc_picoc_hd44780_i2c_SetupFunc( Picoc *pc );
void oxc_picoc_hd44780_i2c_SetupFunc( Picoc *pc )
{
}

void oxc_picoc_hd44780_i2c_init( Picoc *pc );
void oxc_picoc_hd44780_i2c_init( Picoc *pc )
{
  // VariableDefinePlatformVar( pc, NULL, "M_E"       , &pc->FPType, (union AnyValue *)&C_math_M_E       , FALSE );

  IncludeRegister( pc, "oxc_hd44780_i2c.h", &oxc_picoc_hd44780_i2c_SetupFunc, oxc_picoc_hd44780_i2c_Functions, NULL);
}

