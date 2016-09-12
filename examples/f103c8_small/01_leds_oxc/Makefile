PROJ_NAME=leds_oxc

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = n
USE_OXC_CONSOLE_USART = n
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = n
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y

# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc


SRCS  = main.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )

# SRCS += tm32l1xx_hal_adc.c
# SRCS += tm32l1xx_hal_adc_ex.c
# SRCS += tm32l1xx_hal_comp.c
# SRCS += tm32l1xx_hal_crc.c
# SRCS += tm32l1xx_hal_cryp.c
# SRCS += tm32l1xx_hal_cryp_ex.c
# SRCS += tm32l1xx_hal_dac.c
# SRCS += tm32l1xx_hal_dac_ex.c
# SRCS += tm32l1xx_hal_dma.c
# SRCS += tm32l1xx_hal_flash.c
# SRCS += tm32l1xx_hal_flash_ex.c
# SRCS += tm32l1xx_hal_flash_ramfunc.c
# SRCS += tm32l1xx_hal_i2c.c
# SRCS += tm32l1xx_hal_i2s.c
# SRCS += tm32l1xx_hal_irda.c
# SRCS += tm32l1xx_hal_iwdg.c
# SRCS += tm32l1xx_hal_lcd.c
# SRCS += tm32l1xx_hal_msp_template.c
# SRCS += tm32l1xx_hal_nor.c
# SRCS += tm32l1xx_hal_opamp.c
# SRCS += tm32l1xx_hal_opamp_ex.c
# SRCS += tm32l1xx_hal_pcd.c
# SRCS += tm32l1xx_hal_pcd_ex.c
# SRCS += tm32l1xx_hal_pwr.c
# SRCS += tm32l1xx_hal_pwr_ex.c
# SRCS += tm32l1xx_hal_rcc_ex.c
# SRCS += tm32l1xx_hal_rtc.c
# SRCS += tm32l1xx_hal_rtc_ex.c
# SRCS += tm32l1xx_hal_sd.c
# SRCS += tm32l1xx_hal_smartcard.c
# SRCS += tm32l1xx_hal_spi.c
# SRCS += tm32l1xx_hal_spi_ex.c
# SRCS += tm32l1xx_hal_sram.c
# SRCS += tm32l1xx_hal_tim.c
# SRCS += tm32l1xx_hal_tim_ex.c
# SRCS += tm32l1xx_hal_uart.c
# SRCS += tm32l1xx_hal_usart.c
# SRCS += tm32l1xx_hal_wwdg.c

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

