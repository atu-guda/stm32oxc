#include <cerrno>

#include <oxc_auto.h>
#include <oxc_outstr.h>
#include <oxc_hd44780_i2c.h>
#include <oxc_ads1115.h>
#include <oxc_menu4b.h>
#include <oxc_statdata.h>
#include <oxc_ds3231.h>
#include <oxc_mcp23017.h>

#include <ff_gen_drv_st.h>
#include <usbh_diskio.h>
#include <ff.h>

#include <oxc_fs_cmd0.h>

#include <oxc_picoc.h>
#include <oxc_picoc_reghelpers.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

const char* common_help_string = "Appication control cube project" NL;

USBH_HandleTypeDef hUSB_Host;
char USBDISKPath[8]; // USB Host logical drive path
uint8_t sd_buf[512]; // one sector
FATFS fs;
int isUSBH_on = 0, isMSC_ready = 0;

void USBH_HandleEvent( USBH_HandleTypeDef *phost, uint8_t id );
int init_usbh_msc();

int task_idx = 0, t_step_ms = 100, n_loops = 10000000, auto_out = 0;
int T_off = -10, T_hyst = 20; // TMP: to test menu

const Menu4bItem menu_main[] = {
  {  "task_idx",   &task_idx,      1,       0,         42, nullptr },
  { "t_step_ms",  &t_step_ms,    100,     100,      10000, nullptr },
  {   "n_loops",    &n_loops,      1,       1,   10000000, nullptr },
  {  "auto_out",   &auto_out,      1,       0,          9, nullptr },
  {     "T_off",      &T_off,      25,  -10000,    500000, nullptr, 2 },
  //{ "set_base", nullptr,   0,      0,   100000, fun_set_base }
};
xfloat t_c = 0;

using RUN_FUN = int(*)(void);
int run_common( RUN_FUN pre_fun, RUN_FUN loop_fun );
int run_0();
int run_1();
int run_n();

MenuState menu4b_state { menu_main, size( menu_main ), "T\n" };
void on_btn_while_run( int cmd );

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x3F );
HD44780_i2c *p_lcdt = &lcdt;
void oxc_picoc_hd44780_i2c_init( Picoc *pc );

ADS1115 adc( i2cd );
const unsigned adc_n_ch = 4;
unsigned adc_no = 0;
const xfloat adc_20_to_3 = 20.0f / 3.0f;
xfloat adc_v_scales[adc_n_ch] = {  adc_20_to_3, adc_20_to_3,  adc_20_to_3, adc_20_to_3 };
xfloat adc_v_bases[adc_n_ch]  = {       -10.0f,      -10.0f,       -10.0f,      -10.0f };
xfloat adc_v[adc_n_ch]        = {         0.0f,        0.0f,         0.0f,        0.0f };
int    adc_vi[adc_n_ch]       = {            0,           0,            0,           0 };
int adc_scale_mv = 4096;
xfloat adc_kv = 0.001f * adc_scale_mv / 0x7FFF;
int adc_defcfg();
int adc_measure();
void adc_out_stdout();
void adc_out_lcd();
int adc_pre_loop();
int adc_loop();
void C_adc_defcfg( PICOC_FUN_ARGS );
void C_adc_measure( PICOC_FUN_ARGS );
void C_adc_out_stdout( PICOC_FUN_ARGS );
void C_adc_out_lcd( PICOC_FUN_ARGS );

extern DAC_HandleTypeDef hdac;
int MX_DAC_Init();
const unsigned dac_n_ch = 2;
const unsigned dac_bimask = 0x0FFF; // 12 bit
const xfloat dac_3_to_20 = 3.0f / 20.0f;
xfloat dac_vref = 3.0f;
xfloat dac_v_scales[dac_n_ch] = {  dac_3_to_20, dac_3_to_20 };
xfloat dac_v_bases[adc_n_ch]  = {       -10.0f,      -10.0f };
void dac_out1( xfloat v );
void dac_out2( xfloat v );
void dac_out12( xfloat v1, xfloat v2 );
void dac_out1i( int v );
void dac_out2i( int v );
void dac_out12i( int v1, int v2 );
void C_dac_out1( PICOC_FUN_ARGS );
void C_dac_out2( PICOC_FUN_ARGS );
void C_dac_out12( PICOC_FUN_ARGS );
void C_dac_out1i( PICOC_FUN_ARGS );
void C_dac_out2i( PICOC_FUN_ARGS );
void C_dac_out12i( PICOC_FUN_ARGS );

