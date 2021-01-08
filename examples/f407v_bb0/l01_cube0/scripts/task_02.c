// ------------------ first hello-world for cube0

leds_set( 3 );
pr_s( "# Hello cube!\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 1 );

ifm_0_enable();

lcd_cls();


int t_0 = getTick();
int t_ct = t_0;
pr( ' ', t_0, t_ct, "\n" );

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

  // lcd_puts_xy( (i>>2)&15, i&3, "ASD" );
  lcdbufs_out();

  if( delay_ms_until_brk( &t_0, t_step_ms ) ) {
    break;
  }
}

ifm_0_disable();


