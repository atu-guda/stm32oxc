#include <cstring>
#include <cstdlib>
#include <iterator>

// #include <vector>
// #include <array>
#include <memory>

#include <oxc_auto.h>
#include <oxc_outstr.h>
#include <oxc_hd44780_i2c.h>
#include <oxc_menu4b.h>
// #include <oxc_floatfun.h>
#include <oxc_namedints.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
// BOARD_DEFINE_LEDS;
PinsOut leds( GPIOB, 12, 4 ); // 12: tick/menu 13: ?? 14: heather 15: Err
PinsOut leds0( GPIOC, 13, 1 ); // single C13

// TODO: to local include
#define HEATHER_BIT BIT2
#define ERR_BIT     BIT3

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MAX31855 thermocouple control device + LCD output" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_menu( int argc, const char * const * argv );
CmdInfo CMDINFO_MENU { "menu", 'M', cmd_menu, " N - menu action"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_MENU,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );

void on_btn_while_run( int cmd );


PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

const unsigned MAX31855_SIZE = 4; // 32 bit per packet

const uint32_t MAX31855_FAIL = 0x00010000;
const uint32_t MAX31855_BRK  = 0x00000001;
const uint32_t MAX31855_GND  = 0x00000002;
const uint32_t MAX31855_VCC  = 0x00000004;

// common vars: access by menu, named vars and program

// all T in C*100
int T_c = 1275, T_i = 1000, T_min = 1000000, T_max = -100000,
    T_rel = 0, T_base = 0,
    T_off = 3000, T_hyst = 200,
    t_dt = 100, time_c, // ms
    dTdt = 0;
int heather_on = 0;
volatile int out_idx = 0;
volatile int btn_run = 0;

int fun_set_base( int n );

int fun_set_base( int /*n*/ )
{
  T_base = T_c;
  std_out << "#-- set_base: n= " << FloatMult( T_base, 2 )  << NL;
  return 0;
}

struct OutIntDescr {
  int *v;
  int div10;
  int min_int;
  const char *name;
};

const OutIntDescr outInts[] = {
  {    &T_i, 2, 2, "T_i"    },
  {  &T_min, 2, 2, "T_min"  },
  {  &T_max, 2, 2, "T_max"  },
  {  &T_rel, 2, 2, "T_rel"  },
  { &time_c, 3, 4, "time_c" },
  {   &dTdt, 3, 4, "dTdt"   },
};



const Menu4bItem menu_main[] = {
  { "out_idx", (int*)&out_idx,  1, 0, size(outInts)-1, nullptr },
  { "T_off",    &T_off,   25, -10000,   500000, nullptr, 2 },
  { "T_hyst" ,  &T_hyst,  25,      0,   100000, nullptr, 2 },
  { "t_dt",       &t_dt, 100,    100,   100000, nullptr, 3 },
  { "T_i",         &T_i,   0,      0,   500000, nullptr, 2 },
  { "T_min",     &T_min,   0,      0,   500000, nullptr, 2 },
  { "T_max",     &T_max,   0,      0,   500000, nullptr, 2 },
  { "T_base",   &T_base,  25,      0,   500000, nullptr, 2 },
  { "set_base", nullptr,   0,      0,   100000, fun_set_base }
};

MenuState menu4b_state { menu_main, size( menu_main ), "T\n" };

// named vars iface

constexpr NamedInt   o_T_off    {   "T_off",        &T_off                         };
constexpr NamedInt   o_T_hyst   {   "T_hyst",       &T_hyst                        };
constexpr NamedInt   o_T_base   {   "T_base",       &T_base                        };
constexpr NamedInt   o_t_dt     {   "t_dt",         &t_dt                          };
constexpr NamedInt   o_T_rel    {   "T_rel",        &T_rel, 1, NamedObj::Flags::ro };
constexpr NamedInt   o_T_i      {   "T_i",          &T_i,   1, NamedObj::Flags::ro };
constexpr NamedInt   o_T_min    {   "T_min",        &T_min, 1, NamedObj::Flags::ro };
constexpr NamedInt   o_T_max    {   "T_max",        &T_max, 1, NamedObj::Flags::ro };
constexpr NamedInt   o_dTdt     {   "dTdt",         &dTdt,  1, NamedObj::Flags::ro };

constexpr const NamedObj *const objs_main_info[] = {
  & o_T_i,
  & o_T_off,
  & o_T_hyst,
  & o_T_base,
  & o_t_dt,
  & o_T_rel,
  & o_T_i,
  & o_T_min,
  & o_T_max,
  & o_dTdt,
  nullptr
};

const NamedObjs objs_main( objs_main_info );

bool print_var_main( const char *nm, int fmt )
{
  return objs_main.print( nm, fmt );
}

