#include <climits>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_pin_d.h>
#include <oxc_gpio_pins_d.h>

#include <oxc_addr_enco_d.h>
#include <oxc_as5600_enco_d.h>

#include <oxc_tim_pwm_d.h>

#include <oxc_adcint_adc_d.h>

#include <board_robo_cfg.h>


using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test misc robo parts. TMP." NL;


// ------------------------ - local commands; ---------------------------------------
DCL_CMD_REG(      test_pin_d,   'T',      " [arg ] - pin1_d"  );
DCL_CMD_REG(      test_pins_hd, '\0',     " [arg ] - pins_hd"  );
DCL_CMD_REG(      cap_wi,       '\0',     " i ch vi - write cap int"  );
DCL_CMD_REG(      cap_wf,       '\0',     " i ch vf - write cap float"  );
DCL_CMD_REG(      cap_ri,       '\0',     " i ch - read cap int"  );
DCL_CMD_REG(      cap_rf,       '\0',     " i ch - read cap float"  );
DCL_CMD_REG(      prt,          '\0',     " print tim info"  );

// -------------------------------------------------------------------------------------

ReturnCode init_hw_all();

// ------------------------ low-level devices -------------------------------------------

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 as5600_sens( i2cd );


// ------------------------ Devices: capabilities ---------------------------------------

Gpio_Pin_Dev      pin1_hd( PC10 ); // TODO: to header
PinCapability     pin1_d(  pin1_hd );
PinRoboCapability pin1_rd( pin1_hd, 100 );

Gpio_Pin_Dev      pin2_hd( PC11 );
PinCapability     pin2_d(  pin1_hd );
PinRoboCapability pin2_rd( pin2_hd, 101 );

Gpio_Pin_Dev      pini_hd( PC13 );
PinCapability     pini_d(  pini_hd );
PinRoboCapability pini_rd( pini_hd, 102 );

Gpio_Pins_Dev      pins_hd( PC0, 4 ); // copy of leds
PinsCapability     pins_d(  pins_hd );
PinsRoboCapability pins_rd( pins_hd, 200 );

TIM_HandleTypeDef  tim_mpwm_h;
Tim_Pwm_Dev        mpwm_hd( TIM_MPWM_BASE, tim_MPWM_chspins, tim_mpwm_h, 0xFFFF ); // TODO: timer props
float              mpwm_io[3*6];
PwmRoboCapability  mpwm_rd( mpwm_hd, 1, mpwm_io, 500 );
//
Gpio_Pin_Dev      pin_mpwm_l_hd( MPWM_CtlPin_L );
PinRoboCapability pin_mpwm_l_rd( pin_mpwm_l_hd, 501 );
Gpio_Pin_Dev      pin_mpwm_r_hd( MPWM_CtlPin_R );
PinRoboCapability pin_mpwm_r_rd( pin_mpwm_r_hd, 502 );

TIM_HandleTypeDef  tim_servolwm_h;
Tim_Pwm_Dev        servolwm_hd( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h, 0xFFFF );
float              servolwm_io[3*6];
PwmRoboCapability  servolwm_rd( servolwm_hd, 1, servolwm_io, 600 );

TIM_HandleTypeDef  tim_enco_h;
Addr_Enco_Dev      tim_enco_hd( TIM_ENCODER_CNTBASE, 0xFFFF, true ); // TODO: separate device to init
EncoderCapability  tim_enco_d( tim_enco_hd );
EncoderRoboCapability tim_enco_rd( tim_enco_hd, 700 );

AS5600_Enco_Dev    as5600_enco_hd( as5600_sens, true );
EncoderCapability  as5600_enco_d( as5600_enco_hd );
EncoderRoboCapability as5600_enco_rd( as5600_enco_hd, 800 );
constexpr int32_t as5600_zero { 2739 }; // Beware: mech-dependent constant

ADC_Info adc_sensor ( ADC_SENSOR, ADC_SENSOR_CHPINS ); // here: need correct "data" init
Adcint_Adc_Dev adcint_adc_hd( adc_sensor, ADC_SENSOR_N_CH, ADC_resol_to_bits( ADC_SENSOR_Resolution ), 100 );
AdcRoboCapability adcint_adc_rd(adcint_adc_hd, 900 );


