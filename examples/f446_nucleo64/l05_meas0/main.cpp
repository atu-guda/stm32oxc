#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

// test code consumption
// #include <vector>
// #include <string>
// #include <map>

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

int cmd_tim_info( int argc, const char * const * argv );
CmdInfo CMDINFO_TIMINFO { "tim_info", 0, cmd_tim_info, " - info about timers"  };
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_DAC,
  &CMDINFO_PWM,
  &CMDINFO_TIMINFO,
  &CMDINFO_TEST0,
  nullptr
};


D_in_sources d_ins[n_din] = {
  { GPIOA, BIT0 },
  { GPIOA, BIT6 },
  { GPIOB, BIT6 },
  { GPIOA, BIT8 }
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );

volatile uint32_t adc_state = 0; // 0 - pre, 1 - done, 2 + -  error
volatile uint32_t  t3freq = 84000000, t3ccr1, t3ccr2, t5freq = 84000000, t5ccr1, t5ccr2;
volatile uint32_t  t3_i = 0, t5_i = 0;

float    time_f = 0;
int      time_i = 0;

float uin[n_uin];
unsigned nu_uin  = 4;
int uin_i[n_uin_i];
unsigned nu_uin_i = 4;
float uout[n_uout];
unsigned nu_uout = 4;
int uout_i[n_uout_i];
unsigned nu_uout_i = 4;

float vref_out = 3.2256;
float vref_in  = 3.3270;

float    adc[n_adc];
int      adc_i[n_adc];
uint16_t adc_u16[n_adc];

float pwm[n_pwm] = { 0, 0, 0, 0 };
float pwm_f = 10.0;
float dac[n_dac] = { 0, 0 };

int   din[n_din];
int   dins;
float din_f[n_din_f];
float din_dc[n_din_dc];
int   din_c[n_din_c];
float lcd[n_lcd];
int   lcd_b[n_lcd_b];
float tmp[n_tmp];

int measure_adc();
int measure_din();
int measure_din_tim();
int measure_uin();

int process_mode0();
int process_mode1();
int process_mode2();

int tty_output();
int lcd_output();

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') = 20;
  UVAR('f') = 10;
  UVAR('m') = 1; // for debug: freq

  i2c_default_init( i2ch /*, 400000 */ );
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
  if( MX_TIM1_Init() ) {
    lcdt.putch( '1' );
  }
  if( MX_TIM2_Init() ) {
    lcdt.putch( '2' );
  }
  if( MX_TIM3_Init() ) {
    lcdt.putch( '3' );
  }
  if( MX_TIM4_Init() ) {
    lcdt.putch( '4' );
  }
  if( MX_TIM5_Init() ) {
    lcdt.putch( '5' );
  }
  // if( MX_TIM8_Init() ) {
  //   lcdt.putch( '8' );
  // }
  MX_BTN_Init();

  t3freq = get_TIM_cnt_freq( TIM3 );
  t5freq = get_TIM_cnt_freq( TIM5 );


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

  bool show_lcd = true;
  if( t_step < 50 ) {
    show_lcd = false;
    lcdt.cls();
    lcdt.puts( "t < 50 ms!  " );
  }

  switch( UVAR('m') ) {
    case 0: // ADC/DAC mode
      nu_uout = 4; nu_uout_i = 4;
      break;
    case 1: // freq/duty cycle/counter mode
      nu_uout = 4; nu_uout_i = 6;
      break;
    default: // script mode
      break;
  }
  TIM1->CNT = 0; TIM3->CNT = 0; TIM4->CNT = 0; TIM5->CNT = 0;

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t ct = HAL_GetTick();
    time_i = ct - tm00;
    time_f = time_i * 0.001;
    int proc_rc = 0;

    measure_adc();
    measure_din();
    measure_din_tim();
    measure_uin();


    // process data
    switch( UVAR('m') ) {
      case 0: // ADC/DAC mode:
        proc_rc = process_mode0();
        break;
      case 1: // freq/duty cycle/counter mode
        proc_rc = process_mode1();
        break;
      default: // script mode
        proc_rc = process_mode2();
        break;
    }

    if( proc_rc < 1 ) {
      os << "# err: proc_rc = " << proc_rc << NL;
      break;
    }

    // output data

    dac_output();
    pwm_output();
    tty_output();
    if( show_lcd || i == (n-1) ) {
      lcd_output();
    }

    leds.toggle( BIT1 );

    delay_ms_until_brk( &tm0, t_step );
  }

  pr( NL );

  return 0;
}

int process_mode0()
{
  for( int j=0; j<n_adc; ++j ) {
    uout[j] = lcd[j] = adc[j];
    uout_i[j] = lcd_b[j] = din[j];
  }
  // TODO: dac,
  // dac[0] = uin[0]; dac[1] = uin[1];
  // here test only
  dac[0] = 2.0 - adc[0];
  dac[1] = adc[0] - adc[1];
  pwm[0] = 0.25 * adc[0];
  pwm[1] = 0.5;
  pwm_f =  2 + (int)( adc[0] * 100 ); // TODO: use it!
  UVAR('f') = pwm_f;
  return 1;
}

