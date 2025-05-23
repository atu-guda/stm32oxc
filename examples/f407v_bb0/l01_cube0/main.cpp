#include <cstdarg>
#include <cerrno>
#include <algorithm>
// #include <charconv>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_atleave.h>
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
// #include <oxc_io_fatfs.h>

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

const unsigned obuf_sz = 256;
char obuf_str[obuf_sz];
OutStr obuf_dev( obuf_str, obuf_sz );
OutStream obuf( &obuf_dev );


const unsigned lcdbuf_sz { 32 };
const unsigned o_sz      { 40 };
char lcdbuf_str0[lcdbuf_sz], lcdbuf_str1[lcdbuf_sz], lcdbuf_str2[lcdbuf_sz], lcdbuf_str3[lcdbuf_sz];
OutStr lcdbuf_dev0( lcdbuf_str0, lcdbuf_sz );
OutStr lcdbuf_dev1( lcdbuf_str1, lcdbuf_sz );
OutStr lcdbuf_dev2( lcdbuf_str2, lcdbuf_sz );
OutStr lcdbuf_dev3( lcdbuf_str3, lcdbuf_sz );
OutStream lcdbuf0( &lcdbuf_dev0 );
OutStream lcdbuf1( &lcdbuf_dev1 );
OutStream lcdbuf2( &lcdbuf_dev2 );
OutStream lcdbuf3( &lcdbuf_dev3 );

OutStream* obufs[] = { &obuf, &lcdbuf0, &lcdbuf1, &lcdbuf2, &lcdbuf3 };
constexpr int obufs_sz = size( obufs );
void obuf_add_str( const char *s, int b );
void C_obuf_add_str(  PICOC_FUN_ARGS );
void obuf_clear( unsigned b );
void C_obuf_clear( PICOC_FUN_ARGS );
void obuf_clear_all();
void C_obuf_clear_all( PICOC_FUN_ARGS );
void obuf_add_int( int v, int b );
void C_obuf_add_int( PICOC_FUN_ARGS );
void obuf_add_fp( xfloat v, int b );
void C_obuf_add_fp( PICOC_FUN_ARGS );
void obuf_add_fp_x( xfloat v, int cvtff_type, int width, int prec, int b );
void C_obuf_add_fp_x( PICOC_FUN_ARGS );
void obuf_add_fp_c( xfloat v, int b );
void C_obuf_add_fp_c( PICOC_FUN_ARGS );
void obuf_out_stdout( int b );
void C_obuf_out_stdout( PICOC_FUN_ARGS );
void obuf_out_ofile( int b );
void C_obuf_out_ofile( PICOC_FUN_ARGS );
void lcdbufs_out();
void C_lcdbufs_out( PICOC_FUN_ARGS );

FIL ofile;

int var_cvtff_fix = cvtff_fix, var_cvtff_exp = cvtff_exp, var_cvtff_auto = cvtff_auto;

int task_idx = 0, t_step_ms = 100, n_loops = 10000000;
int auto_out = 0, use_loops = 0, script_rv = 0, no_lcd_out = 0, btn_val = 0;
int menu_v_out1 = 0, menu_v_out2 = 0;
int menu_fun_reboot( int );
int menu_fun_dac_out1( int );
int menu_fun_dac_out2( int );
int menu_fun_dac_zero( int );

