// ------------------ like task_02.c, but w/o delay

leds_set( 3 );
pr_s( "# Hello cube!\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 1 );

ifm_0_enable();

lcd_cls();


int t_0 = getTick();
int t_ct = t_0;
pr( ' ', i, t_0, "\n" );

for( int i=0; i<n_loops; ++i ) {
  t_ct = getTick();
  t_c = ( t_ct - t_0 ) * 0.001f;
  adc_measure();
  ifm_0_measure();
  pr( ' ', i, t_ct, t_c, adc_v[0], adc_vi[0], ifm_0_freq, "\n" );

  obuf_clear( 1 );
  obuf_clear( 2 );
  obuf_add_fp( adc_v[0], 1 );
  obuf_add_fp( ifm_0_freq, 2 );
  leds_toggle( 2 );

  lcdbufs_out();
}

ifm_0_disable();


