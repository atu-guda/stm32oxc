#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_adcdata.h>
#include <oxc_adcdata_cmds.h>

#include <oxc_spi_ads1220.h>

#include <oxc_spi_debug.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use ADS1220 SPI ADC " NL;

const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in bytes for 32-bit samples
using AdcDataX = AdcData<-24,xfloat>;
AdcDataX adcd( BOARD_ADC_MALLOC, BOARD_ADC_FREE );
const unsigned n_ADC_ch_max = ADS1220::Params::n_ch;

// --- local commands;
DCL_CMD_REG( test0, 'T', " [n] - test ADC"  );
DCL_CMD_REG( set_coeffs, 'F', " k0 k1 k2 k3 - set ADC coeffs"  );
DCL_CMD_REG( out, 'O', " [N [start]]- output data "  );
DCL_CMD_REG( outhex, 'H', " [N [start]]- output data in hex form"  );
DCL_CMD_REG( show_stats, 'Y', " [N [start]]- show statistics"  );
DCL_CMD_REG( t0, 'X', " - test ???"  );
DCL_CMD_REG( init, 'I', " - init ADC"  );
DCL_CMD_REG( rreg, 'R', " - read regs"  );
DCL_CMD_REG( rregn, 'N', " - read all regs"  );
DCL_CMD_REG( reset, 'Z', " - reset only"  );



PinOut nss_pin(   BOARD_SPI_DEFAULT_PIN_SNSS );
PinsIn ndrdy_pin( BOARD_SPI_DEFAULT_PIN_EXT1, 1 );

// to HW debug
PinOut dbg_pin(   BOARD_SPI_DEFAULT_PIN_EXT2 );

SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
ADS1220 adc( spi_d, &ndrdy_pin );

int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000; // 160 min here
  UVAR_n = 10;
  UVAR_v = (int)ADS1220::Params::vref; // internal REF in uV * 2
  UVAR_c = 4;

  dbg_pin.initHW();
  ndrdy_pin.initHW();
  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256, SPI_lmode::low_2e ) != HAL_OK ) { // low_2e is a must!
    die4led( 0x04_mask );
  }
  spi_d.setTssDelay_100ns( 10 );
  spi_d.initSPI();

  // adc.init();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int t_step = UVAR_t;
  unsigned n_ch = clamp( (unsigned)UVAR_c, 1u, n_ADC_ch_max );

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( ADS1220::Params::bits_out * n_ch / 8 );
  uint32_t n = arg2long_d( 1, argc, argv, UVAR_n, 1, n_ADC_series_max ); // number of series

  std_out << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << " us " NL;

  adc.reset();
  leds.set(   0x07_mask ); delay_ms( 100 );  leds.reset( 0x07_mask  );

  adcd.free();
  if( ! adcd.alloc( n_ch, n ) ) {
    std_out << "# Error: fail to alloc buffer" << NL;
    return 2;
  }
  adcd.set_d_t( t_step * 1e-3f );
  adcd.set_v_ref_uV( UVAR_v );

  adc.set_mode_1shot();
  adc.set_data_rate( ADS1220::Cfg1Bits::CFG1_DR_20 );

  adc.PGA_dis();
  adc.set_pga_gain( ADS1220::CFG0_PGA_GAIN_1 );

  leds[1].set();

  break_flag = 0;
  uint32_t tm0 = 0, tm00 = 0;
  bool good { true };
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    if( UVAR_l ) {  leds[2].set(); }
    auto row = adcd.row( i );

    for( unsigned ch=0; ch<n_ch; ++ch ) {
      adc.set_mux( ADS1220::muxs_se[ch] );
      auto v = adc.read_single();
      if( v ) {
        auto vv = v.value();
        xfloat vf = (xfloat) vv * UVAR_v * xfloat(1.0e-6f) / (int)ADS1220::Params::scale;
        std_out << ' ' << vv << ' ' << vf;
        row[ch] = vv;
      } else {
        std_out << "# Fail! " << NL;
        good = false;
        break;
      }
    }
    if( !good ) {
      break;
    }
    std_out << NL;
    // adc.read( adcd.row( i ), n_ch );

    if( UVAR_l ) {  leds[2].reset(); }

    if( t_step > 0 ) {
      delay_ms_until_brk( &tm0, t_step );
    }
  }


  leds[1].reset();

  std_out << "# n_lines= " << adcd.get_n_row() << " dt_appr= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;

  delay_ms( 10 );

  return 0;
}


