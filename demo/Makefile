-include ../Makefile.config

all: tcs gateway rpm

tcs:
	${MAKE} -C ecu-tcs all

rpm:
	${MAKE} -C ecu-rpm all

gateway:
	${MAKE} -C ecu-gateway all

load: 
	${MAKE} -C ecu-tcs load
	${MAKE} -C ecu-gateway load
	${MAKE} -C ecu-rpm load

clean:
	${MAKE} -C ecu-tcs clean
	${MAKE} -C ecu-rpm clean
	${MAKE} -C ecu-gateway clean
