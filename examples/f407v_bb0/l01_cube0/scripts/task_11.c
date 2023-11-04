// ------------------ program to measure dual OPAMP
// k_v must be set beforehand

leds_set( 3 );
pr_s( "# !\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 3 );

lcd_cls();
pins_out(0);
use_loops = 0;


for( float v=-3.3; v<3.3; v+= 0.01 ) {

  dac_out1( v );
  dac_out2( v * k_v );

  delay_ms( 200 ); leds_set(2);  // pins_out_toggle( 1 );
  adc_measure();
  delay_ms( 10 ); leds_reset(2); // pins_out_toggle( 1 );

  pr( ' ', v , adc_v[0], adc_v[1], adc_v[2], adc_v[3], '\n' );

}

dac_out12( 0.0, 0.0 );
adc_all();
pr_s( "# END\n" );


