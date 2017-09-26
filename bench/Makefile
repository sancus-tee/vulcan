# comment below to benchmark (un)protected Sancus+vatiCAN/LeiA
OPTS = BENCH=0 NOAUTH=0 VULCAN_SM=sm_eval #CAN_DRV_SM=sm_eval

all: send recv

recv:
	${MAKE} -C ecu-recv $(OPTS)

send:
	${MAKE} -C ecu-send $(OPTS)

load:
	${MAKE} -C ecu-send load $(OPTS)
	${MAKE} -C ecu-recv load $(OPTS)

sim-recv:
	${MAKE} -C ecu-recv background-sim $(OPTS) QUIET=1

sim-send:
	${MAKE} -C ecu-send sim $(OPTS)

sim: sim-recv sim-send
	@echo -e "\n\n----------\nSender/receiver $(LIBVULCAN) simulations succeeded!"

clean:
	$(RM) *.bin *_sloc.c sim-recv-out.txt
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
	$(MAKE) -C ecu-recv clean all LIBVULCAN=leia BENCH=1 NO_AUTH=0 > /dev/null 2>&1
	msp430-strip ecu-recv/can-drv.o -R .comment && msp430-size -A ecu-recv/can-drv.o
	msp430-strip ecu-recv/leia.o -R .comment && msp430-size -A ecu-recv/leia.o
	$(MAKE) -C ecu-recv clean all LIBVULCAN=leia BENCH=1 NO_AUTH=0 VULCAN_SM=sm_eval > /dev/null 2>&1
	msp430-strip ecu-recv/leia.o -R .comment && msp430-size -A ecu-recv/leia.o
	$(MAKE) -C ecu-recv clean all LIBVULCAN=vatican BENCH=1 NO_AUTH=0 > /dev/null 2>&1
	msp430-strip ecu-recv/vatican.o -R .comment && msp430-size -A ecu-recv/vatican.o
	$(MAKE) -C ecu-recv clean all LIBVULCAN=vatican BENCH=1 NO_AUTH=0 VULCAN_SM=sm_eval > /dev/null 2>&1
	msp430-strip ecu-recv/vatican.o -R .comment && msp430-size -A ecu-recv/vatican.o

# NOTE: measure the amount of cycles needed to compute a MAC (depends on LIBVULCAN)
mac:
	$(MAKE) -C ecu-send clean sim BENCH=1 BENCH_MAC=1 VULCAN_SM=sm_eval TRAVIS=1