MCP23017 mcp_gpio( i2cd );
void C_pins_out( PICOC_FUN_ARGS );
void C_pins_out_read( PICOC_FUN_ARGS );
void C_pins_out_set( PICOC_FUN_ARGS );
void C_pins_out_reset( PICOC_FUN_ARGS );
void C_pins_out_toggle( PICOC_FUN_ARGS );
void C_pins_in( PICOC_FUN_ARGS );

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5; // TIM5 - pwmo_
extern TIM_HandleTypeDef htim9;
int MX_TIM1_Init();
int MX_TIM2_Init();
int MX_TIM3_Init();
int MX_TIM4_Init();
int MX_TIM5_Init();
int MX_TIM9_Init();
// pwmo_
const unsigned pwmo_n_ch = 4;
#define TIM_PWM TIM5
uint32_t pwmo_calc_ccr( xfloat d );
void pwmo_setFreq( xfloat f );
xfloat pwmo_getFreq();
void pwmo_setARR( uint32_t arr );
uint32_t pwmo_getARR();
void pwmo_setD0( xfloat d );
void pwmo_setD1( xfloat d );
void pwmo_setD2( xfloat d );
void pwmo_setD3( xfloat d );
void pwmo_setDn( const xfloat *da );
void pwmo_setCCR0( uint32_t ccr ); // from 0 here
void pwmo_setCCR1( uint32_t ccr );
void pwmo_setCCR2( uint32_t ccr );
void pwmo_setCCR3( uint32_t ccr );
uint32_t pwmo_getCCR0();
uint32_t pwmo_getCCR1();
uint32_t pwmo_getCCR2();
uint32_t pwmo_getCCR3();

void C_pwmo_setFreq( PICOC_FUN_ARGS );
void C_pwmo_getFreq( PICOC_FUN_ARGS );
void C_pwmo_setARR( PICOC_FUN_ARGS );
void C_pwmo_getARR( PICOC_FUN_ARGS );
void C_pwmo_setD0( PICOC_FUN_ARGS );
void C_pwmo_setD1( PICOC_FUN_ARGS );
void C_pwmo_setD2( PICOC_FUN_ARGS );
void C_pwmo_setD3( PICOC_FUN_ARGS );
void C_pwmo_setDn( PICOC_FUN_ARGS );
void C_pwmo_setCCR0( PICOC_FUN_ARGS );
void C_pwmo_setCCR1( PICOC_FUN_ARGS );
void C_pwmo_setCCR2( PICOC_FUN_ARGS );
void C_pwmo_setCCR3( PICOC_FUN_ARGS );
void C_pwmo_getCCR0( PICOC_FUN_ARGS );
void C_pwmo_getCCR1( PICOC_FUN_ARGS );
void C_pwmo_getCCR2( PICOC_FUN_ARGS );
void C_pwmo_getCCR3( PICOC_FUN_ARGS );

extern volatile uint32_t tim2_ccr1x, tim2_ccr2x, tim2_busy;
uint32_t tim2_ccr1, tim2_ccr2;
xfloat ifm_0_freq = 0, ifm_0_d = 0, ifm_0_t0 = 0, ifm_0_td = 0;
uint32_t ifm_2_cnt = 0;
uint16_t ifm_2_cntx = 0;
void ifm_0_measure();
void ifm_0_enable();
void ifm_0_disable();
void C_ifm_0_measure( PICOC_FUN_ARGS );
void C_ifm_0_enable( PICOC_FUN_ARGS );
void C_ifm_0_disable( PICOC_FUN_ARGS );
void ifm_2_measure();
void C_ifm_2_measure( PICOC_FUN_ARGS );

void ifm_out_stdout();
void ifm_out_lcd();
int  ifm_pre_loop();
int  ifm_loop();

