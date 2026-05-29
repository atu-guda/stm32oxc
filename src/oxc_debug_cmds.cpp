#include <oxc_outstream.h>
#include <oxc_console.h>
#include <oxc_debug1.h>

using namespace oxc;

DCL_CMD_REG( test_rate, 0, "[ n [len [flush] ] ] - test output rate"  );
CMD_FUNCTION( test_rate )
{
  const int max_len = 512;
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int sl = arg2long_d( 2, argc, argv, 64, 0, max_len );
  int do_flush = arg2long_d( 3, argc, argv, 0, 0, 1 );
  test_output_rate( n, sl, do_flush );

  return 0;
}

DCL_CMD_REG( test_delays, '\0', "[n] [step_ms] [type] - test delays [w=flush]"  );
CMD_FUNCTION( test_delays )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = arg2long_d( 2, argc, argv, UVAR('t'), 0, 100000 );
  int tp = arg2long_d( 3, argc, argv, 0, 0, 20 );
  std_out <<  "# test_delays : n= " << n << " t= " << t_step << " tp= " << tp << NL;

  test_delays_misc( n, t_step, tp );

  return 0;
}