// ------------------------ - Channels and transforms ---------------------------------------
// output TODO: channel names

TransFILin tro_pin1( 0.1f, -1.0f );
OutChFI    oc_pin1( pin1_rd, PinRoboCapability::ch_write, tro_pin1 );

TransFILin tro_pin2( 0.5f, -0.5f );
OutChFI    oc_pin2( pin2_rd, PinRoboCapability::ch_write, tro_pin2 );

TransFILinLim tro_pins( 0.1f, 1.0f, 0, 14 );
OutChFI    oc_pins( pins_rd, PinsRoboCapability::ch_write, tro_pins );

OutChFSplit2 oc_split( oc_pin1, oc_pin2, globalTransFFUnity );

OutChFF    oc_mpwm_duty(  mpwm_rd, PwmRoboCapability::ch0_pwm,   globalTransFFUnity );
OutChFF    oc_mpwm_pulse( mpwm_rd, PwmRoboCapability::ch0_pulse, globalTransFFUnity );
OutChFF    oc_mpwm_freq(  mpwm_rd, PwmRoboCapability::ch_freq,   globalTransFFUnity );
OutChII    oci_mpwm_l( pin_mpwm_l_rd, PinRoboCapability::ch_write, globalTransIIUnity );
OutChII    oci_mpwm_r( pin_mpwm_r_rd, PinRoboCapability::ch_write, globalTransIIUnity );

OutChINull oci_null;

TransFFLinLim tro_rad2pulse( 2.0e-3f/pi_f, 1.5e-3f, 0.5e-3f, 2.5e-3f ); // angle +/ pi/2 -> 500-2500 μs
OutChFF    oc_servolwm_rad2pulse( servolwm_rd, PwmRoboCapability::ch0_pulse, tro_rad2pulse );

// input
TransIFLin tri_pin1( 10.0f, 0.3f );
InChFI     ic_pin1( pin1_rd, PinRoboCapability::ch_read, tri_pin1 );
InChII     ici_pin1( pin1_rd, PinRoboCapability::ch_read, globalTransIIUnity );

TransIFLin tri_pin2( 0.1f, -2.0f );
InChFI     ic_pin2(  pin2_rd, PinRoboCapability::ch_read, tri_pin2 );
InChII     ici_pin2( pin2_rd, PinRoboCapability::ch_read, globalTransIIUnity );

TransIFLin tri_pins( 1.1f, 0.01f );
InChFI     ic_pins( pins_rd, PinRoboCapability::ch_read, tri_pins );

InChFConst ic_const( 3.1415f );

InChFSum2  ic_sum( ic_pins, ic_pin1, globalTransFFUnity, 0.1f, -0.1f );

TransIFLin tri_tim_enco( pi_f/1200, 0 );
InChFI     ic_tim_enco( tim_enco_rd, EncoderRoboCapability::ch_pos, tri_tim_enco );

TransIFLin tri_as5600_enco( 2*pi_f/AS5600::val2turn, 0 );
InChFI     ic_as5600_enco( as5600_enco_rd, EncoderRoboCapability::ch_pos, tri_as5600_enco );

TransIFLin tri_adcint_adc( 9.2129e-4f, -1.861f ); // TODO: mech-dependent value
InChFI     ic_adcint_adc_0( adcint_adc_rd, 0, tri_adcint_adc );
InChFI     ic_adcint_adc_1( adcint_adc_rd, 1, tri_adcint_adc );
InChFI     ic_adcint_adc_2( adcint_adc_rd, 2, tri_adcint_adc );
InChFI     ic_adcint_adc_3( adcint_adc_rd, 3, tri_adcint_adc );

//   input int
InChIConst ici_const( 42 );

// ------------------------ Channels and transforms end ---------------------------------------


Gpio_Pin_Dev* hw_pin[] {
  &pin1_hd,
  &pin2_hd,
  &pini_hd,
};

IoCapability* caps[] {
  &pin1_d,         // 0
  &pin2_d,         // 1
  &pini_d,         // 2
  &pins_d,         // 3
  &tim_enco_d,     // 4
  &as5600_enco_d,  // 5
};

