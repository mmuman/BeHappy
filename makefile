SUBMAKEFILESPRE=
SUBDIRS= source
SUBMAKEFILESPOST=

default all clean install:
	for f in $(SUBMAKEFILESPRE); do \
		make -f $$f $@ || exit 1; \
	done
	for d in $(SUBDIRS); do \
		make -C $$d $@ || exit 1; \
	done
	for f in $(SUBMAKEFILESPOST); do \
		make -f $$f $@ || exit 1; \
	done


