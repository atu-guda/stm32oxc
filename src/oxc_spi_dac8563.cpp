#include <oxc_spi_dac8563.h>

void DevSPI_DAC8563::init()
{
  reset();
  pwrup_ab();
  iref_on();
  gain_22();
  ldac_xx();
}