#define PICOC_STACK_SIZE (32*1024)
int picoc_cmdline_handler( char *s );
Picoc pc;
int init_picoc( Picoc *ppc );
// TMP here: move to system include
extern "C" {
void oxc_picoc_math_init( Picoc *pc );
}
//char a_char[] = "ABCDE";
//char *p_char = a_char;
void oxc_picoc_misc_init(  Picoc *pc );
void oxc_picoc_fatfs_init( Picoc *pc );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_lcd_gotoxy( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_GOTOXY{ "lcd_gotoxy", 0, cmd_lcd_gotoxy, " x y - move pos to LCD (x, y)"  };
int cmd_lcd_xychar( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_XYCHAR{ "lcd_xychar", 0, cmd_lcd_xychar, " x y code - put char at x y ln LCD"  };
int cmd_lcd_puts( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_PUTS{ "lcd_puts", 0, cmd_lcd_puts, "string - put string at cur pos ln  LCD"  };
int cmd_menu( int argc, const char * const * argv );
CmdInfo CMDINFO_MENU { "menu", 'M', cmd_menu, " N - menu action"  };
int cmd_t1( int argc, const char * const * argv );
CmdInfo CMDINFO_T1 { "t1", 0, cmd_t1, " - misc test"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_LCD_XYCHAR,
  &CMDINFO_LCD_GOTOXY,
  &CMDINFO_LCD_PUTS,
  &CMDINFO_T1,
  &CMDINFO_MENU,
  FS_CMDS0,
  nullptr
};

int run_common( RUN_FUN pre_fun, RUN_FUN loop_fun )
{
  lcdt.cls();
  std_out << "# Task " << task_idx << " n_loops= " << n_loops << " t_step_ms= " << t_step_ms << NL;
  OSTR(s,40);

  pre_fun();

  uint32_t tm0, tm00;
  break_flag = 0;
  for( decltype(+n_loops) i=0; i<n_loops && !break_flag; ++i ) {
    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    t_c = 0.001f * ( tcc - tm00 );
    // TODO: fun
    s_outstr.reset_out();
    s << XFmt( t_c, cvtff_fix, 10, 2 );
    std_out << s_outstr.c_str() << ' ';
    loop_fun();
    lcdt.puts_xy( 10, 3, s_outstr.c_str() );

    std_out << NL;

    if( t_step_ms > 0 ) {
      delay_ms_until_brk( &tm0, t_step_ms );
    }
  }

  return 0;
}


int run_0()
{
  return run_common( adc_pre_loop, adc_loop );
}

int run_1()
{
  int rc =  run_common( ifm_pre_loop, ifm_loop );
  ifm_0_disable();
  return rc;
}

int run_n()
{
  std_out << "# Task N n= " << task_idx << NL;
  return 0;
}

void idle_main_task()
{
  if( isUSBH_on ) {
    leds.set( 1 );
    USBH_Process( &hUSB_Host );
    leds.reset( 1 );
  };
}

// ---------------------------------------- main -----------------------------------------------

int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') =       100;
  UVAR('n') =  10000000;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;
  lcdt.init_4b();
  lcdt.cls();
  lcdt.puts("I ");

  init_menu4b_buttons();
  lcdt.puts("K");

  mcp_gpio.cfg( MCP23017::iocon_intpol  ); // only for int
  mcp_gpio.set_dir_a( 0xFF ); // all input
  mcp_gpio.set_dir_b( 0x00 ); // all output
  mcp_gpio.set_b( 0x00 );

  lcdt.puts("P");

  MX_TIM2_Init();
  lcdt.puts("2");
  MX_TIM3_Init();
  lcdt.puts("3");
  MX_TIM5_Init();
  lcdt.puts("5");

  int dac_rc = MX_DAC_Init();
  lcdt.puts( dac_rc == 0 ? "D " : "-d" );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2047 );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2047 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_2 );

  fs.fs_type = 0; // none
  fspath[0] = '\0';

  cmdline_handlers[0] = picoc_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  pc.InteractiveHead = nullptr;
  init_picoc( &pc );
  lcdt.puts("C ");

  BOARD_POST_INIT_BLINK;
  leds.reset( 0xFF );

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( menu4b_ev_dispatch );

  UVAR('e') = init_usbh_msc();
  lcdt.puts("usbh ");
  lcdt.puts_xy( 0, 1, menu4b_state.menu_level0_str );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int init_usbh_msc()
{
  int rc = FATFS_LinkDriver( &USBH_Driver, USBDISKPath );
  if( rc != 0 ) {
    std_out << "# Error LinkDriver: " << rc << NL;
    return 1;
  }

  USBH_Init( &hUSB_Host, USBH_HandleEvent, 0 );

  USBH_RegisterClass( &hUSB_Host, USBH_MSC_CLASS );

  isUSBH_on = 1;
  USBH_Start( &hUSB_Host );

  return 0;
}


int cmd_test0( int argc, const char * const * argv )
{
  // uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 100000000 ); // number of series

  std_out << "# Test: " << NL;
  int rc;
  switch( task_idx ) {
    case 0:  rc = run_0(); break;
    case 1:  rc = run_1(); break;
    default: rc = run_n(); break;
  }

  return rc;
}

int cmd_t1( int argc, const char * const * argv )
{
  std_out << "# t1: " << NL;

  ifm_0_measure();
  ifm_2_measure();

  std_out << "# T2.CCR1= " << tim2_ccr1 << " T2.CCR2= " << tim2_ccr2
          << " freq= " << ifm_0_freq << " d= " << ifm_0_d
          << " t0= " << ifm_0_t0 << " td= " << ifm_0_td
          << NL;

  std_out << "# T3.CNT= " << TIM3->CNT << NL;

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

// on: 4,3,2 off: 5
void USBH_HandleEvent( USBH_HandleTypeDef *phost, uint8_t id )
{
  // leds.toggle( BIT1 );
  // std_out << "### UP " << (int)id << NL;
  FRESULT fr;

  switch( id ) {
    case HOST_USER_SELECT_CONFIGURATION: // 1
      break;

    case HOST_USER_CLASS_ACTIVE:         // 2
      fr = f_mount( &fs, fspath, 1 ); // todo: flag for automount?
      if( fr == 0 ) {
        isMSC_ready = 1;
        leds.set( BIT2 );
      } else {
        leds.set( BIT0 );
      }
      break;

    case HOST_USER_CLASS_SELECTED:       // 3
      break;

    case HOST_USER_CONNECTION:           // 4
      break;

    case HOST_USER_DISCONNECTION:        // 5
      leds.reset( BIT2 );
      f_mount( nullptr, (TCHAR const*)"", 0 );
      isMSC_ready = 0;
      break;

    case HOST_USER_UNRECOVERED_ERROR:    // 6
      // leds.set( BIT0 );
      f_mount( nullptr, (TCHAR const*)"", 0 );
      errno = 7555;
      break;

    default:
      break;
  }
}

// --------------------------------- picoc ----------------------------------------------

struct LibraryFunction picoc_local_Functions[] =
{
  { C_adc_defcfg,            "int adc_defcfg(void);" },
  { C_adc_measure,           "int adc_measure(void);" },
  { C_adc_out_stdout,        "void adc_out_stdout(void);" },
  { C_adc_out_lcd,           "void adc_out_lcd(void);" },

  { C_dac_out1,              "void dac_out1(float);" },
  { C_dac_out2,              "void dac_out2(float);" },
  { C_dac_out12,             "void dac_out12(float,float);" },
  { C_dac_out1i,             "void dac_out1i(int);" },
  { C_dac_out2i,             "void dac_out2i(int);" },
  { C_dac_out12i,            "void dac_out12i(int,int);" },

  { C_pins_out,              "void pins_out(int);" },
  { C_pins_out_read,         "int  pins_out_read(void);" },
  { C_pins_out_set,          "void pins_out_set(int);" },
  { C_pins_out_reset,        "void pins_out_reset(int);" },
  { C_pins_out_toggle,       "void pins_out_toggle(int);" },
  { C_pins_in,               "int  pins_in(void);" },

  { C_pwmo_setFreq,          "void pwmo_setFreq( double );" },
  { C_pwmo_getFreq,          "double pwmo_getFreq();" },
  { C_pwmo_setARR,           "void pwmo_setARR( int );" },
  { C_pwmo_getARR,           "int pwmo_getARR();" },
  { C_pwmo_setD0,            "void pwmo_setD0( double );" },
  { C_pwmo_setD1,            "void pwmo_setD1( double );" },
  { C_pwmo_setD2,            "void pwmo_setD2( double );" },
  { C_pwmo_setD3,            "void pwmo_setD3( double );" },
  { C_pwmo_setDn,            "void pwmo_setDn( double* );" },
  { C_pwmo_setCCR0,          "void pwmo_setCCR0( int );  " },
  { C_pwmo_setCCR1,          "void pwmo_setCCR1( int );  " },
  { C_pwmo_setCCR2,          "void pwmo_setCCR2( int );  " },
  { C_pwmo_setCCR3,          "void pwmo_setCCR3( int );  " },
  { C_pwmo_getCCR0,          "int  pwmo_getCCR0();  " },
  { C_pwmo_getCCR1,          "int  pwmo_getCCR1();  " },
  { C_pwmo_getCCR2,          "int  pwmo_getCCR2();  " },
  { C_pwmo_getCCR3,          "int  pwmo_getCCR3();  " },

  { C_ifm_0_measure,         "void ifm_0_measure();  " },
  { C_ifm_0_enable,          "void ifm_0_enable();  " },
  { C_ifm_0_disable,         "void ifm_0_disable();  " },
  { C_ifm_2_measure,         "void ifm_2_measure();  " },

  { NULL,            NULL }
};


void picoc_local_SetupFunc( Picoc *pc );
void picoc_local_SetupFunc( Picoc *pc )
{
}

void picoc_local_init( Picoc *pc );
void picoc_local_init( Picoc *pc )
{
  IncludeRegister( pc, "local.h", &picoc_local_SetupFunc, picoc_local_Functions, NULL );
}

int init_picoc( Picoc *ppc )
{
  if( ppc->InteractiveHead != nullptr ) {
    PicocCleanup( ppc );
  }
  PicocInitialise( ppc, PICOC_STACK_SIZE );
  oxc_picoc_math_init( ppc );
  oxc_picoc_misc_init( ppc );
  oxc_picoc_fatfs_init( ppc );
  oxc_picoc_hd44780_i2c_init( ppc );
  picoc_local_init( ppc );

  PicocIncludeAllSystemHeaders( ppc );

  //VariableDefinePlatformVar( ppc, nullptr, "a_char",   ppc->CharArrayType, (union AnyValue *)a_char,       TRUE );
  //VariableDefinePlatformVar( ppc, nullptr, "p_char",     ppc->CharPtrType, (union AnyValue *)&p_char,      TRUE );

  VariableDefinePlatformVar( ppc , nullptr , "t_c"          , &(ppc->FPType)    , (union AnyValue *)&t_c            , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "__a"          , &(ppc->IntType)   , (union AnyValue *)&(UVAR('a'))    , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "task_idx"     , &(ppc->IntType)   , (union AnyValue *)&task_idx       , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "n_loops"      , &(ppc->IntType)   , (union AnyValue *)&n_loops        , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "t_step_ms"    , &(ppc->IntType)   , (union AnyValue *)&t_step_ms      , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "auto_out"     , &(ppc->IntType)   , (union AnyValue *)&auto_out       , TRUE );
  //VariableDefinePlatformVar( ppc , nullptr , "UVAR"         , ppc->IntArrayType , (union AnyValue *)user_vars       , TRUE );

  VariableDefinePlatformVar( ppc , nullptr , "adc_v"        , ppc->FPArrayType  , (union AnyValue *)adc_v           , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "adc_v_scales" , ppc->FPArrayType  , (union AnyValue *)adc_v_scales    , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "adc_v_bases"  , ppc->FPArrayType  , (union AnyValue *)adc_v_bases     , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "adc_vi"       , ppc->IntArrayType , (union AnyValue *)adc_vi          , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "adc_no"       , &(ppc->IntType)   , (union AnyValue *)&(adc_no)       , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "adc_scale_mv" , &(ppc->IntType)   , (union AnyValue *)&(adc_scale_mv) , TRUE );

  VariableDefinePlatformVar( ppc , nullptr , "dac_v_scales" , ppc->FPArrayType  , (union AnyValue *)dac_v_scales    , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "dac_v_bases"  , ppc->FPArrayType  , (union AnyValue *)dac_v_bases     , TRUE );

  VariableDefinePlatformVar( ppc , nullptr , "ifm_0_freq"   , &(ppc->FPType)    , (union AnyValue *)&ifm_0_freq     , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "ifm_0_d"      , &(ppc->FPType)    , (union AnyValue *)&ifm_0_d        , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "ifm_0_t0"     , &(ppc->FPType)    , (union AnyValue *)&ifm_0_t0       , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "ifm_0_td"     , &(ppc->FPType)    , (union AnyValue *)&ifm_0_td       , TRUE );
  VariableDefinePlatformVar( ppc , nullptr , "ifm_2_cnt"    , &(ppc->IntType)   , (union AnyValue *)&(ifm_2_cnt)    , TRUE );

  return 0;
}

// ---------------------------------------- LCD ------------------------------------------------------


int cmd_lcd_xychar( int argc, const char * const * argv )
{
  uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
  uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );
  uint8_t ch = (uint8_t)arg2long_d( 3, argc, argv, 'Z', 0, 0xFF );

  lcdt.putxych( x, y, (uint8_t)ch );

  return 0;
}

int cmd_lcd_gotoxy( int argc, const char * const * argv )
{
  uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
  uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );

  lcdt.gotoxy( x, y );
  return 0;
}

