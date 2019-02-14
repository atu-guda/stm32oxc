#ifndef _OXC_AUTO_H
#define _OXC_AUTO_H

// Automatic includes

#include "local_hal_conf.h"

#include <oxc_hal_conf_auto.h>

#include <oxc_gpio.h>
#include <oxc_miscfun.h>
#include <oxc_outstream.h>
#include <oxc_ringbuf.h>

#if defined (USE_OXC_TIM)
#include <oxc_tim.h>
#endif

#if defined (USE_OXC_ADC)
#include <oxc_adc.h>
#endif

#if defined (USE_OXC_DEVIO)
#include <oxc_devio.h>
#endif

#if defined (USE_OXC_CONSOLE)
#include <oxc_console.h>
#include <oxc_smallrl.h>
#include <oxc_common1.h>
#endif

#if defined (USE_OXC_DEBUG)
#include <oxc_debug1.h>
#endif

#if defined (USE_OXC_UART)
#include <oxc_usartbase.h>
#endif

#if defined (USE_OXC_CONSOLE_UART)
#include <oxc_usartio.h>
#endif

#if defined (USE_OXC_CONSOLE_USB_CDC)
#include <oxc_usbcdcio.h>
#endif

#if defined (USE_OXC_SDRAM)
#include <oxc_sdram_common.h>
#endif

#if defined (HAL_I2C_MODULE_ENABLED)
#include <oxc_i2c.h>
#if defined (USE_OXC_DEBUG)
  #include <oxc_debug_i2c.h>
#endif
#endif

#if defined (HAL_SPI_MODULE_ENABLED)
#include <oxc_spi.h>
#endif

#endif

// vim: path=.,/usr/share/stm32cube/inc
