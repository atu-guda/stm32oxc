#include <algorithm>
#include <cmath>
#include <cerrno>

// test code consumption
// #include <vector>
// #include <string>
// #include <map>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_hd44780_i2c.h>
#include <oxc_floatfun.h>

#include "meas0.h"
#include "tcalclang.h"

using namespace std;
using namespace SMLRL;
using namespace tcalclang;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

// PinsOut ldbg( GPIOB, 13, 3 ); // B13:B15 (hides SPI2, but used only for debug)

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

int cmd_ld( int argc, const char * const * argv );
CmdInfo CMDINFO_LD { "ld", 0, cmd_ld, "[name] - list data"  };
int cmd_sd( int argc, const char * const * argv );
CmdInfo CMDINFO_SD { "sd", 0, cmd_sd, "name value - set data value"  };

int cmd_addCmd( int argc, const char * const * argv );
CmdInfo CMDINFO_ADDCMD { "addCmd", 'A', cmd_addCmd, "\"command [args]\" - append command to program"  };
int cmd_clearPgm( int argc, const char * const * argv );
CmdInfo CMDINFO_CLEARPGM { "clearPgm", 0, cmd_clearPgm, " - clear program"  };
int cmd_execPgm( int argc, const char * const * argv );
CmdInfo CMDINFO_EXECPGM { "execPgm", 0, cmd_execPgm, " - exec program"  };
int cmd_execCmd( int argc, const char * const * argv );
CmdInfo CMDINFO_EXECCMD { "ec", 0, cmd_execCmd, "\"command [args]\" - exec single command"  };
int cmd_listPgm( int argc, const char * const * argv );
CmdInfo CMDINFO_LISTPGM { "lpgm", 0, cmd_listPgm, " - list current program"  };
int cmd_testPgm( int argc, const char * const * argv );
CmdInfo CMDINFO_TESTPGM { "tpgm", 0, cmd_testPgm, " - load test program"  };

