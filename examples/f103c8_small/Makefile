
SUBDIRS = \
  00_leds \
  01_leds_oxc \
  01a_leds_tim_oxc_mu \
  02_leds_rtos \
  03_uart \
  04_uart_console \
  05a_def_console \
  05c_def_cons_nort \
  06_i2c_test \
  07_i2c_lcdt \
  08_i2c_mpu6050 \
  09_i2c_bmp085 \
  10_i2c_pcf8591 \
  11_i2c_ds3231 \
  12_tim_pwm \
  14_tim_enco \
  18_i2c_ssd1306_oled \
  24_i2c_hmc5983 \
  26_i2c_pca9685 \
  27_i2c_ads1115 \
  29_i2c_ina226 \
  80_math_stat \
  x03_tcop


ALL : examples_f103_small

examples_f103_small : $(SUBDIRS)

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


.PHONY: clean subclean examples_f103_small $(SUBDIRS)