const Menu4bItem menu_main[] = {
  //     name            pv         step     min         max  fun      div10
  {    "task_idx",    &task_idx,      1,       0,         42, nullptr },
  {   "t_step_ms",   &t_step_ms,    100,     100,      10000, nullptr },
  {     "n_loops",     &n_loops,      1,       1,   10000000, nullptr },
  {    "auto_out",    &auto_out,      1,       0,          9, nullptr },
  {    "dac_out1", &menu_v_out1,      1,     -95,         95, menu_fun_dac_out1, 1 },
  {    "dac_out2", &menu_v_out2,      1,     -95,         95, menu_fun_dac_out2, 1 },
  {    "dac_zero",      nullptr,      0,      0,           0, menu_fun_dac_zero },
  {      "reboot",      nullptr,      0,      0,      100000, menu_fun_reboot }
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
DS3231 rtc( i2cd );
void oxc_picoc_hd44780_i2c_init( Picoc *pc );

ADS1115 adc( i2cd );
const unsigned adc_n_ch { 4 };
uint32_t adc_no   { 0 }; // channels get last time, usually adc_n_ch
uint32_t adc_o_w  { 8 }; // output width
uint32_t adc_o_p  { 5 }; // output precision
int      adc_o_nl { 0 }; // add newline after output
int adc_scale_mv { 4096 };
int adc_nm { 1 };  // number of iteration in adc_measure

// calibration measurement results
xfloat adc_c_v_up[adc_n_ch] = {  0,   0,   0,  0   };
xfloat adc_c_v_do[adc_n_ch] = {  0,   0,   0,  0   };
int    adc_c_i_up[adc_n_ch] = {  0,   0,   0,  0   };
int    adc_c_i_do[adc_n_ch] = {  0,   0,   0,  0   };
// calibration result. TODO: store to flash
xfloat adc_v_scales[adc_n_ch] = {  8.299420E-04f,   8.291177E-04f,   8.302860E-04f,    8.298045E-04f   };
xfloat adc_v_bases[adc_n_ch]  = { -9.95298487158f, -9.93398440656f, -9.963334554497f, -9.952166616965f };

xfloat adc_v[adc_n_ch]        = {         0.0f,        0.0f,         0.0f,        0.0f };
int    adc_vi[adc_n_ch]       = {            0,           0,            0,           0 };
xfloat adc_kv = adc_scale_mv / 4096.0f;
int adc_defcfg();
int adc_measure();
void adc_out();
void adc_out_i();
void adc_out_all();
void adc_out_all_i();
void adc_all();
void adc_all_i();
void adc_cal_to( xfloat v, int chan_bits, xfloat *va, int *ia );
void adc_cal_min( xfloat v, int chan_bits );
void adc_cal_max( xfloat v, int chan_bits );
void adc_cal_calc( int chan_bits );
int adc_pre_loop();
int adc_loop();
void C_adc_defcfg( PICOC_FUN_ARGS );
void C_adc_measure( PICOC_FUN_ARGS );
void C_adc_out( PICOC_FUN_ARGS );
void C_adc_out_i( PICOC_FUN_ARGS );
void C_adc_out_all( PICOC_FUN_ARGS );
void C_adc_out_all_i( PICOC_FUN_ARGS );
void C_adc_all( PICOC_FUN_ARGS );
void C_adc_all_i( PICOC_FUN_ARGS );
void C_adc_cal_min( PICOC_FUN_ARGS );
void C_adc_cal_max( PICOC_FUN_ARGS );
void C_adc_cal_calc( PICOC_FUN_ARGS );

extern DAC_HandleTypeDef hdac;
int MX_DAC_Init();
const unsigned dac_n_ch = 2;
const unsigned dac_bitmask = 0x0FFF; // 12 bit
xfloat dac_vref = 3.0f;
const xfloat dac_vmax = 10.0f;

xfloat dac_v_scales[dac_n_ch] = { 206.610971f, 205.901167f };
// xfloat dac_v_scales[dac_n_ch] = {  0.151366f,  0.150828f };
xfloat dac_v_bases[dac_n_ch]  = { 2042.323686, 2038.485565f  };
xfloat dac_v[dac_n_ch]        = {     0.0f,      0.0f  }; // expected to set (not given)
int    dac_vi[dac_n_ch]       = {        0,         0  }; // used to set

void dac_out_n( int n, xfloat v );
void dac_out1( xfloat v );
void dac_out2( xfloat v );
void dac_out12( xfloat v1, xfloat v2 );
void dac_out_ni( int n, int v );
void dac_out1i( int v );
void dac_out2i( int v );
void dac_out12i( int v1, int v2 );
void C_dac_out_n( PICOC_FUN_ARGS );
void C_dac_out1( PICOC_FUN_ARGS );
void C_dac_out2( PICOC_FUN_ARGS );
void C_dac_out12( PICOC_FUN_ARGS );
void C_dac_out1i( PICOC_FUN_ARGS );
void C_dac_out2i( PICOC_FUN_ARGS );
void C_dac_out_ni( PICOC_FUN_ARGS );
void C_dac_out12i( PICOC_FUN_ARGS );

// dadc - for dadc_scan[12]
xfloat v0_min { 0 }, v0_max { 1.0 }, v0_step { 0.05 };
xfloat v1_min { 0 }, v1_max { 1.0 }, v1_step { 0.05 };
int scan_delay { 100 }; // in ms
void dadc_common( xfloat v0, xfloat v1, bool ch2 );
void dadc1( xfloat v0 );
void dadc2( xfloat v0, xfloat v1 );
void dadc_scan1();
void dadc_scan2();
void C_dadc1( PICOC_FUN_ARGS );
void C_dadc2( PICOC_FUN_ARGS );
void C_dadc_scan1( PICOC_FUN_ARGS );
void C_dadc_scan2( PICOC_FUN_ARGS );

MCP23017 mcp_gpio( i2cd );
void C_pins_out( PICOC_FUN_ARGS );
void C_pins_out_read( PICOC_FUN_ARGS );
void C_pins_out_set( PICOC_FUN_ARGS );
void C_pins_out_reset( PICOC_FUN_ARGS );
void C_pins_out_toggle( PICOC_FUN_ARGS );
void C_pins_in( PICOC_FUN_ARGS );
void C_pin_in( PICOC_FUN_ARGS );

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5; // TIM5 - pwmo_
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim9;
int MX_TIM1_Init();
int MX_TIM2_Init();
int MX_TIM3_Init();
int MX_TIM4_Init();
int MX_TIM5_Init();
int MX_TIM8_Init();
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
xfloat ifm_1_freq = 0;
uint32_t ifm_1_cnt = 0;
uint32_t ifm_2_cnt = 0;
uint16_t ifm_2_cntx = 0;
uint32_t ifm_3_cnt = 0;
uint16_t ifm_3_cntx = 0;
void ifm_0_measure();
void ifm_0_enable();
void ifm_0_disable();
void C_ifm_0_measure( PICOC_FUN_ARGS );
void C_ifm_0_enable( PICOC_FUN_ARGS );
void C_ifm_0_disable( PICOC_FUN_ARGS );
void ifm_1_tick();
void ifm_1_measure();
void C_ifm_1_measure( PICOC_FUN_ARGS );
void ifm_2_measure();
void C_ifm_2_measure( PICOC_FUN_ARGS );
void ifm_2_reset();
void C_ifm_2_reset( PICOC_FUN_ARGS );
void ifm_3_measure();
void C_ifm_3_measure( PICOC_FUN_ARGS );
void ifm_3_reset();
void C_ifm_3_reset( PICOC_FUN_ARGS );

void ifm_out();
int  ifm_pre_loop();
int  ifm_loop();

void rtc_getDateTime( int *t ); // array: y m d H M S
void C_rtc_getDateTime( PICOC_FUN_ARGS );
void rtc_getDateStr( char *s ); // YYYY:MM:DD 11+ bytes
void C_rtc_getDateStr( PICOC_FUN_ARGS );
void rtc_getTimeStr( char *s ); // HH:MM:SS 9+ bytes
void C_rtc_getTimeStr( PICOC_FUN_ARGS );
void rtc_getDateTimeStr( char *s ); // YYYYmmDD_HHMMSS 16+ bytes
void C_rtc_getDateTimeStr( PICOC_FUN_ARGS );
void rtc_getFileDateTimeStr( char *s ); // o_YYYYmmDD_HHMMSS_XXX.txt 26+ bytes
void C_rtc_getFileDateTimeStr( PICOC_FUN_ARGS );
void C_flush( PICOC_FUN_ARGS );

#define PICOC_STACK_SIZE (32*1024)
int picoc_cmdline_handler_fb( char *s );
int picoc_cmdline_handler( char *s );
Picoc pc;
int init_picoc( Picoc *ppc );
// TMP here: move to system include
extern "C" {
void oxc_picoc_math_init( Picoc *pc );
}
//char a_char[] = "ABCDE";
//char *p_char = a_char;
void oxc_picoc_misc_init( Picoc *pc );
void oxc_picoc_fatfs_init( Picoc *pc );
char *do_PlatformReadFile( Picoc *pc, const char *FileName );
int  picoc_call( const char *code );
void picoc_local_SetupFunc( Picoc *pc );
void picoc_local_init( Picoc *pc );
void picoc_reg_int( const char *nm, int &var, int rw = TRUE );
void picoc_reg_u32t( const char *nm, uint32_t &var, int rw = TRUE );
void picoc_reg_int_arr( const char *nm, int *arr, int rw = TRUE );
void picoc_reg_float( const char *nm, xfloat &var, int rw = TRUE );
void picoc_reg_float_arr( const char *nm, xfloat *arr, int rw = TRUE );
void picoc_reg_char_arr( const char *nm, char *arr, int rw = TRUE );

int file_pre_loop();
int file_loop();

// void C_prf( PICOC_FUN_ARGS );
void tst_stdarg( const char *s, ... );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - run task"  };
int cmd_lcd_gotoxy( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_GOTOXY{ "lcd_gotoxy", 0, cmd_lcd_gotoxy, " x y - move pos to LCD (x, y)"  };
int cmd_lcd_xychar( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_XYCHAR{ "lcd_xychar", 0, cmd_lcd_xychar, " x y code - put char at x y to LCD"  };
int cmd_lcd_puts( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_PUTS{ "lcd_puts", 0, cmd_lcd_puts, "string - put string at cur pos to LCD"  };
int cmd_set_time( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_TIME { "stime", 0, cmd_set_time, " hour min sec - set RTC time "  };
int cmd_set_date( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_DATE { "sdate", 0, cmd_set_date, " year month day - set RTC date "  };
int cmd_menu( int argc, const char * const * argv );
CmdInfo CMDINFO_MENU { "menu", 'M', cmd_menu, " N - menu action"  };
int cmd_t1( int argc, const char * const * argv );
CmdInfo CMDINFO_T1 { "t1", 0, cmd_t1, " - misc test"  };
// int cmd_tst_stdarg( int argc, const char * const * argv );
// CmdInfo CMDINFO_TST_STDARG { "tst_stdarg", '\0', cmd_tst_stdarg, " - test stdarg"  };
int cmd_lstnames( int argc, const char * const * argv );
CmdInfo CMDINFO_LSTNAMES { "lstnames", 'L', cmd_lstnames, " [part] - list picoc names"  };
int cmd_adcall( int argc, const char * const * argv );
CmdInfo CMDINFO_ADCALL { "adcall", 'A', cmd_adcall, " - measure and output all ADC"  };
int cmd_dadc1( int argc, const char * const * argv );
CmdInfo CMDINFO_DADC1 { "dadc1", 'D', cmd_dadc1, " v0 - set DAC0 and measure and output all ADC"  };
int cmd_dadc2( int argc, const char * const * argv );
CmdInfo CMDINFO_DADC2 { "dadc2", '\0', cmd_dadc2, " v0 v1 - set DAC0,1 and measure and output all ADC"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_LCD_XYCHAR,
  &CMDINFO_LCD_GOTOXY,
  &CMDINFO_LCD_PUTS,
  &CMDINFO_SET_TIME,
  &CMDINFO_SET_DATE,
  &CMDINFO_T1,
  // &CMDINFO_TST_STDARG,
  &CMDINFO_LSTNAMES,
  &CMDINFO_ADCALL,
  &CMDINFO_DADC1,
  &CMDINFO_DADC2,
  &CMDINFO_MENU,
  FS_CMDS0,
  nullptr
};

int run_common( RUN_FUN pre_fun, RUN_FUN loop_fun )
{
  obuf.reset_out();
  lcdt.cls();

  obuf << "# Task " << task_idx << " n_loops= " << n_loops << " t_step_ms= " << t_step_ms << ' ';

  char ofilename[64]; ofilename[0] = '\0';
  bool was_out_open = false;
  rtc_getFileDateTimeStr( ofilename );

  if( auto_out ) {
    obuf << " outfile: " << ofilename;

    f_open( &ofile, ofilename, FA_WRITE | FA_OPEN_ALWAYS );
    if( ofile.err == FR_OK ) {
      was_out_open = true;
      // pfile = &ofile;
      obuf << " OK " << NL;
      obuf_out_ofile( 0 );
    } else {
      obuf << " Error: " << ofile.err << NL;
    }
  }

  std_out << obuf_str << NL;

  OSTR(s,obufs_sz);

  if( pre_fun != nullptr ) {
    pre_fun();
  }

  uint32_t tm0, tm00;
  break_flag = 0;
  for( decltype(+n_loops) i=0; i<n_loops && !break_flag; ++i ) {
    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    t_c = 0.001f * ( tcc - tm00 );

    obuf.reset_out();
    for( auto o : obufs ) { o->reset_out(); }

    s_outstr.reset_out();
    s << XFmt( t_c, cvtff_fix, 10, 2 );
    obuf << s_outstr.c_str() << ' ';

    if( loop_fun != nullptr ) { // really useless
      loop_fun();
    }

    obuf << NL;

    obuf_out_stdout( 0 );
    if( was_out_open ) {
      obuf_out_ofile( 0 );
    }
    lcdbufs_out();

    lcdt.puts_xy( 10, 3, s_outstr.c_str() ); // on flag?

    if( t_step_ms > 0 ) {
      delay_ms_until_brk( &tm0, t_step_ms );
    }
  }

  if( was_out_open ) {
    f_close( &ofile );
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
  char task_file[16];
  strcpy( task_file, "task_00.c" );
  char c0 = (char)( '0' + ( task_idx % 10 ) );
  char c1 = (char)( '0' + ( (task_idx/10) % 10 ) );
  task_file[6] = c0; task_file[5] = c1;
  std_out << "# Task N n= " << task_idx  << " file: " << task_file << NL;

  if( VariableDefined( &pc, TableStrRegister( &pc, "loop" ) ) ) {
    std_out << "# warn: object loop already defined, removing" NL;
    picoc_call( "delete loop;" );
  }
  if( VariableDefined( &pc, TableStrRegister( &pc, "post_loop" ) ) ) {
    std_out << "# warn: object post_loop already defined, removing" NL;
    picoc_call( "delete post_loop;" );
  }

  use_loops = 0; script_rv = 0;
  int rc = PicocPlatformScanFile( &pc, task_file );
  std_out << "# script end, rc= " << rc << " rv= " << script_rv << NL;

  if( rc != 1 ) {
    return 1;
  }

  if( script_rv != 0 ) {
    return script_rv;
  }

  if( !use_loops ) { // set from script
    return 0;
  }

  if( !VariableDefined( &pc, TableStrRegister( &pc, "loop" ) ) ) {
    std_out << "# error: function int loop() is not defined";
    return 1;
  }

  Value *FuncValue = nullptr;
  VariableGet( &pc, NULL, TableStrRegister( &pc, "loop" ), &FuncValue );
  if( FuncValue->Typ->Base != TypeFunction ) {
    std_out << "# error: loop is not a function - can't call it";
    return 1;
  }

  std_out << "# starting loops " << NL;
  rc =  run_common( file_pre_loop, file_loop );

  if( !VariableDefined( &pc, TableStrRegister( &pc, "post_loop" ) ) ) {
    return rc;
  }

  VariableGet( &pc, NULL, TableStrRegister( &pc, "post_loop" ), &FuncValue );
  if( FuncValue->Typ->Base != TypeFunction ) {
    return rc;
  }

  picoc_call( "post_loop();" );

  return rc;
}

int file_pre_loop()
{
  return 1;
}

int file_loop()
{
  picoc_call( "loop();" );
  return 1;
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

  XFmt::set_auto_width( 10 );

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;
  lcdt.init_4b();
  lcdt.cls();
  lcdt.putch( 'I' );

  init_menu4b_buttons();
  lcdt.putch( 'K' );

  mcp_gpio.cfg( MCP23017::iocon_intpol  ); // only for int
  mcp_gpio.set_dir_a( 0xFF ); // all input
  mcp_gpio.set_dir_b( 0x00 ); // all output
  mcp_gpio.set_b( 0x00 );

  lcdt.putch( 'p' );

  MX_TIM2_Init();
  lcdt.putch( '2' );
  MX_TIM3_Init();
  lcdt.putch( '3' );
  MX_TIM4_Init();
  lcdt.putch( '4' );
  MX_TIM5_Init();
  lcdt.putch( '5' );
  MX_TIM8_Init();
  lcdt.putch( '8' );
  MX_TIM9_Init();
  lcdt.putch( '9' );

  adc_defcfg();
  lcdt.putch(  'A'  );

  int dac_rc = MX_DAC_Init();
  lcdt.putch( dac_rc == 0 ? 'D' : 'd' );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2047 );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2047 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_2 );

  rtc.resetDev();
  rtc.setCtl( 0 ); // enable only clock on bat
  lcdt.putch( 't' );

  fs.fs_type = 0; // none
  fspath[0] = '\0';

  delay_ms( 10 );

  cmdline_handlers[0] = picoc_cmdline_handler;
  cmdline_handlers[1] = nullptr;
  cmdline_fallback_handler = picoc_cmdline_handler_fb;

  pc.InteractiveHead = nullptr;
  PlatformReadFile_fun = do_PlatformReadFile;
  init_picoc( &pc );
  lcdt.putch( 'c' );

  BOARD_POST_INIT_BLINK;
  leds.reset( 0xFF );

  pr( NL "##################### " PROJ_NAME NL );
  char s[o_sz];
  rtc.getDateStr( s );
  std_out << "# " << s << ' ';
  lcdt.puts_xy( 0, 2, s );
  rtc.getTimeStr( s );
  std_out << s << NL;
  lcdt.puts_xy( 12, 2, s );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( ifm_1_tick );
  oxc_add_aux_tick_fun( menu4b_ev_dispatch );

  UVAR('e') = init_usbh_msc();
  lcdt.putch( 'u' );
  lcdt.puts_xy( 0, 1, menu4b_state.menu_level0_str );

  delay_ms( 10 );

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

  btn_val = 0;
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
  return 0;
}

int picoc_cmdline_handler_fb( char *s )
{
  std_out << NL "# C: cmd= \"" << s << '"' << NL;
  delay_ms( 10 );

  int ep_rc =  PicocPlatformSetExitPoint( &pc );
  if( ep_rc == 0 ) {
    picoc_call( s );
  } else {
    std_out << "## Exit point: " << ep_rc << NL;
  }

  return ep_rc;
}

int picoc_cmdline_handler( char *s )
{
  if( !s  ||  s[0] != ';' ) { // not my
    return -1;
  }
  return picoc_cmdline_handler_fb ( s + 1 );
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
        errno = 0;
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
      leds.set( BIT0 );
      leds.reset( BIT2 );
      f_mount( nullptr, (TCHAR const*)"", 0 );
      isMSC_ready = 0;
      errno = 7555;
      break;

    default:
      break;
  }
}

// --------------------------------- picoc ----------------------------------------------

struct LibraryFunction picoc_local_Functions[] =
{
  { C_obuf_add_str,          "void obuf_add_str(char*,int);" },
  { C_obuf_add_int,          "void obuf_add_int(int,int);" },
  { C_obuf_add_fp,           "void obuf_add_fp(float,int);" },
  { C_obuf_add_fp_x,         "void obuf_add_fp_x(float,int,int,int,int);" },
  { C_obuf_add_fp_c,         "void obuf_add_fp_c(float,int);" },
  { C_obuf_clear,            "void obuf_clear(int);" },
  { C_obuf_clear_all,        "void obuf_clear_all(void);" },
  { C_obuf_out_stdout,       "void obuf_out_stdout(int);" },
  { C_obuf_out_ofile,        "void obuf_out_ofile(int);" },
  { C_lcdbufs_out,           "void lcdbufs_out();" },

  { C_adc_defcfg,            "int adc_defcfg(void);" },
  { C_adc_measure,           "int adc_measure(void);" },
  { C_adc_out,               "void adc_out(void);" },
  { C_adc_out_all,           "void adc_out_all(void);" },
  { C_adc_all,               "void adc_all(void);" },
  { C_adc_out_i,             "void adc_out_i(void);" },
  { C_adc_out_all_i,         "void adc_out_all_i(void);" },
  { C_adc_all_i,             "void adc_all_i(void);" },
  { C_adc_cal_min,           "void adc_cal_min(float,int);" },
  { C_adc_cal_max,           "void adc_cal_max(float,int);" },
  { C_adc_cal_calc,          "void adc_cal_calc(int);" },

  { C_dac_out_n,             "void dac_out_n(int,float);" },
  { C_dac_out1,              "void dac_out1(float);" },
  { C_dac_out2,              "void dac_out2(float);" },
  { C_dac_out12,             "void dac_out12(float,float);" },
  { C_dac_out1i,             "void dac_out1i(int);" },
  { C_dac_out2i,             "void dac_out2i(int);" },
  { C_dac_out_ni,            "void dac_out_ni(int,int);" },
  { C_dac_out12i,            "void dac_out12i(int,int);" },

  { C_dadc1     ,            "void dadc1(float);" },
  { C_dadc2     ,            "void dadc1(float,float);" },
  { C_dadc_scan1,            "void dadc_scan1();" },
  { C_dadc_scan2,            "void dadc_scan2();" },

  { C_pins_out,              "void pins_out(int);" },
  { C_pins_out_read,         "int  pins_out_read(void);" },
  { C_pins_out_set,          "void pins_out_set(int);" },
  { C_pins_out_reset,        "void pins_out_reset(int);" },
  { C_pins_out_toggle,       "void pins_out_toggle(int);" },
  { C_pins_in,               "int  pins_in(void);" },
  { C_pin_in,                "int  pin_in(int);" },

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
  { C_ifm_1_measure,         "void ifm_1_measure();  " },
  { C_ifm_2_measure,         "void ifm_2_measure();  " },
  { C_ifm_2_measure,         "void ifm_2_reset();  " },
  { C_ifm_3_measure,         "void ifm_3_measure();  " },
  { C_ifm_3_measure,         "void ifm_3_reset();  " },

  { C_rtc_getDateTime,       "void rtc_getDateTime( int* );  " },
  { C_rtc_getDateStr,        "void rtc_getDateStr( char* );  " },
  { C_rtc_getTimeStr,        "void rtc_getTimeStr( char* );  " },
  { C_rtc_getDateTimeStr,    "void rtc_getDateTimeStr( char* );  " },
  { C_rtc_getFileDateTimeStr,"void rtc_getFileDateTimeStr( char* );  " },
  { C_flush,                 "void flush();  " },

  //{ C_prf,                   "int prf( char*, ... );  " },
  { NULL,            NULL }
};

void picoc_local_SetupFunc( Picoc *pc )
{
}

void picoc_local_init( Picoc *pc )
{
  IncludeRegister( pc, "local.h", &picoc_local_SetupFunc, picoc_local_Functions, NULL );
}

void picoc_reg_int( const char *nm, int &var, int rw /*= TRUE */ )
{
  VariableDefinePlatformVar( &pc, nullptr , nm, &(pc.IntType), (union AnyValue *)&var, rw );
}

void picoc_reg_u32t( const char *nm, uint32_t &var, int rw /*= TRUE */ )
{
  VariableDefinePlatformVar( &pc, nullptr , nm, &(pc.IntType), (union AnyValue *)&var, rw );
}

void picoc_reg_int_arr( const char *nm, int *arr, int rw /*= TRUE */  )
{
  VariableDefinePlatformVar( &pc, nullptr , nm, pc.IntArrayType, (union AnyValue *)arr, rw );
}

void picoc_reg_float( const char *nm, xfloat &var, int rw /*= TRUE */ )
{
  VariableDefinePlatformVar( &pc, nullptr , nm, &(pc.FPType), (union AnyValue *)&var, rw );
}


void picoc_reg_float_arr( const char *nm, xfloat *arr, int rw /*= TRUE */  )
{
  VariableDefinePlatformVar( &pc, nullptr , nm, pc.FPArrayType, (union AnyValue *)arr, rw );
}

void picoc_reg_char_arr( const char *nm, char *arr, int rw /*= TRUE */  )
{
  VariableDefinePlatformVar( &pc, nullptr , nm, pc.CharArrayType, (union AnyValue *)arr, rw );
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

  picoc_reg_int( "cvtff_fix", var_cvtff_fix   , FALSE );
  picoc_reg_int( "cvtff_exp", var_cvtff_exp   , FALSE );
  picoc_reg_int( "cvtff_auto", var_cvtff_auto  , FALSE );

  picoc_reg_float( "t_c", t_c );
  picoc_reg_int( "task_idx", task_idx );
  picoc_reg_int( "n_loops", n_loops );
  picoc_reg_int( "t_step_ms", t_step_ms );
  picoc_reg_int( "auto_out", auto_out );
  picoc_reg_int( "use_loops", use_loops );
  picoc_reg_int( "script_rv", script_rv );
  picoc_reg_int( "no_lcd_out", no_lcd_out );
  picoc_reg_int( "btn_val", btn_val );
  picoc_reg_char_arr( "obuf_str", obuf_str );
  picoc_reg_char_arr( "lcdbuf_str0", lcdbuf_str0 );
  picoc_reg_char_arr( "lcdbuf_str1", lcdbuf_str1 );
  picoc_reg_char_arr( "lcdbuf_str2", lcdbuf_str2 );
  picoc_reg_char_arr( "lcdbuf_str3", lcdbuf_str3 );

  picoc_reg_float_arr( "adc_v", adc_v );
  picoc_reg_float_arr( "adc_v_scales", adc_v_scales );
  picoc_reg_float_arr( "adc_v_bases", adc_v_bases );
  picoc_reg_float_arr( "adc_c_v_up", adc_c_v_up );
  picoc_reg_float_arr( "adc_c_v_do", adc_c_v_do );
  picoc_reg_int_arr( "adc_vi", adc_vi );
  picoc_reg_int_arr( "adc_c_i_up", adc_c_i_up );
  picoc_reg_int_arr( "adc_c_i_do", adc_c_i_do );
  picoc_reg_u32t( "adc_no", adc_no );
  picoc_reg_u32t( "adc_o_w", adc_o_w );
  picoc_reg_u32t( "adc_o_p", adc_o_p );
  picoc_reg_int( "adc_o_nl", adc_o_nl );
  picoc_reg_int( "adc_scale_mv", adc_scale_mv );
  picoc_reg_int( "adc_nm", adc_nm );

  picoc_reg_float_arr( "dac_v_scales", dac_v_scales );
  picoc_reg_float_arr( "dac_v_bases", dac_v_bases );
  picoc_reg_float_arr( "dac_v", dac_v );
  picoc_reg_int_arr( "dac_vi", dac_vi );

  picoc_reg_float( "v0_min", v0_min );
  picoc_reg_float( "v0_max", v0_max );
  picoc_reg_float( "v0_mstep", v0_step );
  picoc_reg_float( "v1_min", v1_min );
  picoc_reg_float( "v1_max", v1_max );
  picoc_reg_float( "v1_mstep", v1_step );
  picoc_reg_int( "scan_delay", scan_delay );

  picoc_reg_float( "ifm_0_freq", ifm_0_freq );
  picoc_reg_float( "ifm_0_d", ifm_0_d );
  picoc_reg_float( "ifm_0_t0", ifm_0_t0 );
  picoc_reg_float( "ifm_0_td", ifm_0_td );
  picoc_reg_float( "ifm_1_freq", ifm_1_freq );
  picoc_reg_u32t( "ifm_1_cnt", ifm_1_cnt );
  picoc_reg_u32t( "ifm_2_cnt", ifm_2_cnt );
  picoc_reg_u32t( "ifm_3_cnt", ifm_3_cnt );

  return 0;
}

char* do_PlatformReadFile( Picoc *pc, const char *fn )
{
  if( !pc || !fn || !fn[0] ) {
    return nullptr;
  }

  FILINFO fi;

  auto rc = f_stat( fn, &fi );
  if( rc != FR_OK ) {
    std_out << "# Error: f_stat failed, fn=\"" << fn << "\" rc= " << rc << NL;
    errno = 1000 + rc;
    return nullptr;
  }

  // TODO: limit to 32-bit?
  char *buf = (char*)( malloc( fi.fsize + 2 ) );
  if( !buf ) {
    std_out << "# Error: fail to alloc fn=\"" << (int)fi.fsize << " bytes "  << NL; // TODO: output long long
    errno = ENOMEM;
    return nullptr;
  }
  memset( buf, 0x00, fi.fsize + 2 );

  FIL f;
  rc = f_open( &f, fn, FA_READ );
  if( rc != FR_OK ) {
    free( buf );
    std_out << "# Error: fail to open fn=\"" << fn << "\" rc= " << rc << NL;
    errno = 1000 + rc;
    return nullptr;
  }

  unsigned r;
  rc = f_read( &f, buf, (unsigned)(fi.fsize), &r ); // TODO: iterate?
  if( rc != FR_OK ) {
    free( buf );
    std_out << "# Error: fail to read fn=\"" << fn << "\" rc= " << rc << NL;
    errno = 1000 + rc;
    return nullptr;
  }

  return buf;

}

int picoc_call( const char *code )
{
  if( code == nullptr || code[0] == '\0' ) {
    return 0;
  }

  //                                                    run   cle   cleSo   dbg
  return PicocParse( &pc, "code", code, strlen( code ), TRUE, TRUE, FALSE, FALSE );

}

// ---------------------------------------- buffers---------------------------------------------------

void obuf_add_str( const char *s, int b )
{
  if( b >= 0 && b < obufs_sz ) {
    *obufs[b] << s;
  }
}

void C_obuf_add_str( PICOC_FUN_ARGS )
{
  obuf_add_str( (const char*)(ARG_0_PTR), ARG_1_INT );
}

void obuf_add_int( int v, int b )
{
  if( b >= 0 && b < obufs_sz ) {
    *obufs[b] << v;
  }
}

void C_obuf_add_int( PICOC_FUN_ARGS )
{
  obuf_add_int( ARG_0_INT, ARG_1_INT  );
}

void obuf_add_fp( xfloat v, int b )
{
  if( b >= 0 && b < obufs_sz ) {
    *obufs[b] << v;
  }
}

void C_obuf_add_fp( PICOC_FUN_ARGS )
{
  obuf_add_fp( ARG_0_FP, ARG_1_INT  );
}

void obuf_add_fp_x( xfloat v, int cvtff_type, int width, int prec, int b )
{
  if( b >= 0 && b < obufs_sz ) {
    *obufs[b] << XFmt( v, cvtff_type, width, prec );
  }
}

void C_obuf_add_fp_x( PICOC_FUN_ARGS )
{
  obuf_add_fp_x( ARG_0_FP, ARG_1_INT, ARG_2_INT, ARG_3_INT, ARG_4_INT );
}


void obuf_add_fp_c( xfloat v, int b )
{
  if( b >= 0 && b < obufs_sz ) {
    *obufs[b] << XFmt( v, cvtff_fix, 8, 5 ); // TODO: var for control
  }
}

void C_obuf_add_fp_c( PICOC_FUN_ARGS )
{
  obuf_add_fp_c( ARG_0_FP, ARG_1_INT  );
}


void obuf_clear( int b )
{
  if( b >= 0 && b < obufs_sz ) {
    obufs[b]->reset_out();
  }
}

void C_obuf_clear( PICOC_FUN_ARGS )
{
  obuf_clear( ARG_0_INT );
}

void obuf_clear_all()
{
  for( auto b : obufs ) {
    b->reset_out();
  }
}

void C_obuf_clear_all( PICOC_FUN_ARGS )
{
  obuf_clear_all();
}

void obuf_out_stdout( int b )
{
  if( b >= 0 && b < obufs_sz ) {
    std_out << obufs[b]->getOut()->getBuf();
  }
}

void C_obuf_out_stdout( PICOC_FUN_ARGS )
{
  obuf_out_stdout( ARG_0_INT );
}

void obuf_out_ofile( int b )
{
  if( b >= 0 && b < obufs_sz ) {
    f_puts(  obufs[b]->getOut()->getBuf(), &ofile );
  }
}

void C_obuf_out_ofile( PICOC_FUN_ARGS )
{
  obuf_out_ofile( ARG_0_INT );
}

void lcdbufs_out()
{
  if( ! no_lcd_out ) {
    lcdt.cls();
    lcdt.puts_xy( 0, 0, lcdbuf_str0 );
    lcdt.puts_xy( 0, 1, lcdbuf_str1 );
    lcdt.puts_xy( 0, 2, lcdbuf_str2 );
    lcdt.puts_xy( 0, 3, lcdbuf_str3 );
  }
}

void C_lcdbufs_out( PICOC_FUN_ARGS )
{
  lcdbufs_out();
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
      // break_flag = 1; errno = 10000;
      btn_val = 0;
      break;
    case  MenuCmd::Up:
      ++btn_val;
      break;
    case  MenuCmd::Down:
      --btn_val;
      break;
    case  MenuCmd::Enter:
      break_flag = 1; errno = 10001;
      // btn_run = !btn_run;
      break;
    default: break;
  }
}

int menu_fun_reboot( int )
{
  NVIC_SystemReset();
  return 0; // fake
}

int menu_fun_dac_out1( int v10 )
{
  dac_out1( 0.1f * v10 );
  return 0;
}

int menu_fun_dac_out2( int v20 )
{
  dac_out2( 0.1f * v20 );
  return 0;
}

int menu_fun_dac_zero( int  )
{
  dac_out12( 0.0f, 0.0f );
  return 0;
}


// ---------------------------------------- ADC ------------------------------------------------------

int adc_defcfg()
{
  adc.setDefault();
  uint16_t cfg =  ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  adc.setCfg( cfg );
  int r = adc.getDeviceCfg(); // to fill correct inner
  adc_scale_mv =  adc.getScale_mV();
  adc_kv = adc_scale_mv / 4096.0f; // scale to 1.0 by
  return r;
}

int adc_measure()
{
  int32_t vi_sum[adc_n_ch];
  for( auto &x : vi_sum ) { x = 0; }
  unsigned n_sum {0};

  for( int n = 0; n < adc_nm && !break_flag; ++n ) {
    int16_t vi[adc_n_ch];
    decltype(+adc_no) no = adc.getOneShotNch( 0, adc_n_ch-1, vi );
    n_sum += no;

    for( decltype(+adc_n_ch) j=0; j<adc_n_ch; ++j ) {
      vi_sum[j] += vi[j];
    }
  }

  for( decltype(+adc_n_ch) j=0; j<adc_n_ch; ++j ) {
    adc_vi[j] = vi_sum[j] / adc_nm;
    adc_v[j]  = adc_vi[j] * adc_kv * adc_v_scales[j] + adc_v_bases[j];
  }

  n_sum /= adc_nm;
  adc_no = n_sum;
  return adc_no;
}

void adc_out()
{
  OSTR( s, o_sz );
  for( decltype(+adc_no) j=0; j<adc_no; ++j ) {
    s.reset_out();
    s << XFmt( adc_v[j], cvtff_fix, adc_o_w, adc_o_p );
    obuf << ' ' << s_outstr.c_str();
    *obufs[j+1] << s_outstr.c_str();
  }
}

void adc_out_i()
{
  OSTR( s, o_sz );
  for( decltype(+adc_no) j=0; j<adc_no; ++j ) {
    s.reset_out();
    s << adc_vi[j];
    obuf << ' ' << s_outstr.c_str();
    *obufs[j+1] << s_outstr.c_str();
  }
}


void adc_out_all()
{
  for( auto o : obufs ) { o->reset_out(); }
  adc_out();
  obuf_out_stdout( 0 );
  if( adc_o_nl ) {
    std_out << '\n';
  }
  lcdbufs_out();
}

void adc_out_all_i()
{
  for( auto o : obufs ) { o->reset_out(); }
  adc_out_i();
  obuf_out_stdout( 0 );
  lcdbufs_out();
}


void adc_all()
{
  adc_measure();
  adc_out_all();
}

void adc_all_i()
{
  adc_measure();
  adc_out_all_i();
}


void adc_cal_to( xfloat v, int chan_bits, xfloat *va, int *ia )
{
  RestoreAtLeave r_xx( adc_nm );
  if( adc_nm < 20 ) { adc_nm = 20; }
  adc_measure();
  for( unsigned i=0; i < adc_n_ch; ++i ) {
    if( chan_bits & ( 1 << i ) ) {
      va[i] = v;
      ia[i] = adc_vi[i];
      std_out << "# cal " << i <<  ' ' << ia[i] << ' ' << v << ' ' << adc_v[i] << NL;
    }
  }
}

void adc_cal_min( xfloat v, int chan_bits )
{
  adc_cal_to( v, chan_bits, adc_c_v_do, adc_c_i_do );
}

void adc_cal_max( xfloat v, int chan_bits )
{
  adc_cal_to( v, chan_bits, adc_c_v_up, adc_c_i_up );
}

void adc_cal_calc( int chan_bits )
{
  for( unsigned i=0; i < adc_n_ch; ++i ) {
    if( ! (chan_bits & ( 1 << i ) ) ) {
      continue;
    }
    std_out << "# cal: ch " << i << ' ' << adc_c_v_do[i] << ' ' << adc_c_v_up[i]
      <<  ' ' << adc_c_i_do[i] << ' ' <<  adc_c_i_up[i]
      <<  ' ' << adc_v_scales[i] << ' ' << adc_v_bases[i] << NL;

    if( adc_c_v_do[i] >= adc_c_v_up[i]  ||  adc_c_i_do[i] >= adc_c_i_up[i]  ) {
      std_out << "# cal: bad data" << NL;
      continue;
    }
    xfloat k = ( adc_c_v_up[i] - adc_c_v_do[i] ) / ( adc_c_i_up[i] - adc_c_i_do[i] );
    adc_v_scales[i] = k / adc_kv;
    adc_v_bases[i]  = adc_c_v_do[i] - k * adc_c_i_do[i];
    std_out << "# cal: " << adc_v_scales[i] << ' ' << adc_v_bases[i] << NL;
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
  adc_out();
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

void C_adc_out( PICOC_FUN_ARGS )
{
  adc_out();
}

void C_adc_out_i( PICOC_FUN_ARGS )
{
  adc_out_i();
}

void C_adc_out_all( PICOC_FUN_ARGS )
{
  adc_out_all();
}

void C_adc_out_all_i( PICOC_FUN_ARGS )
{
  adc_out_all_i();
}

void C_adc_all( PICOC_FUN_ARGS )
{
  adc_all();
}

void C_adc_all_i( PICOC_FUN_ARGS )
{
  adc_all_i();
}

void C_adc_cal_min( PICOC_FUN_ARGS )
{
  xfloat v = ARG_0_FP;
  int bits = ARG_1_INT;
  adc_cal_min( v, bits );
}

void C_adc_cal_max( PICOC_FUN_ARGS )
{
  xfloat v = ARG_0_FP;
  int bits = ARG_1_INT;
  adc_cal_max( v, bits );
}

void C_adc_cal_calc( PICOC_FUN_ARGS )
{
  int bits = ARG_0_INT;
  adc_cal_calc( bits );
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

void C_pin_in( PICOC_FUN_ARGS )
{
  uint8_t val = (uint8_t)(ARG_0_INT);
  RV_INT = ( mcp_gpio.get_a() >> val ) & 1;
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

void dac_out_n( int n, xfloat v )
{
  n = clamp( n, 0, (int)dac_n_ch-1 );
  // int iv = (int)( dac_bitmask * ( v - dac_v_bases[n] ) * dac_v_scales[n] / dac_vref );
  int iv = (int) ( v  * dac_v_scales[n] + dac_v_bases[n] ); // TODO: vref restore?
  dac_out_ni( n, iv );
}

void dac_out1( xfloat v )
{
  dac_out_n( 0, v );
}

void dac_out2( xfloat v )
{
  dac_out_n( 1, v );
}

void dac_out12( xfloat v1, xfloat v2 )
{
  dac_out1( v1 );
  dac_out2( v2 );
}

void dac_out_ni( int n, int v )
{
  n = clamp( n, 0, (int)dac_n_ch-1 );
  v = clamp( v, 0, (int)dac_bitmask );
  dac_vi[n] = v;
  dac_v[n]  = ( v - dac_v_bases[n] )/ dac_v_scales[n];
  HAL_DAC_SetValue( &hdac, ( n==0 ) ? DAC_CHANNEL_1 : DAC_CHANNEL_2, DAC_ALIGN_12B_R, v );
}


void dac_out1i( int v )
{
  dac_out_ni( 0, v );
}

void dac_out2i( int v )
{
  dac_out_ni( 1, v );
}

void dac_out12i( int v1, int v2 )
{
  dac_out1i( v1 );
  dac_out2i( v2 );
}

void C_dac_out_n( PICOC_FUN_ARGS )
{
  dac_out_n( ARG_0_INT, ARG_1_FP );
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

void C_dac_out_ni( PICOC_FUN_ARGS )
{
  dac_out_ni( ARG_0_INT, ARG_1_INT );
}


// ---------------------------------------- dadc - scan -------------------------------------------

void dadc_common( xfloat v0, xfloat v1, bool ch2 )
{
  RestoreAtLeave r_xx( adc_o_nl, 1 );
  if( ch2 ) {
    dac_out12( v0, v1 );
  } else {
    dac_out1( v0 );
  }
  delay_ms( scan_delay );
  adc_measure();
  std_out << v0 << ' ' << v1 << ' ';
  if( ch2 ) {
    std_out << v1 << ' ';
  }
  adc_out_all();
  delay_ms( 2 );
}


void dadc1( xfloat v0 )
{
  dadc_common( v0, 0, false );
}

void dadc2( xfloat v0, xfloat v1 )
{
  dadc_common( v0, v1, true );
}


void dadc_scan1()
{
  break_flag = 0;
  for( xfloat v0 = v0_min; v0 <= v0_max && ! break_flag; v0 += v0_step ) {
    dadc1( v0 );
    // leds[1].toggle(); // no: power line influenced
  }
  dac_out1( 0 );
}

void dadc_scan2()
{
  break_flag = 0;
  for( xfloat v1 = v1_min; v1 <= v1_max && ! break_flag; v1 += v1_step ) {
    for( xfloat v0 = v0_min; v0 <= v0_max && ! break_flag; v0 += v0_step ) {
      dadc2( v0, v1 );
      // leds[1].toggle();
    }
    std_out << NL; // for gnuplot blocs
  }
  dac_out12( 0, 0 );
}

void C_dadc1( PICOC_FUN_ARGS )
{
  dadc1( ARG_0_FP );
}

void C_dadc2( PICOC_FUN_ARGS )
{
  dadc2( ARG_0_FP, ARG_1_FP );
}

void C_dadc_scan1( PICOC_FUN_ARGS )
{
  dadc_scan1();
}

void C_dadc_scan2( PICOC_FUN_ARGS )
{
  dadc_scan2();
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
  // TODO: auto enable/disable IRQ + flag: was measured
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

void ifm_out()
{
  OSTR(s,o_sz);
  s.reset_out();
  s << XFmt( ifm_0_freq, cvtff_fix, 9, 3 );
  obuf << ' ' << s_buf;
  lcdbuf0 << s_buf << "   ";

  s.reset_out();
  s << XFmt( ifm_0_d,    cvtff_fix, 6, 4 );
  obuf << ' ' << s_buf;
  lcdbuf0 << s_buf;

  s.reset_out();
  s << XFmt( ifm_1_freq, cvtff_fix, 10, 1 );
  obuf << ' ' << s_buf;
  lcdbuf1 << s_buf;

  s.reset_out();
  s << ' ' << ifm_2_cnt;
  obuf << ' ' << s_buf;
  lcdbuf2 << s_buf;

  s.reset_out();
  s << ' ' << ifm_3_cnt;
  obuf << ' ' << s_buf;
  lcdbuf3 << s_buf;
}


int ifm_pre_loop()
{
  ifm_1_cnt = 0; TIM4->CNT = 0; TIM8->CNT = 0;
  ifm_2_cnt = 0; ifm_2_cntx = TIM3->CNT;
  ifm_3_cnt = 0; ifm_3_cntx = TIM9->CNT;
  ifm_0_enable();
  return 1;
}

int ifm_loop()
{
  ifm_0_measure();
  ifm_1_measure();
  ifm_2_measure();
  ifm_3_measure();
  ifm_out();
  return 1;
}

// ---------------------------------------- ifm1 = TIM4 (+TIM8?)------------------------------------

void ifm_1_tick() // callback from aux_tick
{
  static unsigned nn = 0;
  ++nn;
  if( nn == 100 ) { // every 100 ms
    nn = 0;
    ifm_1_cnt = TIM4->CNT + ( (uint32_t)TIM8->CNT << 16 );
    TIM4->CNT = 0;
    TIM8->CNT = 0;
  }
}

void ifm_1_measure()
{
  ifm_1_freq = ifm_1_cnt * 10;
}

void C_ifm_1_measure( PICOC_FUN_ARGS )
{
  ifm_1_measure();
}


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

void ifm_2_reset()
{
  uint16_t cnt = TIM3->CNT;
  ifm_2_cntx = cnt;
  ifm_2_cnt = 0;
}


void C_ifm_2_reset( PICOC_FUN_ARGS )
{
  ifm_2_reset();
}


// ---------------------------------------- ifm3 = TIM9.CH1 ----------------------------------------


void ifm_3_measure()
{
  uint16_t cnt = TIM9->CNT;
  if( cnt >= ifm_3_cntx ) {
    ifm_3_cnt += cnt - ifm_3_cntx;
  } else {
    ifm_3_cnt += 0x10000 + cnt - ifm_3_cntx;
  }
  ifm_3_cntx = cnt;
}

void C_ifm_3_measure( PICOC_FUN_ARGS )
{
  ifm_3_measure();
}

void ifm_3_reset()
{
  uint16_t cnt = TIM9->CNT;
  ifm_3_cntx = cnt;
  ifm_3_cnt = 0;
}


void C_ifm_3_reset( PICOC_FUN_ARGS )
{
  ifm_3_reset();
}

// ---------------------------------------- RTC ----------------------------------------------------

int cmd_set_time( int argc, const char * const * argv )
{
  if( argc < 4 ) {
    std_out <<  "3 args required" NL ;
    return 1;
  }
  uint8_t t_hour = atoi( argv[1] );
  uint8_t t_min  = atoi( argv[2] );
  uint8_t t_sec  = atoi( argv[3] );
  return rtc.setTime( t_hour, t_min, t_sec );
}


int cmd_set_date( int argc, const char * const * argv )
{
  if( argc < 4 ) {
    std_out <<  "3 args required" NL;
    return 1;
  }
  uint16_t year = atoi( argv[1] );
  int8_t mon    = atoi( argv[2] );
  int8_t day    = atoi( argv[3] );
  return rtc.setDate( year, mon, day );
}

void rtc_getDateTime( int *t ) // array: y m d H M S
{
  if( !t ) {
    return;
  }

  rtc.getDateTime( t );
}

void C_rtc_getDateTime( PICOC_FUN_ARGS )
{
  int *t = (int*)(ARG_0_PTR);
  rtc_getDateTime( t );
}

void rtc_getDateStr( char *s ) // YYYY:MM:DD 11+ bytes
{
  if( !s ) {
    return;
  }
  rtc.getDateStr( s );
}

void C_rtc_getDateStr( PICOC_FUN_ARGS )
{
  char *s = (char*)(ARG_0_PTR);
  rtc_getDateStr( s );
}

void rtc_getTimeStr( char *s ) // HH:MM:SS 9+ bytes
{
  if( !s ) {
    return;
  }
  rtc.getTimeStr( s );
}

void C_rtc_getTimeStr( PICOC_FUN_ARGS )
{
  char *s = (char*)(ARG_0_PTR);
  rtc_getTimeStr( s );
}

void rtc_getDateTimeStr( char *s ) // YYYYmmDD_HHMMSS 16+ bytes
{
  if( !s ) {
    return;
  }
  rtc.getDateTimeStr( s );
}

void C_rtc_getDateTimeStr( PICOC_FUN_ARGS )
{
  char *s = (char*)(ARG_0_PTR);
  rtc_getDateTimeStr( s );
}

void rtc_getFileDateTimeStr( char *s ) // o_YYYYmmDD_HHMMSS_XXX.txt 26+ bytes
{
  if( !s ) {
    return;
  }
  rtc_getDateTimeStr( s+2 );
  s[0] = 'o'; s[1] = '_';
  strcat( s, "_XXX.txt" ); // TODO: XXX = parameter
}

void C_rtc_getFileDateTimeStr( PICOC_FUN_ARGS )
{
  char *s = (char*)(ARG_0_PTR);
  rtc_getFileDateTimeStr( s );
}

void C_flush( PICOC_FUN_ARGS )
{
  std_out.flush();
}

// ---------------------------------------- stdarg test --------------------------------------------


void tst_stdarg( const char *s, ... )
{
  std_out << "# s     = " << HexInt( (void*)s ) << NL;
  std_out << "# s addr= " << HexInt( (void*)&s ) << NL;
  dump8( &s, 0x50 );
  static_assert( sizeof(va_list) == sizeof(void*), "Code only for trivial va_list" );
  va_list ap;
  va_start( ap, s );

  vprintf( s, ap );

  va_end( ap );
  printf( "#--- %18.10g ---\n", 1.2345678912e-87 );
  std_out << "# sz(d)= " << sizeof(double) << " sz(l)= " << sizeof(long) << NL;
}

// int cmd_tst_stdarg( int argc, const char * const * argv )
// {
//   std_out << "# stdarg test " << NL;
//   tst_stdarg(            "-- %X %lg %X %X %c \n", 0x1234, 1.12345e-12, -1, 0x87654321, 'Z' );
//   std_out << R"!!!(;prf( "-- %X %lg %X %X %c \n", 0x1234, 1.12345e-12, -1, 0x87654321, 'Z' ); )!!!" << NL;
//   return 0;
// }

// ;prf("[ %X %lg %s %c]\n", 1, 1.2, "xxx", 'A' );
// void C_prf( PICOC_FUN_ARGS )
// {
//   const unsigned argbuf_sz = 128;
//   alignas(double) char argbuf[argbuf_sz];
//   unsigned a_ofs = 0;
//
//   if( NumArgs < 1 ) {
//     RV_INT = 0;
//     return;
//   }
//
//   memset( argbuf, 0, argbuf_sz );
//
//   const char *fmt = (const char*)(ARG_0_PTR);
//   if( !fmt ) {
//     RV_INT = 0;
//     return;
//   }
//
//   struct Value *na = Param[0];
//   for( int i=0; i<NumArgs; ++i, na = (struct Value *)( (char *)na + MEM_ALIGN(sizeof(struct Value) + TypeStackSizeValue(na)) ) ) {
//
//     if( !na || !na->Typ ) {
//       break; // return
//     }
//     auto tp = na->Typ->Base;
//     auto sz = na->Typ->Sizeof;
//
//     if( i == 0 ) { // first argument = format
//       if( tp != TypePointer ) {
//         RV_INT = 0;
//         return;
//       }
//       continue;
//     }
//
//     if( sz > (int)sizeof(double) ) {
//       RV_INT = 0;
//       return;
//     }
//
//     void *p = nullptr;
//     switch( tp ) {
//       case TypeVoid:
//       case TypeFunction:
//       case TypeMacro:
//       case TypeStruct:
//       case TypeUnion:
//       case TypeGotoLabel:
//       case Type_Type:
//       case TypeArray:
//                               break;
//       case TypeChar:
//       case TypeUnsignedChar:
//                               p = &(na->Val->Character);
//                               break;
//       case TypeInt:
//       case TypeShort:
//       case TypeLong:
//       case TypeUnsignedInt:
//       case TypeUnsignedShort:
//       case TypeUnsignedLong:
//       case TypeEnum:
//                               p = &(na->Val->Integer);
//                               break;
//
//       case TypeFP:
//                               a_ofs += 7; // sizeof(double) - 1;
//                               a_ofs &= 0xFFF8;
//                               p = &(na->Val->FP);
//                               break;
//
//
//       case TypePointer:
//                               p = &(na->Val->Pointer);
//                               break;
//       default: break;
//     }
//     if( p ) {
//       memcpy( argbuf+a_ofs, p, sz );
//       a_ofs += (sz+3) & 0x00FC;
//     }
//     // case TypePointer:  if (Typ->FromType)
//
//     if( a_ofs >= argbuf_sz - 8 ) { // 8 = guard
//       break; // return?
//     }
//   }
//   // dump8( argbuf, argbuf_sz );
//
//   va_list ap;
//   char *xxx = argbuf; // TODO: bit_cast in newer C++
//   memcpy( &ap, &xxx, sizeof(ap) );
//   int rc = vprintf( fmt, ap );
//   RV_INT = rc;
// }

int cmd_lstnames( int argc, const char * const * argv )
{
  const char *subs { "" };
  if( argc > 1 ) {
    subs = argv[1];
  }

  unsigned tsize = pc.GlobalTable.Size;
  std_out << "## lstnames &pc= " << HexInt(&pc) << " size= " << tsize << " OnHeap= " << pc.GlobalTable.OnHeap << NL;

  Table *gtab = &pc.GlobalTable;
  TableEntry **ppte = gtab->HashTable;
  std_out << "## ppte= " << HexInt(ppte) << " subs: \"" << subs << "\"" NL;

  std_out << "## key file type hi "  NL;

  for( unsigned hi=0; hi<tsize; ++hi ) {
    for( const TableEntry* te = gtab->HashTable[hi]; te != nullptr; te = te->Next ) {
      const char *oname = te->p.v.Key;
      if( subs[0] != '\0' && strstr( oname, subs ) == nullptr ) {
        continue;
      }
      std_out<< "# " << oname <<  " \"" << te->DeclFileName << "\" ";
      Value *v = te->p.v.Val;
      if( v ) {
        std_out << ' ' << v->Typ->Base;
      }
      std_out   << ' ' << hi << NL;
    }
  }

  return 0;
}

int cmd_adcall( int argc, const char * const * argv )
{
  adc_all();
  return 0;
}

int cmd_dadc1( int argc, const char * const * argv )
{
  xfloat v0 = arg2float_d( 1, argc, argv, v0_min, -dac_vmax, dac_vmax );
  dadc1( v0 );
  return 0;
}

int cmd_dadc2( int argc, const char * const * argv )
{
  xfloat v0 = arg2float_d( 1, argc, argv, v0_min, -dac_vmax, dac_vmax );
  xfloat v1 = arg2float_d( 2, argc, argv, v1_min, -dac_vmax, dac_vmax );
  dadc2( v0, v1 );
  return 0;
}

// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

