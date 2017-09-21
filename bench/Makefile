# comment below to benchmark (un)protected Sancus+vatiCAN/LeiA
OPTS = BENCH=0 NOAUTH=0 CAN_AUTH_LIB=leia VULCAN_SM=sm_eval #CAN_DRV_SM=sm_eval

all: send recv

recv:
	${MAKE} -C ecu-recv $(OPTS)

send:
	${MAKE} -C ecu-send $(OPTS)

load:
	${MAKE} -C ecu-send load $(OPTS)
	${MAKE} -C ecu-recv load $(OPTS)

clean:
	$(RM) *.bin *_sloc.c
	$(MAKE) -C ecu-recv clean
	$(MAKE) -C ecu-send clean

# NOTE: strip superfluous debug code lines from the final results
sloc:
	./parse_sloc.py ../can-auth/leia.c      > leia_sloc.c
	./parse_sloc.py ../can-auth/vatican.c   > vatican_sloc.c
	./parse_sloc.py ../drivers/mcp2515.c    > mcp2515_sloc.c
	sloccount --details *_sloc.c

# NOTE: strip superfluous .comment ELF section from final results
size:
	$(MAKE) -C ecu-recv clean all CAN_AUTH_LIB=leia BENCH=1 NO_AUTH=0 > /dev/null 2>&1
	msp430-strip ecu-recv/can-drv.o -R .comment && msp430-size -A ecu-recv/can-drv.o
	msp430-strip ecu-recv/leia.o -R .comment && msp430-size -A ecu-recv/leia.o
	$(MAKE) -C ecu-recv clean all CAN_AUTH_LIB=leia BENCH=1 NO_AUTH=0 VULCAN_SM=sm_eval > /dev/null 2>&1
	msp430-strip ecu-recv/leia.o -R .comment && msp430-size -A ecu-recv/leia.o
	$(MAKE) -C ecu-recv clean all CAN_AUTH_LIB=vatican BENCH=1 NO_AUTH=0 > /dev/null 2>&1
	msp430-strip ecu-recv/vatican.o -R .comment && msp430-size -A ecu-recv/vatican.o
	$(MAKE) -C ecu-recv clean all CAN_AUTH_LIB=vatican BENCH=1 NO_AUTH=0 VULCAN_SM=sm_eval > /dev/null 2>&1
	msp430-strip ecu-recv/vatican.o -R .comment && msp430-size -A ecu-recv/vatican.o
