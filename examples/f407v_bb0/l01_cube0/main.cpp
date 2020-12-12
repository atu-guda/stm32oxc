#include <cerrno>

#include <oxc_auto.h>
#include <oxc_outstr.h>
#include <oxc_hd44780_i2c.h>
#include <oxc_ads1115.h>
#include <oxc_menu4b.h>
#include <oxc_statdata.h>
#include <oxc_ds3231.h>

#include <ff_gen_drv_st.h>
#include <usbh_diskio.h>
#include <ff.h>

#include <oxc_fs_cmd0.h>

#include <oxc_picoc.h>

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

int task_idx = 0, T_off = -10, T_hyst = 20; // TMP: to test menu
const Menu4bItem menu_main[] = {
  { "task_idx",   &task_idx,       1,       0,       42, nullptr },
  {   "T_off",       &T_off,      25,  -10000,   500000, nullptr, 2 },
  //{ "set_base", nullptr,   0,      0,   100000, fun_set_base }
};

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
const xfloat adc_3_to_20 = 20.0f / 3.0f;
xfloat adc_v_scales[adc_n_ch] = {  adc_3_to_20, adc_3_to_20,  adc_3_to_20, adc_3_to_20 };
xfloat adc_v_bases[adc_n_ch]  = {       -10.0f,      -10.0f,       -10.0f,      -10.0f };
xfloat adc_v[adc_n_ch]        = {         0.0f,        0.0f,         0.0f,        0.0f };
int    adc_vi[adc_n_ch]       = {            0,           0,            0,           0 };
int adc_scale_mv = 4096;
xfloat adc_kv = 0.001f * adc_scale_mv / 0x7FFF;
int adc_defcfg();
int adc_measure();
int adc_out_stdout();
int adc_out_lcd();
int adc_pre_loop();
int adc_loop();


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

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_LCD_XYCHAR,
  &CMDINFO_LCD_GOTOXY,
  &CMDINFO_LCD_PUTS,
  &CMDINFO_MENU,
  FS_CMDS0,
  nullptr
};

int run_common( RUN_FUN pre_fun, RUN_FUN loop_fun )
{
  uint32_t n  = UVAR('n');
  uint32_t t_step = UVAR('t');
  lcdt.cls();
  std_out << "# Task " << task_idx << " n= " << n << " t_step= " << t_step << " on_cmd_handler= " << on_cmd_handler << NL;

  pre_fun();

  uint32_t tm0, tm00;
  break_flag = 0;
  for( decltype(+n) i=0; i<n && !break_flag; ++i ) {
    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    xfloat tc = 0.001f * ( tcc - tm00 );
    std_out << tc ;
    loop_fun();

    std_out << NL;

    if( t_step > 0 ) {
      delay_ms_until_brk( &tm0, t_step );
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
  std_out << "# Task 1 n= " << NL;
  return 0;
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



int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') =     100;
  UVAR('n') =  100;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;
  lcdt.init_4b();
  lcdt.cls();
  lcdt.puts("I ");

  init_menu4b_buttons();
  lcdt.puts("Btn ");

  fs.fs_type = 0; // none
  fspath[0] = '\0';

  cmdline_handlers[0] = picoc_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  pc.InteractiveHead = nullptr;
  init_picoc( &pc );
  lcdt.puts("picoc ");

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

  PicocIncludeAllSystemHeaders( ppc );

  VariableDefinePlatformVar( ppc, nullptr, "__a",         &(ppc->IntType), (union AnyValue *)&(UVAR('a')), TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "d_arr",      ppc->FPArrayType, (union AnyValue *)d_arr,        TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "d_ptr",        ppc->FPPtrType, (union AnyValue *)&d_ptr,       TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "a_char",   ppc->CharArrayType, (union AnyValue *)a_char,       TRUE );
  VariableDefinePlatformVar( ppc, nullptr, "p_char",     ppc->CharPtrType, (union AnyValue *)&p_char,      TRUE );
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
  // leds.toggle( BIT0 );
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

int adc_out_stdout()
{
  for( decltype(+adc_no) j=0; j<adc_no; ++j ) {
    std_out << ' ' << adc_v[j];
  }
  return 0;
}

int adc_out_lcd()
{
  OSTR(s,40);
  for( decltype(+adc_no) j=0; j<adc_no; ++j ) {
    s_outstr.reset_out();
    s << XFmt( adc_v[j], cvtff_fix, 7, 4 ) << ' ';
    lcdt.puts_xy( 0, j, s_outstr.c_str() );
  }
  return 0;
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

// ----------------------------------------  ------------------------------------------------------

// ----------------------------------------  ------------------------------------------------------

// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

