
SUBDIRS = \
  00_leds \
  01_leds_oxc \
  02_leds_rtos \
  03_uart \
  04_uart_console \
  05_usb_console \
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
  23_spi_sdcard \
  24_i2c_hmc5983 \
  25_spi_max31855_tcouple \
  26_i2c_pca9685 \
  27_i2c_ads1115 \
  l01_accel \
  l02_accel \
  l03_mag


ALL : examples_f303_d3

examples_f303_d3 : $(SUBDIRS)

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


.PHONY: clean subclean examples_f303_d3 $(SUBDIRS)

