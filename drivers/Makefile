-include ../Makefile.config

SOURCES = mcp2515.c sm_mmio_kypd.c sm_mmio_led.c sm_mmio_spi.c
OBJECTS = $(SOURCES:.c=.o)

#XXX compile _unprotected_ CAN driver with MSPGCC to work around clang compiler bugs...
mcp2515.o: mcp2515.c
	msp430-gcc $< -c -I$(SANCUS_SUPPORT_DIR)/include/ -I$(SANCUS_COMPILER_DIR)/include/ -D__MSP430F149__

all: $(OBJECTS)

clean:
	$(RM) *.o
