
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

