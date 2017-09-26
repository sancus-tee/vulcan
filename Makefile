-include Makefile.config

all: bench demo drivers

.PHONY: bench
bench:
	${MAKE} -C bench all

sim:
	${MAKE} -C bench sim

.PHONY: demo
demo:
	${MAKE} -C demo all

mac:
	${MAKE} -C bench mac

sloc:
	${MAKE} -C bench sloc

size:
	${MAKE} -C bench size

clean:
	${MAKE} -C bench clean
	${MAKE} -C demo clean
	${MAKE} -C drivers clean