int cmd_tloop( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "tloop", 'T', cmd_tloop, " - start loop"  };
int cmd_exch( int argc, const char * const * argv );
CmdInfo CMDINFO_EXCH { "exch", 'X', cmd_exch, "[user_in_data] - one step"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_DAC,
  &CMDINFO_PWM,
  &CMDINFO_TIMINFO,
  &CMDINFO_LD,
  &CMDINFO_SD,
  &CMDINFO_ADDCMD,
  &CMDINFO_CLEARPGM,
  &CMDINFO_EXECPGM,
  &CMDINFO_EXECCMD,
  &CMDINFO_LISTPGM,
  &CMDINFO_TESTPGM,
  &CMDINFO_TEST0,
  &CMDINFO_EXCH,
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

Datas datas;
Engine eng( datas );
const float eng_M_PI = (float)(M_PI);

volatile uint32_t adc_state = 0; // 0 - pre, 1 - done, 2 + -  error
volatile uint32_t  t3freq = 84000000, t3ccr1, t3ccr2, t5freq = 84000000, t5ccr1, t5ccr2;
volatile uint32_t  t3_i = 0, t5_i = 0;

float    time_f  = 0;
int      time_i  = 0;
uint32_t ttick_0 = 0; // time in ticks ( ms, start value)

float uin[n_uin];
int   nu_uin  = 4;
int uin_i[n_uin_i];
int   nu_uin_i = 4;
float uout[n_uout];
int   nu_uout = 4;
int uout_i[n_uout_i];
int   nu_uout_i = 4;
int   show_eq = 0;

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

bool show_lcd = true;
bool meas_inited = false;

int one_step();
int init_meas();

int measure_adc();
int measure_din();
int measure_din_tim();
int convert_uin( int argc, const char * const * argv );
int measure_uin();

int process_mode0();
int process_mode1();
int process_mode2();

int tty_output();
int lcd_output();

int main(void)
{
  BOARD_PROLOG;
  // ldbg.initHW();

  UVAR('t') = 100;
  UVAR('n') = 20;
  UVAR('m') = 0;
  pwm_f = 10.0;

  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;

  ADD_DATAS_NM( user_vars, "UVAR"  );
  ADD_DATA_RO( time_i );
  ADD_DATA_RO( time_f );
  ADD_DATA_NM_RO( eng_M_PI, "M_PI" );
  ADD_DATA( vref_in );
  ADD_DATA( vref_out );
  ADD_DATAS( uin );
  ADD_DATA( nu_uin );
  ADD_DATAS( uin_i );
  ADD_DATA( nu_uin_i );
  ADD_DATAS( uout );
  ADD_DATA( nu_uout );
  ADD_DATAS( uout_i );
  ADD_DATA( nu_uout_i );
  ADD_DATA( show_eq );
  ADD_DATAS( adc );
  ADD_DATAS( adc_i );
  ADD_DATAS( dac );
  ADD_DATAS( din );
  ADD_DATA(  dins );
  ADD_DATAS( pwm );
  ADD_DATA(  pwm_f );
  ADD_DATAS( din_f );
  ADD_DATAS( din_dc );
  ADD_DATAS( din_c );
  ADD_DATAS( lcd );
  ADD_DATAS( lcd_b );
  ADD_DATAS( tmp );

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

int one_step()
{
  int proc_rc;
  measure_adc();
  measure_din_tim();


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
    return 0;
  }

  // output data

  dac_output();
  pwm_output();
  tty_output();
  if( show_lcd ) {
    lcd_output();
  }

  leds.toggle( BIT0 );
  return 1;
}

int init_meas()
{
  switch( UVAR('m') ) {
    case 0: // ADC/DAC mode
      nu_uout = 4; nu_uout_i = 4; nu_uin = 6; nu_uin_i = 0; // 6 = 2 DAC + 2 PWM
      break;
    case 1: // freq/duty cycle/counter mode
      nu_uout = 4; nu_uout_i = 6; nu_uin = 2; nu_uin_i = 4; // TODO: fix uin
      break;
    default: // script mode
      break;
  }
  TIM1->CNT = 0; TIM3->CNT = 0; TIM4->CNT = 0; TIM5->CNT = 0;

  ttick_0  = HAL_GetTick();
  meas_inited = true;
  return 1;
}


int process_mode0()
{
  for( int j=0; j<n_adc; ++j ) {
    uout[j] = lcd[j] = adc[j];
    uout_i[j] = lcd_b[j] = din[j];
  }
  dac[0] = uin[0]; dac[1] = uin[1];
  pwm[0] = uin[2]; pwm[1] = uin[3]; pwm[2] = uin[4]; pwm[3] = uin[5];
  // here test only
  // dac[0] = 2.0 - adc[0];
  // dac[1] = adc[0] - adc[1];
  // pwm[0] = 0.25 * adc[0];
  // pwm[1] = 0.5;
  // pwm_f =  2 + (int)( adc[0] * 100 ); // TODO: use it!
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
  return eng.exec();
}


int measure_adc()
{
  adc_state = 0;
  for( auto &v : adc_u16 ) {
    v = 0;
  }

  if( HAL_ADC_Start_DMA( &hadc1, (uint32_t *)adc_u16, n_adc ) != HAL_OK )   {
    errno = 10000;
    // std_out << "## E Fail to start ADC_DMA" << NL;
    return 0;
  }
  for( int j=0; adc_state == 0 && j<50; ++j ) {
    delay_ms( 1 ); //      jj = j;
  }
  if( adc_state != 1 )  {
    errno = 10001;
    // std_out << "## E Fail to wait ADC_DMA " << adc_state << NL;
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
  // static uint32_t old_t3_i = 0, old_t5_i = 0; // TODO: more correct
  if( t5ccr1 > 0 /*  &&  old_t5_i != t5_i */ ) {
    din_f[0]  = (float)(t5freq) / t5ccr1;
    din_dc[0] = (float)(t5ccr2) / t5ccr1;
  } else {
    din_f[0]  = 0;
    din_dc[0] = 0;
  }
  // old_t5_i = t5_i;

  if( t3ccr1 > 0  /* &&  old_t3_i != t3_i */ ) {
    din_f[1]  = (float)(t3freq) / t3ccr1;
    din_dc[1] = (float)(t3ccr2) / t3ccr1;
  } else {
    din_f[1]  = 0;
    din_dc[1] = 0;
  }
  // old_t3_i = t3_i;
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
  char buf[128];
  unsigned sz = tryGetLine( 0, buf, sizeof(buf) );
  if( sz < 2  ||  buf[0] != '=' ) {
    return 0;
  }

  char *b = buf+1;
  char *eptr;
  int ncvt = 0;

  for( int i=0; i < nu_uin; ++i ) {
    float v = strtof( b, &eptr );
    if( b == eptr ) {
      return ncvt;
    }
    uin[i] = v; ++ncvt;
    if( *eptr == '\0' ) {
      return ncvt;
    }
    b = eptr;
  }

  for( int i=0; i < nu_uin_i; ++i ) {
    int v = strtol( b, &eptr, 0 );
    if( b == eptr ) {
      return ncvt;
    }
    uin_i[i] = v; ++ncvt;
    if( *eptr == '\0' ) {
      return ncvt;
    }
    b = eptr;
  }
  return ncvt;
}

int convert_uin( int argc, const char * const * argv )
{
  int carg = 1;

  for( int i=0; carg < argc  &&  i < nu_uin; ++i, ++carg ) {
    uin[i] = strtof( argv[carg], 0 );
  }
  for( int i=0; carg < argc  &&  i < nu_uin_i; ++i, ++carg ) {
    uin_i[i] = strtol( argv[carg], 0, 0 );
  }
  return carg - 1;
}

int tty_output()
{
  if( show_eq ) {
    std_out << "= ";
  }
  std_out << time_f << ' ';

  for( int i=0; i < nu_uout; ++i ) {
    std_out << uout[i] << ' ';
  }

  for( int i=0; i < nu_uout_i; ++i ) {
    std_out << uout_i[i] << ' ';
  }
  std_out << NL;
  std_out.flush();

  return 1;
}

int lcd_output()
{
  char buf[32];
  memset( buf, ' ', sizeof(buf)-1 ); buf[sizeof(buf)-1] = '\0';

  cvtff( lcd[0], buf, 8, cvtff_auto, 6, 4 );
  buf[6] = ' ';
  cvtff( lcd[1], buf+7, 8, cvtff_auto, 6, 4 );
  buf[13] = ' ';

  buf[14] = lcd_b[0] ? '$' : '.';
  buf[15] = lcd_b[1] ? '$' : '.';
  buf[16] = '\0';
  lcdt.gotoxy( 0, 0 );
  lcdt.puts( buf );

  memset( buf, ' ', sizeof(buf)-1 ); buf[sizeof(buf)-1] = '\0';
  cvtff( lcd[2], buf, 8, cvtff_auto, 6, 4 );
  buf[6] = ' ';
  cvtff( lcd[3], buf+7, 8, cvtff_auto, 6, 4 );
  buf[13] = ' ';

  buf[14] = lcd_b[2] ? '$' : '.';
  buf[15] = lcd_b[3] ? '$' : '.';
  buf[16] = '\0';
  lcdt.gotoxy( 0, 1 );
  lcdt.puts( buf );

  return 1;
}

// ----------------------- commands ---------

int cmd_tloop( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 1,   100000000 );
  uint32_t t_step = UVAR('t');

  std_out << "# n= " << n << " t= " << t_step << NL; std_out.flush();

  show_lcd = true;
  if( t_step < 50 ) {
    show_lcd = false;
    lcdt.cls();
    lcdt.puts( "t < 50 ms!  " );
  }

  init_meas();

  uint32_t ttick_base = ttick_0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t ct = HAL_GetTick();
    time_i = ct - ttick_0;
    time_f = time_i * 0.001;

    measure_uin();
    int proc_rc = one_step();

    if( proc_rc < 1 ) {
      std_out << "# err: proc_rc = " << proc_rc << NL;
      break;
    }

    delay_ms_until_brk( &ttick_base, t_step );
  }

  lcd_output();
  meas_inited = false;

  pr( NL );

  return 0;
}

