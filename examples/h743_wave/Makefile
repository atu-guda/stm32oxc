
SUBDIRS = \
  00_leds \
  01_leds_oxc \
  01a_leds_tim_oxc_mu \
  02_leds_rtos \
  03_uart \
  04_uart_console \
  04a_uart_console_nort \
  04x1_uart_usb \
  05_usb_console \
  05a_def_console \
  05b_usb_console_nort \
  05c_def_cons_nort \
  06_i2c_test \
  07_i2c_lcdt \
  08_i2c_mpu6050 \
  09_i2c_bmp085 \
  10_i2c_pcf8591 \
  11_i2c_ds3231 \
  12_tim_pwm \
  14_tim_enco \
  15_tim_usonic \
  16_step1 \
  17_1w_18b20 \
  18_i2c_ssd1306_oled \
  19_spi_test \
  20_spi_mem_a25 \
  21_spi_max7219 \
  22_spi_pcd8544_lcd \
  24_i2c_hmc5983 \
  25_spi_max31855_tcouple \
  25a_spi_max6672_tcouple2 \
  26_i2c_pca9685 \
  27_i2c_ads1115 \
  27a_i2c_ads1115_pwm \
  28_spi_ad9833 \
  29_i2c_ina226 \
  29a_i2c_ina226_pwm \
  29b_i2c_ina226_pwm2 \
  30_sdcard1 \
  31_sdcard2 \
  31a_sdcard2 \
  35_mcp4725_dac_i2c \
  36_mcp23017_gpio_i2c \
  37_ad7606_spi \
  37a_ad7606_spi_tim \
  38_mcp3204_adc_spi \
  38a_mcp3204_adc_spi_tim \
  51a_adcdma_n \
  80_math_stat \
  x01_fmc \
  # 50_adc0 \
  # 50a_adcdma_soft1 \
  # 50b_adcdma_pwm \
  # 51_adcdma \
  # x51_adcdma_fmc \
  # x52_adcirq_fmc \
  # x53a_adcdma_n_fmc_sdcard



ALL : examples_h743_wave

examples_h743_wave : $(SUBDIRS)

$(SUBDIRS) :
	$(MAKE) -C $@

subclean :
	for d in $(SUBDIRS) ; do \
	  echo make $$d subclean ; \
	  $(MAKE) -C $$d subclean ; \
	done


clean :
	for d in $(SUBDIRS) ; do \
	  echo make $$d clean ; \
	  $(MAKE) -C $$d clean ; \
	done


.PHONY: clean subclean examples_h743_wave $(SUBDIRS)

