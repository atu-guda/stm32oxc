
SUBDIRS = \
  f103c8_small \
  f303_disc \
  f407_x2 \
  f446_nucleo64 \
  f429i_disc \
  f746_wave

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