int cmd_exch( int argc, const char * const * argv )
{
  if( ! meas_inited ) {
    init_meas();
  }

  uint32_t ct = HAL_GetTick();
  time_i = ct - ttick_0;
  time_f = time_i * 0.001;

  convert_uin( argc, argv );
  int proc_rc = one_step();

  if( proc_rc < 1 ) {
    std_out << "# err: proc_rc = " << proc_rc << NL;
    return 1;
  }

  return 0;
}

int cmd_ld( int argc, const char * const * argv )
{
  return !datas.dump( argv[1] );
}

int cmd_sd( int argc, const char * const * argv )
{
  if( argc < 3 ) {
    std_out << "# Error: need 2 arguments" << NL;
    return 1;
  }
  int rc = datas.set( argv[1], argv[2] );
  return !rc;
}

int cmd_addCmd( int argc, const char * const * argv )
{
  int rc = eng.addCmd( argv[1] );
  std_out << "# pgm count << " << eng.pgmSize() << NL;
  return !rc;
}

int cmd_clearPgm( int argc, const char * const * argv )
{
  eng.clear();
  return 0;
}

int cmd_execPgm( int argc, const char * const * argv )
{
  int rc = eng.exec();
  eng.dumpState();
  return !rc;
}

int cmd_execCmd( int argc, const char * const * argv )
{
  int rc = eng.execCmdStr( argv[1] );
  eng.dumpState();
  return !rc;
}