bool set_var_main( const char *nm, const char *s )
{
  auto ok =  objs_main.set( nm, s );
  print_var_main( nm, 0 );
  return ok;
}


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10000000;

  print_var_hook    = print_var_main;
  set_var_hook      = set_var_main;

  leds0.initHW();

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
  spi_d.initSPI();

  // BOARD_POST_INIT_BLINK; // NO - do not touch heather!
  leds.set( 0x03 ); leds0.set( 0x01 );
  delay_ms( 200 );
  leds.reset( 0x0F ); leds0.reset( 0x01 );

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( menu4b_ev_dispatch );

  lcdt.init_4b();
  lcdt.puts_xy( 0, 1, menu4b_state.menu_level0_str );

  init_menu4b_buttons();

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 0xFFFFFF );
  uint32_t t_step = t_dt;


  std_out << NL "# go: n= " << n << " t= " << t_step << NL;
  std_out.flush();
  lcdt.cls();

  OSTR( b0, 32 );
  OSTR( b1, 32 );

  T_min = 1000000; T_max = -100000;
  uint32_t vl;
  int rc;
  spi_d.setTssDelay( 200 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0; errno = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    rc = spi_d.recv( (uint8_t*)(&vl), sizeof(vl) );
    vl = __builtin_bswap32( vl );// or __REV(vl)
    b0_outstr.reset_out();
    b1_outstr.reset_out();

    uint32_t tcc = HAL_GetTick();
    time_c = tcc - tm00;
    std_out <<  FloatMult( time_c, 3, 5 )  <<  ' ';

    leds.toggle( BIT0 );  leds0.toggle( BIT0 );
    char ctick = ( i & 1 ) ? ':' : '.';

    int32_t tif = ( vl >> 4 ) & 0x0FFF;
    if( tif & 0x0800 ) { // sign propagation
      tif |= 0xFFFFF000;
    }
    int32_t tid4 = tif * 625; // 4 bit for fraction part
    T_i = tid4 / 100;
    std_out << FloatMult( tid4, 4 );

    int32_t tof =  ( vl >> 18 ) & 0x3FFF; // Temperature out: 14 bit 18:31
    if( tof & 0x2000 ) {
      tof |= 0xFFFFC000;
    }
    int T_o = T_c;
    T_c = tof * 25; // 2 bit for fraction
    T_rel = T_c - T_base;

    if( T_c > T_max ) {
      T_max = T_c;
    }
    if( T_c < T_min ) {
      T_min = T_c;
    }

    if( T_c > T_off ) {
      heather_on = 0;
    }
    if( T_c < ( T_off - T_hyst ) ) {
      heather_on = 1;
    }
    leds.sr( HEATHER_BIT, heather_on );

    dTdt = ( T_c - T_o ) * 10000 / t_dt; // TODO: more correct, more points

    b0 << FloatMult( T_c, 2, 4 ) << ' ' << heather_on << ' '
       << ( ( vl & MAX31855_FAIL ) ? 'F' : ctick );

    leds.sr( ERR_BIT, vl & MAX31855_FAIL );

    if( vl & MAX31855_BRK ) {
      b0 <<  'B';
    }
    if( vl & MAX31855_GND ) {
      b0 <<  'G';
    }
    if( vl & MAX31855_VCC ) {
      b0 <<  'V';
    }
    b0 << ' ' << btn_run << "   ";

    if( out_idx >= (int)size( outInts ) ) {
      out_idx = 0;
    }
    b1 << FloatMult( *(outInts[out_idx].v), outInts[out_idx].div10, outInts[out_idx].min_int ) << ' '
       << outInts[out_idx].name << "   ";

    // TODO: drop b1_buf after debug
    std_out <<  ' ' << b0_buf << ' ' << b1_buf << ' ' << ( vl & 0x07 ) << ' '; // err

    if( UVAR('d') > 0 ) {
      std_out <<  " vl= " << HexInt( vl ) << " tif= "  << HexInt( tif ) <<  " tof= "  << HexInt( tof ) << " rc= " << rc;
    }


    std_out << NL;

    lcdt.puts_xy( 0, 0, b0_buf );
    lcdt.puts_xy( 0, 1, b1_buf );


    delay_ms_until_brk( &tm0, t_step );
  }
  leds.reset( HEATHER_BIT );

  if( UVAR('d') > 0 ) {
    spi_d.pr_info();
  }

  lcdt.puts_xy( 0, 1, menu4b_state.menu_level0_str );

  std_out << "# stop: errno= " << errno << NL;

  return 0;
}


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
  leds.toggle( BIT1 );
  switch( cmd ) {
    case  MenuCmd::Esc:
      break_flag = 1; errno = 10000;
      break;
    case  MenuCmd::Up:
      ++out_idx;
      if( out_idx > (int)size(outInts)-1 ) { out_idx = 0; }
      break;
    case  MenuCmd::Down:
      --out_idx;
      if( out_idx < 0 ) { out_idx = size(outInts)-1; }
      break;
    case  MenuCmd::Enter:
      btn_run = !btn_run;
      break;
    default: break;
  }
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

