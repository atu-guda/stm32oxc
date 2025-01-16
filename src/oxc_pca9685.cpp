#include <oxc_pca9685.h>

bool PCA9685::init( uint8_t presc_new )
{
  if( presc_new > 2 ) {
    presc = presc_new;
  }

  sleep(); // prescale can only be set while sleep
  if( send_reg1_8bit( reg_prescale, presc ) < 1 ) {
    return false;
  }
  if( send_reg1_8bit( reg_mode0, cmd_on_inc ) < 1 ) {
    return false;
  }
  return true;
}

void PCA9685::sleep()
{
  send_reg1_8bit( reg_mode0, cmd_off );
}

void PCA9685::set( uint8_t ch, uint16_t on, uint16_t off )
{
  uint16_t s[2] { uint16_t(on & v_out_mask), uint16_t(off & v_out_mask) };
  send_reg1( ch2regOn( ch ), (uint8_t*)s, sizeof(s) );
}

void PCA9685::setServoMinMax( uint32_t min_us, uint32_t max_us )
{
  servo_min = min_us; servo_max = max_us;
  if( servo_max <= servo_min ) {
    servo_max = servo_min + 1;
  }
  servo_mid = ( servo_us_max + servo_us_min ) / 2 ;
  servo_dvt = ( servo_us_max - servo_us_min );
}

void PCA9685::setServo( uint8_t ch, int val )
{
  uint16_t s[2] { 0, servo2v( val ) };
  send_reg1( ch2regOn( ch ), (uint8_t*)s, sizeof(s) );
}

void PCA9685::off( uint8_t ch )
{
  send_reg1_8bit( ch2regOff( ch ) + 1, bit_onoff );
}

void PCA9685::setAllServo( int val )
{
  uint16_t s[2] { 0, servo2v( val ) };
  send_reg1( reg_all_s, (uint8_t*)&s, sizeof(s) );
}

// uint8_t PCA9685::freq2prec( uint32_t freq ) // static
// {
//   return (uint8_t) std::clamp( ( base_freq / ( 4096 * freq ) ) - 1, 3ul, 255ul );
// }

void PCA9685::offAll()
{
  send_reg1_8bit( reg_all_s + 3, bit_onoff );
}

uint32_t PCA9685::servo2t( int a ) const
{
  return servo_mid + (int)servo_dvt * a / ( 2 * servo_in_max );
}


uint16_t PCA9685::servo2v( int a ) const
{
  uint32_t t = servo_mid + (int)servo_dvt * a / ( 2 * servo_in_max );
  return ( us2v( t ) & v_out_mask );
}