int process_mode1()
{
  lcd[0] = uout[0] = din_f[0];
  lcd[1] = uout[1] = din_dc[0];
  uout[2] = din_f[1]; uout[3] = din_dc[1];
  lcd[2] = uout_i[0] = din_c[0]; lcd[3] = uout_i[1] = din_c[1];
  for( int j=0; j<n_adc; ++j ) {
    uout_i[j+2] = lcd_b[j] = din[j];
  }
  return 1;
}

int process_mode2()
{
  return 0;
}


int measure_adc()
{
  adc_state = 0;
  for( auto &v : adc_u16 ) {
    v = 0;
  }

  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t *)adc_u16, n_adc ) != HAL_OK )   {
    errno = 10000;
    // os << "## E Fail to start ADC_DMA" << NL;
    return 0;
  }
  for( int j=0; adc_state == 0 && j<50; ++j ) {
    delay_ms( 1 ); //      jj = j;
  }
  if( adc_state != 1 )  {
    errno = 10001;
    // os << "## E Fail to wait ADC_DMA " << adc_state << NL;
    return 0;
  }

  for( int j=0; j<n_adc; ++j ) {
    adc_i[j] = adc_u16[j];
    adc[j]   = vref_in * adc_u16[j] / 4095;
  }
  HAL_ADC_Stop_DMA( &hadc1 );
  return 1;
}

int measure_din()
{
  dins = 0;
  for( int j=0; j<n_din; ++j ) {
    if ( d_ins[j].gpio->IDR & d_ins[j].bit ) {
      din[j] = 1;
      dins |= 1 << j;
    } else {
      din[j] = 0;
    }
  }
  return 1;
}

int measure_din_tim()
{
  static uint32_t old_t3_i = 0, old_t5_i = 0;
  if( t5ccr1 > 0  &&  old_t5_i != t5_i ) {
    din_f[0]  = (float)(t5freq) / t5ccr1;
    din_dc[0] = (float)(t5ccr2) / t5ccr1;
  } else {
    din_f[0]  = 0;
    din_dc[0] = 0;
  }
  old_t5_i = t5_i;

  if( t3ccr1 > 0  &&  old_t3_i != t3_i ) {
    din_f[1]  = (float)(t3freq) / t3ccr1;
    din_dc[1] = (float)(t3ccr2) / t3ccr1;
  } else {
    din_f[1]  = 0;
    din_dc[1] = 0;
  }
  old_t3_i = t3_i;
  UVAR('i') = t3ccr1;
  UVAR('j') = t3ccr2;

  din_c[0] = TIM4->CNT;
  din_c[1] = TIM1->CNT;
  TIM4->CNT = 0;
  TIM1->CNT = 0;

  return 1;
}

int measure_uin()
{
  // fake for now
  for( unsigned i=0; i < nu_uin; ++i ) {
    uin[i] = 0.1 * i;
  }
  for( unsigned i=0; i < nu_uin_i; ++i ) {
    uin_i[i] = 10 - i;
  }
  return 1;
}

int tty_output()
{
  STDOUT_os;
  char buf[32];

  snprintf( buf, sizeof(buf), "%#g ", time_f );
  os << buf;

  for( unsigned i=0; i < nu_uout; ++i ) {
    snprintf( buf, sizeof(buf), "%#g", uout[i] );
    os << buf << ' ';
  }

  for( unsigned i=0; i < nu_uout_i; ++i ) {
    os << uout_i[i] << ' ';
  }
  os << NL;
  os.flush();

  return 1;
}

int lcd_output()
{
  char buf[32];
  memset( buf, ' ', sizeof(buf)-1 ); buf[sizeof(buf)-1] = '\0';

  snprintf( buf, 8, "%#6g ", lcd[0] );
  buf[6] = ' ';
  snprintf( buf+7, 8, "%#6g ", lcd[1] );
  buf[13] = ' ';

  buf[14] = lcd_b[0] ? '$' : '.';
  buf[15] = lcd_b[1] ? '$' : '.';
  buf[16] = '\0';
  lcdt.gotoxy( 0, 0 );
  lcdt.puts( buf );

  memset( buf, ' ', sizeof(buf)-1 ); buf[sizeof(buf)-1] = '\0';
  snprintf( buf, 8, "%#6g ", lcd[2] );
  buf[6] = ' ';
  snprintf( buf+7, 8, "%#6g ", lcd[3] );
  buf[13] = ' ';

  buf[14] = lcd_b[2] ? '$' : '.';
  buf[15] = lcd_b[3] ? '$' : '.';
  buf[16] = '\0';
  lcdt.gotoxy( 0, 1 );
  lcdt.puts( buf );

  return 1;
}

// ----------------------- commands ---------