int cmd_lcd_puts( int argc, const char * const * argv )
{
  const char *s = "Z";
  if( argc > 1 ) {
    s = argv[1];
  }
  lcdt.puts( s );

  return 0;
}

// ----------------------------------------  Menu ------------------------------------------------------

int cmd_menu( int argc, const char * const * argv )
{
  int cmd_i = arg2long_d( 1, argc, argv, 0 );
  leds.toggle( BIT1 );
  return menu4b_cmd( cmd_i );
}

int menu4b_output( const char *s1, const char *s2 )
{
  // lcdt.cls();
  if( s1 ) {
    lcdt.puts_xy( 0, 0, s1 );
  }
  if( s2 ) {
    lcdt.puts_xy( 0, 1, s2 );
  }
  return 1;
}

void on_btn_while_run( int cmd )
{
  leds.toggle( BIT0 );
  switch( cmd ) {
    case  MenuCmd::Esc:
      break_flag = 1; errno = 10000;
      break;
    case  MenuCmd::Up:
      // ++out_idx;
      // if( out_idx > (int)size(outInts)-1 ) { out_idx = 0; }
      break;
    case  MenuCmd::Down:
      //--out_idx;
      //if( out_idx < 0 ) { out_idx = size(outInts)-1; }
      break;
    case  MenuCmd::Enter:
      break_flag = 1; errno = 10001;
      // btn_run = !btn_run;
      break;
    default: break;
  }
}



