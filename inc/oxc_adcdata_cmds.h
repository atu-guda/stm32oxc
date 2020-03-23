// =====================================================================================
//
//       Filename:  oxc_adcdata_cmds.h
//
//    Description:  common commands for AdcData 
//
//        Version:  1.0
//        Created:  2020-03-23 12:05:09
//
//         Author:  Anton Guda (atu), atu@nmetau.edu.ua
//
// =====================================================================================
#ifndef _OXC_ADCDATA_CMDS_H
#define _OXC_ADCDATA_CMDS_H

#include <oxc_floatfun.h>

#ifdef USE_OXC_SDFAT
#include <oxc_io_fatfs.h>
#endif

template<typename AD>
int subcmd_set_coeffs( int argc, const char * const * argv, AD &ad )
{
  const unsigned n_row = ad.get_n_row();
  if( argc > 1 ) {
    for( unsigned j=0; j<n_row; ++j ) {
      xfloat v = arg2xfloat_d( j+1, argc, argv, 1, XFLOAT_NLARGE, XFLOAT_LARGE );
      ad.set_col_mult( j, v );
    }
  }
  std_out << "# ADC coefficients:";
    for( unsigned j=0; j<n_row; ++j ) {
    std_out << ' ' << ad.get_col_mult( j );
  }
  std_out << NL;
  return 0;
}


template<typename AD>
int subcmd_out_any( int argc, const char * const * argv, AD &ad, bool isHex )
{
  auto ns = ad.get_n_row();
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  ad.out_any( std_out, isHex, st, n );

  return 0;
}

#ifdef USE_OXC_SDFAT
template<typename AD>
int subcmd_outsd_any( int argc, const char * const * argv, AD &ad, bool isHex )
{
  if( argc < 2 ) {
    std_out << "# Error: need filename [n [start]]" NL;
    return 1;
  }

  auto n_lines = ad.get_n_row();
  uint32_t n = arg2long_d( 2, argc, argv, n_lines, 0, n_lines+1 ); // number output series
  uint32_t st= arg2long_d( 3, argc, argv,       0, 0, n_lines-2 );

  const char *fn = argv[1];
  auto file = DevOut_FatFS( fn );
  if( !file.isGood() ) {
    std_out << "Error: f_open error: " << file.getErr() << NL;
    return 2;
  }

  leds.set( BIT2 );
  OutStream os_f( &file );
  ad.out_any( os_f, isHex, st, n );
  StatIntData sdat( ad );
  sdat.slurp( ad );
  sdat.calc();
  os_f << sdat;
  leds.reset( BIT2 );

  return 0;
}
#endif


#endif

