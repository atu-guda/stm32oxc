// ------------------ program to calibrate ADC

leds_set( 3 );
pr_s( "# !\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 1 );


lcd_cls();
use_loops = 1;
int va[4] = {0,0,0,0};


int loop()
{
  adc_measure();

  obuf_clear( 0 );
  obuf_add_fp_x( t_c, cvtff_fix,  10, 2, 0 );
  obuf_add_str( " ", 0 );
  for( int i=0; i<4; ++i ) {
    va[i] += adc_vi[i];
    obuf_add_fp_c( adc_v[i], 0 );
    obuf_add_str( " ", 0 );
  }

  obuf_clear( 1 );
  obuf_clear( 2 );
  obuf_clear( 3 );
  obuf_add_fp_c( adc_v[0], 1 );  obuf_add_str( " ", 1 );  obuf_add_int( adc_vi[0], 1 );
  obuf_add_fp_c( adc_v[1], 2 );  obuf_add_str( " ", 2 );  obuf_add_int( adc_vi[1], 2 );
  obuf_add_fp_c( adc_v[2], 3 );  obuf_add_str( " ", 3 );  obuf_add_int( adc_vi[2], 3 );

  leds_toggle( 2 );

  return 0;
}

int post_loop()
{
  pr_s( "# post_loop()\n" );
  for( int i=0; i<4; ++i ) {
    pr_i( va[i] / n_loops );
  }
  pr_s( "\n" );
  return 0;
}