// ---------------------------------------- ADC ------------------------------------------------------

int adc_defcfg()
{
  adc.setDefault();
  uint16_t cfg =  ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  adc.setCfg( cfg );
  int r = adc.getDeviceCfg(); // to fill correct inner
  adc_scale_mv =  adc.getScale_mV();
  adc_kv = 0.001f * adc_scale_mv / 0x7FFF;
  return r;
}

int adc_measure()
{
  int16_t vi[adc_n_ch];
  decltype(+adc_no) no = adc.getOneShotNch( 0, adc_n_ch-1, vi );

  for( decltype(no) j=0; j<no; ++j ) {
    adc_vi[j] = vi[j];
    adc_v[j]  = adc_kv * vi[j] * adc_v_scales[j] + adc_v_bases[j];
  }
  adc_no = no;
  return no;
}

void adc_out_stdout()
{
  for( decltype(+adc_no) j=0; j<adc_no; ++j ) {
    std_out << ' ' << XFmt( adc_v[j], cvtff_fix, 9, 6 );
  }
}

void adc_out_lcd()
{
  OSTR(s,40);
  for( decltype(+adc_no) j=0; j<adc_no; ++j ) {
    s_outstr.reset_out();
    s << XFmt( adc_v[j], cvtff_fix, 7, 4 ) << ' ';
    lcdt.puts_xy( 0, j, s_outstr.c_str() );
  }
}

