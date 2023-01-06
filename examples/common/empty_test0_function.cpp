#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

int cmd_test0( int argc, const char * const * argv );
int cmd_test_rate( int argc, const char * const * argv );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = arg2long_d( 2, argc, argv, UVAR('t'), 0, 100000 );
  int tp = arg2long_d( 3, argc, argv, UVAR('v'), 0, 10 );
  std_out <<  "# test_delays : n= " << n << " t= " << t_step << " tp= " << tp << NL;

  test_delays_misc( n, t_step, tp );

  return 0;
}


int cmd_test_rate( int argc, const char * const * argv )
{
  const int max_len = 256;
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int sl = arg2long_d( 2, argc, argv, 64, 0, max_len );
  int do_flush = arg2long_d( 3, argc, argv, 0, 0, 1 );
  test_output_rate( n, sl, do_flush );

  return 0;
}

