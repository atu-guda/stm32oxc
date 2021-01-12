// ------------------ measure VAch with cube0 DAC1: in ADC0: V_in ADC1: V_dev

leds_set( 3 );
pr_s( "# VAch 3!\n" );
delay_ms( 200 );
adc_defcfg();
leds_reset( 1 );

lcd_cls();
use_loops = 1;

float R_m = 100.0; // measurement resistor
float v = -9.0;
n_loops = 180;
float I_m = 0;


int loop()
{
  dac_out1( v );
  delay_ms( 20 );
  adc_measure();

  obuf_clear( 0 );
  obuf_add_fp( v, 0 );
  obuf_add_str( " ", 0 );
  obuf_add_fp( adc_v[0], 0 );
  obuf_add_str( " ", 0 );
  obuf_add_fp( adc_v[1], 0 );
  obuf_add_str( " ", 0 );
  I_m = (adc_v[0]-adc_v[1])/R_m;
  obuf_add_fp_x( I_m, cvtff_auto, 12, 99, 0 );

  obuf_clear( 1 );
  obuf_clear( 2 );
  obuf_clear( 3 );
  obuf_clear( 4 );
  obuf_add_fp( v, 1 );
  obuf_add_fp( adc_v[0], 2 );
  obuf_add_fp( adc_v[1], 3 );
  obuf_add_fp_x( I_m, cvtff_auto, 12, 99, 4 );

  leds_toggle( 2 );
  v += 0.1;

  return 0;
}

int post_loop()
{
  dac_out1( 0 );
  pr_s( "# post_loop()\n" );
  return 0;
}


