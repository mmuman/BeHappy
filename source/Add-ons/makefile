SUBDIRS= 
SUBMAKEFILES=makefile.bebook makefile.bhdoc

default all clean:
	for d in $(SUBDIRS); do \
		make -C $$d $@ || exit 1; \
	done
	for f in $(SUBMAKEFILES); do \
		make -f $$f $@ || exit 1; \
	done


