
SUBDIRS = \
  f103c8_small \
  f303_disc \
  f334_nucleo64 \
  f401cc_kp \
  f407_x2 \
  f407_disc \
  f407v_bb0 \
  f407z_black0 \
  f446_nucleo64 \
  f429i_disc \
  f746_nucleo \
  f746_wave \
  h743_wave

#  f427_netdu \

ALL : examples

examples : $(SUBDIRS)

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


.PHONY: clean subclean examples $(SUBDIRS)