int adc_pre_loop()
{
  adc_defcfg();
  uint16_t x_cfg = adc.getDeviceCfg();
  std_out << "##  x_cfg= " << HexInt16( x_cfg ) << NL;
  return 1;
}

int adc_loop()
{
  adc_measure();
  adc_out_stdout();
  adc_out_lcd();
  return 1;
}

void C_adc_defcfg( PICOC_FUN_ARGS )
{
  int rc = adc_defcfg();
  RV_INT = rc;
}

void C_adc_measure( PICOC_FUN_ARGS )
{
  int rc = adc_measure();
  RV_INT = rc;
}

void C_adc_out_stdout( PICOC_FUN_ARGS )
{
  adc_out_stdout();
}

void C_adc_out_lcd( PICOC_FUN_ARGS )
{
  adc_out_lcd();
}

// ---------------------------------------- PINS ---------------------------------------------------

void C_pins_out( PICOC_FUN_ARGS )
{
  uint8_t val = (uint8_t)(ARG_0_INT);
  mcp_gpio.set_b( val );
}

void C_pins_in( PICOC_FUN_ARGS )
{
  RV_INT = mcp_gpio.get_a();
}

void C_pins_out_read( PICOC_FUN_ARGS )
{
  RV_INT = mcp_gpio.get_b();
}

void C_pins_out_set( PICOC_FUN_ARGS )
{
  uint8_t val = mcp_gpio.get_b() | (uint8_t)(ARG_0_INT);
  mcp_gpio.set_b( val );
}

void C_pins_out_reset( PICOC_FUN_ARGS )
{
  uint8_t val = mcp_gpio.get_b() & ~(uint8_t)(ARG_0_INT);
  mcp_gpio.set_b( val );
}

void C_pins_out_toggle( PICOC_FUN_ARGS )
{
  uint8_t val = mcp_gpio.get_b() ^ (uint8_t)(ARG_0_INT);
  mcp_gpio.set_b( val );
}


// ---------------------------------------- DAC ------------------------------------------------------

void dac_out1( xfloat v )
{
  int iv = (int)( dac_bimask * ( v - dac_v_bases[0] ) * dac_v_scales[0] / dac_vref );
  dac_out1i( iv );
}

void dac_out2( xfloat v )
{
  int iv = (int)( dac_bimask * ( v - dac_v_bases[1] ) * dac_v_scales[1] / dac_vref );
  dac_out2i( iv );
}

void dac_out12( xfloat v1, xfloat v2 )
{
  dac_out1( v1 );
  dac_out2( v2 );
}

void dac_out1i( int v )
{
  v = clamp( v, 0, (int)dac_bimask );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, v );
}

void dac_out2i( int v )
{
  v = clamp( v, 0, (int)dac_bimask );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, v );
}

void dac_out12i( int v1, int v2 )
{
  dac_out1i( v1 );
  dac_out2i( v2 );
}

void C_dac_out1( PICOC_FUN_ARGS )
{
  dac_out1( ARG_0_FP );
}

void C_dac_out2( PICOC_FUN_ARGS )
{
  dac_out2( ARG_0_FP );
}

void C_dac_out12( PICOC_FUN_ARGS )
{
  dac_out12( ARG_0_FP, ARG_1_FP );
}

void C_dac_out1i( PICOC_FUN_ARGS )
{
  dac_out1i( ARG_0_INT );
}

void C_dac_out2i( PICOC_FUN_ARGS )
{
  dac_out2i( ARG_0_INT );
}

void C_dac_out12i( PICOC_FUN_ARGS )
{
  dac_out12i( ARG_0_INT, ARG_1_INT );
}


// ---------------------------------------- pwmo = TIM5 -------------------------------------------

