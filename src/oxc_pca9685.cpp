#include <oxc_pca9685.h>

bool PCA9685::init( uint8_t presc_new )
{
  if( presc_new > 2 ) {
    presc = presc_new;
  }

  sleep(); // prescale can only be set while sleep
  if( send_reg1( reg_prescale, presc ) < 1 ) {
    return false;
  }
  if( send_reg1( reg_mode0, cmd_on_inc ) < 1 ) {
    return false;
  }
  return true;
}

void PCA9685::sleep()
{
  send_reg1( reg_mode0, cmd_off );
}

void PCA9685::set( uint8_t ch, uint16_t on, uint16_t off )
{
  uint16_t s[2];
  s[0] = on & v_out_mask; s[1] =  off & v_out_mask;
  uint8_t reg = ch2regOn( ch );
  send_reg1( reg, (uint8_t*)s, sizeof(s) );
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
  uint16_t s[2];
  s[0] = 0; s[1] = servo2v( val );
  uint8_t reg = ch2regOn( ch );
  send_reg1( reg, (uint8_t*)s, sizeof(s) );
}

void PCA9685::off( uint8_t ch )
{
  uint8_t reg = ch2regOff( ch ) + 1;
  send_reg1( reg, bit_onoff );
}

void PCA9685::setAllServo( int val )
{
  uint16_t s[2];
  s[0] = 0; s[1] = servo2v( val );
  uint8_t reg = reg_all_s;
  send_reg1( reg, (uint8_t*)&s, sizeof(s) );
}

uint8_t PCA9685::freq2prec( uint32_t freq ) // static
{
  uint32_t pr = ( base_freq / ( 4096 * freq ) ) - 1;
  if( pr > 255 ) {
    pr = 255;
  }
  if( pr < 3 ) {
    pr = 3;
  }
  return (uint8_t)pr;
}

void PCA9685::offAll()
{
  uint8_t reg = reg_all_s + 3;
  send_reg1( reg, bit_onoff );
}

uint32_t PCA9685::servo2t( int a )
{
  uint32_t t = servo_mid + (int)servo_dvt * a / ( 2 * servo_in_max );
  return t;
}


uint16_t PCA9685::servo2v( int a )
{
  uint32_t t = servo_mid + (int)servo_dvt * a / ( 2 * servo_in_max );
  uint16_t r = us2v( t );
  r &= v_out_mask;
  return r;
}

