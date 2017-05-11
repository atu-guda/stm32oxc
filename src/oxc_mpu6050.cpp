#include <oxc_mpu6050.h>



void MPU6050::init()
{
  setAccScale( accs_2g );
  setGyroScale( gyros_250 );
  wake( pll_gyro_x );
}