uint32_t pwmo_calc_ccr( xfloat d )
{
  d = clamp( d, 0.0, 1.0 );
  uint32_t arr = TIM_PWM->ARR;
  return (uint32_t)( arr * d );
}

void pwmo_setFreq( xfloat f )
{
  uint32_t cnt_freq = get_TIM_cnt_freq( TIM_PWM );
  xfloat arr_x = clamp( cnt_freq / f - 1, (xfloat)1.0, (xfloat)(0xFFFFFFFF) ); // TODO? change PSC?
  uint32_t arr = (uint32_t)( arr_x );
  pwmo_setARR( arr );
}

xfloat pwmo_getFreq()
{
  uint32_t freq = get_TIM_cnt_freq( TIM_PWM );
  uint32_t arr = TIM_PWM->ARR;
  return (xfloat)freq / ( (xfloat)(1) + arr );
}

void pwmo_setARR( uint32_t arr )
{
  TIM_PWM->CR1 &= ~1u; // disable

  uint32_t old = TIM_PWM->ARR;
  if( old < 1 ) {
    old = 1;
  }
  TIM_PWM->ARR = arr;
  TIM_PWM->CNT  = 0;
  TIM_PWM->CCR1 = (uint32_t)( (uint64_t)TIM_PWM->CCR1 * arr / old );
  TIM_PWM->CCR2 = (uint32_t)( (uint64_t)TIM_PWM->CCR2 * arr / old );
  TIM_PWM->CCR3 = (uint32_t)( (uint64_t)TIM_PWM->CCR3 * arr / old );
  TIM_PWM->CCR4 = (uint32_t)( (uint64_t)TIM_PWM->CCR4 * arr / old );

  TIM_PWM->CR1 |=  1u; // enable
  TIM_PWM->EGR  = 1;
}

uint32_t pwmo_getARR()
{
  return TIM_PWM->ARR;
}

void pwmo_setD0( xfloat d )
{
  pwmo_setCCR0( pwmo_calc_ccr( d ) );
}

void pwmo_setD1( xfloat d )
{
  pwmo_setCCR1( pwmo_calc_ccr( d ) );

}

void pwmo_setD2( xfloat d )
{
  pwmo_setCCR2( pwmo_calc_ccr( d ) );

}

void pwmo_setD3( xfloat d )
{
  pwmo_setCCR3( pwmo_calc_ccr( d ) );
}

void pwmo_setDn( const xfloat *da )
{
  if( da ) {
    pwmo_setD0( da[0] );
    pwmo_setD1( da[1] );
    pwmo_setD2( da[2] );
    pwmo_setD3( da[3] );
  }
}

void pwmo_setCCR0( uint32_t ccr )
{
  TIM_PWM->CCR1 = ccr;
}

void pwmo_setCCR1( uint32_t ccr )
{
  TIM_PWM->CCR2 = ccr;
}

void pwmo_setCCR2( uint32_t ccr )
{
  TIM_PWM->CCR3 = ccr;
}

void pwmo_setCCR3( uint32_t ccr )
{
  TIM_PWM->CCR4 = ccr;
}

uint32_t pwmo_getCCR0()
{
  return TIM_PWM->CCR1;
}

uint32_t pwmo_getCCR1()
{
  return TIM_PWM->CCR2;
}

uint32_t pwmo_getCCR2()
{
  return TIM_PWM->CCR3;
}

uint32_t pwmo_getCCR3()
{
  return TIM_PWM->CCR4;
}

void C_pwmo_setFreq( PICOC_FUN_ARGS )
{
  pwmo_setFreq( ARG_0_FP );
}

void C_pwmo_getFreq( PICOC_FUN_ARGS )
{
  RV_FP = pwmo_getFreq();
}

void C_pwmo_setARR( PICOC_FUN_ARGS )
{
  pwmo_setARR( ARG_0_INT );

}

void C_pwmo_getARR( PICOC_FUN_ARGS )
{
  RV_INT = pwmo_getARR();
}

void C_pwmo_setD0( PICOC_FUN_ARGS )
{
  pwmo_setD0( ARG_0_FP );
}

void C_pwmo_setD1( PICOC_FUN_ARGS )
{
  pwmo_setD1( ARG_0_FP );
}

void C_pwmo_setD2( PICOC_FUN_ARGS )
{
  pwmo_setD2( ARG_0_FP );
}

void C_pwmo_setD3( PICOC_FUN_ARGS )
{
  pwmo_setD3( ARG_0_FP );
}

void C_pwmo_setDn( PICOC_FUN_ARGS )
{
  pwmo_setDn( (const double*)(ARG_0_PTR) );
}

void C_pwmo_setCCR0( PICOC_FUN_ARGS )
{
  pwmo_setCCR0( ARG_0_INT );
}

