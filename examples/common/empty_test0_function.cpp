#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

int cmd_test0( int argc, const char * const * argv );
int cmd_test_rate( int argc, const char * const * argv );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  // log_add( "Test0 " );
  uint32_t tm0 = HAL_GetTick();

  #ifdef USE_FREERTOS
  int prty = uxTaskPriorityGet( 0 );
  pr_sdx( prty );
  const char *nm = pcTaskGetName( 0 );
  std_out << "name: \"" << nm << "\"" NL;

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  #else
  uint32_t tc0 = tm0, tc00 = tm0;
  #endif


  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    #ifdef USE_FREERTOS
    TickType_t tcc = xTaskGetTickCount();
    #else
    uint32_t  tcc = HAL_GetTick();
    #endif
    uint32_t tmc = HAL_GetTick();
    std_out <<  " Fake Action i= " << i << "  tick: " << ( tcc - tc00 )
            << "  ms_tick: " << ( tmc - tm0 ) << NL;
    if( UVAR('w') ) {
      std_out.flush();
    }
    // vTaskDelayUntil( &tc0, t_step );
    // delay_ms_brk( t_step );
    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}


int cmd_test_rate( int argc, const char * const * argv )
{
  const int max_len = 256;
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int sl = arg2long_d( 2, argc, argv, 64, 0, max_len );
  std_out << "test_rate: n= " << n << " sl= " << sl << NL;
  char buf[max_len+4]; // for ends and align

  for( int i=0; i<sl-2; ++i ) {
    buf[i] = (char)( '@' + ( i & 0x3F ) );
  }
  buf[sl-2] = '\r';
  buf[sl-1] = '\n';
  buf[sl]   = '\0'; // sic, transfer sl bytes,

  uint32_t tm0 = HAL_GetTick();

  unsigned n_lines = 0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    buf[0] =  (char)( '@' + ( i & 0x3F ) );
    std_out << buf;
    if( UVAR('w') ) {
      std_out.flush();
    }
    ++n_lines;
  }
  uint32_t tm1 = HAL_GetTick();
  delay_ms( 1000 ); // settle
  unsigned dt = tm1 - tm0;
  unsigned n_ch = sl * n_lines;
  std_out << NL "dt= " << dt << " ms, chars: " << n_ch << "  lines: " << n_lines
          << "  cps: " << ( 1000*n_ch / dt ) << "  lps: " << ( 1000*n_lines / dt ) << NL;

  return 0;
}

