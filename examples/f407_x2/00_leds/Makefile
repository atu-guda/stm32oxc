PROJ_NAME=leds
MCTYPE = STM32F407

LINK=$(CC)

USE_OXC = y
USE_FREERTOS = n
NO_STDLIB = y

# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc

CFLAGS_ADD := -DSTM32F407xx -DHSE_VALUE=8000000
#
LDSCRIPT = $(OXCDIR)/ld/STM32F407VE_FLASH.ld

SRCS  = main.c
SRCS += stm32f4xx_hal_msp.c
SRCS += system_stm32f4xx.c
SRCS += startup_stm32f407xx.s
#SRCS += ox_base.cpp
#SRCS += ox_dev.cpp
#SRCS += ox_console.cpp
#SRCS += ox_debug1.cpp
#SRCS += ox_gpio.cpp
#SRCS += ox_i2c.cpp

# stm32cube sources.
# Base:
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_rcc.c

# aux:
# SRCS += stm32f4xx_hal_adc.c
# SRCS += stm32f4xx_hal_adc_ex.c
# SRCS += stm32f4xx_hal_can.c
# SRCS += stm32f4xx_hal_cec.c
# SRCS += stm32f4xx_hal_crc.c
# SRCS += stm32f4xx_hal_cryp.c
# SRCS += stm32f4xx_hal_cryp_ex.c
# SRCS += stm32f4xx_hal_dac.c
# SRCS += stm32f4xx_hal_dac_ex.c
# SRCS += stm32f4xx_hal_dcmi.c
# SRCS += stm32f4xx_hal_dcmi_ex.c
# SRCS += stm32f4xx_hal_dma2d.c
# SRCS += stm32f4xx_hal_dma.c
# SRCS += stm32f4xx_hal_dma_ex.c
# SRCS += stm32f4xx_hal_eth.c
# SRCS += stm32f4xx_hal_flash.c
# SRCS += stm32f4xx_hal_flash_ex.c
# SRCS += stm32f4xx_hal_flash_ramfunc.c
# SRCS += stm32f4xx_hal_fmpi2c.c
# SRCS += stm32f4xx_hal_fmpi2c_ex.c
# SRCS += stm32f4xx_hal_hash.c
# SRCS += stm32f4xx_hal_hash_ex.c
# SRCS += stm32f4xx_hal_hcd.c
# SRCS += stm32f4xx_hal_i2c.c
# SRCS += stm32f4xx_hal_i2c_ex.c
# SRCS += stm32f4xx_hal_i2s.c
# SRCS += stm32f4xx_hal_i2s_ex.c
# SRCS += stm32f4xx_hal_irda.c
# SRCS += stm32f4xx_hal_iwdg.c
# SRCS += stm32f4xx_hal_ltdc.c
# SRCS += stm32f4xx_hal_msp_template.c
# SRCS += stm32f4xx_hal_nand.c
# SRCS += stm32f4xx_hal_nor.c
# SRCS += stm32f4xx_hal_pccard.c
# SRCS += stm32f4xx_hal_pcd.c
# SRCS += stm32f4xx_hal_pcd_ex.c
# SRCS += stm32f4xx_hal_pwr.c
# SRCS += stm32f4xx_hal_pwr_ex.c
# SRCS += stm32f4xx_hal_qspi.c
# SRCS += stm32f4xx_hal_rcc_ex.c
# SRCS += stm32f4xx_hal_rng.c
# SRCS += stm32f4xx_hal_rtc.c
# SRCS += stm32f4xx_hal_rtc_ex.c
# SRCS += stm32f4xx_hal_sai.c
# SRCS += stm32f4xx_hal_sai_ex.c
# SRCS += stm32f4xx_hal_sd.c
# SRCS += stm32f4xx_hal_sdram.c
# SRCS += stm32f4xx_hal_smartcard.c
# SRCS += stm32f4xx_hal_spdifrx.c
# SRCS += stm32f4xx_hal_spi.c
# SRCS += stm32f4xx_hal_sram.c
# SRCS += stm32f4xx_hal_tim.c
# SRCS += stm32f4xx_hal_tim_ex.c
# SRCS += stm32f4xx_hal_uart.c
# SRCS += stm32f4xx_hal_usart.c
# SRCS += stm32f4xx_hal_wwdg.c
# SRCS += stm32f4xx_ll_fmc.c
# SRCS += stm32f4xx_ll_fsmc.c
# SRCS += stm32f4xx_ll_sdmmc.c
# SRCS += stm32f4xx_ll_usb.c


include $(OXCDIR)/mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

