#include <errno.h>
#include <cmath>
#include <algorithm>

#include <oxc_devio.h>
#include <oxc_adc.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>
#include <oxc_debug1.h>

#ifdef USE_OXC_SD
#include <fatfs_sd_st.h>
#include <oxc_io_fatfs.h>
#endif

using namespace std;

extern ADC_Info adc;

void adc_out_to( OutStream &os, uint32_t n, uint32_t st );
void adc_show_stat( OutStream &os, uint32_t n = 0xFFFFFFFF, uint32_t st = 0 );
void pr_ADCDMA_state();
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_show_stats( int argc, const char * const * argv );
CmdInfo CMDINFO_SHOWSTATS { "show_stats", 'Y', cmd_show_stats, " [N [start]]- show statistics"  };
#ifdef USE_OXC_SD
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };
#endif


void adc_out_to( OutStream &os, uint32_t n, uint32_t st )
{
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)adc.n_ch_max );
  if( n+st >= adc.n_series+1 ) {
    n = adc.n_series - st;
  }

  os << "# n= " << n << " n_ch= " << n_ch << " st= " << st << NL;

  float t = st * adc.t_step_f;
  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    t = adc.t_step_f * ii;
    os << t << ' ';
    for( int j=0; j< n_ch; ++j ) {
      float v = 0.001f * (float) adc.data[ii*n_ch+j] * UVAR('v') / 4096;
      os << v;
    }
    os << NL;
  }

}


void adc_show_stat( OutStream &os, uint32_t n, uint32_t st )
{
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)adc.n_ch_max );
  if( n+st >= adc.n_series+1 ) {
    n = adc.n_series - st;
  }

  StatData sdat( n_ch );

  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    sreal vv[n_ch];
    for( int j=0; j< n_ch; ++j ) {
      vv[j] = 0.001f * (float) adc.data[ii*n_ch+j] * UVAR('v') / 4096;
    }
    sdat.add( vv );
  }
  sdat.calc();
  sdat.out_parts( os );

}


// TODO: move
void pr_ADCDMA_state()
{
  if( UVAR('d') > 0 ) {
    std_out <<  "# DMA: CR= " << HexInt( adc.hdma_adc.Instance->CR, true )
       << " NDTR= "      << adc.hdma_adc.Instance->NDTR
       << " PAR= "       << HexInt( adc.hdma_adc.Instance->PAR, true )
       << " M0AR= "      << HexInt( adc.hdma_adc.Instance->M0AR, true )
       << " M1AR= "      << HexInt( adc.hdma_adc.Instance->M1AR, true )
       << " FCR= "       << HexInt( adc.hdma_adc.Instance->FCR, true )
       << NL;
  }
}

int cmd_out( int argc, const char * const * argv )
{
  auto ns = adc.n_series;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adc_out_to( std_out, n, st );
  adc_show_stat( std_out, n, st );

  return 0;
}


#ifdef USE_OXC_SD
int cmd_outsd( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    std_out << "# Error: need filename [n [start]]" NL;
    return 1;
  }

  uint32_t n = arg2long_d( 2, argc, argv, adc.n_series, 0, adc.n_series+1 ); // number output series
  uint32_t st= arg2long_d( 3, argc, argv,            0, 0, adc.n_series-2 );

  const char *fn = argv[1];
  auto file = DevOut_FatFS( fn );
  if( !file.isGood() ) {
    std_out << "Error: f_open error: " << file.getErr() << NL;
    return 2;
  }
  OutStream os_f( &file );
  adc_out_to( os_f, n, st );
  adc_show_stat( os_f, n, st );

  return 0;
}
#endif


int cmd_show_stats( int argc, const char * const * argv )
{
  auto ns = adc.n_series;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adc_show_stat( std_out, n, st );

  return 0;
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 1;
  adc.good_SR =  adc.last_SR = adc.hadc.Instance->SR;
  adc.last_end = 1;
  adc.last_error = 0;
  ++adc.n_good;
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 2;
  adc.bad_SR = adc.last_SR = adc.hadc.Instance->SR;
  // tim2_deinit();
  adc.last_end  = 2;
  adc.last_error = HAL_ADC_GetError( hadc );
  adc.dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  ++adc.n_bad;
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &adc.hdma_adc );
}