int cmd_dac( int argc, const char * const * argv )
{
  parse_floats( max(argc-1,2) , argv+1, dac );

  dac_output();

  STDOUT_os;
  char buf0[16], buf1[16];
  snprintf( buf0, sizeof(buf0), "%f", dac[0] );
  snprintf( buf1, sizeof(buf1), "%f", dac[1] );
  os << "# DAC output: v0= " << buf0 << " v1= " << buf1 << NL; os.flush();

  return 0;
}

int cmd_pwm( int argc, const char * const * argv )
{
  parse_floats( max(argc-1,4) , argv+1, pwm );
  pwm_output();
  tim_print_cfg( TIM2 );
  return 0;
}

int cmd_tim_info( int argc, const char * const * argv )
{
  STDOUT_os;
  os << "TIM1: ";  tim_print_cfg( TIM1 );
  os << "TIM2: ";  tim_print_cfg( TIM2 );
  os << "TIM3: ";  tim_print_cfg( TIM3 );
  os << "TIM4: ";  tim_print_cfg( TIM4 );
  os << "TIM5: ";  tim_print_cfg( TIM5 );
  dump8( TIM3, 0x60 );
  dump8( TIM5, 0x60 );
  return 0;
}


// -------------------- misc functions ----------------------------------

void pwm_output()
{
  static decltype( &TIM2->CCR1 ) ccrs[] = { &TIM2->CCR1, &TIM2->CCR2, &TIM2->CCR3, &TIM2->CCR4  };
  uint32_t arr = TIM2->ARR;
  uint32_t new_arr = calc_TIM_arr_for_base_freq( TIM2, UVAR('f') ); // TODO: pwm_f
  if( arr != new_arr ) {
    TIM2->ARR = new_arr;
    if( TIM2->CNT > new_arr - 2 ) {
      TIM2->CNT = new_arr - 2;
    }
    arr = new_arr;
  }
  for( int i = 0; i<n_pwm; ++i ) {
    *ccrs[i] = (uint32_t) ( pwm[i] * arr );
  }
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

int MX_BTN_Init()
{
  __GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef gio;
  gio.Pin       = GPIO_PIN_13;
  gio.Mode      = GPIO_MODE_IT_FALLING;
  gio.Pull      = GPIO_PULLUP;
  gio.Speed     = GPIO_SPEED_MAX;
  HAL_GPIO_Init( GPIOC, &gio );

  HAL_NVIC_SetPriority( EXTI15_10_IRQn, 10, 0 );
  HAL_NVIC_EnableIRQ(   EXTI15_10_IRQn );
  return 1;
}

void EXTI15_10_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT13 );
}

void HAL_GPIO_EXTI_Callback( uint16_t pin )
{
  static uint32_t last_exti_tick = 0;
  uint32_t curr_tick = HAL_GetTick();
  const char go_cmd[] = "T 100000000\n";
  if( curr_tick - last_exti_tick < 200 ) {
    return; // ignore too fast events
  }

  if( pin == BIT13 )  {
    leds.toggle( BIT0 );
    if( ! on_cmd_handler ) {
      if( devio_fds[0] ) {
        for( const char *s = go_cmd; *s; ++s ) {
          devio_fds[0]->unget( *s );
        }
      }
    } else {
      break_flag = 1;
    }
  }
  last_exti_tick = curr_tick;
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
  // HAL_TIM_IC_CaptureCallback( &htim3 );
}

void TIM5_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &htim5 );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  uint32_t sr;
  auto tim = htim->Instance;
  if( tim == TIM3 ) {
    if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 ) {
      sr = TIM3->SR;
      UVAR('a') = sr;
      if( ! ( sr & ( TIM_SR_CC2OF | TIM_SR_CC2OF ) ) ) {
        t3ccr1 = TIM3->CCR1;
        t3ccr2 = TIM3->CCR2;
        UVAR('b') = 1;
      } else {
        t3ccr1 = 0;
        t3ccr2 = 0;
        UVAR('b') = 0;
        UVAR('k') = sr;
        ++UVAR('g');
      }
      ++t3_i;
      TIM3->SR = 0;
    }
    return;
  }

  if( tim == TIM5 ) {
    if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 ) {
      sr = TIM5->SR;
      UVAR('c') = sr;
      if(  sr & ( TIM_SR_CC2OF | TIM_SR_CC2OF ) ) {
        t5ccr1 = 0;
        t5ccr2 = 0;
        UVAR('d') = 0;
        UVAR('l') = sr;
      } else {
        t5ccr1 = TIM5->CCR1;
        t5ccr2 = TIM5->CCR2;
        UVAR('d') = 1;
      }
      TIM5->SR = 0;
      ++t5_i;
    }
    return;
  }
  ++UVAR('e');
}

  // test code size consumption for STL parts
  // vector<char> tmp_x1;
  // tmp_x1.assign( 100, 'x' ); // + 100
  // os << tmp_x1[5] << NL;
  // string s1 = "xdfg";        // + 100
  // os << s1.c_str() << NL;
  // map<string, int> m_x;      // + 11k
  // m_x[s1] = 12;




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