IoRoboCapability* robo_caps[] {
  &pin1_rd,          // 0
  &pin2_rd,          // 1
  &pini_rd,          // 2
  &pins_rd,          // 3
  &mpwm_rd,          // 4
  &pin_mpwm_l_rd,    // 5
  &pin_mpwm_r_rd,    // 6
  &servolwm_rd,      // 7
  &tim_enco_rd,      // 8
  &as5600_enco_rd,   // 9
  &adcint_adc_rd,    // 10
};

std::span<IoRoboCapability*> oxc::global_robo_caps { robo_caps };

RoboObject* robo_objs[] {
  &pin1_rd,          // 0
  &pin2_rd,          // 1
  &pini_rd,          // 2
  &pins_rd,          // 3
  &mpwm_rd,          // 4
  &pin_mpwm_l_rd,    // 5
  &pin_mpwm_r_rd,    // 6
  &servolwm_rd,      // 7
  &tim_enco_rd,      // 8
  &as5600_enco_rd,   // 9
  &adcint_adc_rd,    // 10
};

std::span<RoboObject*> oxc::global_robo_objs { robo_objs };

OutChFBase* channels_out_f[] {
  &oc_pin1,               // 0
  &oc_pin2,               // 1
  &oc_pins,               // 2
  &oc_split,              // 3
  &oc_mpwm_duty,          // 4
  &oc_mpwm_pulse,         // 5
  &oc_servolwm_rad2pulse, // 6
  &oc_mpwm_freq,          // 7
};

std::span<OutChFBase*> oxc::global_channels_out_f { channels_out_f };

OutChIBase* channels_out_i[] {
  &oci_mpwm_l,             // 0
  &oci_mpwm_r,             // 1
  &oci_null,               // 2
};

std::span<OutChIBase*> oxc::global_channels_out_i { channels_out_i };

InChFBase* channels_in_f[] {
  &ic_pin1,            // 0
  &ic_pin2,            // 1
  &ic_pins,            // 2
  &ic_const,           // 3
  &ic_sum,             // 4
  &ic_tim_enco,        // 5
  &ic_as5600_enco,     // 6
  &ic_adcint_adc_0,    // 7
  &ic_adcint_adc_1,    // 8
  &ic_adcint_adc_2,    // 9
  &ic_adcint_adc_3,    // 10
};

std::span<InChFBase*>  oxc::global_channels_in_f { channels_in_f };

InChIBase* channels_in_i[] {
  &ici_const,          // 0
  &ici_pin1,           // 1
  &ici_pin2,           // 2
};

std::span<InChIBase*>  oxc::global_channels_in_i { channels_in_i };


RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( robo_objs, robo_joints );

RoboAssembly& oxc::global_robo{ robo };


void idle_main_task()
{
  robo.at_main_idle();
}

void test_pinx_hd();
void test_pins_hd();

int main(void)
{
  BOARD_PROLOG;

  UVAR_a =    1; // auto commit
  UVAR_l =    1; // idle after run ?
  UVAR_n =   20; // n test
  UVAR_s = 1000; // scale
  UVAR_t =  100; // default delay

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  // oxc_add_aux_tick_fun( led_task_nortos ); // tmp disable to test leds

  robo.start_time();

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  pin1_hd.initHW();
  pin2_hd.initHW();
  // pini_hd.getPin()->dev().cfgIn();
  PC13.cfgIn();

  pins_hd.initHW(); // dup from leds, but may be another pins?

  pin2_rd.setFlags( RoboObject::noMeasure );
  pini_rd.setFlags( RoboObject::noCommit );

  TIM_MPWM_CLKEN(); // TODO: auto
  mpwm_hd.initHW();
  mpwm_hd.setFreq( MPWM_FREQ );
  mpwm_hd.enable();
  pin_mpwm_l_hd.initHW();
  pin_mpwm_r_hd.initHW();

  TIM_SERVOLWM_CLKEN();
  servolwm_hd.initHW();
  servolwm_hd.setFreq( SERVOLWM_FREQ );
  servolwm_hd.enable();

  TIM_ENCODER_CLKEN(); // manual config, as not in device
  tim_enco_h.Instance = TIM_ENCO;
  tim_enco_cfg_default( tim_enco_h, tim_ENCODER_chspins );

  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &as5600_sens;
  // TODO: to bsp
  as5600_sens.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  as5600_enco_hd.setStartPos( as5600_zero );

  adc_sensor_init();

  return robo.init_all();
}


