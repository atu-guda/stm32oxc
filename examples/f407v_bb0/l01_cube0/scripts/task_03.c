// ------------------ simple program for cube0 with loop(), post_loop()

leds_set( 3 );
pr_s( "# Hello cube 3!\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 1 );

ifm_0_enable();

lcd_cls();
use_loops = 1;


int loop()
{
  adc_measure();
  ifm_0_measure();
  // pr( ' ', i, t_ct, t_c, adc_v[0], adc_vi[0], ifm_0_freq, "\n" );

  obuf_clear( 0 );
  obuf_add_fp_x( t_c, cvtff_fix,  10, 2, 0 );
  obuf_add_str( " ", 0 );
  obuf_add_fp_x( t_c, cvtff_exp,  10, 4, 0 );
  obuf_add_str( " ", 0 );
  obuf_add_fp_x( t_c, cvtff_auto, 10, 4, 0 );
  obuf_add_str( " ", 0 );
  obuf_add_fp_c( adc_v[0], 0 );
  obuf_add_str( " ", 0 );
  obuf_add_fp( ifm_0_freq, 0 );

  obuf_clear( 1 );
  obuf_clear( 2 );
  obuf_clear( 3 );
  obuf_add_fp_c( adc_v[0], 1 );
  obuf_add_fp_c( ifm_0_freq, 2 );
  obuf_add_fp_c( t_c, 3 );

  leds_toggle( 2 );

  return 0;
}

int post_loop()
{
  ifm_0_disable();
  pr_s( "# post_loop()\n" );
  return 0;
}