int cmd_listPgm( int argc, const char * const * argv )
{
  eng.listPgm();
  return 0;
}

int cmd_testPgm( int argc, const char * const * argv )
{
  eng.clear();
  static const char *t[] = {
    "L adc[0]",
    "+ adc[1]",
    "/ 2",
    "S dac[0]",
    "L uin[0]",
    "fabs",
    "S dac[1]",
    "L uin[1]",
    "* 10",
    "+ 2.1",
    "S pwm_f",
    "L uin[2]",
    "S pwm[0]",
    "* 2",
    "* M_PI",
    "S pwm[1]",
    // "S time_f", // bad: ro
  };
  for( auto s : t ) {
    eng.addCmd( s );
  }
  return 0;
}

int cmd_dac( int argc, const char * const * argv )
{
  parse_floats( max(argc-1,2) , argv+1, dac );

  dac_output();

  std_out << "# DAC output: v0= " << dac[0] << " v1= " << dac[1] << NL; std_out.flush();

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
  std_out << "TIM1: ";  tim_print_cfg( TIM1 );
  std_out << "TIM2: ";  tim_print_cfg( TIM2 );
  std_out << "TIM3: ";  tim_print_cfg( TIM3 );
  std_out << "TIM4: ";  tim_print_cfg( TIM4 );
  std_out << "TIM5: ";  tim_print_cfg( TIM5 );
  dump8( TIM3, 0x60 );
  dump8( TIM5, 0x60 );
  return 0;
}


// -------------------- misc functions ----------------------------------

void pwm_output()
{
  static decltype( &TIM2->CCR1 ) ccrs[] = { &TIM2->CCR1, &TIM2->CCR2, &TIM2->CCR3, &TIM2->CCR4  };
  uint32_t arr = TIM2->ARR;
  uint32_t new_arr = calc_TIM_arr_for_base_freq( TIM2, pwm_f );
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
    char *eptr;
    float v = strtof( s, &eptr );

    if( *eptr == '\0' ) {
      d[i] = v;
      ++n;
    }
  }
  return n;
}

int MX_BTN_Init()
{
  GpioC.enableClk();
  GpioC.cfgIn( 13, GpioRegs::Pull::up );
  GpioC.setEXTI( 13, GpioRegs::ExtiEv::down );

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
  // ldbg.set( BIT0 );
  HAL_TIM_IRQHandler( &htim3 );
  // HAL_TIM_IC_CaptureCallback( &htim3 );
  // ldbg.reset( BIT0 );
}

void TIM5_IRQHandler(void)
{
  // ldbg.set( BIT1 );
  HAL_TIM_IRQHandler( &htim5 );
  // ldbg.reset( BIT1 );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  // ldbg.set( BIT2 );
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
    // ldbg.reset( BIT2 );
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
    // ldbg.reset( BIT2 );
    return;
  }
  // ldbg.reset( BIT2 );
  ++UVAR('e');
}

  // test code size consumption for STL parts
  // vector<char> tmp_x1;
  // tmp_x1.assign( 100, 'x' ); // + 100
  // std_out << tmp_x1[5] << NL;
  // string s1 = "xdfg";        // + 100
  // std_out << s1.c_str() << NL;
  // map<string, int> m_x;      // + 11k
  // m_x[s1] = 12;




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