void C_pwmo_setCCR1( PICOC_FUN_ARGS )
{
  pwmo_setCCR1( ARG_0_INT );
}

void C_pwmo_setCCR2( PICOC_FUN_ARGS )
{
  pwmo_setCCR2( ARG_0_INT );
}

void C_pwmo_setCCR3( PICOC_FUN_ARGS )
{
  pwmo_setCCR3( ARG_0_INT );
}


void C_pwmo_getCCR0( PICOC_FUN_ARGS )
{
  RV_INT = pwmo_getCCR0();
}

void C_pwmo_getCCR1( PICOC_FUN_ARGS )
{
  RV_INT = pwmo_getCCR1();
}

void C_pwmo_getCCR2( PICOC_FUN_ARGS )
{
  RV_INT = pwmo_getCCR2();
}

void C_pwmo_getCCR3( PICOC_FUN_ARGS )
{
  RV_INT = pwmo_getCCR3();
}

// ---------------------------------------- ifm_0 = TIM2 ------------------------------------------

void ifm_0_measure()
{
  uint32_t cr = TIM2->CR1;

  if( ! ( cr & 1  ) ) { // disabled
    tim2_ccr1 =  0; tim2_ccr2  = 0;
    ifm_0_freq = 0; ifm_0_d    = 0;
    ifm_0_t0   = 0; ifm_0_td   = 0;
    return;
  }

  oxc_disable_interrupts();
  tim2_ccr1 = tim2_ccr1x; tim2_ccr2 = tim2_ccr2x;
  oxc_enable_interrupts();

  uint32_t cnt_freq = get_TIM_cnt_freq( TIM2 );
  if( cnt_freq < 1 ) { // fallback
    cnt_freq = 1;
  }

  xfloat tdt = (xfloat)(1) / (xfloat)(cnt_freq);
  ifm_0_t0 = tdt * tim2_ccr1;
  ifm_0_td = tdt * tim2_ccr2;

  if( tim2_ccr1 > 0 ) {
    ifm_0_freq = (xfloat)(cnt_freq) / tim2_ccr1  ;
    ifm_0_d    = ( (xfloat)(tim2_ccr2) / tim2_ccr1  );
  } else {
    ifm_0_freq = 0;
    ifm_0_d    = 0;
  }
}

void ifm_0_enable()
{
  TIM2->CR1 |= 1u;
  HAL_NVIC_EnableIRQ( TIM2_IRQn );
}

void ifm_0_disable()
{
  TIM2->CR1 &= ~1u;
  HAL_NVIC_DisableIRQ( TIM2_IRQn );
}

void C_ifm_0_measure( PICOC_FUN_ARGS )
{
  ifm_0_measure();
}

void C_ifm_0_enable( PICOC_FUN_ARGS )
{
  ifm_0_enable();
}

void C_ifm_0_disable( PICOC_FUN_ARGS )
{
  ifm_0_disable();
}

void ifm_out_stdout()
{
  std_out << ' ' << XFmt( ifm_0_freq, cvtff_fix, 9, 3 );
  std_out << ' ' << XFmt( ifm_0_d,    cvtff_fix, 6, 4 );
  std_out << ' ' << "11111"; // place for ch1
  std_out << ' ' << ifm_2_cnt;
  std_out << ' ' << "33333"; // place for ch3
}

void ifm_out_lcd()
{
  OSTR(s,40);

  s_outstr.reset_out();
  s << XFmt( ifm_0_freq, cvtff_fix, 9, 3 ) << ' ';
  lcdt.puts_xy( 0, 0, s_outstr.c_str() );

  s_outstr.reset_out();
  s << XFmt( ifm_0_d, cvtff_fix, 6, 4 ) << ' ';
  lcdt.puts_xy( 12, 0, s_outstr.c_str() );

  s_outstr.reset_out();
  s << ifm_2_cnt << ' ';
  lcdt.puts_xy( 0, 2, s_outstr.c_str() );

}

int ifm_pre_loop()
{
  ifm_2_cnt = 0;
  ifm_2_cntx = TIM3->CNT;
  ifm_0_enable();
  return 1;
}

int ifm_loop()
{
  ifm_0_measure();
  ifm_2_measure();
  ifm_out_stdout();
  ifm_out_lcd();
  return 1;
}

// ----------------------------------------  ------------------------------------------------------


// ---------------------------------------- ifm2 = TIM3.ETR ----------------------------------------


void ifm_2_measure()
{
  uint16_t cnt = TIM3->CNT;
  if( cnt >= ifm_2_cntx ) {
    ifm_2_cnt += cnt - ifm_2_cntx;
  } else {
    ifm_2_cnt += 0x10000 + cnt - ifm_2_cntx;
  }
  ifm_2_cntx = cnt;
}

void C_ifm_2_measure( PICOC_FUN_ARGS )
{
  ifm_2_measure();
}

// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

