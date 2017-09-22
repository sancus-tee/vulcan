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

clean:
	${MAKE} -C bench clean
	${MAKE} -C demo clean
	${MAKE} -C drivers clean
