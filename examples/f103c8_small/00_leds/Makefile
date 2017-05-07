PROJ_NAME=leds


USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = n
USE_OXC_CONSOLE_USART = n
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = n
NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

SRCS += gpio.cpp

SRCS += stm32f1xx_hal_rcc_ex.c


#SRCS += ox_i2c.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )

# SRCS += stm32f1xx_hal_adc.c
# SRCS += stm32f1xx_hal_adc_ex.c
# SRCS += stm32f1xx_hal_comp.c
# SRCS += stm32f1xx_hal_crc.c
# SRCS += stm32f1xx_hal_cryp.c
# SRCS += stm32f1xx_hal_cryp_ex.c
# SRCS += stm32f1xx_hal_dac.c
# SRCS += stm32f1xx_hal_dac_ex.c
# SRCS += stm32f1xx_hal_dma.c
# SRCS += stm32f1xx_hal_flash.c
# SRCS += stm32f1xx_hal_flash_ex.c
# SRCS += stm32f1xx_hal_flash_ramfunc.c
# SRCS += stm32f1xx_hal_i2c.c
# SRCS += stm32f1xx_hal_i2s.c
# SRCS += stm32f1xx_hal_irda.c
# SRCS += stm32f1xx_hal_iwdg.c
# SRCS += stm32f1xx_hal_lcd.c
# SRCS += stm32f1xx_hal_msp_template.c
# SRCS += stm32f1xx_hal_nor.c
# SRCS += stm32f1xx_hal_opamp.c
# SRCS += stm32f1xx_hal_opamp_ex.c
# SRCS += stm32f1xx_hal_pcd.c
# SRCS += stm32f1xx_hal_pcd_ex.c
# SRCS += stm32f1xx_hal_pwr.c
# SRCS += stm32f1xx_hal_pwr_ex.c
# SRCS += stm32f1xx_hal_rcc_ex.c
# SRCS += stm32f1xx_hal_rtc.c
# SRCS += stm32f1xx_hal_rtc_ex.c
# SRCS += stm32f1xx_hal_sd.c
# SRCS += stm32f1xx_hal_smartcard.c
# SRCS += stm32f1xx_hal_spi.c
# SRCS += stm32f1xx_hal_spi_ex.c
# SRCS += stm32f1xx_hal_sram.c
# SRCS += stm32f1xx_hal_tim.c
# SRCS += stm32f1xx_hal_tim_ex.c
# SRCS += stm32f1xx_hal_uart.c
# SRCS += stm32f1xx_hal_usart.c
# SRCS += stm32f1xx_hal_wwdg.c




include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