int cmd_set_coeffs( int argc, const char * const * argv )
{
  return subcmd_set_coeffs( argc, argv, adcd );
}




int cmd_out( int argc, const char * const * argv )
{
  return subcmd_out_any( argc, argv, adcd, false );
}

int cmd_outhex( int argc, const char * const * argv )
{
  return subcmd_out_any( argc, argv, adcd, true );
}


int cmd_show_stats( int argc, const char * const * argv )
{
  // auto ns = adcd.get_n_row();
  // uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  // uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  StatIntData sdat( adcd );
  sdat.slurp( adcd ); // TODO: limits
  sdat.calc();
  std_out << sdat;

  return 0;
}


int cmd_t0( int argc, const char * const * argv )
{
  uint8_t ch = arg2long_d( 1, argc, argv, 0, 0, std::size(ADS1220::muxs_se)-1 );

  adc.set_mux( ADS1220::muxs_se[ch] );
  adc.PGA_dis();
  adc.set_pga_gain( ADS1220::CFG0_PGA_GAIN_1 );

  dbg_pin.set(); delay_bad_100ns( 1 );
  auto v = adc.read_single();
  dbg_pin.reset();

  if( v ) {
    auto vv = v.value();
    xfloat vf = (xfloat) vv * UVAR_v * xfloat(1.0e-6f) / (int)ADS1220::Params::scale;
    std_out << "# v= " << vv << ' ' << HexInt(vv) << ' ' << vf << NL;
  } else {
    std_out << "# Fail! " << NL;
  }

  return 0;
}


int cmd_init( int argc, const char * const * argv )
{
  dump8( adc.getCfgs(), 4 );
  adc.read_config();
  dump8( adc.getCfgs(), 4 );

  adc.init();

  adc.read_config();
  dump8( adc.getCfgs(), 4 );
  return 0;
}


int cmd_rreg( int argc, const char * const * argv )
{
  constexpr uint8_t n_cfgs { ADS1220::n_cfgs };
  uint8_t c_st = arg2long_d( 1, argc, argv, 1, 0, n_cfgs-1  );
  uint8_t c_en = arg2long_d( 2, argc, argv, 2, 0, n_cfgs  );
  dump8( adc.getCfgs(), 4 );

  for( uint8_t i=c_st; i < c_en ; ++i ) {
    dbg_pin.set();  delay_bad_100ns( 1 );
    auto c = adc.readReg( i );
    dbg_pin.reset();
    std_out << "# " << i << ' ' << HexInt8( c ) << ' '
      << HexInt8( ADS1220::mk_wr_cmd(i) ) << ' ' << HexInt8( ADS1220::mk_rd_cmd(i) ) << NL;
  }

  adc.read_config();
  dump8( adc.getCfgs(), 4 );


  return 0;
}

int cmd_rregn( int argc, const char * const * argv )
{
  dump8( adc.getCfgs(), 4 );

  dbg_pin.set();  delay_bad_100ns( 1 );

  auto read_rc = adc.readAllRegs();
  dbg_pin.reset();

  dump8( adc.getCfgs(), 4 );
  std_out << "# post: last_err: " << spi_d.getLastErr() << " err: " << spi_d.getErr() << " rc: " << spi_d.getLastRc() << " state: " << spi_d.getState() << NL;
  std_out << "# read_rc= " << read_rc << NL;

  return 0;
}


int cmd_reset( int argc, const char * const * argv )
{
  dbg_pin.set(); delay_bad_100ns( 1 );
  auto rc = adc.reset();
  dbg_pin.reset();
  int err = spi_d.getErr();

  std_out << "# reset rc= " << rc << " err = " << err << NL;
  spi_d.pr_info();
  return 0;
}





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

