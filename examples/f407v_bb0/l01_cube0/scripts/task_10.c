// ------------------ nex program to calibrate ADC/DAC

leds_set( 3 );
pr_s( "# !\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 3 );


lcd_cls();
pins_out(0);
use_loops = 0;


for( int i=0; i<4096; i+= 50 ) {

  dac_out1i( i );
  dac_out2i( i );

  delay_ms( 3000 ); leds_set(2);   pins_out_toggle( 1 );
  adc_measure();
  delay_ms( 2000 ); leds_reset(2); pins_out_toggle( 1 );

  pr( ' ', i , adc_vi[0], adc_vi[1], adc_vi[2], adc_vi[3], '\n' );

}

dac_out1( 0.0 );
dac_out2( 0.0 );
pr_s( "# END\n" );