CMD_FUNCTION( test_pin_d )
{
  test_pinx_hd();
  return 0;
}

CMD_FUNCTION( test_pins_hd )
{
  test_pins_hd();
  return 0;
}


void out_pin_st( Gpio_Pin_Dev *ppin )
{
  std_out << '[' << ppin->read().value_or(5) << ' ' << ppin->readwr().value_or(6) << ']';
}

void test_pinx_hd()
{
  const uint32_t n = UVAR_n;
  for( uint32_t i=0; i<n; ++i ) {
    for( auto ppin : hw_pin ) {
      ppin->set(); out_pin_st( ppin );
    }
    delay_ms( UVAR_t );
    for( auto ppin : hw_pin ) {
      ppin->reset(); out_pin_st( ppin );
    }
    std_out << NL;
    delay_ms( UVAR_t );
  }

  for( uint32_t i=0; i<n; ++i ) {
    for( auto ppin : hw_pin ) {
      ppin->toggle(); out_pin_st( ppin );
    }
    std_out << NL;
    delay_ms( UVAR_t );
  }

  for( uint32_t i=0; i<n; ++i ) {
    for( auto ppin : hw_pin ) {
      ppin->write( i ); out_pin_st( ppin );
    }
    delay_ms( UVAR_t );
    std_out << NL;
  }

}


void out_pins_st( Gpio_Pins_Dev *ppins )
{
  std_out << '[' << ppins->read().value_or(7) << ' ' << ppins->readwr().value_or(8) << ']';
}

void test_pins_hd()
{
  const uint32_t n = UVAR_n;

  std_out << "# write" NL;
  for( uint32_t i=0; i<n; ++i ) {
    std_out << i << ' ';
    pins_hd.write( i ); out_pins_st( &pins_hd );
    std_out << NL;
    delay_ms( UVAR_t );
  }

  std_out << "# set/reset" NL;
  pins_hd.reset( 0xFF );
  for( uint32_t i=0; i<n; ++i ) {
    std_out << i << ' ';
    pins_hd.set( i ); out_pins_st( &pins_hd );
    delay_ms( UVAR_t );
    pins_hd.set( 0xFF );
    pins_hd.reset( i ); out_pins_st( &pins_hd );
    std_out << NL;
  }

  std_out << "# toggle" NL;
  pins_hd.reset( 0xFF );
  for( uint32_t i=0; i<n; ++i ) {
    std_out << i << ' ';
    pins_hd.toggle( i ); out_pins_st( &pins_hd );
    delay_ms( UVAR_t );
    std_out << NL;
  }


}

CMD_FUNCTION( cap_wi  )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  int32_t vi  = arg2long_d(  3, argc, argv, 0 );
  auto rc = caps[idx]->setVal( ch, vi );
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  return 0;
}


CMD_FUNCTION( cap_wf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  float vf = arg2float_d( 3, argc, argv, 0 );
  auto rc = caps[idx]->setValF( ch, vf );
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  return 0;
}


CMD_FUNCTION( cap_ri )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = caps[idx]->getVal( ch );
  if( r ) {
    std_out << "# val= " << r.value() << NL;
  } else {
    std_out << "# eer " << r.error().code << NL;
  }
  return 0;
}

CMD_FUNCTION( cap_rf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = caps[idx]->getValF( ch );
  if( r ) {
    std_out << "# val= " << r.value() << NL;
  } else {
    std_out << "# eer " << r.error().code << NL;
  }
  return 0;
}



CMD_FUNCTION( prt )
{
  std_out << "# MPWM: " NL;
  tim_print_cfg( TIM_MPWM_BASE );
  std_out << "# ENCODER: " NL;
  tim_print_cfg( TIM_ENCODER_BASE );
  std_out << "# SERVOLWM: " NL;
  tim_print_cfg( TIM_SERVOLWM_BASE );
  delay_ms( 100 );
  std_out << "# AS5600: " << as5600_sens.getAngleNoTurn() << NL;
  adc_sensor.pr_state();
  return 0;
}





