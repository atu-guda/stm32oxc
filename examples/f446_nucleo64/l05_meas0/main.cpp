#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>

#include "meas0.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;
// PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App measure and control analog and digital signals" NL;

//* helper to parse float params in cmdline ('=' = the same),
// argc and argv - relative, start on first (0) argv
int parse_floats( int argc, const char * const * argv, float *d );

// --- local commands;
int cmd_dac( int argc, const char * const * argv );
CmdInfo CMDINFO_DAC { "dac", 'D', cmd_dac, " v0 v1 - output values to dac"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 'W', cmd_pwm, " v0 v1 v2 v3 - set pwm output"  };
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_DAC,
  &CMDINFO_PWM,
  &CMDINFO_TEST0,
  nullptr
};


D_in_sources d_ins[n_din_ch] = {
  { GPIOA, BIT6 },
  { GPIOA, BIT7 },
  { GPIOB, BIT0 },
  { GPIOB, BIT1 }
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );

float vref_out = 3.2256;
float vref_in  = 3.3270;
uint16_t adc_buf[n_adc_ch];
volatile uint32_t adc_state = 0; // 0 - pre, 1 - done, 2 + -  error

float pwm_out[n_pwm_ch] = { 0, 0, 0, 0 };
float dac_out[2] = { 0, 0 };

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  lcdt.init_4b();
  lcdt.cls();
  lcdt.putch( '.' );

  if( MX_DMA_Init() ) {
    lcdt.putch( 'd' );
  }
  if( MX_DAC_Init() ) {
    lcdt.putch( 'D' );
  }
  if( MX_ADC1_Init() ) {
    lcdt.putch( 'A' );
  }
  if( MX_TIM2_Init() ) {
    lcdt.putch( '2' );
  }
  if( MX_TIM3_Init() ) {
    lcdt.putch( '3' );
  }
  if( MX_TIM8_Init() ) {
    lcdt.putch( '8' );
  }

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );
  lcdt.gotoxy( 0, 1 ); lcdt.puts( PROJ_NAME );

  srl.re_ps();

  // oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 1,   100000000 );
  uint32_t t_step = UVAR('t');


  STDOUT_os;
  os << "# n= " << n << " t= " << t_step << NL; os.flush();

  char buf0[32], buf1[32];

  float vf[n_adc_ch];
  char adc_txt_bufs[n_adc_ch][16];
  int  d_in[n_din_ch];

  bool show_lcd = true;
  if( t_step < 50 ) {
    show_lcd = false;
    lcdt.cls();
    lcdt.puts( "t < 50 ms!  " );
  }

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    // collect input data
    adc_state = 0;
    uint32_t ct = HAL_GetTick();

    // fake data
    // for( int j=0; j<n_adc_ch; ++j ) {
    //   vf[j] = 0.001f * ( i % 1000 ) + j;
    // }
    for( int j=0; j<n_adc_ch; ++j ) {
      adc_buf[j] = 0;
    }

    // dma_subinit();
    // delay_ms( 1 );
    if( HAL_ADC_Start_DMA( &hadc1, (uint32_t *)adc_buf, n_adc_ch ) != HAL_OK )   {
      os << "## E Fail to start ADC_DMA" << NL;
    }
    for( int j=0; adc_state == 0 && j<50; ++j ) {
      delay_ms( 1 ); //      jj = j;
    }
    if( adc_state != 1 )  {
      os << "## E Fail to wait ADC_DMA " << adc_state << NL;
    }
    for( int j=0; j<n_adc_ch; ++j ) {
      vf[j] = vref_in * adc_buf[j] / 4095;
    }
    HAL_ADC_Stop_DMA( &hadc1 );

    for( int j=0; j<n_din_ch; ++j ) {
      d_in[j] = ( d_ins[j].gpio->IDR & d_ins[j].bit ) ? 1 : 0;
    }

    for( int j=0; j<n_adc_ch; ++j ) {
      snprintf( adc_txt_bufs[j],   sizeof(adc_txt_bufs[j]),  "%6.4f ", vf[j] );
    }

    // process data

    // TODO:

    // output data
    dac_output( vf[0], vf[2] );

    // output info

    os << i << ' ' << ( ct - tm00 ) << ' ';
    for( int j=0; j<n_adc_ch; ++j ) {
      os << adc_txt_bufs[j] << ' ';
    }
    for( int j=0; j<n_din_ch; ++j ) {
      os << d_in[j] << ' ';
    }
    os << NL;

    if( show_lcd || i == (n-1) ) {
      strcpy( buf0, adc_txt_bufs[0] );
      strcat( buf0, adc_txt_bufs[1] );
      strcpy( buf1, adc_txt_bufs[2] );
      strcat( buf1, adc_txt_bufs[3] );
      buf0[14] = d_in[0] ? '$' : '.';
      buf0[15] = d_in[1] ? '$' : '.';
      buf1[14] = d_in[2] ? '$' : '.';
      buf1[15] = d_in[3] ? '$' : '.';
      buf0[16] = '\0';  buf1[16] = '\0';
      lcdt.gotoxy( 0, 0 );
      lcdt.puts( buf0 );
      lcdt.gotoxy( 0, 1 );
      lcdt.puts( buf1 );
    }

    leds.toggle( BIT1 );

    os.flush();
    delay_ms_until_brk( &tm0, t_step );
  }

  pr( NL );

  return 0;
}

int cmd_dac( int argc, const char * const * argv )
{
  parse_floats( max(argc-1,2) , argv+1, dac_out );

  dac_output( dac_out[0], dac_out[1] );

  STDOUT_os;
  char buf0[16], buf1[16];
  snprintf( buf0, sizeof(buf0), "%f", dac_out[0] );
  snprintf( buf1, sizeof(buf1), "%f", dac_out[1] );
  os << "# DAC output: v0= " << buf0 << " v1= " << buf1 << NL; os.flush();

  return 0;
}

int cmd_pwm( int argc, const char * const * argv )
{
  static decltype( &TIM2->CCR1 ) ccrs[] = { &TIM2->CCR1, &TIM2->CCR2, &TIM2->CCR3, &TIM2->CCR4  };
  parse_floats( max(argc-1,4) , argv+1, pwm_out );
  uint32_t arr = TIM2->ARR;
  for( int i = 0; i<n_pwm_ch; ++i ) {
    *ccrs[i] = (uint32_t) ( pwm_out[i] * arr );
  }
  return 0;
}

// TODO: to common float funcs
int parse_floats( int argc, const char * const * argv, float *d )
{
  if( !argv || !d ) {
    return 0;
  }

  int n = 0;
  for( int i=0; i<argc; ++i ) {
    if( !argv[i] ) {
      break;
    }
    const char *s = argv[i];
    if( !s ) {
      break;
    }
    if( s[0] == '=' ) {
      if( s[1] == '\0' ) { // the same value
        ++n;
        continue;
      }
      // TODO: more variants
      continue;
    }
    // TODO: 0xNNNNNN
    float v;
    int rc = sscanf( s, "%f", &v );
    if( rc == 1 ) {
      d[i] = v;
      ++n;
    }
  }
  return n;
}

void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *AdcHandle )
{
  adc_state = 1;
  // leds.toggle( BIT0 );
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // NOP
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc_state = 2;
  // leds.toggle( BIT0 );
}


void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &hdma_adc1 );
}

void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &htim3 );
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

