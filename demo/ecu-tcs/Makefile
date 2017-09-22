-include ../../Makefile.config

SOURCES = $(shell ls *.c) ../../drivers/sm_mmio_kypd.c ../../drivers/mcp2515.c
TARGET = main.elf
DEV = /dev/ttyUSB0

# calculate and verify MACs in sm_tcs
CFLAGS += -DVULCAN_SM=sm_tcs #-DDEBUG

# NOTE: include (insecure) vatican nonce generator for demostration purposes
#       omit LeiA's AUTH_FAIL mechanism to save CAN IDs in demo scenario
ifeq ($(LIBVULCAN), vatican)
    CFLAGS += -DVATICAN_INCLUDE_NONCE_GENERATOR
else
    CFLAGS += -DLEIA_OMIT_AUTH_FAIL
endif

LDFLAGS += -ldev-pmodkypd

TARGET_NO_MAC = no_mac_$(TARGET)
OBJECTS = $(SOURCES:.c=.o) $(LIBVULCAN).o

all: $(TARGET)

#custom targets since intra-SM libraries cannot be shared
$(LIBVULCAN).o: $(CAN_AUTH_DIR)/$(LIBVULCAN).c
	$(CC) $(CFLAGS) -c $< -o $@

#NOTE: custom memsize flags since TCS system uses another FPGA image
$(TARGET_NO_MAC): $(OBJECTS)
	$(LD) $(LDFLAGS) --ram 24K --rom 32K -o $@ $^

$(TARGET): $(TARGET_NO_MAC)
	$(SANCUS_CRYPTO) $(CRYPTOFLAGS) -o $@ $<

load: $(TARGET)
	$(SANCUS_LOAD) -device $(DEV) -baudrate 115200 $<

clean:
	$(RM) $(TARGET) $(TARGET_NO_MAC) $(OBJECTS) $(DEPS) *.o
	$(RM) *.fst *.bin
